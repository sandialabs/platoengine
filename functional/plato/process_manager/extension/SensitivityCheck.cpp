#include "plato/process_manager/extension/SensitivityCheck.hpp"

#include <fstream>
#include <string_view>

#include "plato/core/Compose.hpp"
#include "plato/criteria/extension/NodalSumObjective.hpp"
#include "plato/geometry/library/OutputManager.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/process_manager/extension/ROLUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/process_manager/library/StageOrdering.hpp"
#include "plato/third_party_integration/rol/Utilities.hpp"

namespace plato::process_manager::extension
{
namespace
{
[[nodiscard]] auto make_rol_sensitivity_check_process_manager(
    const library::NewValidatedProcessManagerInput& aValidInput) -> library::StageAndProcessManager
{
    return {library::RunStage::kValidate, [aValidInput](const library::ProcessManagerData& aProcessManangerData)
            { SensitivityCheck{aValidInput}.run(aProcessManangerData); }};
}

[[maybe_unused]] static auto kNewSensitivityCheckProcessManagerRegistration =
    library::NewProcessManagerRegistration{input_parser::block_name<input_parser::sensitivity_check>(),
                                           [](const library::NewValidatedProcessManagerInput& aValidInput)
                                           { return make_rol_sensitivity_check_process_manager(aValidInput); }};

[[maybe_unused]] static auto kSensitivityCheckValidationRegistration =
    input_validation::CrossReferencedInputValidationRegistration<>{
        [](const input_parser::sensitivity_check& aInput) { return detail::validate_output_file_name(aInput); }};

auto make_rol_sensitivity_objective(const library::ProcessManagerData& aProblem)
    -> std::unique_ptr<plato::third_party_integration::rol::ROLObjectiveFunction>
{
    auto tSimpleObjectiveFunction = criteria::extension::make_nodal_sum_function();
    return std::make_unique<plato::third_party_integration::rol::ROLObjectiveFunction>(
        core::compose(tSimpleObjectiveFunction, aProblem.mGeometry.mCompute));
}

}  // namespace

SensitivityCheck::SensitivityCheck(const library::NewValidatedProcessManagerInput& aInput)
    : mOutputFileName(
          input_validation::get_input_block<input_parser::sensitivity_check>(aInput).output_file_name.value().mToken)
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

auto create_valid_example_sensitivity_check_input() -> input_parser::sensitivity_check
{
    return input_parser::sensitivity_check{/*.output_file_name=*/input_parser::FileName{"sensitivity_check.txt"}};
}

namespace detail
{
auto validate_output_file_name(const input_parser::sensitivity_check& aInput) -> std::optional<std::string>
{
    return input_validation::error_message_for_empty_parameter(
        input_parser::block_name<input_parser::sensitivity_check>(), aInput.output_file_name, "output_file_name");
}
}  // namespace detail
}  // namespace plato::process_manager::extension
