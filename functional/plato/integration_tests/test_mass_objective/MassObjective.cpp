#include "plato/integration_tests/test_mass_objective/MassObjective.hpp"

#include <numeric>
#include <stk_io/FillMesh.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Comm.hpp>
#include <stk_mesh/base/Entity.hpp>
#include <stk_mesh/base/GetEntities.hpp>
#include <stk_mesh/base/MeshBuilder.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/Types.hpp>

#include "plato/utilities/STKUtilities.hpp"

namespace plato::integration_tests::test_mass_objective
{
namespace
{
constexpr unsigned int kNumTetrahedronVertices = 4;
constexpr unsigned int kNumTetsInHex = 6;
using TetrahedronIndices = std::array<unsigned int, kNumTetrahedronVertices>;

/// Tet indices from the Delaunay tesselation of a cube
constexpr auto kHexTetIndices = std::array<TetrahedronIndices, kNumTetsInHex>{
    TetrahedronIndices{4, 0, 3, 1}, TetrahedronIndices{7, 4, 3, 1}, TetrahedronIndices{7, 5, 4, 1},
    TetrahedronIndices{7, 2, 5, 1}, TetrahedronIndices{7, 3, 2, 1}, TetrahedronIndices{7, 6, 5, 2}};

struct Coordinate
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

struct Vector3
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

Vector3 operator-(const Coordinate& p0, const Coordinate& p1)
{
    return Vector3{/*.x=*/p0.x - p1.x,
                   /*.y=*/p0.y - p1.y,
                   /*.z=*/p0.z - p1.z};
}

Vector3 cross(const Vector3& a, const Vector3& b)
{
    return Vector3{/*.x=*/a.y * b.z - a.z * b.y,
                   /*.y=*/a.z * b.x - a.x * b.z,
                   /*.z=*/a.x * b.y - a.y * b.x};
}

double dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

struct Tetrahedron
{
    Coordinate p0;
    Coordinate p1;
    Coordinate p2;
    Coordinate p3;
};

Tetrahedron tetrahedron_from_hex_coordinates(const TetrahedronIndices& aIndices,
                                             const std::vector<Coordinate>& aCoordinates)
{
    return Tetrahedron{/*.p0 = */ aCoordinates[aIndices[0]],
                       /*.p1 = */ aCoordinates[aIndices[1]],
                       /*.p2 = */ aCoordinates[aIndices[2]],
                       /*.p3 = */ aCoordinates[aIndices[3]]};
}

double tetrahedron_volume(const Tetrahedron& aTetrahedron)
{
    const Vector3 a = aTetrahedron.p1 - aTetrahedron.p0;
    const Vector3 b = aTetrahedron.p2 - aTetrahedron.p0;
    const Vector3 c = aTetrahedron.p3 - aTetrahedron.p0;
    return dot(a, cross(b, c)) / 6.0;
}

stk::mesh::EntityVector element_vector(const stk::mesh::BulkData& aBulk)
{
    stk::mesh::EntityVector tElements;
    constexpr bool tSortById = true;
    stk::mesh::get_entities(aBulk, stk::topology::ELEM_RANK, aBulk.mesh_meta_data().locally_owned_part(), tElements,
                            tSortById);
    return tElements;
}

std::vector<Coordinate> element_coordinates(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
{
    std::vector<Coordinate> tCoordinates;
    const stk::mesh::FieldBase* const tCoords = aBulk.mesh_meta_data().coordinate_field();
    std::transform(aBulk.begin_nodes(aElement), aBulk.end_nodes(aElement), std::back_inserter(tCoordinates),
                   [tCoords](const stk::mesh::Entity& node)
                   {
                       const auto tData = static_cast<const double*>(stk::mesh::field_data(*tCoords, node));
                       return Coordinate{/*.x=*/tData[0],
                                         /*.y=*/tData[1],
                                         /*.z=*/tData[2]};
                   });
    return tCoordinates;
}

double element_mass(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk, const double aDensity)
{
    const std::vector<Coordinate> hexCoordinates = element_coordinates(aElement, aBulk);

    std::array<Tetrahedron, kNumTetsInHex> tTetrahedra;
    std::transform(kHexTetIndices.cbegin(), kHexTetIndices.cend(), tTetrahedra.begin(),
                   [&hexCoordinates](const TetrahedronIndices& tetIndices)
                   { return tetrahedron_from_hex_coordinates(tetIndices, hexCoordinates); });

    const double tVolume = std::accumulate(tTetrahedra.cbegin(), tTetrahedra.cend(), 0.0,
                                           [](const double aResult, const Tetrahedron& aTetrahedra)
                                           { return aResult + tetrahedron_volume(aTetrahedra); });
    return aDensity * tVolume;
}

}  // namespace

MassObjective::MassObjective(const double aDensity, const double aTarget) : mDensity(aDensity), mTarget(aTarget) {}

double MassObjective::mass(const std::string_view aMeshFileName) const
{
    std::shared_ptr<stk::mesh::BulkData> tBulk = plato::utilities::read_mesh_bulk_data(aMeshFileName);
    const stk::mesh::EntityVector tElements = element_vector(*tBulk);

    const double tMass = std::accumulate(tElements.begin(), tElements.end(), 0.0,
                                         [&tBulk, this](const double result, const stk::mesh::Entity& element)
                                         { return result + element_mass(element, *tBulk, mDensity); });
    return tMass;
}

unsigned int MassObjective::numMeshNodes(const std::string_view aMeshFileName) const
{
    return plato::utilities::read_mesh_node_size(aMeshFileName);
}
}  // namespace plato::integration_tests::test_mass_objective
