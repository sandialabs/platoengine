#include "plato/utilities/STKVolumeUtilities.hpp"

#include <memory>
#include <numeric>
#include <vector>

#include "plato/utilities/PairWiseAccumulate.hpp"

namespace plato::utilities
{
namespace
{

Coordinate coordinate_from_data_array(const double* aData, const unsigned int aNumDimensions)
{
    if (aNumDimensions == 3)
    {
        return Coordinate{/*.x=*/aData[0],
                          /*.y=*/aData[1],
                          /*.z=*/aData[2]};
    }
    else
    {
        return Coordinate{/*.x=*/aData[0],
                          /*.y=*/aData[1],
                          /*.z=*/0};
    }
}

}  // namespace

double element_volume(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
{
    return detail::element_volume_impl<stk::topology::HEXAHEDRON_8, stk::topology::HEXAHEDRON_20,
                                       stk::topology::TETRAHEDRON_4, stk::topology::TETRAHEDRON_10,
                                       stk::topology::QUADRILATERAL_4, stk::topology::TRIANGLE_3,
                                       stk::topology::TRIANGLE_3_2D, stk::topology::QUAD_4_2D,
                                       stk::topology::SHELL_QUAD_4, stk::topology::SHELL_TRIANGLE_3>(
        aElement, aBulk, aBulk.bucket(aElement).topology()());
}

double mesh_volume(const std::filesystem::path& aMeshFileName)
{
    std::shared_ptr<stk::mesh::BulkData> tBulk = read_mesh_bulk_data(aMeshFileName);
    assert(tBulk);
    const stk::mesh::EntityVector tElements = element_vector(*tBulk);
    std::vector<double> tVolume;
    std::transform(tElements.begin(), tElements.end(), std::back_inserter(tVolume),
                   [&tBulk](const auto& iElement) { return element_volume(iElement, *tBulk); });

    return pair_wise_accumulate(tVolume);
}

std::vector<Coordinate> element_coordinates(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
{
    std::vector<Coordinate> tCoordinates;
    const unsigned int tNumDimensions = spatial_dimensions(aBulk);
    const stk::mesh::FieldBase* const tCoords = aBulk.mesh_meta_data().coordinate_field();
    std::transform(aBulk.begin_nodes(aElement), aBulk.end_nodes(aElement), std::back_inserter(tCoordinates),
                   [tNumDimensions, tCoords](const stk::mesh::Entity& node)
                   {
                       const auto tData = static_cast<const double*>(stk::mesh::field_data(*tCoords, node));
                       return coordinate_from_data_array(tData, tNumDimensions);
                   });
    return tCoordinates;
}

}  // namespace plato::utilities
