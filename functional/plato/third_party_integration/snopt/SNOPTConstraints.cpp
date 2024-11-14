#include "plato/third_party_integration/snopt/SNOPTConstraints.hpp"

#include <algorithm>

#include "plato/utilities/IndexRange.hpp"

namespace plato::third_party_integration::snopt
{
namespace
{
[[nodiscard]] auto partition_constraints(ConstraintVectorType& aConstraints) -> ConstraintVectorType::iterator
{
    return std::stable_partition(aConstraints.begin(), aConstraints.end(),
                                 [](const auto& aConstraint)
                                 { return aConstraint.mLinearity == Linearity::kNonlinear; });
}

template <typename BoundFunction>
[[nodiscard]] auto make_constraint_bound_vector(const SNOPTConstraints& aConstraints,
                                                const BoundFunction& aBoundFunction) -> std::vector<double>
{
    auto tTargets = std::vector<double>();
    tTargets.reserve(aConstraints.constraints().size());
    std::transform(aConstraints.constraints().begin(), aConstraints.constraints().end(), std::back_inserter(tTargets),
                   aBoundFunction);
    return tTargets;
}

[[nodiscard]] auto scalar_constraint_from_vector_constraint(const InterfaceConstraintType& aVectorConstraint,
                                                            const std::size_t aComponentIndex) -> CriterionType
{
    return CriterionType{
        [tVectorFunction = aVectorConstraint.mFunction,
         aComponentIndex](const linear_algebra::DynamicVector<double>& aDesignVariables)
        { return tVectorFunction.evaluate<core::evaluation::kFunction>(aDesignVariables)[aComponentIndex]; },
        [tVectorConstraint = aVectorConstraint,
         aComponentIndex](const linear_algebra::DynamicVector<double>& aDesignVariables)
        {
            const auto tJacobian =
                tVectorConstraint.mFunction.evaluate<core::evaluation::kFirstDerivative>(aDesignVariables);
            auto tBasisVector = std::vector<double>(tVectorConstraint.mConstraintDimension, 0.0);
            tBasisVector[aComponentIndex] = 1.0;
            return linear_algebra::DynamicVector<double>(std::move(tBasisVector)) * tJacobian;
        }};
}

}  // namespace

SNOPTConstraints::SNOPTConstraints(InterfaceConstraintVectorType&& aConstraints,
                                   const std::size_t aNumberOfDesignVariables)
    : mConstraints{constraints_with_affine_offset_removed(constraints_with_vectors_expanded(std::move(aConstraints)),
                                                          aNumberOfDesignVariables)},
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
    const auto tLowerBoundFunction = [](const ConstraintData& aConstraint)
    { return aConstraint.mConstraintType == ConstraintType::kLesserThan ? -kSNOPTUnbounded : aConstraint.mTarget; };

    const auto tUpperBoundFunction = [](const ConstraintData& aConstraint)
    { return aConstraint.mConstraintType == ConstraintType::kGreaterThan ? kSNOPTUnbounded : aConstraint.mTarget; };

    return std::make_pair(make_constraint_bound_vector(aConstraints, tLowerBoundFunction),
                          make_constraint_bound_vector(aConstraints, tUpperBoundFunction));
}

auto constraint_bounds_with_unbounded_objective(const SNOPTConstraints& aConstraints) -> SNOPTBounds
{
    auto [tLowerBounds, tUpperBounds] = constraint_bounds(aConstraints);
    tLowerBounds.insert(tLowerBounds.begin(), -kSNOPTUnbounded);
    tUpperBounds.insert(tUpperBounds.begin(), kSNOPTUnbounded);
    return std::make_pair(std::move(tLowerBounds), std::move(tUpperBounds));
}

auto constraints_with_affine_offset_removed(ConstraintVectorType&& aConstraints,
                                            const std::size_t aNumberOfDesignVariables) -> ConstraintVectorType
{
    const auto tZero = ConstraintFunctionArgument(aNumberOfDesignVariables, 0.0);
    for (auto& tConstraint : aConstraints)
    {
        if (tConstraint.mLinearity == Linearity::kLinear)
        {
            tConstraint.mTarget -= tConstraint.mFunction.template evaluate<core::evaluation::kFunction>(tZero);
        }
    }
    return std::move(aConstraints);
}

auto constraints_with_vectors_expanded(InterfaceConstraintVectorType&& aVectorConstraints) -> ConstraintVectorType
{
    auto tAllScalarConstraints = ConstraintVectorType{};
    tAllScalarConstraints.reserve(detail::total_number_of_scalar_constraints(aVectorConstraints));
    for (const auto& tVectorConstraint : aVectorConstraints)
    {
        auto tScalarConstraints = detail::constraint_with_vectors_expanded(tVectorConstraint);
        std::move(tScalarConstraints.begin(), tScalarConstraints.end(), std::back_inserter(tAllScalarConstraints));
    }
    return tAllScalarConstraints;
}

namespace detail
{
auto total_number_of_scalar_constraints(const InterfaceConstraintVectorType& aVectorConstraints) -> std::size_t
{
    return std::accumulate(aVectorConstraints.begin(), aVectorConstraints.end(), std::size_t{0},
                           [](const std::size_t aSum, const auto& aVectorConstraint)
                           { return aSum + aVectorConstraint.mConstraintDimension; });
}

auto constraint_with_vectors_expanded(const InterfaceConstraintType& aVectorConstraint) -> ConstraintVectorType
{
    const auto tNumberOfScalarConstraints = aVectorConstraint.mConstraintDimension;
    auto tScalarConstraints = ConstraintVectorType{};
    tScalarConstraints.reserve(aVectorConstraint.mConstraintDimension);
    for (const auto tConstraintComponent : utilities::IndexRange{tNumberOfScalarConstraints})
    {
        tScalarConstraints.emplace_back(ConstraintData{
            /*.mFunction=*/scalar_constraint_from_vector_constraint(aVectorConstraint, tConstraintComponent),
            /*.mTarget=*/aVectorConstraint.mTargets.at(tConstraintComponent),
            /*.mLinearity=*/aVectorConstraint.mLinearity,
            /*.mConstraintType=*/aVectorConstraint.mConstraintType});
    }
    return tScalarConstraints;
}
}  // namespace detail

}  // namespace plato::third_party_integration::snopt
