#include "plato/process_manager/extension/snopt/SNOPTOptimization.hpp"

#include <boost/numeric/conversion/cast.hpp>
#include <filesystem>
#include <iterator>
#include <map>
#include <optional>

#include "plato/core/Compose.hpp"
#include "plato/core/ValidationRegistration.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/geometry/library/OutputManager.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/process_manager/extension/CommonInputValidation.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/process_manager/library/StageOrdering.hpp"
#include "plato/third_party_integration/snopt/SNOPTInterface.hpp"
#include "plato/third_party_integration/snopt/SNOPTTypes.hpp"
#include "plato/utilities/BoostOptionalToStdOptional.hpp"

namespace plato::process_manager::extension::snopt
{
namespace
{

constexpr std::string_view kSNOPTOptimizerFileName = "SNOPT_Optimization.txt";

[[nodiscard]] library::StageAndProcessManager make_snopt_optimization_process_manager(
    const library::ValidatedProcessManagerInput& aValidInput)
{
    return {library::RunStage::kExecute, [aValidInput](const library::ProcessManagerData& aProcessManangerData)
            {
                const auto& tInput = library::process_manager_input<input_parser::snopt_optimization>(aValidInput);
                SNOPTOptimization{tInput}.run(aProcessManangerData);
            }};
}

[[maybe_unused]] static auto kSNOPTOptimizerProcessManagerRegistration =
    library::ProcessManagerRegistration{input_parser::block_name<input_parser::snopt_optimization>(),
                                        [](const library::ValidatedProcessManagerInput& aValidInput)
                                        { return make_snopt_optimization_process_manager(aValidInput); }};

[[maybe_unused]] static auto kSNOPTOptimizerValidationRegistration =
    core::ValidationRegistration<input_parser::snopt_optimization>{
        [](const input_parser::snopt_optimization& aInput) { return detail::validate_time_limit_in_minutes(aInput); },
        [](const input_parser::snopt_optimization& aInput)
        { return ::plato::process_manager::extension::detail::validate_max_iterations(aInput); },
        [](const input_parser::snopt_optimization& aInput)
        {
            return ::plato::process_manager::extension::detail::validate_optional_input_file_name<
                input_parser::snopt_optimization>(aInput);
        }};
}  // namespace

SNOPTOptimization::SNOPTOptimization(const ValidatedOptimizationParameters& aInput)
    : mOptions{utilities::to_unwrapped_optional(aInput.rawInput().input_file_name,
                                                [](const auto& aFileName) { return aFileName.mToken; }),
               utilities::to_std_optional(aInput.rawInput().time_limit_in_minutes),
               utilities::to_std_optional(aInput.rawInput().max_iterations),
               utilities::to_std_optional(aInput.rawInput().output_design_history)}
{
}

void SNOPTOptimization::run(const library::ProcessManagerData& aProcessManagerData) const
{
    namespace gl = geometry::library;
    namespace tpis = third_party_integration::snopt;
    using SNOPTObjectiveFunction = typename tpis::ObjectiveType;

    const tpis::SNOPTBounds tBounds{aProcessManagerData.mGeometry.mBounds};
    const auto tInitialGuess = aProcessManagerData.mGeometry.mInitialGuess.stdVector();
    auto tObjective = core::compose(aProcessManagerData.mObjective, aProcessManagerData.mGeometry.mCompute)
                          .compatibleFunction<SNOPTObjectiveFunction>();
    auto tConstraints = detail::make_constraints(aProcessManagerData);

    const auto tOutputMode = mOptions.mOutputDesignHistory.value_or(false) ? gl::OutputMode::kEveryIterationAppend
                                                                           : gl::OutputMode::kEveryIterationOverwrite;
    auto tOutputManager = gl::OutputManager{aProcessManagerData.mGeometry.mOutput, tOutputMode};

    const auto tSolution =
        tpis::run_snopt_problem(tInitialGuess, tBounds, std::move(tObjective), std::move(tConstraints),
                                std::move(tOutputManager), std::string{kSNOPTOptimizerFileName}, mOptions);
}

namespace detail
{
namespace
{
const auto tConstraintTypeConversion =
    std::map<criteria::library::ConstraintType, third_party_integration::snopt::ConstraintType>{
        {criteria::library::ConstraintType::kEqualTo, third_party_integration::snopt::ConstraintType::kEqualTo},
        {criteria::library::ConstraintType::kGreaterThan, third_party_integration::snopt::ConstraintType::kGreaterThan},
        {criteria::library::ConstraintType::kLessThan, third_party_integration::snopt::ConstraintType::kLesserThan}};

[[nodiscard]] auto make_snopt_constraint(
    const geometry::library::FactoryTypes& aGeometry,
    const criteria::library::VectorConstraint<const analysis::AnalysisDomainMesh&>& aConstraint)
    -> third_party_integration::snopt::InterfaceConstraintType
{
    namespace tpis = third_party_integration::snopt;
    const auto tLinearity = aConstraint.mLinear ? tpis::Linearity::kLinear : tpis::Linearity::kNonlinear;
    using SNOPTConstraintFunction = typename tpis::InterfaceConstraintType::ConstraintFunction;
    auto tFunction = core::compose(aConstraint.mConstraintFunction, aGeometry.mCompute)
                         .compatibleFunction<SNOPTConstraintFunction>();
    const auto tConstraintSize =
        tFunction.template evaluate<core::evaluation::kFunction>(aGeometry.mInitialGuess).size();
    auto tConstraintTargets = std::vector(tConstraintSize, aConstraint.mConstraintTarget);
    return tpis::InterfaceConstraintType{std::move(tFunction), std::move(tConstraintTargets), tLinearity,
                                         tConstraintSize, tConstraintTypeConversion.at(aConstraint.mConstraintType)};
}

}  // namespace

auto make_constraints(const library::ProcessManagerData& aProcessManagerData)
    -> third_party_integration::snopt::InterfaceConstraintVectorType
{
    auto tConstraints = third_party_integration::snopt::InterfaceConstraintVectorType{};
    tConstraints.reserve(aProcessManagerData.mConstraints.size());
    std::transform(aProcessManagerData.mConstraints.begin(), aProcessManagerData.mConstraints.end(),
                   std::back_inserter(tConstraints),
                   [&aProcessManagerData](const auto& aConstraint)
                   { return make_snopt_constraint(aProcessManagerData.mGeometry, aConstraint); });

    return tConstraints;
}

std::optional<std::string> validate_time_limit_in_minutes(const input_parser::snopt_optimization& aInput)
{
    return core::error_message_for_optional_parameter_out_of_bounds(
        input_parser::block_name<input_parser::snopt_optimization>(), aInput.time_limit_in_minutes,
        "time_limit_in_minutes", utilities::lower_bounded(utilities::Inclusive{0U}));
}

}  // namespace detail

}  // namespace plato::process_manager::extension::snopt
