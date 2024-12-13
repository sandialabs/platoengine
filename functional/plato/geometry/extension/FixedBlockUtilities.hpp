#ifndef PLATO_GEOMETRY_EXTENSION_FIXEDBLOCKUTILITIES
#define PLATO_GEOMETRY_EXTENSION_FIXEDBLOCKUTILITIES

#include <algorithm>
#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshBlocks.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::geometry::extension
{
/// @brief Converts the vector of fixed block names in @a aInput to a set.
///
/// A set is used since the list of fixed blocks must be unique. That the raw input is a unique list of names
/// can be validated with validate_unique_fixed_block_names.
template <typename InputBlock>
[[nodiscard]] auto fixed_blocks(const InputBlock& aInput) -> std::set<std::string>;

/// @brief Validates that all fixed block names in the input are unique.
template <typename InputBlock>
[[nodiscard]] auto validate_unique_fixed_block_names(const InputBlock& aInput) -> std::optional<std::string>;

/// @brief Validates that all fixed block names in @a aInput exist in the mesh.
template <typename InputBlock>
[[nodiscard]] auto validate_fixed_block_names_exist(const InputBlock& aInput) -> std::optional<std::string>;

/// @brief Validates that at least 1 block remains as the design domain
template <typename InputBlock>
[[nodiscard]] auto validate_at_least_one_design_block(const InputBlock& aInput) -> std::optional<std::string>;

namespace detail
{
template <typename InputBlock>
auto mesh_block_names(const InputBlock& aInput) -> std::vector<std::string>
{
    if (!aInput.mesh_name.has_value() || !std::filesystem::exists(aInput.mesh_name.value().mToken))
    {
        return {};
    }
    return mesh::MeshBlocks{mesh::Mesh{aInput.mesh_name.value().mToken}}.blockNames();
}

template <typename InputBlock>
auto mesh_block_names_for_error_message(const InputBlock& aInput) -> std::string
{
    if (const auto tBlockNames = mesh_block_names(aInput); !tBlockNames.empty())
    {
        auto tAllBlockNames = utilities::concatenate_container(tBlockNames, ", ");
        return utilities::concatenate(
            "fixed_block must be one or more of the following names found in the input mesh: ",
            std::move(tAllBlockNames));
    }
    return "No blocks found in the mesh, or mesh_file does not exist.";
}
}  // namespace detail

template <typename InputBlock>
auto fixed_blocks(const InputBlock& aInput) -> std::set<std::string>
{
    if (!aInput.fixed_blocks.has_value())
    {
        return {};
    }
    auto tUniqueFixedBlocks = std::set<std::string>{};
    const auto& tRawFixedBlockInput = aInput.fixed_blocks.value().mList;
    std::copy(tRawFixedBlockInput.cbegin(), tRawFixedBlockInput.cend(),
              std::inserter(tUniqueFixedBlocks, tUniqueFixedBlocks.begin()));
    return tUniqueFixedBlocks;
}

template <typename InputBlock>
auto validate_unique_fixed_block_names(const InputBlock& aInput) -> std::optional<std::string>
{
    if (!aInput.fixed_blocks.has_value())
    {
        return {};
    }

    const auto tUniqueFixedBlocks = fixed_blocks(aInput);
    if (tUniqueFixedBlocks.size() != aInput.fixed_blocks.value().mList.size())
    {
        auto tFixedBlockNames = utilities::concatenate_container(aInput.fixed_blocks.value().mList, ", ");
        auto tErrorMessage =
            utilities::concatenate("The fixed_block entries are not unique: ", std::move(tFixedBlockNames), ". ");
        return std::optional{std::move(tErrorMessage) + detail::mesh_block_names_for_error_message(aInput)};
    }
    return {};
}

template <typename InputBlock>
auto validate_fixed_block_names_exist(const InputBlock& aInput) -> std::optional<std::string>
{
    if (!aInput.fixed_blocks.has_value())
    {
        return {};
    }
    const auto tMeshBlockNames = detail::mesh_block_names(aInput);
    const auto tUniqueFixedBlocks = fixed_blocks(aInput);
    auto tMissingFixedBlocks = std::vector<std::string>{};
    for (const auto& tInputBlockName : tUniqueFixedBlocks)
    {
        if (const auto tMeshBlockIter = std::find(tMeshBlockNames.cbegin(), tMeshBlockNames.cend(), tInputBlockName);
            tMeshBlockIter == tMeshBlockNames.cend())
        {
            tMissingFixedBlocks.push_back(tInputBlockName);
        }
    }
    if (!tMissingFixedBlocks.empty())
    {
        auto tAllMissingFixedBlockNames = utilities::concatenate_container(tMissingFixedBlocks, ", ");
        return std::optional{utilities::concatenate(
            "The following fixed_block entries could not be found in the mesh: ", std::move(tAllMissingFixedBlockNames),
            ". ", detail::mesh_block_names_for_error_message(aInput))};
    }
    return std::nullopt;
}

template <typename InputBlock>
auto validate_at_least_one_design_block(const InputBlock& aInput) -> std::optional<std::string>
{
    if (!aInput.fixed_blocks.has_value())
    {
        return {};
    }
    const auto tMeshBlockNames = detail::mesh_block_names(aInput);
    const auto tUniqueFixedBlocks = fixed_blocks(aInput);
    if (tMeshBlockNames.size() == tUniqueFixedBlocks.size())
    {
        return std::optional{"All blocks have been listed under fixed_block, there is no design domain."};
    }
    return std::nullopt;
}

}  // namespace plato::geometry::extension

#endif
