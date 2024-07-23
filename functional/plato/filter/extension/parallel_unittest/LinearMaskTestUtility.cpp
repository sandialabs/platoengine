#include "plato/filter/extension/parallel_unittest/LinearMaskTestUtility.hpp"

namespace plato::filter::extension::unittest
{

LinearMaskBuilder create_simple_linear_mask_builder(
    const std::vector<third_party_integration::common::Coordinate>& aCentroids)
{
    const auto tCoordinates =
        std::vector<third_party_integration::common::Coordinate>{{0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {3, 0, 0}};
    if (!aCentroids.empty())
    {
        return LinearMaskBuilder(NodalVector{tCoordinates}, CenterVector{aCentroids}, SearchRadius{2.0},
                                 kMaximumConnectivity, boost::mpi::communicator{});
    }

    return LinearMaskBuilder(NodalVector{tCoordinates}, CenterVector{tCoordinates}, SearchRadius{2.0},
                             kMaximumConnectivity, boost::mpi::communicator{});
}

LinearMask create_simple_linear_mask(const std::vector<third_party_integration::common::Coordinate>& aCentroids)
{
    return LinearMask{create_simple_linear_mask_builder(aCentroids).mask(), boost::mpi::communicator{}};
}

}  // namespace plato::filter::extension::unittest
