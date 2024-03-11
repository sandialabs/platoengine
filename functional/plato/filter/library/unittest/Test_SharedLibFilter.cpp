#include <gtest/gtest.h>

#include "plato/core/MeshProxy.hpp"
#include "plato/filter/library/FilterFactory.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"

namespace plato::filter::extension::unittest
{
namespace
{
const std::filesystem::path kSharedLibPath = "libPlatoIdentityFilter.so";
constexpr std::string_view kMeshName = "the-mesh-is-a-lie.exo";
const auto kRho = std::vector{-1.0, 0.0, 1.0};
const auto kMeshArgument = core::MeshProxy{kMeshName, kRho};
}  // namespace

TEST(SharedLibFilter, LoadAndValue)
{
    const std::unique_ptr<const library::FilterInterface> tFilter =
        library::load_filter(input_parser::density_topology{}, kSharedLibPath);
    EXPECT_EQ(tFilter->filter(kMeshArgument).mNodalDensities, kRho);
}

}  // namespace plato::filter::extension::unittest