#ifndef PLATO_GEOMETRY_EXTENSION_MESHVALIDATIONUTILITIES
#define PLATO_GEOMETRY_EXTENSION_MESHVALIDATIONUTILITIES

#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/FixedBlockUtilities.hpp"
#include "plato/mesh/Mesh.hpp"

namespace plato::geometry::extension
{

/// @brief Creates a Mesh from an Geometry input block.
/// @pre The mesh_name field in @a aInput has a value. Checked with an assertion.
template <typename Geometry>
[[nodiscard]] auto mesh_from_input(const Geometry& aInput) -> mesh::Mesh;

/// @brief Retrieve the initial field from a mesh in @a aInput
/// @pre The mesh_name in @a aInput has already been checked during validation and it does exist and
/// that the field 'aInput.initial_field_name' exists on the mesh
/// @post The returned vector size will match the number nodal design variables
template <typename Geometry>
[[nodiscard]] auto initial_field_from_mesh(const Geometry& aInput) -> std::vector<double>;

/// @brief Retrieve the initial field from a mesh in @a aInput and read it from the mesh @a aMesh
/// @pre The mesh_name in @a aInput has already been checked during validation and it does exist and
/// that the field 'aInput.initial_field_name' exists on the mesh
/// @post The returned vector size will match the number nodal design variables
template <typename Geometry>
[[nodiscard]] auto initial_field_from_mesh(const Geometry& aInput, const mesh::Mesh& aMesh) -> std::vector<double>;

/// @brief Create an error message from the fields in the mesh in @a aInput
/// @pre The mesh_name in @a aInput has already been checked during validation and it does exist
template <typename Geometry>
[[nodiscard]] auto mesh_field_names_for_error_message(const Geometry& aInput) -> std::string;

/// @brief Validates that the `initial_field_name` field in @a aInput has a value or that the mesh in
/// `mesh_name` has a Topology field to be read.
template <typename Geometry>
[[nodiscard]] std::optional<std::string> validate_initial_field_source(const Geometry& aInput);

template <typename Geometry>
[[nodiscard]] auto mesh_from_input(const Geometry& aInput) -> mesh::Mesh
{
    assert(aInput.mesh_name.has_value());
    return mesh::Mesh{aInput.mesh_name.value().mToken, mesh::fixed_blocks(aInput)};
}

template <typename Geometry>
[[nodiscard]] auto initial_field_from_mesh(const Geometry& aInput) -> std::vector<double>
{
    return initial_field_from_mesh(aInput, mesh_from_input(aInput));
}

template <typename Geometry>
[[nodiscard]] auto initial_field_from_mesh(const Geometry& aInput, const mesh::Mesh& aMesh) -> std::vector<double>
{
    assert(aInput.initial_field_name.has_value());
    const auto tFieldName = aInput.initial_field_name.value().mToken;
    return mesh::EntityRetrieval{aMesh}.designDomainNodalField(tFieldName);
}

template <typename Geometry>
[[nodiscard]] auto mesh_field_names_for_error_message(const Geometry& aInput) -> std::string
{
    const auto tMesh = mesh_from_input(aInput);
    const auto tNodalFields = mesh::EntityRetrieval{tMesh}.nodalFields();

    return utilities::concatenate("Field name must be one of the following defined on the nodes: ",
                                  utilities::concatenate_container(tNodalFields, ", "), ".");
}

template <typename Geometry>
[[nodiscard]] std::optional<std::string> validate_initial_field_source(const Geometry& aInput)
{
    if (aInput.initial_field_name.has_value() && aInput.mesh_name.has_value() &&
        std::filesystem::exists(aInput.mesh_name.value().mToken))
    {
        const auto tFieldName = aInput.initial_field_name.value().mToken;
        const auto tFileName = aInput.mesh_name.value().mToken;
        const auto tNodalFieldExists =
            mesh::EntityCounts{mesh_from_input(aInput)}.hasNodalFieldVariable(std::string{tFieldName});

        if (!tNodalFieldExists)
        {
            return utilities::concatenate(input_parser::block_name<Geometry>(), ": The mesh  ", tFileName,
                                          " does not have a nodal field called '", tFieldName, "'.",
                                          mesh_field_names_for_error_message(aInput));
        }
    }
    return std::nullopt;
}

}  // namespace plato::geometry::extension

#endif
