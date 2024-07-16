#ifndef PLATO_FILTER_EXTENSION_PARALLELUNITTEST
#define PLATO_FILTER_EXTENSION_PARALLELUNITTEST

#include "plato/filter/extension/LinearMaskFactory.hpp"

namespace plato::filter::extension::unittest
{
constexpr int kMaximumConnectivity = 3;
template <typename ReturnType>
ReturnType create_simple_linear_mask(
    const std::optional<std::vector<third_party_integration::common::Coordinate>>& aCentroids)
{
    const std::vector<third_party_integration::common::Coordinate> tCoordinates{
        {0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {3, 0, 0}};
    if (aCentroids.has_value())
    {
        return ReturnType(NodalVector{tCoordinates}, CenterVector{aCentroids.value()}, SearchRadius{2.0},
                          kMaximumConnectivity, boost::mpi::communicator{});
    }

    return ReturnType(NodalVector{tCoordinates}, CenterVector{tCoordinates}, SearchRadius{2.0}, kMaximumConnectivity,
                      boost::mpi::communicator{});
}

}  // namespace plato::filter::extension::unittest

#endif
