#ifndef PLATO_CRITERIA_LIBRARY_CONSTRAINTADAPTOR
#define PLATO_CRITERIA_LIBRARY_CONSTRAINTADAPTOR

#include "plato/core/Function.hpp"
#include "plato/criteria/library/ConstraintFactory.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::criteria::library
{

template <typename FunctionArg>
using ScalarFunction = core::Function<double, linear_algebra::DynamicVector<double>, FunctionArg>;

template <typename FunctionArg>
using VectorFunction =
    core::Function<linear_algebra::DynamicVector<double>, linear_algebra::JacobianMultiplier, FunctionArg>;

namespace detail
{

template <typename FunctionArg>
auto make_vector_function(const VectorFunction<FunctionArg>& aVectorFunction) -> VectorFunction<FunctionArg>
{
    std::cout << "relay" << std::endl;
    return aVectorFunction;
}

template <typename FunctionArg>
auto make_vector_function(const ScalarFunction<FunctionArg>& aScalarFunction) -> VectorFunction<FunctionArg>
{
    std::cout << "no relay" << std::endl;
    return VectorFunction<FunctionArg>{
        [aScalarFunction](const auto& aFunctionArg)
        { return linear_algebra::DynamicVector<double>{aScalarFunction.f(aFunctionArg)}; },
        [aScalarFunction](const auto& aFunctionArg)
        {
            constexpr unsigned int tNumberOfColumns = 1;
            const linear_algebra::DynamicVector<double> tDf = aScalarFunction.df(aFunctionArg);
            const linear_algebra::JacobianMultiplier::JacobianTimesVectorFunction tFunction =
                [tDf](const linear_algebra::DynamicVector<double>& aV)
            { return linear_algebra::DynamicVector<double>{tDf.dot(aV)}; };
            return linear_algebra::JacobianMultiplier{tNumberOfColumns, tFunction};
        }};
}

template <typename FunctionArg>
auto make_adjoint_jacobian_vector_function(const ScalarFunction<FunctionArg>& aScalarFunction)
    -> VectorFunction<FunctionArg>
{
    return VectorFunction<FunctionArg>{
        [aScalarFunction](const auto& aFunctionArg)
        { return linear_algebra::DynamicVector<double>{aScalarFunction.f(aFunctionArg)}; },
        [aScalarFunction](const auto& aFunctionArg)
        {
            constexpr unsigned int tNumberOfColumns = 1;
            const linear_algebra::DynamicVector<double> tDf = aScalarFunction.df(aFunctionArg);
            const linear_algebra::JacobianMultiplier::JacobianTimesVectorFunction tFunction =
                [tDf](const linear_algebra::DynamicVector<double>& aV)
            {
                const auto tEntry = aV.stdVector()[0];
                return linear_algebra::DynamicVector<double>{tDf * tEntry};
            };
            return linear_algebra::JacobianMultiplier{tNumberOfColumns, tFunction};
        }};
}

}  // namespace detail

template <typename FunctionArg>
auto make_vector_constraint(const VectorConstraint<FunctionArg>& aConstraint) -> VectorConstraint<FunctionArg>
{
    std::cout << "relay" << std::endl;
    return aConstraint;
}

template <typename FunctionArg>
auto make_vector_constraint(const Constraint<FunctionArg>& aConstraint) -> VectorConstraint<FunctionArg>
{
    std::cout << "not relay" << std::endl;
    constexpr unsigned int tNumberOfConstraints = 1;
    return VectorConstraint<FunctionArg>{aConstraint.mName,
                                         detail::make_vector_function(aConstraint.mConstraintFunction),
                                         detail::make_adjoint_jacobian_vector_function(aConstraint.mConstraintFunction),
                                         aConstraint.mConstraintTarget,
                                         tNumberOfConstraints,
                                         aConstraint.mLinear,
                                         aConstraint.mConstraintType};
}

}  // namespace plato::criteria::library

#endif
