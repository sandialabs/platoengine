#include <gtest/gtest.h>

#include "plato/input_parser/ComponentType.hpp"

namespace plato::input_parser::unittest
{
TEST(ComponentType, ComponentTypeFromIndex)
{
    EXPECT_EQ(component_type_from_index<0U>(), ComponentType::kObjective);
    EXPECT_EQ(component_type_from_index<1U>(), ComponentType::kConstraint);
    EXPECT_EQ(component_type_from_index<2U>(), ComponentType::kGeometry);
    EXPECT_EQ(component_type_from_index<3U>(), ComponentType::kFilter);
    EXPECT_EQ(component_type_from_index<4U>(), ComponentType::kProcessManager);
}
}  // namespace plato::input_parser::unittest
