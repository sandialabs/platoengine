#include "plato/process_manager/extension/GradientCheck.hpp"

#include <ROL_StdObjective.hpp>
#include <cstdlib>
#include <fstream>

#include "plato/geometry/library/OutputManager.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
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
[[nodiscard]] auto make_gradient_check_process_manager(const library::NewValidatedProcessManagerInput& aValidInput)
    -> library::StageAndProcessManager
{
    return {library::RunStage::kValidate, [aValidInput](const library::ProcessManagerData& aProcessManangerData)
            { GradientCheck{aValidInput}.run(aProcessManangerData); }};
}

[[maybe_unused]] static auto kGradientCheckParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::new_gradient_check>{};

[[maybe_unused]] static auto kNewGradientCheckProcessManagerRegistration =
    library::NewProcessManagerRegistration{input_parser::block_name<input_parser::new_gradient_check>(),
                                           [](const library::NewValidatedProcessManagerInput& aValidInput)
                                           { return make_gradient_check_process_manager(aValidInput); }};

[[maybe_unused]] static auto kGradientCheckValidationRegistration =
    input_validation::CrossReferencedInputValidationRegistration<>{
        [](const input_parser::new_gradient_check& aInput) { return detail::validate_output_file_name(aInput); },
        [](const input_parser::new_gradient_check& aInput) { return detail::validate_number_of_steps(aInput); },
        [](const input_parser::new_gradient_check& aInput)
        { return detail::validate_initial_direction_magnitude(aInput); },
        [](const input_parser::new_gradient_check& aInput)
        { return detail::validate_step_size_reduction_factor(aInput); },
        [](const input_parser::new_gradient_check& aInput) { return detail::validate_random_direction_seed(aInput); }};

[[nodiscard]] auto gradient_check_input(const library::NewValidatedProcessManagerInput& aValidInput)
    -> const input_parser::new_gradient_check&
{
    return input_validation::get_input_block<input_parser::new_gradient_check>(aValidInput);
}

}  // namespace

GradientCheck::GradientCheck(const library::NewValidatedProcessManagerInput& aInput)
    : mOutputFileName(gradient_check_input(aInput).output_file_name.value().mToken),
      mNumberOfSteps(gradient_check_input(aInput).number_of_steps.value()),
      mInitialDirectionMagnitude(gradient_check_input(aInput).initial_direction_magnitude.value()),
      mStepSizeReductionFactor(gradient_check_input(aInput).step_size_reduction_factor.value()),
      mRandomDirectionSeed(gradient_check_input(aInput).random_direction_seed.value())
{
}

void GradientCheck::run(const library::ProcessManagerData& aProblem) const
{
    std::ofstream tOutFile(mOutputFileName);
    constexpr bool tPrintOutput = true;
    auto [tROLProblem, tControls] = make_rol_problem(aProblem);
    const LogspaceGenerator tLogspaceGenerator{mInitialDirectionMagnitude, mStepSizeReductionFactor, mNumberOfSteps};
    std::srand(mRandomDirectionSeed);

    const auto tObjective = tROLProblem->getObjective();
    auto tDirection = tROLProblem->getPrimalOptimizationVector()->clone();
    third_party_integration::rol::randomize_and_normalize(*tDirection);
    tObjective->checkGradient(*tROLProblem->getPrimalOptimizationVector(), *tDirection, tLogspaceGenerator.steps(),
                              tPrintOutput, tOutFile);
}

auto create_valid_example_gradient_check_input() -> input_parser::new_gradient_check
{
    return input_parser::new_gradient_check{/*.output_file_name=*/input_parser::FileName{"gradient_check.txt"},
                                            /*.number_of_steps=*/12,
                                            /*.initial_direction_magnitude=*/0.5,
                                            /*.step_size_reduction_factor = */ 0.5,
                                            /*.random_direction_seed = */ 42};
}

namespace detail
{
auto validate_output_file_name(const input_parser::new_gradient_check& aInput) -> std::optional<std::string>
{
    return input_validation::error_message_for_empty_parameter(
        input_parser::block_name<input_parser::new_gradient_check>(), aInput.output_file_name, "output_file_name");
}

}  // namespace detail

}  // namespace plato::process_manager::extension
