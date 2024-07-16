#include "plato/process_manager/extension/ROLOptimization.hpp"

#include <fstream>
#include <string_view>

#include "plato/core/ValidationUtilities.hpp"
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

[[maybe_unused]] static auto kROLOptimizerProcessManagerRegistration =
    library::ProcessManagerRegistration{input_parser::block_name<input_parser::rol_optimization>(),
                                        [](const library::ValidatedProcessManagerInput& aValidInput)
                                        { return make_rol_optimization_process_manager(aValidInput); }};

[[maybe_unused]] static auto kOptimizerValidationRegistration =
    core::ValidationRegistration<input_parser::rol_optimization>{
        [](const input_parser::rol_optimization& aInput) { return detail::validate_max_iterations(aInput); },
        [](const input_parser::rol_optimization& aInput) { return detail::validate_step_tolerance(aInput); },
        [](const input_parser::rol_optimization& aInput) { return detail::validate_gradient_tolerance(aInput); }};

}  // namespace

ROLOptimization::ROLOptimization(const ValidatedOptimizationParameters& aInput)
    : mROLOptions{third_party_integration::rol::rol_parameter_list(aInput)}
{
}

void ROLOptimization::run(const library::ProcessManagerData& aProblem) const
{
    auto tROLProblem = ROL::Ptr<ROL::Problem<double>>{make_rol_problem(aProblem).release()};
    auto tROLInputs = mROLOptions;
    auto tROLSolver = third_party_integration::rol::make_rol_solver(tROLInputs, tROLProblem);

    auto tOutFile = std::ofstream{std::string{kROLOptimizerFileName}};
    tROLSolver.solve(tOutFile);

    if (mCommunicator.rank() == 0)
    {
        aProblem.mGeometry.mOutput(
            third_party_integration::rol::to_dynamic_vector(*tROLProblem->getPrimalOptimizationVector()));
    }
}

namespace detail
{
std::optional<std::string> validate_max_iterations(const input_parser::rol_optimization& aInput)
{
    namespace pfu = plato::utilities;
    if (!aInput.input_file_name || (aInput.input_file_name && aInput.max_iterations))
    {
        return core::error_message_for_parameter_out_of_bounds(
            input_parser::block_name<input_parser::rol_optimization>(), aInput.max_iterations, "max_iterations",
            pfu::lower_bounded(pfu::Inclusive{1u}));
    }
    else
    {
        return std::nullopt;
    }
}

std::optional<std::string> validate_step_tolerance(const input_parser::rol_optimization& aInput)
{
    namespace pfu = plato::utilities;
    if (!aInput.input_file_name || (aInput.input_file_name && aInput.step_tolerance))
    {
        return core::error_message_for_parameter_out_of_bounds(
            input_parser::block_name<input_parser::rol_optimization>(), aInput.step_tolerance, "step_tolerance",
            pfu::lower_bounded(pfu::Exclusive{0.0}));
    }
    else
    {
        return std::nullopt;
    }
}

std::optional<std::string> validate_gradient_tolerance(const input_parser::rol_optimization& aInput)
{
    namespace pfu = plato::utilities;
    if (!aInput.input_file_name || (aInput.input_file_name && aInput.gradient_tolerance))
    {
        return core::error_message_for_parameter_out_of_bounds(
            input_parser::block_name<input_parser::rol_optimization>(), aInput.gradient_tolerance, "gradient_tolerance",
            pfu::lower_bounded(pfu::Exclusive{0.0}));
    }
    else
    {
        return std::nullopt;
    }
}
}  // namespace detail
}  // namespace plato::process_manager::extension
