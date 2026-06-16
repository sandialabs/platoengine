#ifndef PLATO_COMPONENTS_DOCUMENTATION
#define PLATO_COMPONENTS_DOCUMENTATION

#include <string_view>

#include "plato/components/ComponentType.hpp"

namespace plato::components
{
/// @brief A base template for documenting component types.
///
/// This must be specialized for all ComponentType instantiations.
template <ComponentType kComponentType>
struct ComponentTypeDocumentation
{
};

template <>
struct ComponentTypeDocumentation<ComponentType::kObjective>
{
    static constexpr auto mDocumentation = std::string_view{"An objective is important!"};
};

template <>
struct ComponentTypeDocumentation<ComponentType::kConstraint>
{
    static constexpr auto mDocumentation = std::string_view{"A constraint is maybe important?"};
};

template <>
struct ComponentTypeDocumentation<ComponentType::kGeometry>
{
    static constexpr auto mDocumentation = std::string_view{"A geometry is important!"};
};

template <>
struct ComponentTypeDocumentation<ComponentType::kFilter>
{
    static constexpr auto mDocumentation = std::string_view{"A filter is maybe important?"};
};

template <>
struct ComponentTypeDocumentation<ComponentType::kProcessManager>
{
    static constexpr auto mDocumentation = std::string_view{"A process manager is maybe important?"};
};

}  // namespace plato::components

#endif
