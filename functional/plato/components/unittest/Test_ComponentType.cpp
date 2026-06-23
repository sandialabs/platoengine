#include <gtest/gtest.h>

#include <vector>

#include "plato/components/ComponentType.hpp"

namespace plato::components::unittest
{
TEST(ComponentType, ComponentTypeFromIndex)
{
    EXPECT_EQ(component_type_from_index<0U>(), ComponentType::kGeometry);
    EXPECT_EQ(component_type_from_index<1U>(), ComponentType::kFilter);
    EXPECT_EQ(component_type_from_index<2U>(), ComponentType::kObjective);
    EXPECT_EQ(component_type_from_index<3U>(), ComponentType::kConstraint);
    EXPECT_EQ(component_type_from_index<4U>(), ComponentType::kProcessManager);
}

TEST(ComponentType, AllComponentTypes)
{
    const auto tAllComponentTypes = all_component_types();
    const auto tExpected = std::vector{ComponentType::kGeometry, ComponentType::kFilter, ComponentType::kObjective,
                                       ComponentType::kConstraint, ComponentType::kProcessManager};
    EXPECT_EQ(tExpected, tAllComponentTypes);
}
}  // namespace plato::components::unittest
