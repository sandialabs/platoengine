#include "plato/third_party_integration/snopt/SNOPTConstraints.hpp"

#include <algorithm>

#include "SNOPTTypes.hpp"

namespace plato::third_party_integration::snopt
{
namespace
{
auto partition_constraints(ConstraintVectorType& aConstraints) -> ConstraintVectorType::iterator
{
    return std::partition(aConstraints.begin(), aConstraints.end(),
                          [](const auto& aConstraint) { return aConstraint.mLinearity == Linearity::kNonlinear; });
}

[[nodiscard]] auto make_equality_constraint_vector(const SNOPTConstraints& aConstraints) -> std::vector<double>
{
    auto tTargets = std::vector<double>();
    tTargets.reserve(aConstraints.constraints().size());
    std::transform(aConstraints.constraints().begin(), aConstraints.constraints().end(), std::back_inserter(tTargets),
                   [](const auto& tConstraint) { return tConstraint.mTarget; });
    return tTargets;
}

/// @brief Remove affine term from the constraint bounds to conform to SNOPT's interface
/// Example: \f$c(x) = ax + b, l \leq c(x) \leq u \rightarrow l - c(0) \leq a*x \leq u - c(0)\f$
auto constraints_with_affine_offset_removed(ConstraintVectorType&& aConstraints,
                                            const std::size_t aNumberOfDesignVariables)
{
    const auto tZero = ConstraintFunctionArgument(aNumberOfDesignVariables, 0.0);
    for (auto& tConstraint : aConstraints)
    {
        if (tConstraint.mLinearity == Linearity::kLinear)
        {
            tConstraint.mTarget -= tConstraint.mFunction.f(tZero);
        }
    }
    return std::move(aConstraints);
}

}  // namespace

SNOPTConstraints::SNOPTConstraints(ConstraintVectorType&& aConstraints, const std::size_t aNumberOfDesignVariables)
    : mConstraints{constraints_with_affine_offset_removed(std::move(aConstraints), aNumberOfDesignVariables)},
      mLinearConstraintsBeginIterator{partition_constraints(mConstraints)}
{
}

auto SNOPTConstraints::numberOfLinearConstraints() const -> std::size_t
{
    return std::distance(mLinearConstraintsBeginIterator, mConstraints.end());
}

auto SNOPTConstraints::numberOfNonlinearConstraints() const -> std::size_t
{
    return std::distance(mConstraints.begin(), mLinearConstraintsBeginIterator);
}
auto SNOPTConstraints::linearConstraintsBegin() const -> ConstraintVectorType::const_iterator
{
    return mLinearConstraintsBeginIterator;
}

auto SNOPTConstraints::linearConstraintsEnd() const -> ConstraintVectorType::const_iterator
{
    return mConstraints.end();
}

auto SNOPTConstraints::nonlinearConstraintsBegin() const -> ConstraintVectorType::const_iterator
{
    return mConstraints.begin();
}

auto SNOPTConstraints::nonlinearConstraintsEnd() const -> ConstraintVectorType::const_iterator
{
    return mLinearConstraintsBeginIterator;
}

auto SNOPTConstraints::constraints() const -> const ConstraintVectorType& { return mConstraints; }

auto SNOPTConstraints::release() && -> ConstraintVectorType { return std::move(mConstraints); }

auto constraint_bounds(const SNOPTConstraints& aConstraints) -> SNOPTBounds
{
    return std::make_pair(make_equality_constraint_vector(aConstraints), make_equality_constraint_vector(aConstraints));
}

auto constraint_bounds_with_unbounded_objective(const SNOPTConstraints& aConstraints) -> SNOPTBounds
{
    auto [tLowerBounds, tUpperBounds] = constraint_bounds(aConstraints);
    tLowerBounds.insert(tLowerBounds.begin(), -kSNOPTUnbounded);
    tUpperBounds.insert(tUpperBounds.begin(), kSNOPTUnbounded);
    return std::make_pair(std::move(tLowerBounds), std::move(tUpperBounds));
}
}  // namespace plato::third_party_integration::snopt
