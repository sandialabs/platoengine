#ifndef PLATO_FILTER_EXTENSION_PARALLELUNITTEST_LINEARMASKTESTUTILITY
#define PLATO_FILTER_EXTENSION_PARALLELUNITTEST_LINEARMASKTESTUTILITY

#include "plato/filter/extension/LinearMask.hpp"
#include "plato/filter/extension/LinearMaskBuilder.hpp"

namespace plato::filter::extension::unittest
{
constexpr int kMaximumConnectivity = 3;

/// @brief Creates a LinearMaskFactory with a small number of nodes for testing.
///
/// If @a aCentroids is empty, the LinearMask will use the nodes as the filter range.
LinearMaskFactory create_simple_linear_mask_factory(
    const std::vector<third_party_integration::common::Coordinate>& aCentroids = {});

/// @brief Creates a LinearMask with a small number of nodes for testing.
///
/// If @a aCentroids is empty, the LinearMask will use the nodes as the filter range.
LinearMask create_simple_linear_mask(const std::vector<third_party_integration::common::Coordinate>& aCentroids = {});

}  // namespace plato::filter::extension::unittest

#endif
