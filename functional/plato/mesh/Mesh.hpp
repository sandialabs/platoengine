#ifndef PLATO_MESH_MESH
#define PLATO_MESH_MESH

#include <filesystem>
#include <functional>
#include <memory>
#include <set>
#include <vector>

#include "plato/third_party_integration/common/BlockData.hpp"

namespace stk::mesh
{
// In trilinos 15.1, there are conflicting forward declarations of BulkData, of which clang-tidy disapproves.
class BulkData;  // NOLINT
class Part;
}  // namespace stk::mesh

namespace plato::design_variables
{
struct MeshDesignVariables;
}

namespace plato::mesh
{
/// @brief A basic Mesh class that is meant to be extended with mixin classes.
///
/// The base class Mesh is mainly a wrapper for `stk::mesh::BulkData` and a set of fixed blocks that define
/// the non-optimizable parts of the mesh domain. The nodal coordinates, elements, IDs, volume, etc. are implemented
/// in mixin classes that extend the functionality of this class. For example, the number of nodes is given by:
/// @code{.cpp}
/// const auto tMesh = mesh::Mesh{tFilePath};
/// const auto tNumberOfNodes = mesh::EntityCounts{tMesh}.numberOfNodes();
/// @endcode
/// @sa EntityCounts
/// @sa EntityRetrieval
/// @sa MeshBlocks
/// @sa MeshQuantities
class Mesh
{
   public:
    using BlockOrdinalType = third_party_integration::common::BlockData::BlockOrdinalType;
    using PartReferenceVector = std::vector<std::reference_wrapper<const stk::mesh::Part>>;

    /// @brief Loads a mesh from disk at the path @a aMeshName
    /// @param aFixedBlockNames A list of fixed blocks, the inverse of which will define the design domain.
    /// @pre The block names in @a aFixedBlockNames must be valid names in the mesh. Checked with an assertion.
    explicit Mesh(const std::filesystem::path& aMeshName, const std::set<std::string>& aFixedBlockNames = {});

    /// @brief Constructs a mesh from a design_variables::MeshDesignVariables object.
    ///
    /// Loads a mesh from disk as referred to by the file name in @a aMeshDesignVariables, and determines the fixed
    /// vs. design blocks from the data.
    explicit Mesh(const design_variables::MeshDesignVariables& aMeshDesignVariables);

    /// @brief Returns the path to the mesh on disk.
    const std::filesystem::path& filePath() const;

    /// @brief Returns the fixed block internal metadata ordinals
    /// @note These are stk internally generated IDs and not the same as the mesh's original block IDs.
    const std::vector<BlockOrdinalType>& fixedBlockOrdinals() const;

    /// @brief Returns the design block internal metadata ordinals
    /// @note These are stk internally generated IDs and not the same as the mesh's original block IDs.
    const std::vector<BlockOrdinalType>& designBlockOrdinals() const;

   protected:
    /// @brief Returns a reference to the underlying BulkData.
    const stk::mesh::BulkData& bulkData() const;

    /// @brief Returns a vector of stk::mesh::Parts associated with the fixed (non-optimizable) domain
    PartReferenceVector fixedDomainBlocks() const;

    /// @brief Returns a vector of stk::mesh::Parts associated with the design (optimizable) domain
    PartReferenceVector designDomainBlocks() const;

   private:
    std::filesystem::path mFilePath;
    std::shared_ptr<stk::mesh::BulkData> mBulk;
    std::vector<BlockOrdinalType> mFixedBlockOrdinals;
    std::vector<BlockOrdinalType> mDesignBlockOrdinals;
};

}  // namespace plato::mesh

#endif
