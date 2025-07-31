#include <gtest/gtest.h>

#include "plato/components/ComponentTypeColors.hpp"
#include "plato/utilities/EnumIndexing.hpp"
#include "plato/utilities/MultidimensionalRange.hpp"

namespace plato::components::unittest
{
TEST(ComponentTypeColors, ColorsAreDistinct)
{
    // Check that all colors are different.
    // We don't necessarily care which color each component is, just that they are distinct.
    constexpr auto tNumberOfComponents = utilities::number_of_enumerates<ComponentType>();
    for (const auto& [tComponent1, tComponent2] :
         utilities::MultidimensionalRange{tNumberOfComponents, tNumberOfComponents})
    {
        if (tComponent1 != tComponent2)
        {
            EXPECT_NE(component_text_color(static_cast<ComponentType>(tComponent1)),
                      component_text_color(static_cast<ComponentType>(tComponent2)));
        }
    }
}
}  // namespace plato::components::unittest
