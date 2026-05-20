#ifndef PLATO_MESH_ENTITYCOUNTS
#define PLATO_MESH_ENTITYCOUNTS

#include "plato/mesh/Mesh.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::mesh
{
/// @brief A mixin class for Mesh that provides utilities counting entities in a mesh, such as nodes.
struct EntityCounts : public Mesh
{
    explicit EntityCounts(Mesh aMeshBase);

    /// @brief The total number of elements in the mesh
    [[nodiscard]] auto numberOfElements() const -> unsigned int;

    /// @brief The number of elements in the design domain.
    [[nodiscard]] auto numberOfDesignDomainElements() const -> unsigned int;

    /// @brief The total number of nodes in the mesh
    [[nodiscard]] auto numberOfNodes() const -> unsigned int;

    /// @brief The number of nodes in the design domain.
    [[nodiscard]] auto numberOfDesignDomainNodes() const -> unsigned int;

    /// @brief The total number of blocks in the mesh
    [[nodiscard]] auto numberOfBlocks() const -> unsigned int;

    /// @brief The dimensions of the mesh (2 or 3).
    [[nodiscard]] auto spatialDimensions() const -> unsigned int;

    /// @brief Returns whether or not the mesh is 2D.
    ///
    /// Equivalent to `spatialDimentions() == 2u`
    [[nodiscard]] auto is2D() const -> bool;

    /// @brief Returns whether or not the mesh is 3D.
    ///
    /// Equivalent to `spatialDimentions() == 3u`
    [[nodiscard]] auto is3D() const -> bool;

    /// @brief Returns `true` if @a aAnalysisDomainMesh contains a design variable field associated with nodes.
    [[nodiscard]] auto areNodalDesignVariables(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const -> bool;

    /// @brief Returns `true` if @a aAnalysisDomainMesh contains a design variable field associated with elements.
    [[nodiscard]] auto areElementDesignVariables(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const -> bool;

    /// @brief Returns `true` if the field name @a aFieldName exists on the nodes in the mesh
    [[nodiscard]] auto hasNodalFieldVariable(std::string_view aFieldName) const -> bool;

    /// @brief Returns a vector of the time steps contained in the mesh.
    [[nodiscard]] auto timeSteps() const -> std::vector<double>;

    /// @brief Returns `true` if the mesh contains only tet or tri type elements.
    [[nodiscard]] auto isAllTetAndTri() const -> bool;
};

}  // namespace plato::mesh

#endif
