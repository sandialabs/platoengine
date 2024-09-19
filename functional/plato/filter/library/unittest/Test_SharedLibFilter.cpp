#include <gtest/gtest.h>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/filter/library/FilterFactory.hpp"
#include "plato/filter/library/FilterInterface.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::filter::extension::unittest
{
namespace
{
const std::filesystem::path kSharedLibPath = "libPlatoIdentityFilter.so";
constexpr std::string_view kMeshName = "the-mesh-is-a-lie.exo";
const auto kRho = std::vector{-1.0, 0.0, 1.0};
const auto kIDs = std::vector<std::size_t>{0, 1, 2};
const auto kDensitiesAndIDs = analysis::combine_scalar_field_values_and_ids(kRho, kIDs);
const auto kMeshArgument =
    analysis::AnalysisDomainMesh{kMeshName, analysis::AnalysisDomainMesh::BlockScalarField{{1, kDensitiesAndIDs}}};
}  // namespace

TEST(SharedLibFilter, LoadAndValue)
{
    const std::unique_ptr<const library::FilterInterface> tFilter =
        library::load_filter(library::FilterParameters{}, kSharedLibPath);
    const auto tAnalysisDomainMeshResult = tFilter->filter(kMeshArgument);
    const auto tMeshView = analysis::AnalysisDomainMeshSequentialView{tAnalysisDomainMeshResult};
    for (const auto [tComputed, tExpected] : utilities::Zip{tMeshView, kRho})
    {
        EXPECT_EQ(static_cast<analysis::ScalarFieldValue>(tComputed).mValue, tExpected);
    }
}

}  // namespace plato::filter::extension::unittest
