#include <gtest/gtest.h>

#include "plato/components/ComponentType.hpp"
#include "plato/components/Documentation.hpp"
#include "plato/utilities/EnumIndexing.hpp"

namespace plato::components::unittest
{
TEST(Documentation, AllExist)
{
    constexpr auto tNumberOfComponents = utilities::number_of_enumerates<components::ComponentType>();
    const auto tDocumentation = []<std::size_t... kIndices>(std::index_sequence<kIndices...>)
    {
        auto tAllDocumentation = std::string{};
        (tAllDocumentation += ... +=
         std::string{ComponentTypeDocumentation<component_type_from_index<kIndices>()>::mDocumentation});
        return tAllDocumentation;
    }(std::make_index_sequence<tNumberOfComponents>());
}
}  // namespace plato::components::unittest
