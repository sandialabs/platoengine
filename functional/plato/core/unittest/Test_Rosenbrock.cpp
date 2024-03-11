#include <gtest/gtest.h>

#include "plato/core/Compose.hpp"
#include "plato/core/test_utilities/Utilities.hpp"
#include "plato/test_utilities/Penalty.hpp"
#include "plato/test_utilities/Rosenbrock.hpp"

namespace plato::core::unittest
{
TEST(Rosenbrock, Rosenbrock)
{
    namespace pft = plato::test_utilities;

    const auto tRosenbrock = pft::Rosenbrock{1.0, 100.0};
    EXPECT_EQ(tRosenbrock.f(1.0, 1.0), 0.0);
    EXPECT_EQ(tRosenbrock.df(1.0, 1.0)(0), 0.0);
    EXPECT_EQ(tRosenbrock.df(1.0, 1.0)(1), 0.0);
}

TEST(Rosenbrock, MakeRosenbrock)
{
    namespace pft = plato::test_utilities;

    constexpr double tA = 2.0;
    constexpr double tB = 50.0;
    const auto tRosenbrock = pft::Rosenbrock{tA, tB};
    const auto tRosenbrockFunction = test_utilities::make_rosenbrock_function(tRosenbrock);

    {
        const auto tX = pft::TwoDVector{0.5, 10.0};
        EXPECT_EQ(tRosenbrock.f(tX(0), tX(1)), tRosenbrockFunction.f(tX));
        EXPECT_EQ(tRosenbrock.df(tX(0), tX(1)), tRosenbrockFunction.df(tX));
    }
    {
        const auto tX = pft::TwoDVector{-0.5, 5.0};
        EXPECT_EQ(tRosenbrock.f(tX(0), tX(1)), tRosenbrockFunction.f(tX));
        EXPECT_EQ(tRosenbrock.df(tX(0), tX(1)), tRosenbrockFunction.df(tX));
    }
}

TEST(Rosenbrock, RosenbrockAndPenalty)
{
    namespace pft = plato::test_utilities;

    const auto tRosenbrock = pft::Rosenbrock{1.0, 100.0};
    const auto tPenalty = pft::Penalty{0.5 - 3, 3.0};

    const auto tH = core::compose(test_utilities::make_rosenbrock_function(tRosenbrock),
                                  test_utilities::make_penalty_function(tPenalty));

    const auto tX = pft::TwoDVector{1.0, 1.0};
    EXPECT_EQ(tH.f(tX), 0.0);
    EXPECT_EQ(tH.df(tX)(0), 0.0);
    EXPECT_EQ(tH.df(tX)(1), 0.0);
}
}  // namespace plato::core::unittest
