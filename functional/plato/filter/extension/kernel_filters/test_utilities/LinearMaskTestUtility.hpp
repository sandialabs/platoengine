#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_TESTUTILITIES_LINEARMASKTESTUTILITY
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_TESTUTILITIES_LINEARMASKTESTUTILITY

#include "plato/filter/extension/kernel_filters/CanonicalKernelFilter.hpp"
#include "plato/filter/extension/kernel_filters/LinearMask.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"

namespace plato::filter::extension::kernel_filters::test_utilities
{
constexpr int kMaximumConnectivity = 4;

/// @brief Creates a LinearMaskBuilder with a small number of nodes for testing.
///
/// If @a aCentroids is empty, the LinearMask will use the nodes as the filter range.
[[nodiscard]] auto create_simple_linear_mask_builder(
    const std::vector<third_party_integration::common::Coordinate>& aCentroids = {})
    -> LinearMaskBuilder<input_parser::kernel_filter>;

/// @brief Creates a LinearMask with a small number of nodes for testing.
///
/// If @a aCentroids is empty, the LinearMask will use the nodes as the filter range.
LinearMask create_simple_linear_mask(const std::vector<third_party_integration::common::Coordinate>& aCentroids = {});

}  // namespace plato::filter::extension::kernel_filters::test_utilities

#endif
