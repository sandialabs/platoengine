#ifndef PLATO_GEOMETRY_LIBRARY_GEOMETRYWITHFILTERVALIDATION
#define PLATO_GEOMETRY_LIBRARY_GEOMETRYWITHFILTERVALIDATION

#include <optional>
#include <string>

#include "plato/filter/library/FilterFactory.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/input_validation/ValidatedInputTypeWrapper.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::mesh
{
class Mesh;
}

namespace plato::geometry::library
{
using AdaptedFilterFunction =
    core::Function<const linear_algebra::DynamicVector<double>&,
                   core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFunction>,
                   core::FunctionInfo<linear_algebra::JacobianMultiplier, core::evaluation::kFirstDerivative>,
                   core::FunctionInfo<linear_algebra::AdjointJacobianMultiplier,
                                      core::evaluation::kFirstDerivative,
                                      core::MatrixOrdering::kAdjoint>>;

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

/// @brief Constructs a FilterFunction from a geometry component's filter cross-link.
/// @tparam Geometry Must be an input block with fields `filter`.
template <typename InputBlockType>
[[nodiscard]] auto make_filter_from_geometry_input(
    const input_validation::ValidatedInputDataBlock<components::ComponentType::kGeometry>& aInput)
    -> filter::library::FilterFunction;

/// @brief Creates a filter function that takes a DynamicVector as input and returns a DynamicVector as output.
///
/// The purpose of this function is to adapt the normal filter function so that it can be used as a pre-processing step
/// before a geometry component.
[[nodiscard]] auto adapt_filter(const filter::library::FilterFunction& aFilterFunction, const mesh::Mesh& aMesh)
    -> AdaptedFilterFunction;

template <typename Geometry>
auto filter_is_cross_linked(const Geometry& aInput) -> bool
{
    return aInput.filter.has_value() && aInput.filter.value().mInputBlock.hasValue();
}

template <typename Geometry, typename MeshFieldAccessor>
auto validate_filter_with_mesh(const Geometry& aInput, const MeshFieldAccessor& aMeshFieldAccessor)
    -> std::optional<std::string>
{
    if (!aInput.filter || !filter_is_cross_linked(aInput) || !aMeshFieldAccessor(aInput).has_value())
    {
        return std::nullopt;
    }
    const auto tMeshPath = std::filesystem::path{aInput.mesh_name.value().mToken};
    const auto tCurrentMessageList = input_validation::validate(aInput.filter->mInputBlock, {}, tMeshPath);
    if (!tCurrentMessageList.empty())
    {
        return utilities::concatenate_container(tCurrentMessageList, "\n");
    }
    return std::nullopt;
}

template <typename InputBlockType>
auto make_filter_from_geometry_input(
    const input_validation::ValidatedInputDataBlock<components::ComponentType::kGeometry>& aInput)
    -> filter::library::FilterFunction
{
    const auto tFilterInput = input_validation::validated_cross_reference<components::ComponentType::kFilter>(
        aInput, [](const InputBlockType& aRawInput) { return aRawInput.filter; });
    return filter::library::make_filter_function(tFilterInput);
}

}  // namespace plato::geometry::library

#endif
