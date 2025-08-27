#ifndef PLATO_COMPONENTS_COMPONENTTYPE
#define PLATO_COMPONENTS_COMPONENTTYPE

#include "plato/utilities/EnumIndexing.hpp"

namespace plato::components
{
/// @brief The allowable types of components. These map to general categories of objects that may be instantiated to
/// create a workflow.
enum struct ComponentType
{
    kObjective,
    kConstraint,
    kGeometry,
    kFilter,
    kProcessManager,
    kNumberOfEnumerates
};

/// @brief Returns the ComponentType associated with an index @a kIndex.
template <std::size_t kIndex>
[[nodiscard]] constexpr auto component_type_from_index() -> ComponentType;

template <std::size_t kIndex>
constexpr auto component_type_from_index() -> ComponentType
{
    static_assert(kIndex < utilities::number_of_enumerates<ComponentType>(),
                  "Index used to instantiate component_type_from_index must be less than the total number of "
                  "enumerates in ComponentType.");
    return std::get<0>(utilities::enums_from_index<ComponentType>(kIndex));
}

}  // namespace plato::components

#endif
