#ifndef PLATO_CORE_UNITTEST_TESTHELPERS
#define PLATO_CORE_UNITTEST_TESTHELPERS

#include <gtest/gtest.h>

#include "plato/core/Aggregate.hpp"
#include "plato/core/Function.hpp"
#include "plato/core/test_utilities/Utilities.hpp"

namespace plato::core::unittest
{
using VectorFInfo = FunctionInfo<plato::test_utilities::TwoDVector, evaluation::kFunction>;
using VectorFirstDerivativeInfo = FunctionInfo<plato::test_utilities::TwoDMatrix, evaluation::kFirstDerivative>;
using VectorFirstDerivativeAdjointInfo =
    FunctionInfo<plato::test_utilities::TwoDMatrix, evaluation::kFirstDerivative, MatrixOrdering::kAdjoint>;
using VectorFunction = Function<plato::test_utilities::TwoDVector,
                                VectorFInfo,
                                VectorFirstDerivativeInfo,
                                VectorFirstDerivativeAdjointInfo>;

namespace detail
{
using RosenbrockF =
    std::decay_t<decltype(test_utilities::make_rosenbrock_function(plato::test_utilities::Rosenbrock{}))>;
using FunctionAndWeight = std::vector<std::pair<RosenbrockF, double>>;
}  // namespace detail

template <typename F>
void run_rosenbrock_aggregator_test(const F& aGenerateCommand)
{
    namespace pft = plato::test_utilities;
    const std::pair<double, double> tRosenbrockParams{2.0, 3.0};
    const std::pair<double, double> tWeights{.25, .5};
    const auto tF1 = test_utilities::make_rosenbrock_function(pft::Rosenbrock{});
    const auto tF2 =
        test_utilities::make_rosenbrock_function(pft::Rosenbrock{tRosenbrockParams.first, tRosenbrockParams.second});

    const auto tFunctionAndWeight =
        detail::FunctionAndWeight{std::make_pair(tF1, tWeights.first), std::make_pair(tF2, tWeights.second)};
    const auto tAggregate = aGenerateCommand(tFunctionAndWeight);

    for (auto aTestPoint : {pft::TwoDVector{0.0, 1.0}, pft::TwoDVector{1.0, 1.0}, pft::TwoDVector{-1.0, 2.0}})
    {
        EXPECT_EQ(tAggregate.template evaluate<evaluation::kFunction>(aTestPoint),
                  tWeights.first * tF1.evaluate<evaluation::kFunction>(aTestPoint) +
                      tWeights.second * tF2.evaluate<evaluation::kFunction>(aTestPoint));
        EXPECT_EQ(tAggregate.template evaluate<evaluation::kFirstDerivative>(aTestPoint),
                  tWeights.first * tF1.evaluate<evaluation::kFirstDerivative>(aTestPoint) +
                      tWeights.second * tF2.evaluate<evaluation::kFirstDerivative>(aTestPoint));
    }
}

}  // namespace plato::core::unittest

#endif
