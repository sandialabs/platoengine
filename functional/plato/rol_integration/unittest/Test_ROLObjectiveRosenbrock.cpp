#include <gtest/gtest.h>

#include <ROL_StdVector.hpp>
#include <memory>

#include "plato/core/Aggregate.hpp"
#include "plato/rol_integration/ROLObjectiveFunction.hpp"
#include "plato/rol_integration/unittest/DynamicVectorRosenbrockFunction.hpp"
#include "plato/test_utilities/Rosenbrock.hpp"

namespace plato::rol_integration::unittest
{
TEST(ROLObjectiveFunction, RosenbrockObjectiveValueAndGradient)
{
    namespace pft = plato::test_utilities;

    auto tObjective = ROLObjectiveFunction{make_rosenbrock_dynamic_vector_function(pft::Rosenbrock{})};
    const auto tControl = ROL::StdVector<double>{1.0, 1.0};
    double tTolerance;
    EXPECT_EQ(tObjective.value(tControl, tTolerance), 0.0);

    auto tGradient = ROL::StdVector<double>{0.0, 0.0};
    tObjective.gradient(tGradient, tControl, tTolerance);
    EXPECT_EQ(tGradient[0], 0.0);
    EXPECT_EQ(tGradient[1], 0.0);
}

TEST(ROLObjectiveFunction, AggregateTwoRosenbrockObjectives)
{
    namespace pft = plato::test_utilities;

    auto tRosenbrockFunction = make_rosenbrock_dynamic_vector_function(pft::Rosenbrock{});
    using RosenbrockF = std::decay_t<decltype(tRosenbrockFunction)>;
    auto tObjective = ROLObjectiveFunction{core::make_aggregate_function(
        std::vector<std::pair<RosenbrockF, double>>{std::make_pair(tRosenbrockFunction, 1.0)})};
    const auto tControl = ROL::StdVector<double>{1.0, 1.0};
    double tTolerance;
    EXPECT_EQ(tObjective.value(tControl, tTolerance), 0.0);

    auto tGradient = ROL::StdVector<double>{0.0, 0.0};
    tObjective.gradient(tGradient, tControl, tTolerance);
    EXPECT_EQ(tGradient[0], 0.0);
    EXPECT_EQ(tGradient[1], 0.0);
}
}  // namespace plato::rol_integration::unittest
