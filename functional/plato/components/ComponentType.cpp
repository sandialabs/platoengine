#include "plato/components/ComponentType.hpp"

#include <cstddef>
#include <utility>
#include <vector>

#include "plato/utilities/EnumIndexing.hpp"

namespace plato::components
{
auto all_component_types() -> std::vector<ComponentType>
{
    auto tComponentTypes = std::vector<ComponentType>{};
    [&tComponentTypes]<std::size_t... kIndices>(std::index_sequence<kIndices...>) {
        (tComponentTypes.push_back(component_type_from_index<kIndices>()), ...);
    }(std::make_index_sequence<utilities::number_of_enumerates<ComponentType>()>());
    return tComponentTypes;
}
}  // namespace plato::components
