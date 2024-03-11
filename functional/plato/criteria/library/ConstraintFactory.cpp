#include "plato/criteria/library/ConstraintFactory.hpp"

#include <string>
#include <unordered_map>

#include "plato/core/MeshProxy.hpp"
#include "plato/criteria/library/CriterionFactory.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::criteria::library
{
std::vector<Constraint<const core::MeshProxy&>> make_constraints(const ValidatedConstraints& aInput)
{
    std::vector<Constraint<const core::MeshProxy&>> tConstraints;
    std::transform(aInput.rawInput().cbegin(), aInput.rawInput().cend(), std::back_inserter(tConstraints),
                   [](const core::ValidatedInputTypeWrapper<input_parser::constraint>& aValidatedInput)
                   { return detail::make_constraint(aValidatedInput); });
    return tConstraints;
}

linear_algebra::DynamicVector<double> make_dual_vector() { return linear_algebra::DynamicVector<double>{1.0}; }

namespace detail
{
Constraint<const core::MeshProxy&> make_constraint(
    const core::ValidatedInputTypeWrapper<input_parser::constraint>& aConstraintInput)
{
    const input_parser::constraint& tRawInput = aConstraintInput.rawInput();
    const double tValue = tRawInput.equal_to.value();
    const bool tIsLinear = tRawInput.is_linear.value_or(false);
    return Constraint<const core::MeshProxy&>{tRawInput.name.value_or("Unnamed Constraint"),
                                              make_criterion_function(aConstraintInput), tValue, tIsLinear};
}

}  // namespace detail
}  // namespace plato::criteria::library
