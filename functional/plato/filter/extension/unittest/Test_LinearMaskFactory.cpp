#include <gtest/gtest.h>

#include <boost/math/constants/constants.hpp>

#include "plato/filter/extension/LinearMaskFactory.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::filter::extension::unittest
{
namespace
{
constexpr std::string_view kMeshFile = "mesh.exo";
}

TEST(KernelFilterDetail, FilterVolume)
{
    constexpr double tRadius = 1.23;
    const double tResult = detail::filter_volume(SearchRadius{tRadius});
    const double tGold = boost::math::constants::pi<double>() * 4.0 / 3.0 * tRadius * tRadius * tRadius;
    EXPECT_DOUBLE_EQ(tResult, tGold);
}

TEST(KernelFilterDetail, FilterArea)
{
    constexpr double tRadius = 1.23;
    const double tResult = detail::filter_area(SearchRadius{tRadius});
    const double tGold = boost::math::constants::pi<double>() * tRadius * tRadius;
    EXPECT_DOUBLE_EQ(tResult, tGold);
}

TEST(KernelFilterDetail, DetermineMaximumConnectivityEstimate)
{
    const third_party_integration::stk_io::CommandGenerator tCommandGenerator{
        {21, 21, 21}, {-10, -10, -10}, {10, 10, 10}};
    third_party_integration::stk_io::write_mesh(kMeshFile, tCommandGenerator);

    const SearchRadius tFilterRadius{5};
    const double tNodalDensity = tCommandGenerator.numberOfNodes() / tCommandGenerator.volume();
    const double tSearchVolume = detail::filter_volume(tFilterRadius);
    const int tGold = static_cast<int>(tNodalDensity * tSearchVolume * detail::kMaxMultiplier);

    const int tResult = detail::maximum_connectivity_estimate(mesh::Mesh{kMeshFile}, tFilterRadius);
    EXPECT_EQ(tGold, tResult);

    constexpr double tNumberOfActualNodes = 515;  // matlab
    EXPECT_GT(tResult, tNumberOfActualNodes);

    test_utilities::test_for_existence_and_remove({kMeshFile}, TEST_CONTEXT("Removing temporary files."));
}
}  // namespace plato::filter::extension::unittest
