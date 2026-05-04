#include "plato/criteria/library/ConstraintFactory.hpp"

#include <map>
#include <ranges>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/core/Compose.hpp"
#include "plato/core/SplitParallelFunction.hpp"
#include "plato/criteria/library/ConstraintAdapter.hpp"
#include "plato/criteria/library/ConstraintAdapterFunctions.hpp"
#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/criteria/library/LoggingFunction.hpp"
#include "plato/linear_algebra/DynamicVectorFormatter.hpp"
#include "plato/linear_algebra/ParallelJacobianMultiplier.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::criteria::library
{
namespace
{
using ValidatedConstraint = input_validation::ValidatedInputDataBlock<components::ComponentType::kConstraint>;

template <typename... Args>
[[nodiscard]] auto make_criterion_impl(const ValidatedConstraint& aConstraintInput, Args&&... aAdditionalArgs)
{
    const auto& tRawInput = input_validation::get_input_block<input_parser::constraint>(aConstraintInput);
    const auto tRegistrationName = criterion_registration_name(tRawInput.app, tRawInput.criterion.value());

    if (criterion_function_has_trait(tRegistrationName, FunctionDimension::kVector))
    {
        return make_criterion_function<VectorCriterionFunction, input_parser::constraint>(
            aConstraintInput, std::forward<Args>(aAdditionalArgs)...);
    }

    auto [tFunction, tConfiguration] = make_criterion_function<CriterionFunction, input_parser::constraint>(
        aConstraintInput, std::forward<Args>(aAdditionalArgs)...);
    return FunctionWithConfiguration{.mFunction = to_vector_function<const analysis::AnalysisDomainMesh&>(tFunction),
                                     .mConfiguration = std::move(tConfiguration)};
}

[[nodiscard]] auto make_criterion(const boost::mpi::communicator& aCommunicator,
                                  const ValidatedConstraint& aConstraintInput)
{
    const auto& tRawInput = input_validation::get_input_block<input_parser::constraint>(aConstraintInput);
    const auto tRegistrationName = criterion_registration_name(tRawInput.app, tRawInput.criterion.value());
    const auto tNumberOfProcessors = tRawInput.number_of_processors.value_or(1U);

    if (tNumberOfProcessors > 1U && criterion_function_has_trait(tRegistrationName, Parallelization::kParallel))
    {
        return make_criterion_impl(aConstraintInput, aCommunicator);
    }
    return make_criterion_impl(aConstraintInput);
}

[[nodiscard]] auto to_vector_subset_function(const VectorCriterionFunction& aFunction,
                                             const input_parser::constraint& aInput,
                                             const services::CriterionConfiguration& aConfiguration)
{
    const auto tComponentIndices = detail::constraint_component_indices(aInput, aConfiguration);
    if (tComponentIndices)
    {
        return core::compose(make_vector_subset_function(tComponentIndices.value()), aFunction);
    }
    return aFunction;
}

[[nodiscard]] auto to_target_offset_function(const VectorCriterionFunction& aFunction,
                                             const input_parser::constraint& aInput,
                                             const services::CriterionConfiguration& aConfiguration)
{
    const auto tComponentTargets = detail::make_constraint_target_value(aInput, aConfiguration);
    auto tTargetOffsetFunction = std::visit<ConstraintAdapterFunction>(
        [](const auto& aTarget) { return make_target_offset_function(aTarget); }, tComponentTargets);

    return core::compose(tTargetOffsetFunction, aFunction);
}

}  // namespace

const std::map<input_parser::ConstraintTypes, ConstraintType> kConstraintMap{
    {input_parser::ConstraintTypes::kEqualTo, ConstraintType::kEqualTo},
    {input_parser::ConstraintTypes::kGreaterThan, ConstraintType::kGreaterThan},
    {input_parser::ConstraintTypes::kLessThan, ConstraintType::kLessThan}};

auto make_constraints(const ValidatedConstraints& aInput)
    -> std::vector<VectorConstraint<const analysis::AnalysisDomainMesh&>>
{
    const auto tConstraints =
        aInput.rawInput() |
        std::views::transform([](const auto& aValidatedInput) { return detail::make_constraint(aValidatedInput); });
    return std::vector(tConstraints.begin(), tConstraints.end());
}

namespace detail
{
auto make_constraint(const ValidatedConstraint& aConstraintInput)
    -> VectorConstraint<const analysis::AnalysisDomainMesh&>
{
    const auto& tRawInput = input_validation::get_input_block<input_parser::constraint>(aConstraintInput);
    const auto tIsLinear = tRawInput.is_linear.value_or(false);
    const auto tConstraintType = kConstraintMap.at(tRawInput.constraint_type.value());
    const auto tConstraintName = tRawInput.name.value_or("Unnamed Constraint");

    const auto tBaseCommunicator = boost::mpi::communicator{};
    auto tCriterionConfiguration = services::CriterionConfiguration{};
    const auto tCriterionFunction = core::make_split_parallel_function(
        tBaseCommunicator, tRawInput.number_of_processors.value_or(1U),
        [&aConstraintInput, &tCriterionConfiguration](const boost::mpi::communicator& aCommunicator)
        {
            auto tCriterion = make_criterion(aCommunicator, aConstraintInput);
            tCriterionConfiguration = tCriterion.mConfiguration;
            return tCriterion.mFunction;
        });

    const auto tConstraintWithSubset = to_vector_subset_function(
        make_logging_function(tCriterionFunction, components::ComponentType::kConstraint, tRawInput.name.value()),
        tRawInput, tCriterionConfiguration);
    auto tConstraintWithTargetOffset =
        to_target_offset_function(tConstraintWithSubset, tRawInput, tCriterionConfiguration);
    return VectorConstraint<const analysis::AnalysisDomainMesh&>{
        .mName = tConstraintName,
        .mConstraintFunction = std::move(tConstraintWithTargetOffset),
        .mLinear = tIsLinear,
        .mConstraintType = tConstraintType};
}

auto constraint_component_indices(const input_parser::constraint& aInput,
                                  const services::CriterionConfiguration& aConfiguration)
    -> std::optional<std::set<std::size_t>>
{
    if (aInput.constraint_value || !aConfiguration.mVectorComponents)
    {
        return std::nullopt;
    }

    assert(aInput.constraint_value_list);

    auto tIndices = std::set<std::size_t>{};
    std::ranges::transform(aInput.constraint_value_list.value().mList, std::inserter(tIndices, tIndices.begin()),
                           [&aConfiguration](const auto& aComponentAndTarget)
                           {
                               return utilities::key_with_value(aConfiguration.mVectorComponents.value(),
                                                                aComponentAndTarget.component.mToken)
                                   .value();
                           });
    return tIndices;
}

[[nodiscard]] auto make_constraint_target_value(const input_parser::constraint& aInput,
                                                const services::CriterionConfiguration& aConfiguration)
    -> ConstraintTargetValue
{
    if (aInput.constraint_value)
    {
        return aInput.constraint_value.value();
    }

    assert(aConfiguration.mVectorComponents);
    assert(aInput.constraint_value_list);

    return make_constraint_vector_target(to_vector(aInput.constraint_value_list->mList),
                                         aConfiguration.mVectorComponents.value());
}

auto make_constraint_vector_target(const std::vector<std::pair<std::string, double>>& aConstraintTargets,
                                   const std::map<std::size_t, std::string>& aComponentIndexNameAssociations)
    -> std::vector<double>
{
    const auto tFindComponent = [&aConstraintTargets](const std::string& aComponentName)
    {
        return std::ranges::find_if(aConstraintTargets, [&aComponentName](const auto& aTargetValueAndName)
                                    { return aTargetValueAndName.first == aComponentName; });
    };

    auto tTargets = utilities::reserved_container<std::vector<double>>(aConstraintTargets.size());
    utilities::transform_if(
        aComponentIndexNameAssociations, std::back_inserter(tTargets),
        [&tFindComponent](const auto& aComponentIndexAndName)
        { return tFindComponent(aComponentIndexAndName.second)->second; },
        [&aConstraintTargets, &tFindComponent](const auto& aComponentIndexAndName)
        { return tFindComponent(aComponentIndexAndName.second) != aConstraintTargets.end(); });
    return tTargets;
}

}  // namespace detail
}  // namespace plato::criteria::library
