#include "plato/filter/extension/parallel_unittest/LinearMaskTestUtility.hpp"

namespace plato::filter::extension::unittest
{

LinearMaskFactory create_simple_linear_mask_factory(
    const std::vector<third_party_integration::common::Coordinate>& aCentroids)
{
    const auto tCoordinates =
        std::vector<third_party_integration::common::Coordinate>{{0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {3, 0, 0}};
    if (!aCentroids.empty())
    {
        return LinearMaskFactory(NodalVector{tCoordinates}, CenterVector{aCentroids}, SearchRadius{2.0},
                                 kMaximumConnectivity, boost::mpi::communicator{});
    }

    return LinearMaskFactory(NodalVector{tCoordinates}, CenterVector{tCoordinates}, SearchRadius{2.0},
                             kMaximumConnectivity, boost::mpi::communicator{});
}

LinearMask create_simple_linear_mask(const std::vector<third_party_integration::common::Coordinate>& aCentroids)
{
    return LinearMask{create_simple_linear_mask_factory(aCentroids).returnMask(), boost::mpi::communicator{}};
}

}  // namespace plato::filter::extension::unittest
