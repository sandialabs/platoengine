#include "plato/process_manager/extension/ROLOptimization.hpp"

#include <ROL_Ptr.hpp>
#include <fstream>
#include <string_view>

#include "plato/core/ValidationUtilities.hpp"
#include "plato/geometry/library/OutputManager.hpp"
#include "plato/input_parser/ComponentParserRegistration.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/process_manager/extension/CommonInputValidation.hpp"
#include "plato/process_manager/extension/ROLUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/process_manager/library/StageOrdering.hpp"
#include "plato/third_party_integration/rol/OptimizerFactory.hpp"
#include "plato/third_party_integration/rol/ROLObjectiveFunction.hpp"
#include "plato/third_party_integration/rol/Utilities.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::process_manager::extension
{
namespace
{
constexpr std::string_view kROLOptimizerFileName = "ROL_Optimizer.txt";

[[nodiscard]] library::StageAndProcessManager make_rol_optimization_process_manager(
    const library::ValidatedProcessManagerInput& aValidInput)
{
    return {library::RunStage::kExecute, [aValidInput](const library::ProcessManagerData& aProcessManangerData)
            {
                const auto& tInput = library::process_manager_input<input_parser::rol_optimization>(aValidInput);
                ROLOptimization{tInput}.run(aProcessManangerData);
            }};
}

[[maybe_unused]] static auto kROLOptimizerParserRegistration =
    input_parser::ComponentParserRegistration<input_parser::new_rol_optimization,
                                              input_parser::ComponentType::kProcessManager>{};

[[maybe_unused]] static auto kROLOptimizerProcessManagerRegistration =
    library::ProcessManagerRegistration{input_parser::block_name<input_parser::rol_optimization>(),
                                        [](const library::ValidatedProcessManagerInput& aValidInput)
                                        { return make_rol_optimization_process_manager(aValidInput); }};

[[maybe_unused]] static auto kOptimizerValidationRegistration =
    input_validation::ValidationRegistration<input_parser::new_rol_optimization>{
        [](const input_parser::new_rol_optimization& aInput) { return detail::validate_rol_max_iterations(aInput); },
        [](const input_parser::new_rol_optimization& aInput) { return detail::validate_step_tolerance(aInput); },
        [](const input_parser::new_rol_optimization& aInput) { return detail::validate_gradient_tolerance(aInput); },
        [](const input_parser::new_rol_optimization& aInput) { return detail::validate_initial_search_radius(aInput); },
        [](const input_parser::new_rol_optimization& aInput) { return detail::validate_unique_output_name(aInput); },
        [](const input_parser::new_rol_optimization& aInput)
        { return detail::validate_optional_input_file_name(aInput); }};
}  // namespace

ROLOptimization::ROLOptimization(const ValidatedOptimizationParameters& aInput)
    : mROLOptions{third_party_integration::rol::make_optimization_parameters(aInput)}
{
}

void ROLOptimization::run(const library::ProcessManagerData& aProcessManagerData) const
{
    namespace gl = geometry::library;
    const auto tOutputMode = mROLOptions.writeOutputHistory() ? gl::OutputMode::kEveryIterationAppend
                                                              : gl::OutputMode::kEveryIterationOverwrite;
    auto tOutputManager = gl::OutputManager{aProcessManagerData.mGeometry.mOutput, tOutputMode};

    auto tObjective = ROL::Ptr<plato::third_party_integration::rol::ROLObjectiveFunction>(
        make_rol_objective(aProcessManagerData, std::move(tOutputManager)).release());
    auto [tROLProblem, tROLControls] = make_rol_problem(aProcessManagerData, tObjective);
    auto tROLInputs = mROLOptions.parameters();
    auto tROLSolver = third_party_integration::rol::make_rol_solver(tROLInputs, tROLProblem);

    auto tOutFile = std::ofstream{std::string{kROLOptimizerFileName}};
    tROLSolver.solve(tOutFile);

    tObjective->finalUpdate(third_party_integration::rol::to_dynamic_vector(*tROLControls));
}

auto create_valid_example_rol_optimization_input() -> input_parser::new_rol_optimization
{
    return input_parser::new_rol_optimization{/*.input_file_name=*/boost::none,
                                              /*.export_settings_file_name=*/input_parser::FileName{"output_rol.xml"},
                                              /*.max_iterations =  */ 42,
                                              /*.step_tolerance = */ 1e-7,
                                              /*.gradient_tolerance = */ 1e-5,
                                              /*.initial_search_radius = */ 15,
                                              /*.verbose_output = */ false,
                                              /*.approximate_hessian = */ false,
                                              /*.output_design_history = */ false};
}

namespace detail
{
auto validate_rol_max_iterations(const input_parser::new_rol_optimization& aInput) -> std::optional<std::string>
{
    return ::plato::process_manager::extension::detail::validate_max_iterations(aInput);
}

auto validate_step_tolerance(const input_parser::new_rol_optimization& aInput) -> std::optional<std::string>
{
    return core::error_message_for_optional_parameter_out_of_bounds(
        input_parser::block_name<input_parser::rol_optimization>(), aInput.step_tolerance, "step_tolerance",
        utilities::lower_bounded(utilities::Exclusive{0.0}));
}

auto validate_gradient_tolerance(const input_parser::new_rol_optimization& aInput) -> std::optional<std::string>
{
    return core::error_message_for_optional_parameter_out_of_bounds(
        input_parser::block_name<input_parser::new_rol_optimization>(), aInput.gradient_tolerance, "gradient_tolerance",
        utilities::lower_bounded(utilities::Exclusive{0.0}));
}

auto validate_initial_search_radius(const input_parser::new_rol_optimization& aInput) -> std::optional<std::string>
{
    return core::error_message_for_optional_parameter_out_of_bounds(
        input_parser::block_name<input_parser::new_rol_optimization>(), aInput.initial_search_radius,
        "initial_search_radius", utilities::lower_bounded(utilities::Exclusive{0.0}));
}

auto validate_unique_output_name(const input_parser::new_rol_optimization& aInput) -> std::optional<std::string>
{
    if (aInput.input_file_name.has_value() && aInput.export_settings_file_name.has_value() &&
        aInput.input_file_name.value().mToken == aInput.export_settings_file_name.value().mToken)
    {
        return utilities::concatenate(input_parser::block_name<input_parser::new_rol_optimization>(),
                                      " 'export_settings_file_name' cannot be the same as 'input_file_name'.");
    }
    return std::nullopt;
}

}  // namespace detail
}  // namespace plato::process_manager::extension
