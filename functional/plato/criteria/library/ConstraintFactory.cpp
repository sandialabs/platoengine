#include "plato/criteria/library/ConstraintFactory.hpp"

#include <map>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/criteria/library/ConstraintAdapter.hpp"
#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/utilities/TransformIf.hpp"

namespace plato::criteria::library
{
namespace
{
const auto kIsActive = [](const auto& aConstraint) { return core::is_active(aConstraint.rawInput()); };
}

const std::map<input_parser::ConstraintTypes, ConstraintType> kConstraintMap{
    {input_parser::ConstraintTypes::kEquality, ConstraintType::kEquality},
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

auto make_dual_vector(const unsigned int aSize) -> linear_algebra::DynamicVector<double>
{
    return linear_algebra::DynamicVector<double>{std::vector<double>(aSize, 1.0)};
}

namespace detail
{

auto make_constraint(const core::ValidatedInputTypeWrapper<input_parser::constraint>& aConstraintInput)
    -> VectorConstraint<const analysis::AnalysisDomainMesh&>
{
    const input_parser::constraint& tRawInput = aConstraintInput.rawInput();
    const double tValue = tRawInput.constraint_value.value();
    const bool tIsLinear = tRawInput.is_linear.value_or(false);

    const auto tConstraint = Constraint<const analysis::AnalysisDomainMesh&>{
        tRawInput.name.value_or("Unnamed Constraint"), make_criterion_function(aConstraintInput), tValue, tIsLinear,
        kConstraintMap.at(tRawInput.constraint_type.value())};

    return make_vector_constraint(tConstraint);
}

}  // namespace detail
}  // namespace plato::criteria::library
