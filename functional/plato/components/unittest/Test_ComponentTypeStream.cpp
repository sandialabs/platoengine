#include <gtest/gtest.h>

#include "plato/components/ComponentType.hpp"
#include "plato/components/ComponentTypeStream.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::components::unittest
{
TEST(ComponentAttribute, StreamInsertion)
{
    const auto tExpectStreamMatches = [](const ComponentType aComponentType, const std::string_view aExpected,
                                         const plato::test_utilities::TestContext& aTestContext)
    {
        auto tStream = std::stringstream{};
        tStream << aComponentType;
        EXPECT_EQ(tStream.view(), aExpected) << aTestContext;
    };

    tExpectStreamMatches(ComponentType::kConstraint, "constraint", TEST_CONTEXT("Constraint"));
    tExpectStreamMatches(ComponentType::kFilter, "filter", TEST_CONTEXT("Filter"));
    tExpectStreamMatches(ComponentType::kGeometry, "geometry", TEST_CONTEXT("Geometry"));
    tExpectStreamMatches(ComponentType::kObjective, "objective", TEST_CONTEXT("Objective"));
    tExpectStreamMatches(ComponentType::kProcessManager, "process manager", TEST_CONTEXT("Process manager"));
}

TEST(ComponentAttribute, ToString)
{
    EXPECT_EQ(to_string(ComponentType::kConstraint), "constraint");
    EXPECT_EQ(to_string(ComponentType::kFilter), "filter");
    EXPECT_EQ(to_string(ComponentType::kGeometry), "geometry");
    EXPECT_EQ(to_string(ComponentType::kObjective), "objective");
    EXPECT_EQ(to_string(ComponentType::kProcessManager), "process manager");
}

}  // namespace plato::components::unittest
