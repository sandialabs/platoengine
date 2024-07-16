#include "plato/process_manager/extension/SensitivityCheck.hpp"

#include <fstream>
#include <string_view>

#include "plato/core/Compose.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/criteria/extension/NodalSumObjective.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/process_manager/extension/ROLUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/process_manager/library/StageOrdering.hpp"
#include "plato/third_party_integration/rol/OptimizerFactory.hpp"
#include "plato/third_party_integration/rol/Utilities.hpp"

namespace plato::process_manager::extension
{
namespace
{
[[nodiscard]] library::StageAndProcessManager make_rol_sensitivity_check_process_manager(
    const library::ValidatedProcessManagerInput& aValidInput)
{
    return {library::RunStage::kValidate, [aValidInput](const library::ProcessManagerData& aProcessManangerData)
            {
                const auto& tInput = library::process_manager_input<input_parser::sensitivity_check>(aValidInput);
                const SensitivityCheck tSensitivityCheck(tInput);
                tSensitivityCheck.run(aProcessManangerData);
            }};
}

[[maybe_unused]] static auto kSensitivityCheckProcessManagerRegistration =
    library::ProcessManagerRegistration{input_parser::block_name<input_parser::sensitivity_check>(),
                                        [](const library::ValidatedProcessManagerInput& aValidInput)
                                        { return make_rol_sensitivity_check_process_manager(aValidInput); }};

[[maybe_unused]] static auto kSensitivityCheckValidationRegistration =
    core::ValidationRegistration<input_parser::sensitivity_check>{
        [](const input_parser::sensitivity_check& aInput) { return detail::validate_output_file_name(aInput); }};

std::unique_ptr<plato::third_party_integration::rol::ROLObjectiveFunction> make_rol_sensitivity_objective(
    const library::ProcessManagerData& aProblem)
{
    auto tSimpleObjectiveFunction = criteria::extension::make_nodal_sum_function();
    return std::make_unique<plato::third_party_integration::rol::ROLObjectiveFunction>(
        core::compose(tSimpleObjectiveFunction, aProblem.mGeometry.mCompute));
}

}  // namespace

SensitivityCheck::SensitivityCheck(const ValidatedSensitivityCheckInput& aInput)
    : mOutputFileName(aInput.rawInput().output_file_name.value().mToken)
{
}

void SensitivityCheck::run(const library::ProcessManagerData& aProblem) const
{
    std::ofstream tOutFile(mOutputFileName);
    constexpr bool tPrintOutput = true;

    auto tSensitivityObjective = make_rol_sensitivity_objective(aProblem);
    const auto tInitialGuessSize = static_cast<int>(aProblem.mGeometry.mInitialGuess.size());
    tSensitivityObjective->checkGradient(third_party_integration::rol::to_rol_vector(aProblem.mGeometry.mInitialGuess),
                                         third_party_integration::rol::generate_perturbation(tInitialGuessSize),
                                         tPrintOutput, tOutFile);
}

namespace detail
{
std::optional<std::string> validate_output_file_name(const input_parser::sensitivity_check& aInput)
{
    return core::error_message_for_empty_parameter(input_parser::block_name<input_parser::sensitivity_check>(),
                                                   aInput.output_file_name, "output_file_name");
}
}  // namespace detail
}  // namespace plato::process_manager::extension
