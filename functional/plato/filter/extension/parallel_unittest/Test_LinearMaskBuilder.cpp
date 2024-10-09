#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <optional>

#include "plato/filter/extension/LinearMaskBuilder.hpp"
#include "plato/filter/extension/test_utilities/LinearMaskTestUtility.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_search/Utilities.hpp"
#include "plato/third_party_integration/tpetra/TestUtilities.hpp"

namespace plato::filter::extension::parallel_unittest
{

namespace
{

constexpr auto kNumRanks = int{4};
constexpr int kNumSpatialDimensions{3};

}  // namespace

TEST(LinearMaskBuilderDetail, MPISize)
{
    const auto tComm = boost::mpi::communicator{};
    EXPECT_EQ(tComm.size(), kNumRanks);
}

TEST(LinearMaskBuilderDetail, LinearRamp)
{
    constexpr SearchRadius tSearchRadius{4};

    // closest point
    {
        constexpr Distance tDistance{0};
        constexpr double tGold = 1;
        const double tResult = detail::linear_ramp_weight(tDistance, tSearchRadius);
        EXPECT_EQ(tResult, tGold);
    }

    // outside radius
    {
        constexpr Distance tDistance{10};
        constexpr double tGold = 0;
        const double tResult = detail::linear_ramp_weight(tDistance, tSearchRadius);
        EXPECT_EQ(tResult, tGold);
    }
    // equal to radius
    {
        constexpr Distance tDistance{tSearchRadius.mValue};
        constexpr double tGold = 0;
        const double tResult = detail::linear_ramp_weight(tDistance, tSearchRadius);
        EXPECT_EQ(tResult, tGold);
    }
    // equal to half radius
    {
        constexpr Distance tDistance{tSearchRadius.mValue / 2.0};
        constexpr double tGold = 0.5;
        const double tResult = detail::linear_ramp_weight(tDistance, tSearchRadius);
        EXPECT_DOUBLE_EQ(tResult, tGold);
    }
}

TEST(LinearMaskBuilderDetail, MakeSearchPointsWithIdentifiers)
{
    namespace tpi = third_party_integration;
    const auto tCommunicator = Tpetra::getDefaultComm();
    const auto tThisRank = tCommunicator->getRank();

    const std::vector<tpi::common::Coordinate> tNodalCoordinates{
        {1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {4, 4, 4}};  // non-zero to ensure not default ctor
    const auto tContiguousMap = tpi::tpetra::create_contiguous_map(tNodalCoordinates.size(), tCommunicator);

    constexpr bool tZeroOut = true;
    auto tMultiVector = tpi::tpetra::TpetraMultiVector(tContiguousMap, kNumSpatialDimensions, tZeroOut);

    tpi::tpetra::distribute_on_tpetra_multivector(tNodalCoordinates, tMultiVector);

    /// round robin assignment to ranks 0->3 of tNodalCoordinates above
    const tpi::common::Coordinate tGoldCoordinate{tThisRank + 1.0, tThisRank + 1.0, tThisRank + 1.0};

    {
        const std::vector<tpi::stk_search::SearchPointWithIdentifier> tGoldLocalSearchPointWithIdentifiers{
            tpi::stk_search::SearchPointWithIdentifier{tpi::stk_search::convert_coordinate(tGoldCoordinate),
                                                       tpi::stk_search::Identifier{tThisRank, tThisRank}}};

        const auto tSearchPoints = detail::stk_search_points(tMultiVector, tThisRank);

        ASSERT_EQ(tSearchPoints.size(), 1u);
        EXPECT_EQ(tGoldLocalSearchPointWithIdentifiers[0].first, tSearchPoints[0].first);
        EXPECT_EQ(tGoldLocalSearchPointWithIdentifiers[0].second, tSearchPoints[0].second);
    }
    {
        constexpr double tRadius = 2;
        const std::vector<tpi::stk_search::SearchSphereWithIdentifier> tGoldLocalSphereWithIdentifier{
            tpi::stk_search::SearchSphereWithIdentifier{
                tpi::stk_search::create_sphere(tGoldCoordinate, tpi::stk_search::STKRadius{tRadius}),
                tpi::stk_search::Identifier{tThisRank, tThisRank}}};
        const auto tSpheres = detail::stk_search_spheres(tMultiVector, SearchRadius{tRadius}, tThisRank);

        ASSERT_EQ(tSpheres.size(), 1U);
        EXPECT_EQ(tGoldLocalSphereWithIdentifier[0].first, tSpheres[0].first);
        EXPECT_EQ(tGoldLocalSphereWithIdentifier[0].second, tSpheres[0].second);
    }
}

TEST(LinearMaskBuilder, GenerateDistanceMapNodal)
{
    namespace tpit = third_party_integration::tpetra;
    const LinearMaskBuilder tLinearMaskBuilder = test_utilities::create_simple_linear_mask_builder();

    const auto tDistanceMap = tLinearMaskBuilder.mask();

    /* 2/3          1/3             0           0
       1/4         1/2            1/4        0
       0            1/4            1/2        1/4
       0            0               1/3         2/3*/

    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 0, 0), 2.0 / 3.0);
    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 0, 1), 1.0 / 3.0);
    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 0, 2), 0);
    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 0, 3), 0);

    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 1, 0), 1.0 / 4.0);
    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 1, 1), 1.0 / 2.0);
    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 1, 2), 1.0 / 4.0);
    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 1, 3), 0);

    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 2, 0), 0);
    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 2, 1), 1.0 / 4.0);
    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 2, 2), 1.0 / 2.0);
    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 2, 3), 1.0 / 4.0);

    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 3, 0), 0);
    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 3, 1), 0);
    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 3, 2), 1.0 / 3.0);
    EXPECT_DOUBLE_EQ(tpit::get_entry(tDistanceMap, 3, 3), 2.0 / 3.0);
}

