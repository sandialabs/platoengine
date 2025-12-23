#ifndef PLATO_MESH_MESHQUANTITIES
#define PLATO_MESH_MESHQUANTITIES

#include "plato/mesh/Mesh.hpp"
#include "plato/third_party_integration/stk_io/MeshFieldOperations.hpp"

namespace plato::mesh
{
/// @brief A mixin extension for Mesh, this provides functions that compute global mesh quantities such as volume.
struct MeshQuantities : public Mesh
{
    explicit MeshQuantities(Mesh aMeshBase);

    /// @brief Returns the total volume of the mesh.
    [[nodiscard]] double volume() const;

    /// @brief Returns the average nodal density of the mesh, computed as the total number of nodes divided by the
    /// volume.
    [[nodiscard]] double averageNodalDensity() const;

    /// @brief Returns a vector of element volumes corresponding to the fixed domain only.
    ///
    /// The order is given by the order of iteration of AnalysisDomainMeshSequentialView.
    [[nodiscard]] std::vector<double> fixedDomainElementVolumes() const;

    /// @brief Returns a vector of element volumes corresponding to the elements in the design domain only.
    ///
    /// The order is given by the order of iteration of AnalysisDomainMeshSequentialView.
    [[nodiscard]] std::vector<double> designDomainElementVolumes() const;

    /// @brief Computes the nodal average of the scalar field defined by @a aNodalScalarField on each element.
    /// @pre The size of @a aNodalScalarField must be equal to the total number of nodes on the mesh and the entries are
    /// assumed to be sorted by ascending global node ID.
    /// @post The size of the returned vector will be equal to the number of elements on the mesh and will be sorted by
    /// ascending global element ID.
    [[nodiscard]] auto elementAveragedNodalValues(const std::ranges::random_access_range auto& aNodalScalarField) const
        -> std::vector<double>;

    /// @brief Computes the projection of an element field to the nodes of a mesh using the nodal average.
    /// @pre The size of @a aElementScalarField must be equal to the total number of elements on the mesh and the
    /// entries are assumed to be sorted by ascending global element ID.
    /// @post The size of the returned vector will be equal to the number of nodes on the mesh and the entries will be
    /// sorted by ascending global node ID.
    [[nodiscard]] auto nodalAverageElementProjection(
        const std::ranges::random_access_range auto& aElementScalarField) const -> std::vector<double>;
};

auto MeshQuantities::elementAveragedNodalValues(const std::ranges::random_access_range auto& aScalarField) const
    -> std::vector<double>
{
    return third_party_integration::stk_io::element_averaged_nodal_values(aScalarField, bulkData());
}

auto MeshQuantities::nodalAverageElementProjection(
    const std::ranges::random_access_range auto& aElementScalarField) const -> std::vector<double>
{
    return third_party_integration::stk_io::nodal_average_element_projection(aElementScalarField, bulkData());
}

}  // namespace plato::mesh

#endif
