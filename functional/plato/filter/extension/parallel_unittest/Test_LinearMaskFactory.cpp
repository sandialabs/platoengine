#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <optional>

#include "plato/filter/extension/LinearMaskFactory.hpp"
#include "plato/filter/extension/parallel_unittest/LinearMaskTestUtility.hpp"
#include "plato/third_party_integration/common/unittest/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"
#include "plato/third_party_integration/stk_search/Utilities.hpp"

namespace plato::filter::extension::unittest
{

namespace
{

constexpr auto kNumRanks = int{4};
constexpr int kNumSpatialDimensions{3};

[[nodiscard]] Teuchos::RCP<const third_party_integration::tpetra::TpetraMap> create_contiguous_map(
    const Tpetra::global_size_t aSize, Teuchos::RCP<const Teuchos::Comm<int>> aCommunicator)
{
    return Teuchos::rcp(new third_party_integration::tpetra::TpetraMap(
        aSize, third_party_integration::tpetra::kIndexBase, std::move(aCommunicator)));
}

[[nodiscard]] third_party_integration::tpetra::TpetraVector create_projection_vector(
    const size_t aSize, const third_party_integration::tpetra::TpetraGlobalOrdinal aGlobalIndex)
{
    const auto tCommunicator = Tpetra::getDefaultComm();
    const auto tContiguousMap = create_contiguous_map(aSize, tCommunicator);
    constexpr bool tZeroOut = true;
    auto tVector = third_party_integration::tpetra::TpetraVector(tContiguousMap, tZeroOut);
    tVector.replaceGlobalValue(aGlobalIndex, 1);
    return tVector;
}

[[nodiscard]] third_party_integration::tpetra::TpetraScalar get_entry(
    const third_party_integration::tpetra::TpetraCRSMatrix& aMatrix,
    const third_party_integration::tpetra::TpetraGlobalOrdinal aGlobalIndexI,
    const third_party_integration::tpetra::TpetraGlobalOrdinal aGlobalIndexJ)
{
    const third_party_integration::tpetra::TpetraGlobalOrdinal tNRows = aMatrix.getGlobalNumRows();
    const third_party_integration::tpetra::TpetraGlobalOrdinal tMColumns = aMatrix.getGlobalNumCols();
    const auto tCommunicator = Tpetra::getDefaultComm();
    const auto tContiguousMap = create_contiguous_map(tNRows, tCommunicator);
    constexpr bool tZeroOut = true;
    auto tResult = third_party_integration::tpetra::TpetraVector(tContiguousMap, tZeroOut);

    const auto tProjectionI = create_projection_vector(tNRows, aGlobalIndexI);
    const auto tProjectionJ = create_projection_vector(tMColumns, aGlobalIndexJ);
    aMatrix.apply(tProjectionJ, tResult);
    return tProjectionI.dot(tResult);
}

}  // namespace

TEST(LinearMaskFactoryDetail, MPISize)
{
    const auto tComm = boost::mpi::communicator{};
    EXPECT_EQ(tComm.size(), kNumRanks);
}

TEST(LinearMaskFactoryDetail, LinearRamp)
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

TEST(LinearMaskFactoryDetail, ReturnNormalizedNonzeroWeights)
{
    const auto tCommunicator = Tpetra::getDefaultComm();
    const Tpetra::global_size_t tMapSize = 8;
    const int tEstimatedConnectivity = tMapSize / kNumRanks;
    const auto tContiguousMap = create_contiguous_map(tMapSize, tCommunicator);
    constexpr bool tZeroOut = true;
    third_party_integration::tpetra::TpetraVector tVector =
        third_party_integration::tpetra::TpetraVector(tContiguousMap, tZeroOut);
    constexpr double tRowSum = tMapSize * 1.0;
    {
        const auto [tGlobalNonZeroIndices, tLocalNonZeroWeights] = detail::normalize_nonzero_weights(
            tVector, detail::RowSum{tRowSum}, detail::EstimatedConnectivity{tEstimatedConnectivity});
        EXPECT_EQ(tGlobalNonZeroIndices.size(), tLocalNonZeroWeights.size());
        EXPECT_EQ(tGlobalNonZeroIndices.size(), 0u);
    }
    tVector.putScalar(1.0);
    {
        const auto [tGlobalNonZeroIndices, tLocalNonZeroWeights] = detail::normalize_nonzero_weights(
            tVector, detail::RowSum{tRowSum}, detail::EstimatedConnectivity{tEstimatedConnectivity});
        ASSERT_EQ(tGlobalNonZeroIndices.size(), tMapSize / kNumRanks);
        EXPECT_DOUBLE_EQ(tLocalNonZeroWeights[0], 1.0 / tRowSum);
        EXPECT_DOUBLE_EQ(tLocalNonZeroWeights[1], 1.0 / tRowSum);
    }
}

TEST(LinearMaskFactoryDetail, MakeSearchPointsWithIdentifiers)
{
    const auto tCommunicator = Tpetra::getDefaultComm();
    const auto tThisRank = tCommunicator->getRank();

    const std::vector<third_party_integration::common::Coordinate> tNodalCoordinates{
        {1, 1, 1}, {2, 2, 2}, {3, 3, 3}, {4, 4, 4}};  // non-zero to ensure not default ctor
    const auto tContiguousMap = create_contiguous_map(tNodalCoordinates.size(), tCommunicator);

    constexpr bool tZeroOut = true;
    auto tMultiVector =
        third_party_integration::tpetra::TpetraMultiVector(tContiguousMap, kNumSpatialDimensions, tZeroOut);

    third_party_integration::tpetra::distribute_on_tpetra_multivector(tNodalCoordinates, tMultiVector);

    /// round robin assignment to ranks 0->3 of tNodalCoordinates above
    const third_party_integration::common::Coordinate tGoldCoordinate{tThisRank + 1.0, tThisRank + 1.0,
                                                                      tThisRank + 1.0};
    const std::vector<third_party_integration::stk_search::SearchPointWithIdentifier>
        tGoldLocalSearchPointWithIdentifiers{third_party_integration::stk_search::SearchPointWithIdentifier{
            third_party_integration::stk_search::convert_coordinate(tGoldCoordinate),
            third_party_integration::stk_search::Identifier{0, tThisRank}}};

    const auto tSearchPoints = detail::stk_search_points(tMultiVector, tThisRank);

    ASSERT_EQ(tSearchPoints.size(), 1u);
    EXPECT_EQ(tGoldLocalSearchPointWithIdentifiers[0].first, tSearchPoints[0].first);
    EXPECT_EQ(tGoldLocalSearchPointWithIdentifiers[0].second, tSearchPoints[0].second);
}

TEST(LinearMaskFactory, GenerateDistanceMapNodal)
{
    const LinearMaskFactory tLinearMaskFactory = create_simple_linear_mask<LinearMaskFactory>(std::nullopt);

    const auto tDistanceMap = tLinearMaskFactory.returnMask();

    /* 2/3          1/3             0           0
       1/4         1/2            1/4        0
       0            1/4            1/2        1/4
       0            0               1/3         2/3*/

    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 0, 0), 2.0 / 3.0);
    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 0, 1), 1.0 / 3.0);
    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 0, 2), 0);
    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 0, 3), 0);

    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 1, 0), 1.0 / 4.0);
    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 1, 1), 1.0 / 2.0);
    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 1, 2), 1.0 / 4.0);
    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 1, 3), 0);

    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 2, 0), 0);
    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 2, 1), 1.0 / 4.0);
    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 2, 2), 1.0 / 2.0);
    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 2, 3), 1.0 / 4.0);

    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 3, 0), 0);
    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 3, 1), 0);
    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 3, 2), 1.0 / 3.0);
    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 3, 3), 2.0 / 3.0);
}

TEST(LinearMaskFactory, GenerateDistanceMapGivenCentroid)
{
    const std::vector<third_party_integration::common::Coordinate> tRelativeToCoordinate{{1, 0, 0}};
    const LinearMaskFactory tLinearMaskFactory = create_simple_linear_mask<LinearMaskFactory>(tRelativeToCoordinate);

    const auto tDistanceMap = tLinearMaskFactory.returnMask();
    /*
           1/4         1/2            1/4        0
    */

    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 0, 0), 1.0 / 4.0);
    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 0, 1), 1.0 / 2.0);
    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 0, 2), 1.0 / 4.0);
    EXPECT_DOUBLE_EQ(get_entry(tDistanceMap, 0, 3), 0);
}

}  // namespace plato::filter::extension::unittest
