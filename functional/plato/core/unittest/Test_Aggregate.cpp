#include <gtest/gtest.h>

#include "plato/core/Aggregate.hpp"
#include "plato/core/test_utilities/Utilities.hpp"
#include "plato/test_utilities/Rosenbrock.hpp"

namespace plato::core::unittest
{
TEST(Aggregate, DirectConstruction)
{
    namespace pft = plato::test_utilities;

    const auto tF1 = test_utilities::make_rosenbrock_function(pft::Rosenbrock{});
    const auto tF2 = test_utilities::make_rosenbrock_function(pft::Rosenbrock{});
    const double tW1 = 0.5;
    const double tW2 = 0.5;
    using RosenbrockF = std::decay_t<decltype(tF1)>;
    using FunctionAndWeight = std::vector<std::pair<RosenbrockF, double>>;
    const auto tAggregate = Aggregate<double, pft::TwoDVector, const pft::TwoDVector&>(
        FunctionAndWeight{std::make_pair(tF1, tW1), std::make_pair(tF2, tW2)});
    {
        const auto tArg = pft::TwoDVector{1.0, 1.0};
        EXPECT_EQ(tAggregate.f(tArg), 0.0);
        const pft::TwoDVector tDfdx = tAggregate.df(tArg);
        EXPECT_EQ(tDfdx(0), 0.0);
        EXPECT_EQ(tDfdx(1), 0.0);
    }
    {
        const auto tArg = pft::TwoDVector{2.0, -1.0};
        const double tExpectedF = tW1 * tF1.f(tArg) + tW2 * tF2.f(tArg);
        EXPECT_EQ(tAggregate.f(tArg), tExpectedF);
        const pft::TwoDVector tExpectedDF = tW1 * tF1.df(tArg) + tW2 * tF2.df(tArg);
        EXPECT_EQ(tAggregate.df(tArg), tExpectedDF);
    }
}

TEST(Aggregate, UsingMakeFunction)
{
    namespace pft = plato::test_utilities;

    const auto tF1 = test_utilities::make_rosenbrock_function(pft::Rosenbrock{});
    const auto tF2 = test_utilities::make_rosenbrock_function(pft::Rosenbrock{});
    const double tW1 = 2.0;
    const double tW2 = 3.0;
    using RosenbrockF = std::decay_t<decltype(tF1)>;
    using FunctionAndWeight = std::vector<std::pair<RosenbrockF, double>>;
    const auto tAggregate =
        make_aggregate_function(FunctionAndWeight{std::make_pair(tF1, tW1), std::make_pair(tF2, tW2)});
    {
        const auto tArg = pft::TwoDVector{1.0, 1.0};
        constexpr double tExpected = 0.0;
        EXPECT_EQ(tAggregate.f(tArg), tExpected);
        const pft::TwoDVector tDfdx = tAggregate.df(tArg);
        EXPECT_EQ(tDfdx(0), tExpected);
        EXPECT_EQ(tDfdx(1), tExpected);
    }
    {
        const auto tArg = pft::TwoDVector{-2.0, 1.0};
        const double tExpectedF = tW1 * tF1.f(tArg) + tW2 * tF2.f(tArg);
        EXPECT_EQ(tAggregate.f(tArg), tExpectedF);
        const pft::TwoDVector tExpectedDF = tW1 * tF1.df(tArg) + tW2 * tF2.df(tArg);
        EXPECT_EQ(tAggregate.df(tArg), tExpectedDF);
    }
}

TEST(Aggregate, FunctionDifferentParameters)
{
    namespace pft = plato::test_utilities;

    const auto tF1 = test_utilities::make_rosenbrock_function(pft::Rosenbrock{});
    const auto tF2 = test_utilities::make_rosenbrock_function(pft::Rosenbrock{2.0, 200.0});
    const double tW1 = 2.0;
    const double tW2 = 3.0;
    using RosenbrockF = std::decay_t<decltype(tF1)>;
    using FunctionAndWeight = std::vector<std::pair<RosenbrockF, double>>;
    const auto tAggregate = Aggregate<double, pft::TwoDVector, const pft::TwoDVector&>(
        FunctionAndWeight{std::make_pair(tF1, tW1), std::make_pair(tF2, tW2)});
    const auto tArg = pft::TwoDVector{-2.0, 1.0};
    const double tExpectedF = tW1 * tF1.f(tArg) + tW2 * tF2.f(tArg);
    EXPECT_NE(tF1.f(tArg), tF2.f(tArg));
    EXPECT_EQ(tAggregate.f(tArg), tExpectedF);
    const pft::TwoDVector tExpectedDF = tW1 * tF1.df(tArg) + tW2 * tF2.df(tArg);
    EXPECT_EQ(tAggregate.df(tArg), tExpectedDF);
}

TEST(Aggregate, OneFunction)
{
    namespace pft = plato::test_utilities;

    const auto tF1 = test_utilities::make_rosenbrock_function(pft::Rosenbrock{});
    const double tW1 = 1.0;
    using RosenbrockF = std::decay_t<decltype(tF1)>;
    using FunctionAndWeight = std::vector<std::pair<RosenbrockF, double>>;
    const auto tAggregate =
        Aggregate<double, pft::TwoDVector, const pft::TwoDVector&>(FunctionAndWeight{std::make_pair(tF1, tW1)});
    {
        const auto tArg = pft::TwoDVector{1.0, 1.0};
        EXPECT_EQ(tAggregate.f(tArg), 0.0);
        const pft::TwoDVector tDfdx = tAggregate.df(tArg);
        EXPECT_EQ(tDfdx(0), 0.0);
        EXPECT_EQ(tDfdx(1), 0.0);
    }
}
}  // namespace plato::core::unittest
