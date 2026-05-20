#include "plato/integration_tests/utilities/DynamicVectorSumConstraintUtilities.hpp"

#include <utility>
#include <vector>

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/test_utilities/SumConstraint.hpp"
#include "plato/test_utilities/TwoDTestTypesToDynamicVector.hpp"

namespace plato::integration_tests::utilities
{
namespace
{
auto make_adjoint_jacobian_multiplier(const linear_algebra::DynamicVector<double>& aVector,
                                      const std::vector<test_utilities::SumConstraint>& aConstraints)
    -> linear_algebra::AdjointJacobianMultiplier
{
    const auto tDerivativeFunctionEvaluationZero =
        test_utilities::to_dynamic_vector(aConstraints[0].df(aVector[0], aVector[1]));
    const auto tDerivativeFunctionEvaluationOne =
        test_utilities::to_dynamic_vector(aConstraints[1].df(aVector[0], aVector[1]));

    auto tVectorTimesAdjointJacobianFunction = linear_algebra::JacobianMultiplier::VectorTimesJacobianFunction{
        [tDerivativeFunctionEvaluationZero,
         tDerivativeFunctionEvaluationOne](const linear_algebra::DynamicVector<double>& aVector)
        {
            return linear_algebra::DynamicVector<double>{tDerivativeFunctionEvaluationZero.dot(aVector),
                                                         tDerivativeFunctionEvaluationOne.dot(aVector)};
        }};

    return linear_algebra::AdjointJacobianMultiplier{
        linear_algebra::JacobianMultiplier{std::move(tVectorTimesAdjointJacobianFunction)}};
}

auto make_jacobian_multiplier(const linear_algebra::DynamicVector<double>& aVector,
                              const std::vector<test_utilities::SumConstraint>& aConstraints)
    -> linear_algebra::JacobianMultiplier
{
    const auto tDerivativeFunctionEvaluationZero =
        test_utilities::to_dynamic_vector(aConstraints[0].df(aVector[0], aVector[1]));
    const auto tDerivativeFunctionEvaluationOne =
        test_utilities::to_dynamic_vector(aConstraints[1].df(aVector[0], aVector[1]));

    auto tVectorTimesJacobianFunction = linear_algebra::JacobianMultiplier::VectorTimesJacobianFunction{
        [tDerivativeFunctionEvaluationZero,
         tDerivativeFunctionEvaluationOne](const linear_algebra::DynamicVector<double>& aDual)
        {
            return linear_algebra::DynamicVector<double>{tDerivativeFunctionEvaluationZero * aDual[0] +
                                                         tDerivativeFunctionEvaluationOne * aDual[1]};
        }};

    return {std::move(tVectorTimesJacobianFunction)};
}
}  // namespace

auto make_line_and_circle_jacobian_function() -> LineAndCircleFunction
{
    const auto tLineFunction = test_utilities::SumConstraint{};
    const auto tCircleFunction = test_utilities::SumConstraint{{}, 2U};

    return LineAndCircleFunction{
        [tLineFunction, tCircleFunction](const linear_algebra::DynamicVector<double>& aVector)
        {
            return linear_algebra::DynamicVector<double>{tLineFunction.f(aVector[0], aVector[1]),
                                                         tCircleFunction.f(aVector[0], aVector[1])};
        },
        [tLineFunction, tCircleFunction](const linear_algebra::DynamicVector<double>& aVector)
        { return make_jacobian_multiplier(aVector, {tLineFunction, tCircleFunction}); },
        [tLineFunction, tCircleFunction](const linear_algebra::DynamicVector<double>& aVector)
        { return make_adjoint_jacobian_multiplier(aVector, {tLineFunction, tCircleFunction}); }};
}

}  // namespace plato::integration_tests::utilities
