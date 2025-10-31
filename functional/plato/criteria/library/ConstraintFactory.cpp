#include "plato/criteria/library/ConstraintFactory.hpp"

#include <map>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/core/Compose.hpp"
#include "plato/criteria/library/ConstraintAdapter.hpp"
#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/ConstraintTarget.hpp"
#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/criteria/library/VectorSubsetFunction.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::criteria::library
{
namespace
{
using ValidatedConstraint = input_validation::ValidatedInputDataBlock<components::ComponentType::kConstraint>;

const auto kIsActive = [](const auto& aConstraint)
{ return input_validation::is_active(input_validation::get_input_block<input_parser::constraint>(aConstraint)); };

[[nodiscard]] auto make_vector_criterion(const ValidatedConstraint& aConstraintInput)
{
    auto [tFunction, tConfiguration] =
        make_criterion_function<CriterionFunction, input_parser::constraint>(aConstraintInput);
    return FunctionWithConfiguration{.mFunction = to_vector_function<const analysis::AnalysisDomainMesh&>(tFunction),
                                     .mConfiguration = std::move(tConfiguration)};
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

}  // namespace

const std::map<input_parser::ConstraintTypes, ConstraintType> kConstraintMap{
    {input_parser::ConstraintTypes::kEqualTo, ConstraintType::kEqualTo},
    {input_parser::ConstraintTypes::kGreaterThan, ConstraintType::kGreaterThan},
    {input_parser::ConstraintTypes::kLessThan, ConstraintType::kLessThan}};

auto make_constraints(const ValidatedConstraints& aInput)
    -> std::vector<VectorConstraint<const analysis::AnalysisDomainMesh&>>
{
    auto tConstraints = std::vector<VectorConstraint<const analysis::AnalysisDomainMesh&>>{};
    utilities::transform_if(
        aInput.rawInput(), std::back_inserter(tConstraints),
        [](const auto& aValidatedInput) { return detail::make_constraint(aValidatedInput); }, kIsActive);

    return tConstraints;
}

auto make_dual_vector(const std::size_t aSize) -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>{std::vector<double>(aSize, 1.0)};
}

namespace detail
{
auto make_constraint(const ValidatedConstraint& aConstraintInput)
    -> VectorConstraint<const analysis::AnalysisDomainMesh&>
{
    const auto& tRawInput = input_validation::get_input_block<input_parser::constraint>(aConstraintInput);
    const auto tIsLinear = tRawInput.is_linear.value_or(false);
    const auto tRegistrationName = criterion_registration_name(tRawInput.app, tRawInput.criterion.value());
    const auto tConstraintType = kConstraintMap.at(tRawInput.constraint_type.value());
    const auto tConstraintName = tRawInput.name.value_or("Unnamed Constraint");

    constexpr auto tVectorTraits = CriterionTraits{Parallelization::kSerial, FunctionDimension::kVector};
    auto [tCriterionFunction, tCriterionConfiguration] =
        criterion_function_has_traits(tRegistrationName, tVectorTraits)
            ? make_criterion_function<VectorCriterionFunction, input_parser::constraint>(aConstraintInput)
            : make_vector_criterion(aConstraintInput);

    auto tConstraintValue = make_constraint_target(tRawInput, tCriterionConfiguration);

    return VectorConstraint<const analysis::AnalysisDomainMesh&>{
        .mName = tConstraintName,
        .mConstraintFunction = to_vector_subset_function(tCriterionFunction, tRawInput, tCriterionConfiguration),
        .mConstraintTarget = std::move(tConstraintValue),
        .mLinear = tIsLinear,
        .mConstraintType = tConstraintType};
}

auto make_constraint_target(const input_parser::constraint& aInput,
                            const services::CriterionConfiguration& aConfiguration) -> ConstraintTarget
{
    if (aInput.constraint_value)
    {
        return ConstraintTarget{aInput.constraint_value.value()};
    }

    assert(aConfiguration.mVectorComponents);
    assert(aInput.constraint_value_list);

    return criteria::library::make_constraint_target(to_vector(aInput.constraint_value_list->mList),
                                                     aConfiguration.mVectorComponents.value());
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
                               return utilities::find_key_with_value(aConfiguration.mVectorComponents.value(),
                                                                     aComponentAndTarget.component.mToken)
                                   .value();
                           });
    return tIndices;
}
}  // namespace detail
}  // namespace plato::criteria::library
