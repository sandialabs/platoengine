#include "plato/criteria/library/ConstraintFactory.hpp"

#include <map>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/library/ConstraintAdapter.hpp"
#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/ConstraintTarget.hpp"
#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/services/AppConfigurationUtilities.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::criteria::library
{
namespace
{
using ValidatedConstraint = input_validation::ValidatedInputDataBlock<components::ComponentType::kConstraint>;

const auto kIsActive = [](const auto& aConstraint)
{ return input_validation::is_active(input_validation::get_input_block<input_parser::constraint>(aConstraint)); };

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
    auto tConstraintValue = make_constraint_target(tRawInput, services::plugin_configurations());
    const auto tIsLinear = tRawInput.is_linear.value_or(false);
    const auto tRegistrationName = criterion_registration_name(tRawInput.app, tRawInput.criterion.value());
    const auto tConstraintType = kConstraintMap.at(tRawInput.constraint_type.value());
    const auto tConstraintName = tRawInput.name.value_or("Unnamed Constraint");

    constexpr auto tVectorTraits = CriterionTraits{Parallelization::kSerial, FunctionDimension::kVector};
    auto tCriterionFunction =
        criterion_function_has_traits(tRegistrationName, tVectorTraits)
            ? make_criterion_function<VectorCriterionFunction, input_parser::constraint>(aConstraintInput)
            : to_vector_function<const analysis::AnalysisDomainMesh&>(
                  make_criterion_function<CriterionFunction, input_parser::constraint>(aConstraintInput));

    return VectorConstraint<const analysis::AnalysisDomainMesh&>{.mName = tConstraintName,
                                                                 .mConstraintFunction = std::move(tCriterionFunction),
                                                                 .mConstraintTarget = std::move(tConstraintValue),
                                                                 .mLinear = tIsLinear,
                                                                 .mConstraintType = tConstraintType};
}

auto make_constraint_target(const input_parser::constraint& aInput,
                            const std::vector<services::AppConfigurationWithDirectory>& aAppConfigurations)
    -> ConstraintTarget
{
    if (aInput.constraint_value)
    {
        return ConstraintTarget{aInput.constraint_value.value()};
    }

    assert(aInput.app);
    assert(aInput.criterion);

    const auto tCriterionConfiguration = services::criterion_configuration_with_name(
        services::CriterionName{.mAppName = aInput.app.value().mToken,
                                .mCriterionName = aInput.criterion.value().mToken},
        aAppConfigurations);

    assert(tCriterionConfiguration);
    assert(tCriterionConfiguration.value().mVectorComponents);
    assert(aInput.constraint_value_list);

    return criteria::library::make_constraint_target(to_vector(aInput.constraint_value_list->mList),
                                                     tCriterionConfiguration.value().mVectorComponents.value());
}

}  // namespace detail
}  // namespace plato::criteria::library
