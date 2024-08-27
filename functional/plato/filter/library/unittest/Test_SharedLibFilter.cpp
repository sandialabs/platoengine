#include <gtest/gtest.h>

#include "plato/filter/library/FilterFactory.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/mesh/MeshDesignVariables.hpp"
#include "plato/mesh/MeshDesignVariablesDensitiesView.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::filter::extension::unittest
{
namespace
{
const std::filesystem::path kSharedLibPath = "libPlatoIdentityFilter.so";
constexpr std::string_view kMeshName = "the-mesh-is-a-lie.exo";
const auto kRho = std::vector{-1.0, 0.0, 1.0};
const auto kIDs = std::vector<std::size_t>{0, 1, 2};
const auto kDensitiesAndIDs = mesh::detail::combine_densities_and_ids(kRho, kIDs);
const auto kMeshArgument =
    mesh::MeshDesignVariables{kMeshName, mesh::MeshDesignVariables::BlockDensities{{1, kDensitiesAndIDs}}};
}  // namespace

TEST(SharedLibFilter, LoadAndValue)
{
    const std::unique_ptr<const library::FilterInterface> tFilter =
        library::load_filter(library::FilterParameters{}, kSharedLibPath);
    const auto tMeshDesignVariablesResult = tFilter->filter(kMeshArgument);
    const auto tMeshView = mesh::MeshDesignVariablesDensitiesView{tMeshDesignVariablesResult};
    for (const auto [tComputed, tExpected] : utilities::Zip{tMeshView, kRho})
    {
        EXPECT_EQ(static_cast<mesh::Density>(tComputed).mDensity, tExpected);
    }
}

}  // namespace plato::filter::extension::unittest