TEST(LinearMaskBuilderDetail, DistributeSearchVectorsAndStkSearch)
{
    namespace tpi = third_party_integration;
    const auto tCommunicator = boost::mpi::communicator{};
    const auto tRank = tCommunicator.rank();

    const std::vector<tpi::common::Coordinate> tCoordinates{{1, 0, 0}, {2, 0, 0}, {3, 0, 0},
                                                            {4, 0, 0}, {5, 0, 0}, {6, 0, 0}};

    const double tSearchRadius = 2.1;

    const auto tSearchResults = detail::distribute_search_vectors_and_stk_search(
        CenterVector{tCoordinates}, NodalVector{tCoordinates}, tSearchRadius, tCommunicator);

    // Rank 0 gets Search Coordinate index 0, 1.
    // Index 0 has itself within 2, and index 1,2
    // Index 1 has itself within 2, and index 0, 2, 3
    if (tRank == 0)
    {
        EXPECT_EQ(tSearchResults.size(), 7U);
    }
    // Rank 1 gets Coordinate index 2, 3 which both have 5 inclusive neighbors
    if (tRank == 1)
    {
        EXPECT_EQ(tSearchResults.size(), 10U);
    }
    // Rank 2 gets Coordinate index 4 which has 4 inclusive neighbors
    if (tRank == 2)
    {
        ASSERT_EQ(tSearchResults.size(), 4U);
        EXPECT_EQ(tSearchResults[0].second.id(), 2U);
        EXPECT_EQ(tSearchResults[1].second.id(), 3U);
        EXPECT_EQ(tSearchResults[2].second.id(), 4U);
        EXPECT_EQ(tSearchResults[3].second.id(), 5U);
    }
    // Rank 3 gets Coordinate index 5 which has 3 inclusive neighbors
    if (tRank == 3)
    {
        EXPECT_EQ(tSearchResults.size(), 3U);
    }

    const auto tTotalSearchResults = detail::reduce_search_result_size(tSearchResults, tCommunicator);
    EXPECT_EQ(tTotalSearchResults, 24U);
}

}  // namespace plato::filter::extension::parallel_unittest
