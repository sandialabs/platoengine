#include "plato/process_manager/extension/GradientCheck.hpp"

#include <cstdlib>
#include <fstream>

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
[[nodiscard]] library::StageAndProcessManager make_gradient_check_process_manager(
    const library::ValidatedProcessManagerInput& aValidInput)
{
    return {library::RunStage::kValidate, [aValidInput](const library::ProcessManagerData& aProcessManangerData)
            {
                const auto& tInput = library::process_manager_input<input_parser::gradient_check>(aValidInput);
                GradientCheck{tInput}.run(aProcessManangerData);
            }};
}

[[maybe_unused]] static auto kGradientCheckProcessManagerRegistration =
    library::ProcessManagerRegistration{input_parser::block_name<input_parser::gradient_check>(),
                                        [](const library::ValidatedProcessManagerInput& aValidInput)
                                        { return make_gradient_check_process_manager(aValidInput); }};

[[maybe_unused]] static auto kGradientCheckValidationRegistration =
    core::ValidationRegistration<input_parser::gradient_check>{
        [](const input_parser::gradient_check& aInput) { return detail::validate_output_file_name(aInput); },
        [](const input_parser::gradient_check& aInput) { return detail::validate_number_of_steps(aInput); },
        [](const input_parser::gradient_check& aInput) { return detail::validate_initial_direction_magnitude(aInput); },
        [](const input_parser::gradient_check& aInput) { return detail::validate_step_size_reduction_factor(aInput); },
        [](const input_parser::gradient_check& aInput) { return detail::validate_random_direction_seed(aInput); }};

}  // namespace

GradientCheck::GradientCheck(const ValidatedGradientCheckInput& aInput)
    : mOutputFileName(aInput.rawInput().output_file_name.value().mToken),
      mNumberOfSteps(aInput.rawInput().number_of_steps.value()),
      mInitialDirectionMagnitude(aInput.rawInput().initial_direction_magnitude.value()),
      mStepSizeReductionFactor(aInput.rawInput().step_size_reduction_factor.value()),
      mRandomDirectionSeed(aInput.rawInput().random_direction_seed.value())
{
}

void GradientCheck::run(const library::ProcessManagerData& aProblem) const
{
    std::ofstream tOutFile(mOutputFileName);
    constexpr bool tPrintOutput = true;
    auto tROLProblem = ROL::Ptr<ROL::Problem<double>>{make_rol_problem(aProblem).release()};
    const LogspaceGenerator tLogspaceGenerator{mInitialDirectionMagnitude, mStepSizeReductionFactor, mNumberOfSteps};

    std::srand(mRandomDirectionSeed);
    const auto tInitialGuessSize = static_cast<int>(aProblem.mGeometry.mInitialGuess.size());
    tROLProblem->getObjective()->checkGradient(
        third_party_integration::rol::to_rol_vector(aProblem.mGeometry.mInitialGuess),
        third_party_integration::rol::generate_perturbation(tInitialGuessSize), tLogspaceGenerator.steps(),
        tPrintOutput, tOutFile);
}

namespace detail
{
std::optional<std::string> validate_output_file_name(const input_parser::gradient_check& aInput)
{
    return core::error_message_for_empty_parameter(input_parser::block_name<input_parser::gradient_check>(),
                                                   aInput.output_file_name, "output_file_name");
}

}  // namespace detail

}  // namespace plato::process_manager::extension
