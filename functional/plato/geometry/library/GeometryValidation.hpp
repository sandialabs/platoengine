#ifndef PLATO_GEOMETRY_LIBRARY_GEOMETRYVALIDATION
#define PLATO_GEOMETRY_LIBRARY_GEOMETRYVALIDATION

#include <filesystem>
#include <optional>

#include "plato/core/ValidationUtilities.hpp"
#include "plato/input_parser/InputBlocks.hpp"

namespace plato::geometry::library
{
/// @brief Performs full validation of the geometry entries in @a aInput and appends any errors to @a
/// aCurrentMessageList.
[[nodiscard]] std::vector<std::string> validate_geometry(const input_parser::ParsedInput& aInput,
                                                         std::vector<std::string>&& aCurrentMessageList);

namespace detail
{
/// @brief Checks that only one geometry block is defined in @a aInput.
[[nodiscard]] std::optional<std::string> validate_only_one_geometry(const input_parser::ParsedInput& aInput);

/// @brief Checks that the `mesh_name` field has a value.
template <typename Geometry>
[[nodiscard]] std::optional<std::string> validate_mesh_name(const Geometry& aInput)
{
    return core::error_message_for_empty_parameter(input_parser::block_name<Geometry>(), aInput.mesh_name, "mesh_name");
}

/// @brief Checks that the `output_name` field has a value.
template <typename Geometry>
[[nodiscard]] std::optional<std::string> validate_output_name(const Geometry& aInput)
{
    return core::error_message_for_empty_parameter(input_parser::block_name<Geometry>(), aInput.output_name,
                                                   "output_name");
}

/// @brief Checks that the `mesh_name` field points to an existing file, if it has a value.
template <typename Geometry>
[[nodiscard]] std::optional<std::string> validate_mesh_file_exists(const Geometry& aInput)
{
    const auto& tMeshFileName = aInput.mesh_name;
    if (tMeshFileName.has_value() && !std::filesystem::exists(tMeshFileName.value().mToken))
    {
        return input_parser::block_name<Geometry>() +
               ": The mesh_name entry does not refer to a valid input file. The entered path is " +
               tMeshFileName.value().mToken;
    }
    return std::nullopt;
}

}  // namespace detail

}  // namespace plato::geometry::library

#endif
