#ifndef PLATO_FILTER_EXTENSION_KERNELFILTERS_TESTUTILITIES_REFLECTIONMESHTESTUTILITY
#define PLATO_FILTER_EXTENSION_KERNELFILTERS_TESTUTILITIES_REFLECTIONMESHTESTUTILITY

#include <filesystem>

#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"

namespace plato::filter::extension::kernel_filters::test_utilities
{

/// @brief Generate a simple 10x10x10 hex mesh with upper bounds (1,1,1). Depending on the lower bounds can be used to
/// create a positive octant mesh or a mesh with negative coordinates
void create_mesh_for_reflection_filter(const std::filesystem::path& aMeshFile,
                                       const third_party_integration::stk_io::CommandBounds& aLowerBounds);

}  // namespace plato::filter::extension::kernel_filters::test_utilities

#endif
