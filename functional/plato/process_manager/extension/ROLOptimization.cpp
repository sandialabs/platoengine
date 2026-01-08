#include "plato/process_manager/extension/ROLOptimization.hpp"

#include <ROL_Ptr.hpp>
#include <fstream>
#include <string_view>

#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/output/OutputManager.hpp"
#include "plato/process_manager/extension/CommonInputValidation.hpp"
#include "plato/process_manager/extension/ROLUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerLogger.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/process_manager/library/StageOrdering.hpp"
#include "plato/third_party_integration/rol/ROLObjectiveFunction.hpp"
#include "plato/third_party_integration/rol/Utilities.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::process_manager::extension
{
namespace
{
constexpr std::string_view kROLOptimizerFileName = "ROL_Optimizer.txt";

[[nodiscard]] auto make_rol_optimization_process_manager(const library::ValidatedProcessManagerInput& aValidInput)
    -> library::StageAndProcessManager
{
    return {library::RunStage::kExecute, [aValidInput](const library::ProcessManagerData& aProcessManangerData)
            { ROLOptimization{aValidInput}.run(aProcessManangerData); }};
}

[[maybe_unused]] static auto kROLOptimizerParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::rol_optimization>{};

[[maybe_unused]] static auto kROLOptimizerProcessManagerRegistration =
    library::ProcessManagerRegistration{input_parser::block_name<input_parser::rol_optimization>(),
                                        [](const library::ValidatedProcessManagerInput& aValidInput)
                                        { return make_rol_optimization_process_manager(aValidInput); }};

[[maybe_unused]] static auto kOptimizerValidationRegistration = input_validation::InputBlockValidationRegistration<>{
    [](const input_parser::rol_optimization& aInput) { return detail::validate_rol_max_iterations(aInput); },
    [](const input_parser::rol_optimization& aInput) { return detail::validate_step_tolerance(aInput); },
    [](const input_parser::rol_optimization& aInput) { return detail::validate_gradient_tolerance(aInput); },
    [](const input_parser::rol_optimization& aInput) { return detail::validate_initial_search_radius(aInput); },
    [](const input_parser::rol_optimization& aInput) { return detail::validate_unique_output_name(aInput); },
    [](const input_parser::rol_optimization& aInput) { return detail::validate_optional_input_file_name(aInput); }};
}  // namespace

ROLOptimization::ROLOptimization(const library::ValidatedProcessManagerInput& aInput)
    : mROLOptions{make_optimization_parameters(aInput)}
{
}

void ROLOptimization::run(const library::ProcessManagerData& aProcessManagerData) const
{
    [[maybe_unused]] const auto tTaskLogger = library::run_task_log<input_parser::rol_optimization>();

    const auto tOutputMode = mROLOptions.writeOutputHistory() ? output::OutputMode::kEveryIterationAppend
                                                              : output::OutputMode::kEveryIterationOverwrite;
    auto tOutputManager = output::OutputManager{aProcessManagerData.mGeometry.mOutput, tOutputMode};

    auto tObjective = ROL::Ptr<plato::third_party_integration::rol::ROLObjectiveFunction>(
        make_rol_objective(aProcessManagerData, std::move(tOutputManager)).release());
    auto [tROLProblem, tROLControls] =
        make_rol_problem(aProcessManagerData, input_parser::block_name<input_parser::rol_optimization>(), tObjective);
    auto tROLInputs = mROLOptions.parameters();
    auto tROLSolver = make_rol_solver(tROLInputs, tROLProblem);

    auto tOutFile = std::ofstream{std::string{kROLOptimizerFileName}};
    tROLSolver.solve(tOutFile);

    tObjective->finalUpdate(third_party_integration::rol::to_dynamic_vector(*tROLControls));
}

namespace detail
{
auto validate_rol_max_iterations(const input_parser::rol_optimization& aInput) -> std::optional<std::string>
{
    return ::plato::process_manager::extension::detail::validate_max_iterations(aInput);
}

auto validate_step_tolerance(const input_parser::rol_optimization& aInput) -> std::optional<std::string>
{
    return input_validation::error_message_for_optional_parameter_out_of_bounds(
        input_parser::block_name<input_parser::rol_optimization>(), aInput.step_tolerance, "step_tolerance",
        utilities::lower_bounded(utilities::Exclusive{0.0}));
}

auto validate_gradient_tolerance(const input_parser::rol_optimization& aInput) -> std::optional<std::string>
{
    return input_validation::error_message_for_optional_parameter_out_of_bounds(
        input_parser::block_name<input_parser::rol_optimization>(), aInput.gradient_tolerance, "gradient_tolerance",
        utilities::lower_bounded(utilities::Exclusive{0.0}));
}

auto validate_initial_search_radius(const input_parser::rol_optimization& aInput) -> std::optional<std::string>
{
    return input_validation::error_message_for_optional_parameter_out_of_bounds(
        input_parser::block_name<input_parser::rol_optimization>(), aInput.initial_search_radius,
        "initial_search_radius", utilities::lower_bounded(utilities::Exclusive{0.0}));
}

auto validate_unique_output_name(const input_parser::rol_optimization& aInput) -> std::optional<std::string>
{
    if (aInput.input_file_name.has_value() && aInput.export_settings_file_name.has_value() &&
        aInput.input_file_name.value().mToken == aInput.export_settings_file_name.value().mToken)
    {
        return utilities::concatenate(input_parser::block_name<input_parser::rol_optimization>(),
                                      " 'export_settings_file_name' cannot be the same as 'input_file_name'.");
    }
    return std::nullopt;
}

}  // namespace detail
}  // namespace plato::process_manager::extension
