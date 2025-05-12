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
    [[nodiscard]] unsigned int numberOfElements() const;

    /// @brief The number of elements in the design domain.
    [[nodiscard]] unsigned int numberOfDesignDomainElements() const;

    /// @brief The total number of nodes in the mesh
    [[nodiscard]] unsigned int numberOfNodes() const;

    /// @brief The number of nodes in the design domain.
    [[nodiscard]] unsigned int numberOfDesignDomainNodes() const;

    /// @brief The total number of blocks in the mesh
    [[nodiscard]] unsigned int numberOfBlocks() const;

    /// @brief The dimensions of the mesh (2 or 3).
    [[nodiscard]] unsigned int spatialDimensions() const;

    /// @brief Returns whether or not the mesh is 2D.
    ///
    /// Equivalent to `spatialDimentions() == 2u`
    [[nodiscard]] bool is2D() const;

    /// @brief Returns whether or not the mesh is 3D.
    ///
    /// Equivalent to `spatialDimentions() == 3u`
    [[nodiscard]] bool is3D() const;

    /// @brief Returns `true` if @a aAnalysisDomainMesh contains a design variable field associated with nodes.
    [[nodiscard]] bool areNodalDesignVariables(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const;

    /// @brief Returns `true` if @a aAnalysisDomainMesh contains a design variable field associated with elements.
    [[nodiscard]] bool areElementDesignVariables(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const;

    /// @brief Returns `true` if the field name @a aFieldName exists on the nodes in the mesh
    [[nodiscard]] bool hasNodalFieldVariable(const std::string_view aFieldName) const;

    /// @brief Returns a vector of the time steps contained in the mesh.
    [[nodiscard]] auto timeSteps() const -> std::vector<double>;
};

}  // namespace plato::mesh

#endif
