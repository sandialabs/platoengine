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
using ScalarFunction =
    core::Function<FunctionArg,
                   core::FunctionInfo<double, core::evaluation::kFunction>,
                   core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFirstDerivative>>;

template <typename FunctionArg>
using VectorFunction = typename VectorConstraint<FunctionArg>::ConstraintFunction;

/// @brief Constructs a VectorFunction from a ScalarFunction object.
///
/// The purpose of this function is to adapt a scalar constraint function so that it can be used as a vector constraint
/// function.
template <typename FunctionArg>
[[nodiscard]] auto to_vector_function(const ScalarFunction<FunctionArg>& aScalarFunction)
    -> VectorFunction<FunctionArg>;

template <typename FunctionArg>
auto to_vector_function(const ScalarFunction<FunctionArg>& aScalarFunction) -> VectorFunction<FunctionArg>
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
}  // namespace plato::criteria::library

#endif
