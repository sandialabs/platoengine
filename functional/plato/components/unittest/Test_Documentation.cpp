#include <gtest/gtest.h>

#include "plato/components/ComponentType.hpp"
#include "plato/components/ComponentTypeStream.hpp"
#include "plato/components/Documentation.hpp"

namespace plato::components::unittest
{
TEST(Documentation, AllExist)
{
    for (const auto tComponentType : all_component_types())
    {
        const auto tDocumentation = component_documentation_brief(tComponentType);
        EXPECT_FALSE(tDocumentation.empty()) << to_string(tComponentType);
    }
}
}  // namespace plato::components::unittest
