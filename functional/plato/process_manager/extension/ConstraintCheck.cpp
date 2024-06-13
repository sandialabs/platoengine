#include "plato/process_manager/extension/ConstraintCheck.hpp"

#include <ROL_Algorithm.hpp>
#include <fstream>
#include <optional>
#include <string>

#include "plato/core/ValidationRegistration.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/process_manager/extension/CommonInputValidation.hpp"
#include "plato/process_manager/extension/LogspaceGenerator.hpp"
#include "plato/process_manager/extension/ROLUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/process_manager/library/StageOrdering.hpp"
#include "plato/rol_integration/ROLHelpers.hpp"

namespace plato::process_manager::extension
{
namespace
{
[[nodiscard]] library::StageAndProcessManager make_constraint_check_process_manager(
    const library::ValidatedProcessManagerInput& aValidInput)
{
    return {library::RunStage::kValidate, [aValidInput](const library::ProcessManagerData& aProcessManagerData)
            {
                const auto& tInput = library::process_manager_input<input_parser::constraint_check>(aValidInput);
                ConstraintCheck{tInput}.run(aProcessManagerData);
            }};
}

[[maybe_unused]] static auto kConstraintCheckProcessManagerRegistration =
    library::ProcessManagerRegistration{input_parser::block_name<input_parser::constraint_check>(),
                                        [](const library::ValidatedProcessManagerInput& aValidInput)
                                        { return make_constraint_check_process_manager(aValidInput); }};

[[maybe_unused]] static auto kConstraintCheckValidationRegistration =
    core::ValidationRegistration<input_parser::constraint_check>{
        [](const input_parser::constraint_check& aInput)
        { return detail::validate_linearity_check_output_file_name(aInput); },
        [](const input_parser::constraint_check& aInput)
        { return detail::validate_jacobian_check_output_file_name(aInput); },
        [](const input_parser::constraint_check& aInput)
        { return detail::validate_jacobian_adjoint_consistency_check_output_file_name(aInput); },
        [](const input_parser::constraint_check& aInput) { return detail::validate_number_of_steps(aInput); },
        [](const input_parser::constraint_check& aInput)
        { return detail::validate_initial_direction_magnitude(aInput); },
        [](const input_parser::constraint_check& aInput)
        { return detail::validate_step_size_reduction_factor(aInput); },
        [](const input_parser::constraint_check& aInput) { return detail::validate_random_direction_seed(aInput); }};
}  // namespace

ConstraintCheck::ConstraintCheck(const ValidatedConstraintCheckInput& aInput)
    : mLinearityCheckOutputFileName{aInput.rawInput().linearity_check_output_file_name.value().mName},
      mJacobianCheckOutputFileName{aInput.rawInput().jacobian_check_output_file_name.value().mName},
      mJacobianAdjointConsistencyCheckOutputFileName{
          aInput.rawInput().jacobian_adjoint_consistency_output_file_name.value().mName},
      mNumberOfSteps{aInput.rawInput().number_of_steps.value()},
      mInitialDirectionMagnitude{aInput.rawInput().initial_direction_magnitude.value()},
      mStepSizeReductionFactor{aInput.rawInput().step_size_reduction_factor.value()},
      mRandomDirectionSeed{aInput.rawInput().random_direction_seed.value()}
{
}

void ConstraintCheck::run(const library::ProcessManagerData& aProcessManagerData) const
{
    constexpr bool tPrintOutput = true;

    auto tROLProblem = ROL::Ptr<ROL::Problem<double>>{make_rol_problem(aProcessManagerData).release()};

    std::ofstream tCheckLinearityOutFile{mLinearityCheckOutputFileName};
    tROLProblem->checkLinearity(
        tPrintOutput,
        tCheckLinearityOutFile);  // TODO: should we do something with the error value returned by this function?

    auto tConstraint = tROLProblem->getConstraint();
    if (tConstraint)
    {
        std::srand(mRandomDirectionSeed);
        const auto tNumDesignVariables = static_cast<int>(aProcessManagerData.mGeometry.mInitialGuess.size());

        auto tConstraintVectorStandIn = tROLProblem->getResidualVector();
        tConstraintVectorStandIn->randomize(-mInitialDirectionMagnitude, mInitialDirectionMagnitude);

        constexpr int tFiniteDifferenceOrder = 1;  // TODO: Should we make this an actual input?
        std::ofstream tCheckJacobianOutFile{mJacobianCheckOutputFileName};
        tConstraint->checkApplyJacobian(
            rol_integration::to_rol_vector(aProcessManagerData.mGeometry.mInitialGuess),
            rol_integration::generate_perturbation(tNumDesignVariables), *tConstraintVectorStandIn,
            LogspaceGenerator{mInitialDirectionMagnitude, mStepSizeReductionFactor, mNumberOfSteps}.steps(),
            tPrintOutput, tCheckJacobianOutFile, tFiniteDifferenceOrder);

        const auto tTolerance = tConstraint->checkAdjointConsistencyJacobian(
            rol_integration::generate_perturbation(tROLProblem->getMultiplierVector()->dimension()),
            rol_integration::generate_perturbation(tNumDesignVariables),
            rol_integration::to_rol_vector(aProcessManagerData.mGeometry.mInitialGuess), false);
        detail::write_jacobian_adjoint_consistency_check_output(mJacobianAdjointConsistencyCheckOutputFileName,
                                                                tTolerance);
    }
}

namespace detail
{
std::optional<std::string> validate_linearity_check_output_file_name(const input_parser::constraint_check& aInput)
{
    return core::error_message_for_empty_parameter(input_parser::block_name<input_parser::constraint_check>(),
                                                   aInput.linearity_check_output_file_name,
                                                   "linearity_check_output_file_name");
}

std::optional<std::string> validate_jacobian_check_output_file_name(const input_parser::constraint_check& aInput)
{
    return core::error_message_for_empty_parameter(input_parser::block_name<input_parser::constraint_check>(),
                                                   aInput.jacobian_check_output_file_name,
                                                   "jacobian_check_output_file_name");
}

std::optional<std::string> validate_jacobian_adjoint_consistency_check_output_file_name(
    const input_parser::constraint_check& aInput)
{
    return core::error_message_for_empty_parameter(input_parser::block_name<input_parser::constraint_check>(),
                                                   aInput.jacobian_adjoint_consistency_output_file_name,
                                                   "jacobian_adjoint_consistency_output_file_name");
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