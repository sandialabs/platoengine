#include <gtest/gtest.h>

#include <Tpetra_KokkosCompat_DefaultNode.hpp>
#include <boost/mpi/communicator.hpp>

#include "plato/third_party_integration/common/unittest/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/tpetra/Utilities.hpp"

namespace plato::third_party_integration::tpetra::unittest
{

namespace
{

constexpr auto kNumRanks = int{4};
constexpr int kNumSpatialDimensions{3};

[[nodiscard]] Teuchos::RCP<const TpetraMap> create_contiguous_map(const Tpetra::global_size_t aSize,
                                                                  Teuchos::RCP<const Teuchos::Comm<int>> aCommunicator)
{
    return Teuchos::rcp(new TpetraMap(aSize, kIndexBase, std::move(aCommunicator)));
}

}  // namespace

TEST(TpetraHelpers, MPISize)
{
    const auto tComm = boost::mpi::communicator{};
    EXPECT_EQ(tComm.size(), kNumRanks);
}

TEST(TpetraHelpers, PutStdVectorTPetraVector)
{
    const auto tCommunicator = Tpetra::getDefaultComm();

    const std::vector<double> tValues = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

    const auto tSize = tValues.size();
    const auto tContiguousMap = create_contiguous_map(tSize, tCommunicator);

    constexpr bool tZeroOut = true;
    auto tVector = TpetraVector(tContiguousMap, tZeroOut);
    distribute_on_tpetra_vector(tValues, tVector);

    for (TpetraLocalOrdinal tIndex = 0; tIndex < number_of_local_elements(tContiguousMap); ++tIndex)
    {
        const auto tGlobalInd = tContiguousMap->getGlobalElement(tIndex);
        const auto tResult = tVector.getData()[tIndex];
        EXPECT_EQ(tResult, tValues[tGlobalInd]);
    }

    const auto tReverseProcess = reduce_tpetra_vector(tVector, tCommunicator);

    for (TpetraLocalOrdinal tIndex = 0; tIndex < number_of_local_elements(tContiguousMap); ++tIndex)
    {
        const auto tGlobalInd = tContiguousMap->getGlobalElement(tIndex);
        EXPECT_EQ(tValues[tGlobalInd], tReverseProcess[tGlobalInd]);
    }
}

TEST(TpetraHelpers, PutStdVectorOfCoordOnTPetraMultiVector)
{
    const auto tCommunicator = Tpetra::getDefaultComm();
    // clang-format off
    const std::vector<common::Coordinate> tCoordinates = {{1, 11, 21},   
                                                  {2, 12, 22}, 
                                                  {3, 13, 23},
                                                  {4, 14, 24}, 
                                                  {5, 15, 25}, 
                                                  {6, 16, 26},
                                                  {7, 17, 27},
                                                  {8, 18, 28},
                                                  {9, 19, 29},
                                                  {10, 20, 30}
                                                  };
    // clang-format on
    const int tSize = tCoordinates.size();
    const auto tContiguousMap = create_contiguous_map(tSize, tCommunicator);

    constexpr bool tZeroOut = true;
    auto tPVector = TpetraMultiVector(tContiguousMap, kNumSpatialDimensions, tZeroOut);
    distribute_on_tpetra_multivector(tCoordinates, tPVector);

    for (TpetraLocalOrdinal tIndex = 0; tIndex < number_of_local_elements(tContiguousMap); ++tIndex)
    {
        const auto tGlobalInd = tContiguousMap->getGlobalElement(tIndex);
        const auto tCoordinate = multivector_coordinate(tPVector, tIndex);
        common::unittest::test_double_equality_of_components(tCoordinate, tCoordinates[tGlobalInd],
                                                             TEST_CONTEXT("Get multivector Coordinate"));
    }
}

TEST(TpetraHelpers, CreateZeroedRowAndColumnVectorsFromCRSMatrix)
{
    const auto tCommunicator = Tpetra::getDefaultComm();
    constexpr int tNumberOfRows = 4;
    constexpr int tNumberOfColumns = 2;
    const auto tCrsRowMap = Teuchos::rcp(new TpetraMap(tNumberOfColumns, 0, tCommunicator));
    const auto tCrsDomainMap = Teuchos::rcp(new TpetraMap(tNumberOfRows, 0, tCommunicator));

    TpetraCRSMatrix tMatrix(tCrsRowMap, 3);
    tMatrix.fillComplete(tCrsDomainMap, tCrsRowMap);

    const auto [tRow, tColumn] = create_zeroed_row_and_column_vectors_from_crs_map(tMatrix, tCommunicator);

    EXPECT_EQ(tRow.getMap()->getGlobalNumElements(), tNumberOfColumns);
    EXPECT_EQ(tColumn.getMap()->getGlobalNumElements(), tNumberOfRows);

    EXPECT_EQ(tColumn.dot(tColumn), 0);
    EXPECT_EQ(tRow.dot(tRow), 0);
}

TEST(TpetraHelpers, GetNumberOfLocalElements)
{
    const auto tCommunicator = Tpetra::getDefaultComm();
    const Tpetra::global_size_t tMapSize = 8;
    const auto tContiguousMap = create_contiguous_map(tMapSize, tCommunicator);
    const TpetraLocalOrdinal tGold{tMapSize / kNumRanks};

    EXPECT_EQ(number_of_local_elements(tContiguousMap), tGold);
}

}  // namespace plato::third_party_integration::tpetra::unittest
