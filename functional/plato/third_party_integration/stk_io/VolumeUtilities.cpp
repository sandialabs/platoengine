#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"

#include <memory>
#include <numeric>
#include <vector>

#include "plato/utilities/PairWiseAccumulate.hpp"

namespace plato::third_party_integration::stk_io
{
namespace
{

common::Coordinate coordinate_from_data_array(const double* aData, const unsigned int aNumDimensions)
{
    if (aNumDimensions == 3)
    {
        return common::Coordinate{/*.x=*/aData[0],
                                  /*.y=*/aData[1],
                                  /*.z=*/aData[2]};
    }
    else
    {
        return common::Coordinate{/*.x=*/aData[0],
                                  /*.y=*/aData[1],
                                  /*.z=*/0};
    }
}

}  // namespace

double element_volume(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
{
    return detail::element_apply<detail::VolumeTag>(aElement, aBulk);
}

common::Coordinate element_centroid(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
{
    return detail::element_apply<detail::CentroidTag>(aElement, aBulk);
}

double mesh_volume(const stk::mesh::BulkData& aBulk)
{
    const stk::mesh::EntityVector tElements = element_vector(aBulk);
    std::vector<double> tVolume;
    std::transform(tElements.begin(), tElements.end(), std::back_inserter(tVolume),
                   [&aBulk](const auto& iElement) { return element_volume(iElement, aBulk); });

    return utilities::pair_wise_accumulate(tVolume);
}

std::vector<common::Coordinate> element_centroids(const stk::mesh::BulkData& aBulk)
{
    const stk::mesh::EntityVector tElements = element_vector(aBulk);
    std::vector<common::Coordinate> tCentroids;
    std::transform(tElements.begin(), tElements.end(), std::back_inserter(tCentroids),
                   [&aBulk](const auto& iElement) { return element_centroid(iElement, aBulk); });

    return tCentroids;
}

std::vector<common::Coordinate> element_coordinates(const stk::mesh::Entity& aElement, const stk::mesh::BulkData& aBulk)
{
    std::vector<common::Coordinate> tCoordinates;
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

double average_nodal_density(const stk::mesh::BulkData& aBulk)
{
    const auto tTotalNumberOfNodes = node_size(aBulk);
    const double tTotalVolume = mesh_volume(aBulk);
    return static_cast<double>(tTotalNumberOfNodes) / tTotalVolume;
}

}  // namespace plato::third_party_integration::stk_io
