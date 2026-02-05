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

namespace plato::analysis
{
struct AnalysisDomainMesh;
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

    /// @brief Constructs a mesh from a AnalysisDomainMesh object.
    ///
    /// Loads a mesh from disk as referred to by the file name in @a aAnalysisDomainMesh, and determines the fixed
    /// vs. design blocks from the data.
    explicit Mesh(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh);

    /// @brief Returns the path to the mesh on disk.
    [[nodiscard]] auto filePath() const -> const std::filesystem::path&;

    /// @brief Returns the fixed block internal metadata ordinals
    /// @note These are stk internally generated IDs and not the same as the mesh's original block IDs.
    [[nodiscard]] auto fixedBlockOrdinals() const -> const std::vector<BlockOrdinalType>&;

    /// @brief Returns the design block internal metadata ordinals
    /// @note These are stk internally generated IDs and not the same as the mesh's original block IDs.
    [[nodiscard]] auto designBlockOrdinals() const -> const std::vector<BlockOrdinalType>&;

    /// @brief Returns whether or not this object was constructed with a path to a valid mesh.
    [[nodiscard]] auto valid() const -> bool;

   protected:
    /// @brief Returns a const reference to the underlying BulkData.
    /// @pre valid returns `true`
    [[nodiscard]] auto bulkData() const -> const stk::mesh::BulkData&;

    /// @brief Returns a reference to the underlying BulkData.
    /// @pre valid returns `true`
    [[nodiscard]] auto bulkData() -> stk::mesh::BulkData&;

    /// @brief Returns a vector of stk::mesh::Parts associated with the fixed (non-optimizable) domain
    [[nodiscard]] auto fixedDomainBlocks() const -> PartReferenceVector;

    /// @brief Returns a vector of stk::mesh::Parts associated with the design (optimizable) domain
    [[nodiscard]] auto designDomainBlocks() const -> PartReferenceVector;

    /// @brief Returns a vector of stk::mesh::Parts associated with the block names in @a aBlockNames
    [[nodiscard]] auto specifiedDomainBlocks(const std::set<std::string>& aBlockNames) const -> PartReferenceVector;

   private:
    std::filesystem::path mFilePath;
    std::shared_ptr<stk::mesh::BulkData> mBulk;
    std::vector<BlockOrdinalType> mFixedBlockOrdinals;
    std::vector<BlockOrdinalType> mDesignBlockOrdinals;
};

}  // namespace plato::mesh

#endif
