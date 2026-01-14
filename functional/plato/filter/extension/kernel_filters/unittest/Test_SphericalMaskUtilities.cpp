#include <gtest/gtest.h>

#include <numbers>

#include "plato/filter/extension/kernel_filters/SphericalMaskUtilities.hpp"
#include "plato/filter/extension/kernel_filters/test_utilities/LinearMaskTestUtility.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/tpetra/TestUtilities.hpp"

namespace plato::filter::extension::kernel_filters::unittest
{
namespace
{
constexpr std::string_view kMeshFile = "mesh.exo";
}

TEST(SphericalMaskUtilitiesDetail, FilterVolume)
{
    constexpr double tRadius = 1.23;
    const double tResult = detail::filter_volume(SearchRadius{tRadius});
    const double tGold = std::numbers::pi * 4.0 / 3.0 * tRadius * tRadius * tRadius;
    EXPECT_DOUBLE_EQ(tResult, tGold);
}

TEST(SphericalMaskUtilitiesDetail, FilterArea)
{
    constexpr double tRadius = 1.23;
    const double tResult = detail::filter_area(SearchRadius{tRadius});
    const double tGold = std::numbers::pi * tRadius * tRadius;
    EXPECT_DOUBLE_EQ(tResult, tGold);
}

TEST(LinearMaskBuilder, GenerateDistanceMapGivenCentroid)
{
    const std::vector<third_party_integration::common::Coordinate> tRelativeToCoordinate{{1, 0, 0}};
    const auto tLinearMaskBuilder = test_utilities::create_simple_linear_mask_builder(tRelativeToCoordinate);

    const auto tDistanceMap = tLinearMaskBuilder.mask();
    /*
           1/4         1/2            1/4        0
    */
    namespace tpit = third_party_integration::tpetra;
    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 0, 0), 1.0 / 4.0);
    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 0, 1), 1.0 / 2.0);
    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 0, 2), 1.0 / 4.0);
    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 0, 3), 0);
}

TEST(SphericalMaskUtilitiesDetail, AverageNodesInFilterRadius)
{
    constexpr auto tCommandGenerator =
        third_party_integration::stk_io::CommandGenerator{{21, 21, 21}, {-10, -10, -10}, {10, 10, 10}};
    third_party_integration::stk_io::write_mesh(kMeshFile, tCommandGenerator);

    constexpr auto tFilterRadius = SearchRadius{5};

    const auto tNodalDensity = tCommandGenerator.numberOfNodes() / tCommandGenerator.volume();
    const auto tSearchVolume = detail::filter_volume(tFilterRadius);
    const auto tExpected = static_cast<int>(tNodalDensity * tSearchVolume);

    const auto tResult = detail::average_nodes_in_filter_radius_estimate(mesh::Mesh{kMeshFile}, tFilterRadius);
    EXPECT_EQ(tExpected, tResult);

    constexpr auto tNumberOfActualNodes = 515;  // matlab
    EXPECT_GT(tResult, tNumberOfActualNodes);

    plato::test_utilities::test_for_existence_and_remove({kMeshFile}, TEST_CONTEXT("Removing temporary files."));
}

}  // namespace plato::filter::extension::kernel_filters::unittest
