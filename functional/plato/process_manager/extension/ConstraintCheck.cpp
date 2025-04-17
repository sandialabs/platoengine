#include "plato/process_manager/extension/ConstraintCheck.hpp"

#include <ROL_Algorithm.hpp>
#include <fstream>
#include <optional>
#include <string>

#include "plato/geometry/library/OutputManager.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/process_manager/extension/CommonInputValidation.hpp"
#include "plato/process_manager/extension/LogspaceGenerator.hpp"
#include "plato/process_manager/extension/ROLUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/process_manager/library/StageOrdering.hpp"
#include "plato/third_party_integration/rol/Utilities.hpp"

namespace plato::process_manager::extension
{
namespace
{
[[nodiscard]] auto make_constraint_check_process_manager(const library::NewValidatedProcessManagerInput& aValidInput)
    -> library::StageAndProcessManager
{
    return {library::RunStage::kValidate, [aValidInput](const library::ProcessManagerData& aProcessManagerData)
            { ConstraintCheck{aValidInput}.run(aProcessManagerData); }};
}

[[nodiscard]] auto constraint_check_input(const library::NewValidatedProcessManagerInput& aValidInput)
    -> const input_parser::new_constraint_check&
{
    return input_validation::get_input_block<input_parser::new_constraint_check>(aValidInput);
}

[[maybe_unused]] static auto kConstraintCheckParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::new_constraint_check>{};

[[maybe_unused]] static auto kNewConstraintCheckProcessManagerRegistration =
    library::NewProcessManagerRegistration{input_parser::block_name<input_parser::new_constraint_check>(),
                                           [](const library::NewValidatedProcessManagerInput& aValidInput)
                                           { return make_constraint_check_process_manager(aValidInput); }};

[[maybe_unused]] static auto kConstraintCheckValidationRegistration =
    input_validation::CrossReferencedInputValidationRegistration<>{
        [](const input_parser::new_constraint_check& aInput)
        { return detail::validate_linearity_check_output_file_name(aInput); },
        [](const input_parser::new_constraint_check& aInput)
        { return detail::validate_jacobian_check_output_file_name(aInput); },
        [](const input_parser::new_constraint_check& aInput)
        { return detail::validate_jacobian_adjoint_consistency_check_output_file_name(aInput); },
        [](const input_parser::new_constraint_check& aInput) { return detail::validate_number_of_steps(aInput); },
        [](const input_parser::new_constraint_check& aInput)
        { return detail::validate_initial_direction_magnitude(aInput); },
        [](const input_parser::new_constraint_check& aInput)
        { return detail::validate_step_size_reduction_factor(aInput); },
        [](const input_parser::new_constraint_check& aInput)
        { return detail::validate_random_direction_seed(aInput); }};
}  // namespace

ConstraintCheck::ConstraintCheck(const library::NewValidatedProcessManagerInput& aInput)
    : mLinearityCheckOutputFileName{constraint_check_input(aInput).linearity_check_output_file_name.value().mToken},
      mJacobianCheckOutputFileName{constraint_check_input(aInput).jacobian_check_output_file_name.value().mToken},
      mJacobianAdjointConsistencyCheckOutputFileName{
          constraint_check_input(aInput).jacobian_adjoint_consistency_output_file_name.value().mToken},
      mNumberOfSteps{constraint_check_input(aInput).number_of_steps.value()},
      mInitialDirectionMagnitude{constraint_check_input(aInput).initial_direction_magnitude.value()},
      mStepSizeReductionFactor{constraint_check_input(aInput).step_size_reduction_factor.value()},
      mRandomDirectionSeed{constraint_check_input(aInput).random_direction_seed.value()}
{
}

void ConstraintCheck::run(const library::ProcessManagerData& aProcessManagerData) const
{
    namespace tpir = third_party_integration::rol;
    constexpr bool tPrintOutput = true;

    auto [tROLProblem, tROLControls] = make_rol_problem(aProcessManagerData);

    std::ofstream tCheckLinearityOutFile{mLinearityCheckOutputFileName};
    tROLProblem->checkLinearity(
        tPrintOutput,
        tCheckLinearityOutFile);  // TODO: should we do something with the error value returned by this function?

    auto tConstraint = tROLProblem->getConstraint();
    if (tConstraint)
    {
        std::srand(mRandomDirectionSeed);

        auto tConstraintVectorStandIn = tROLProblem->getResidualVector();
        tConstraintVectorStandIn->randomize(-mInitialDirectionMagnitude, mInitialDirectionMagnitude);

        constexpr int tFiniteDifferenceOrder = 1;  // TODO: Should we make this an actual input?
        std::ofstream tCheckJacobianOutFile{mJacobianCheckOutputFileName};
        auto tDirectionVector = tROLProblem->getPrimalOptimizationVector()->clone();
        tpir::randomize_and_normalize(*tDirectionVector);

        tConstraint->checkApplyJacobian(
            *tROLProblem->getPrimalOptimizationVector(), *tDirectionVector, *tConstraintVectorStandIn,
            LogspaceGenerator{mInitialDirectionMagnitude, mStepSizeReductionFactor, mNumberOfSteps}.steps(),
            tPrintOutput, tCheckJacobianOutFile, tFiniteDifferenceOrder);

        const auto tDualVector = tROLProblem->getMultiplierVector()->clone();
        tpir::randomize_and_normalize(*tDualVector);

        const auto tTolerance = tConstraint->checkAdjointConsistencyJacobian(
            *tDualVector, *tDirectionVector, *tROLProblem->getPrimalOptimizationVector(), tPrintOutput);

        detail::write_jacobian_adjoint_consistency_check_output(mJacobianAdjointConsistencyCheckOutputFileName,
                                                                tTolerance);
    }
}

auto create_valid_example_constraint_check_input() -> input_parser::new_constraint_check
{
    return input_parser::new_constraint_check{
        /*.linearity_check_output_file_name=*/input_parser::FileName{"constraint_linearity_check.txt"},
        /*.jacobian_check_output_file_name=*/input_parser::FileName{"constraint_jacobian_check.txt"},
        /*.jacobian_adjoint_consistency_output_file_name=*/
        input_parser::FileName{"constraint_jacobian_adjoint_consistency_check.txt"},
        /*.number_of_steps*/ 10u,
        /*.initial_direction_magnitude*/ 1.0,
        /*.step_size_reduction_factor*/ 0.1,
        /*.random_direction_seed*/ 123};
}

namespace detail
{
auto validate_linearity_check_output_file_name(const input_parser::new_constraint_check& aInput)
    -> std::optional<std::string>
{
    return input_validation::error_message_for_empty_parameter(
        input_parser::block_name<input_parser::new_constraint_check>(), aInput.linearity_check_output_file_name,
        "linearity_check_output_file_name");
}

auto validate_jacobian_check_output_file_name(const input_parser::new_constraint_check& aInput)
    -> std::optional<std::string>
{
    return input_validation::error_message_for_empty_parameter(
        input_parser::block_name<input_parser::new_constraint_check>(), aInput.jacobian_check_output_file_name,
        "jacobian_check_output_file_name");
}

auto validate_jacobian_adjoint_consistency_check_output_file_name(const input_parser::new_constraint_check& aInput)
    -> std::optional<std::string>
{
    return input_validation::error_message_for_empty_parameter(
        input_parser::block_name<input_parser::new_constraint_check>(),
        aInput.jacobian_adjoint_consistency_output_file_name, "jacobian_adjoint_consistency_output_file_name");
}

void write_jacobian_adjoint_consistency_check_output(const std::string& aName, double aTolerance)
{
    std::ofstream tAdjointConsistencyOutFile{aName};
    tAdjointConsistencyOutFile << "Test Consistency of Jacobian and its adjoint: \n";
    tAdjointConsistencyOutFile << "Absolute Error Computed As: |<w,Jv> - <adj(J)w,v>| \n";
    tAdjointConsistencyOutFile << "Relative Error Normalized By: |<w,Jv>| \n";
    tAdjointConsistencyOutFile << "Relative Error = " << aTolerance;
    tAdjointConsistencyOutFile.close();
}

}  // namespace detail
}  // namespace plato::process_manager::extension
