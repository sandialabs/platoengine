#include <gtest/gtest.h>

#include "plato/core/Aggregate.hpp"
#include "plato/core/test_utilities/Utilities.hpp"
#include "plato/core/unittest/TestHelpers.hpp"
#include "plato/test_utilities/Rosenbrock.hpp"

namespace plato::core::unittest
{
namespace
{
using Domain = const plato::test_utilities::TwoDVector&;
using EvaluationInfo = FunctionInfo<double, evaluation::kFunction>;
using GradientInfo = FunctionInfo<plato::test_utilities::TwoDVector, evaluation::kFirstDerivative>;
}  // namespace

TEST(Aggregate, DirectConstruction)
{
    run_rosenbrock_aggregator_test([](const detail::FunctionAndWeight& tFunctionAndWeight)
                                   { return Aggregate<Domain, EvaluationInfo, GradientInfo>(tFunctionAndWeight); });
}

TEST(Aggregate, UsingMakeFunction)
{
    run_rosenbrock_aggregator_test([](const detail::FunctionAndWeight& tFunctionAndWeight)
                                   { return make_aggregate_function_with_first_derivative(tFunctionAndWeight); });
}

TEST(Aggregate, OneFunction)
{
    namespace pft = plato::test_utilities;

    const auto tF1 = test_utilities::make_rosenbrock_function(pft::Rosenbrock{});
    const double tW1 = 1.0;
    using RosenbrockF = std::decay_t<decltype(tF1)>;
    using FunctionAndWeight = std::vector<std::pair<RosenbrockF, double>>;
    const auto tAggregate =
        Aggregate<Domain, EvaluationInfo, GradientInfo>(FunctionAndWeight{std::make_pair(tF1, tW1)});
    {
        const auto tArg = pft::TwoDVector{1.0, 1.0};
        EXPECT_EQ(tAggregate.evaluate<evaluation::kFunction>(tArg), 0.0);
        const pft::TwoDVector tDfdx = tAggregate.evaluate<evaluation::kFirstDerivative>(tArg);
        EXPECT_EQ(tDfdx(0), 0.0);
        EXPECT_EQ(tDfdx(1), 0.0);
    }
}
}  // namespace plato::core::unittest
