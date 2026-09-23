#include "plato/process_manager/extension/ObjectiveGradientCheck.hpp"

#include <cstdlib>
#include <fstream>
#include <optional>
#include <string>

#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/process_manager/extension/CommonInputValidation.hpp"
#include "plato/process_manager/extension/DirectionVectorTypes.hpp"
#include "plato/process_manager/extension/ROLUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerLogger.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/process_manager/library/StageOrdering.hpp"
#include "plato/utilities/LogSpaceGenerator.hpp"

namespace plato::process_manager::extension
{
namespace
{
[[nodiscard]] auto make_objective_gradient_check_process_manager(
    const library::ValidatedProcessManagerInput& aValidInput) -> library::StageAndProcessManager
{
    return {library::RunStage::kValidate, [aValidInput](const library::ProcessManagerData& aProcessManangerData)
            { ObjectiveGradientCheck{aValidInput}.run(aProcessManangerData); }};
}

[[maybe_unused]] const auto kObjectiveGradientCheckParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::objective_gradient_check>{};

[[maybe_unused]] const auto kObjectiveGradientCheckProcessManagerRegistration =
    library::ProcessManagerRegistration{input_parser::block_name<input_parser::objective_gradient_check>(),
                                        [](const library::ValidatedProcessManagerInput& aValidInput)
                                        { return make_objective_gradient_check_process_manager(aValidInput); }};

[[maybe_unused]] const auto kObjectiveGradientCheckValidationRegistration =
    input_validation::InputBlockValidationRegistration<>{
        [](const input_parser::objective_gradient_check& aInput) { return detail::validate_output_file_name(aInput); },
        [](const input_parser::objective_gradient_check& aInput) { return detail::validate_number_of_steps(aInput); },
        [](const input_parser::objective_gradient_check& aInput)
        { return detail::validate_initial_direction_magnitude(aInput); },
        [](const input_parser::objective_gradient_check& aInput)
        { return detail::validate_step_size_reduction_factor(aInput); },
        [](const input_parser::objective_gradient_check& aInput)
        { return detail::validate_random_direction_seed(aInput); },
        [](const input_parser::objective_gradient_check& aInput)
        {
            return input_validation::error_message_for_empty_parameter(
                input_parser::block_name<input_parser::objective_gradient_check>(), aInput.direction_vector_type,
                "direction_vector_type");
        }};

[[nodiscard]] auto objective_gradient_check_input(const library::ValidatedProcessManagerInput& aValidInput)
    -> const input_parser::objective_gradient_check&
{
    return input_validation::get_input_block<input_parser::objective_gradient_check>(aValidInput);
}

}  // namespace

ObjectiveGradientCheck::ObjectiveGradientCheck(const library::ValidatedProcessManagerInput& aInput)
    : mOutputFileName(objective_gradient_check_input(aInput).output_file_name.value().mToken),
      mNumberOfSteps(objective_gradient_check_input(aInput).number_of_steps.value()),
      mInitialDirectionMagnitude(objective_gradient_check_input(aInput).initial_direction_magnitude.value()),
      mStepSizeReductionFactor(objective_gradient_check_input(aInput).step_size_reduction_factor.value()),
      mDirectionVectorType(objective_gradient_check_input(aInput).direction_vector_type.value()),
      mSeed(objective_gradient_check_input(aInput).random_direction_seed.value_or(fixed_seed_value()))
{
}

void ObjectiveGradientCheck::run(const library::ProcessManagerData& aProblem) const
{
    [[maybe_unused]] const auto tTaskLogger = library::run_task_log<input_parser::objective_gradient_check>();
    std::srand(mSeed);
    std::ofstream tOutFile(mOutputFileName);
    constexpr bool tPrintOutput = true;

    auto [tROLProblem, tControls] =
        make_rol_problem(aProblem, input_parser::block_name<input_parser::objective_gradient_check>());
    const utilities::LogSpaceGenerator tLogspaceGenerator{mInitialDirectionMagnitude, mStepSizeReductionFactor,
                                                          mNumberOfSteps};

    const auto tObjective = tROLProblem->getObjective();
    auto tDirection = tROLProblem->getPrimalOptimizationVector()->clone();
    make_uniform_or_random_perturbation(*tDirection, mDirectionVectorType);
    tObjective->checkGradient(*tROLProblem->getPrimalOptimizationVector(), *tDirection, tLogspaceGenerator.steps(),
                              tPrintOutput, tOutFile);
}

namespace detail
{
auto validate_output_file_name(const input_parser::objective_gradient_check& aInput) -> std::optional<std::string>
{
    return input_validation::error_message_for_empty_parameter(
        input_parser::block_name<input_parser::objective_gradient_check>(), aInput.output_file_name,
        "output_file_name");
}

}  // namespace detail

}  // namespace plato::process_manager::extension
