#include "plato/criteria/library/ConstraintFactory.hpp"

#include <map>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/criteria/library/ConstraintAdapter.hpp"
#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::criteria::library
{
namespace
{
const auto kIsActive = [](const auto& aConstraint) { return core::is_active(aConstraint.rawInput()); };
}  // namespace

const std::map<input_parser::ConstraintTypes, ConstraintType> kConstraintMap{
    {input_parser::ConstraintTypes::kEqualTo, ConstraintType::kEqualTo},
    {input_parser::ConstraintTypes::kGreaterThan, ConstraintType::kGreaterThan},
    {input_parser::ConstraintTypes::kLessThan, ConstraintType::kLessThan}};

auto make_constraints(const ValidatedConstraints& aInput)
    -> std::vector<VectorConstraint<const analysis::AnalysisDomainMesh&>>
{
    std::vector<VectorConstraint<const analysis::AnalysisDomainMesh&>> tConstraints;
    utilities::transform_if(
        aInput.rawInput(), std::back_inserter(tConstraints),
        [](const core::ValidatedInputTypeWrapper<input_parser::constraint>& aValidatedInput)
        { return detail::make_constraint(aValidatedInput); },
        kIsActive);

    return tConstraints;
}

auto make_dual_vector(const std::size_t aSize) -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>{std::vector<double>(aSize, 1.0)};
}

namespace detail
{

auto make_constraint(const core::ValidatedInputTypeWrapper<input_parser::constraint>& aConstraintInput)
    -> VectorConstraint<const analysis::AnalysisDomainMesh&>
{
    const auto& tRawInput = aConstraintInput.rawInput();
    const auto tValue = tRawInput.constraint_value.value();
    const auto tIsLinear = tRawInput.is_linear.value_or(false);
    const auto tRegistrationName = criterion_registration_name(tRawInput.app, tRawInput.criterion.value());
    const auto tConstraintType = kConstraintMap.at(tRawInput.constraint_type.value());
    const auto tConstraintName = tRawInput.name.value_or("Unnamed Constraint");

    constexpr auto tVectorTraits = CriterionTraits{Parallelization::kSerial, FunctionDimension::kVector};
    auto tCriterionFunction = criterion_function_has_traits(tRegistrationName, tVectorTraits)
                                  ? make_criterion_function<VectorCriterionFunction>(aConstraintInput)
                                  : to_vector_function<const analysis::AnalysisDomainMesh&>(
                                        make_criterion_function<CriterionFunction>(aConstraintInput));

    return VectorConstraint<const analysis::AnalysisDomainMesh&>{tConstraintName, std::move(tCriterionFunction), tValue,
                                                                 tIsLinear, tConstraintType};
}

}  // namespace detail
}  // namespace plato::criteria::library
