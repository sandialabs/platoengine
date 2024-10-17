#ifndef PLATO_CRITERIA_LIBRARY_CONSTRAINTADAPTOR
#define PLATO_CRITERIA_LIBRARY_CONSTRAINTADAPTOR

#include <algorithm>
#include <iterator>

#include "plato/core/Function.hpp"
#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::criteria::library
{

template <typename FunctionArg>
using ScalarFunction = typename Constraint<FunctionArg>::ConstraintFunction;

template <typename FunctionArg>
using VectorFunction = typename VectorConstraint<FunctionArg>::ConstraintFunction;

namespace detail
{

template <typename FunctionArg>
auto make_vector_function(const VectorFunction<FunctionArg>& aVectorFunction) -> VectorFunction<FunctionArg>
{
    return aVectorFunction;
}

template <typename FunctionArg>
auto make_vector_function(const ScalarFunction<FunctionArg>& aScalarFunction) -> VectorFunction<FunctionArg>
{
    auto tEvaluation = [aScalarFunction](FunctionArg aFunctionArg)
    {
        return linear_algebra::DynamicVector<double>{
            aScalarFunction.template evaluate<core::evaluation::kFunction>(aFunctionArg)};
    };
    auto tJacobian = [aScalarFunction](FunctionArg aFunctionArg)
    {
        auto tGradient = aScalarFunction.template evaluate<core::evaluation::kFirstDerivative>(aFunctionArg);
        auto tMultiplierFunction = [tGradient = std::move(tGradient)](const linear_algebra::DynamicVector<double>& aV)
        { return tGradient * aV.stdVector()[0]; };
        return linear_algebra::JacobianMultiplier{std::move(tMultiplierFunction)};
    };
    auto tAdjointJacobian = [aScalarFunction](FunctionArg aFunctionArg)
    {
        auto tGradient = aScalarFunction.template evaluate<core::evaluation::kFirstDerivative>(aFunctionArg);
        auto tMultiplierFunction = [tGradient = std::move(tGradient)](const linear_algebra::DynamicVector<double>& aV)
        { return linear_algebra::DynamicVector{tGradient.dot(aV)}; };
        return linear_algebra::AdjointJacobianMultiplier{
            linear_algebra::JacobianMultiplier{std::move(tMultiplierFunction)}};
    };
    return VectorFunction<FunctionArg>{std::move(tEvaluation), std::move(tJacobian), std::move(tAdjointJacobian)};
}
}  // namespace detail

template <typename FunctionArg>
auto make_vector_constraint(const VectorConstraint<FunctionArg>& aConstraint) -> VectorConstraint<FunctionArg>
{
    return aConstraint;
}

template <typename FunctionArg>
auto make_vector_constraint(const Constraint<FunctionArg>& aConstraint) -> VectorConstraint<FunctionArg>
{
    return VectorConstraint<FunctionArg>{
        aConstraint.mName, detail::make_vector_function<FunctionArg>(aConstraint.mConstraintFunction),
        aConstraint.mConstraintTarget, aConstraint.mLinear, aConstraint.mConstraintType};
}

}  // namespace plato::criteria::library

#endif
