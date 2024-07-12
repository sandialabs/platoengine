#include <gtest/gtest.h>

#include "plato/filter/library/FilterFactory.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/mesh/MeshProxy.hpp"
#include "plato/mesh/MeshProxyViews.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::filter::extension::unittest
{
namespace
{
const std::filesystem::path kSharedLibPath = "libPlatoIdentityFilter.so";
constexpr std::string_view kMeshName = "the-mesh-is-a-lie.exo";
const auto kRho = std::vector{-1.0, 0.0, 1.0};
const auto kMeshArgument = mesh::MeshProxy{kMeshName, kRho};
}  // namespace

TEST(SharedLibFilter, LoadAndValue)
{
    const std::unique_ptr<const library::FilterInterface> tFilter =
        library::load_filter(library::FilterParameters{}, kSharedLibPath);
    const auto tMeshProxyResult = tFilter->filter(kMeshArgument);
    const auto tMeshView = mesh::MeshProxyDensitiesView{tMeshProxyResult};
    for (const auto [tComputed, tExpected] : utilities::Zip{tMeshView, kRho})
    {
        EXPECT_EQ(tComputed, tExpected);
    }
}

}  // namespace plato::filter::extension::unittest
