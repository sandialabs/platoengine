#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/filter/extension/kernel_filters/CylindricalMaskUtilities.hpp"
#include "plato/filter/extension/kernel_filters/LinearMaskBuilder.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::filter::extension::kernel_filters::unittest
{
namespace
{
const auto kSourceMeshFile = utilities::data_file_path("three_node_2d.cdf");
const auto kTargetMeshFile = utilities::data_file_path("one_node_3d.cdf");
}  // namespace

TEST(CylindricalMaskUtilitiesDetail, CylinderRampWeight)
{
    namespace tpic = third_party_integration::common;
    constexpr auto tCylinderCenter = tpic::Coordinate{1, 2, 3};
    constexpr auto tCylinderAxis = tpic::Vector3{0, 0, 1};
    constexpr auto tTestLocation = tpic::Coordinate{2., 2., 1};

    {
        constexpr auto tCylinderRadius = 1;
        constexpr auto tGold = 0;
        const auto tResult =
            detail::cylinder_ramp_weight(detail::CylinderCenter{tCylinderCenter}, CylinderAxis{tCylinderAxis},
                                         CylinderRadius{tCylinderRadius}, detail::QueryLocation{tTestLocation});
        EXPECT_EQ(tResult, tGold);
    }
    {
        constexpr auto tCylinderRadius = 2;
        constexpr auto tGold = 0.5;
        const auto tResult =
            detail::cylinder_ramp_weight(detail::CylinderCenter{tCylinderCenter}, CylinderAxis{tCylinderAxis},
                                         CylinderRadius{tCylinderRadius}, detail::QueryLocation{tTestLocation});
        EXPECT_EQ(tResult, tGold);
    }
    {
        constexpr auto tCylinderRadius = 2;
        constexpr auto tTestLocationOne = tpic::Coordinate{1.3, 1.2, -5};
        const auto tResult =
            detail::cylinder_ramp_weight(detail::CylinderCenter{tCylinderCenter}, CylinderAxis{tCylinderAxis},
                                         CylinderRadius{tCylinderRadius}, detail::QueryLocation{tTestLocationOne});
        constexpr auto tTestLocationTwo = tpic::Coordinate{1.3, 1.2, 5};
        const auto tResultTwo =
            detail::cylinder_ramp_weight(detail::CylinderCenter{tCylinderCenter}, CylinderAxis{tCylinderAxis},
                                         CylinderRadius{tCylinderRadius}, detail::QueryLocation{tTestLocationTwo});
        EXPECT_EQ(tResult, tResultTwo) << "points with different z should have same distance";
    }
}

TEST(CylindricalMaskUtilitiesDetail, CylinderRampWeightTwo)
{
    namespace tpic = third_party_integration::common;
    constexpr auto tTestLocation = tpic::Coordinate{1., -2., 3};
    constexpr auto tCylinderCenter = tpic::Coordinate{-4, 5, -6};
    constexpr auto tCylinderAxis = tpic::Vector3{7, -8, 9};

    constexpr auto tCylinderRadius = 10;
    constexpr auto tGold = 1.0 - 0.158276803075348;
    const auto tResult =
        detail::cylinder_ramp_weight(detail::CylinderCenter{tCylinderCenter}, CylinderAxis{tCylinderAxis},
                                     CylinderRadius{tCylinderRadius}, detail::QueryLocation{tTestLocation});
    EXPECT_DOUBLE_EQ(tResult, tGold);
}

TEST(CylindricalMaskUtilitiesDetail, DistributeSearchVectorsAndStkSearch)
{
    namespace tpis = third_party_integration::stk_search;
    ASSERT_TRUE(kSourceMeshFile.has_value());
    ASSERT_TRUE(kTargetMeshFile.has_value());

    const auto tSourceMeshNodes = mesh::EntityRetrieval{mesh::Mesh{kSourceMeshFile.value()}}.nodalCoordinates();
    const auto tTargetMeshNodes = mesh::EntityRetrieval{mesh::Mesh{kTargetMeshFile.value()}}.nodalCoordinates();
    constexpr auto tRadius = 3.0;
    constexpr auto tAxis = third_party_integration::common::Vector3{0, 0, 1};
    const auto tComm = boost::mpi::communicator{};

    const auto tSearchResults = detail::distribute_search_vectors_and_stk_search_with_cylinders(
        TargetRowVector{tTargetMeshNodes}, SourceColumnVector{tSourceMeshNodes}, CylinderAxis{tAxis},
        CylinderRadius{tRadius}, tComm);

    constexpr auto tRankZero = 0;
    ASSERT_EQ(tSearchResults.size(), 3U);
    {
        const auto tExpected = std::pair<tpis::Identifier, tpis::Identifier>{{0, tRankZero}, {0, tRankZero}};
        EXPECT_EQ(tSearchResults.at(0), tExpected);
    }
    {
        const auto tExpected = std::pair<tpis::Identifier, tpis::Identifier>{{0, tRankZero}, {1, tRankZero}};
        EXPECT_EQ(tSearchResults.at(1), tExpected);
    }
    {
        const auto tExpected = std::pair<tpis::Identifier, tpis::Identifier>{{0, tRankZero}, {2, tRankZero}};
        EXPECT_EQ(tSearchResults.at(2), tExpected);
    }
}

TEST(CylindricalMaskUtilitiesDetail, CylindricalPolarCoordinates)
{
    constexpr auto tPoint = third_party_integration::common::Coordinate{3, 4, 5};
    const auto tPolarPoint = detail::cylindrical_polar_coordinates(tPoint).mValue;
    const auto tGold = third_party_integration::common::Coordinate{5, std::atan2(4, 3), 5};
    third_party_integration::common::test_utilities::test_double_equality_of_components(
        tPolarPoint, tGold, TEST_CONTEXT("Convert single point to polar coordinates"));
}

TEST(CylindricalMaskUtilitiesDetail, CylindricalPolarCoordinatesVector)
{
    constexpr auto tPoint = third_party_integration::common::Coordinate{3, 4, 5};
    constexpr auto tPointTwo = third_party_integration::common::Coordinate{5, 12, 13};
    const auto tPolarPoints = detail::cylindrical_polar_coordinates({tPoint, tPointTwo}).mValue;
    const auto tGold =
        std::vector<third_party_integration::common::Coordinate>{{5, std::atan2(4, 3), 5}, {13, std::atan2(12, 5), 13}};
    ASSERT_EQ(tPolarPoints.size(), 2U);
    third_party_integration::common::test_utilities::test_double_equality_of_components(
        tPolarPoints.front(), tGold.front(), TEST_CONTEXT("Convert single point to polar coordinates"));
    third_party_integration::common::test_utilities::test_double_equality_of_components(
        tPolarPoints.back(), tGold.back(), TEST_CONTEXT("Convert single point to polar coordinates"));
}

}  // namespace plato::filter::extension::kernel_filters::unittest
