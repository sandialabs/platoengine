#ifndef PLATO_GEOMETRY_LIBRARY_GEOMETRYWITHFILTERVALIDATION
#define PLATO_GEOMETRY_LIBRARY_GEOMETRYWITHFILTERVALIDATION

#include <optional>
#include <string>

#include "plato/core/ValidationUtilities.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::geometry::library
{
/// @brief This function should be called in a geometry component that also uses a filter. It will call all mesh-based
/// validation functions that a filter may implement.
/// @tparam Geometry Must be an input block with fields `filter`.
/// @tparam MeshFieldAccessor A function that takes a Geometry argument and returns a `boost::optional` representing the
/// mesh field that holds the file name for the mesh.
template <typename Geometry, typename MeshFieldAccessor>
[[nodiscard]] auto validate_filter_with_mesh(const Geometry& aInput, const MeshFieldAccessor& aMeshFieldAccessor)
    -> std::optional<std::string>;

/// @brief Returns true if the input struct @a aInput has a field named `filter`, and it has been cross-linked with the
/// filter input using make_cross_linked_input.
template <typename Geometry>
[[nodiscard]] auto filter_is_cross_linked(const Geometry& aInput) -> bool;

template <typename Geometry>
auto filter_is_cross_linked(const Geometry& aInput) -> bool
{
    return aInput.filter->mInputBlock.template holds_expected_type<plato::filter::library::FilterInput>();
}

template <typename Geometry, typename MeshFieldAccessor>
auto validate_filter_with_mesh(const Geometry& aInput, const MeshFieldAccessor& aMeshFieldAccessor)
    -> std::optional<std::string>
{
    if (!aInput.filter || !filter_is_cross_linked(aInput) || !aMeshFieldAccessor(aInput).has_value())
    {
        return std::nullopt;
    }

    std::vector<std::string> tCurrentMessageList{};
    tCurrentMessageList = std::visit(
        [&aInput, tList = std::move(tCurrentMessageList)](const auto& aVariant) mutable -> std::vector<std::string>
        {  // NOLINTNEXTLINE
            return core::validate(aVariant, std::move(tList), std::filesystem::path{aInput.mesh_name.value().mToken});
        },
        library::get_cross_referenced_filter<plato::filter::library::FilterInput>(aInput));

    if (!tCurrentMessageList.empty())
    {
        return utilities::concatenate_container(tCurrentMessageList, "\n");
    }
    return std::nullopt;
}

}  // namespace plato::geometry::library

#endif
