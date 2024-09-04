#include <gtest/gtest.h>

#include <boost/math/constants/constants.hpp>

#include "plato/filter/extension/LinearMaskBuilder.hpp"
#include "plato/filter/extension/test_utilities/LinearMaskTestUtility.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/IOUtilities.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"
#include "plato/third_party_integration/tpetra/TestUtilities.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::filter::extension::unittest
{
namespace
{
constexpr std::string_view kMeshFile = "mesh.exo";
}

TEST(LinearMaskBuilderDetail, FilterVolume)
{
    constexpr double tRadius = 1.23;
    const double tResult = detail::filter_volume(SearchRadius{tRadius});
    const double tGold = boost::math::constants::pi<double>() * 4.0 / 3.0 * tRadius * tRadius * tRadius;
    EXPECT_DOUBLE_EQ(tResult, tGold);
}

TEST(LinearMaskBuilderDetail, FilterArea)
{
    constexpr double tRadius = 1.23;
    const double tResult = detail::filter_area(SearchRadius{tRadius});
    const double tGold = boost::math::constants::pi<double>() * tRadius * tRadius;
    EXPECT_DOUBLE_EQ(tResult, tGold);
}

TEST(LinearMaskBuilderDetail, DetermineMaximumConnectivityEstimate)
{
    const third_party_integration::stk_io::CommandGenerator tCommandGenerator{
        {21, 21, 21}, {-10, -10, -10}, {10, 10, 10}};
    third_party_integration::stk_io::write_mesh(kMeshFile, tCommandGenerator);

    const SearchRadius tFilterRadius{5};

    const double tNodalDensity = tCommandGenerator.numberOfNodes() / tCommandGenerator.volume();
    const double tAverageElementVolume = tCommandGenerator.volume() / tCommandGenerator.numberOfElements();
    const double tSearchVolume = detail::filter_volume(tFilterRadius);
    const double tSmallestElementVolume = std::pow(20.0 / 21., 3);

    const double tRatioAverageToSmall = tAverageElementVolume / tSmallestElementVolume;
    const int tGold = static_cast<int>(tNodalDensity * tSearchVolume * detail::kMaxMultiplier * tRatioAverageToSmall);

    const int tResult = detail::maximum_connectivity_estimate(mesh::Mesh{kMeshFile}, tFilterRadius);
    EXPECT_EQ(tGold, tResult);

    constexpr double tNumberOfActualNodes = 515;  // matlab
    EXPECT_GT(tResult, tNumberOfActualNodes);

    plato::test_utilities::test_for_existence_and_remove({kMeshFile}, TEST_CONTEXT("Removing temporary files."));
}

TEST(LinearMaskBuilderDetail, AddWeightFromSearchResultToMap)
{
    using TGO = third_party_integration::tpetra::TpetraGlobalOrdinal;
    const TGO tSphereID{1};
    const RowDetail tRowDetail{{TGO{1}, TGO{2}, TGO{3}}, {1, 1, 1}, 3};
    constexpr double tWeight = 0.4;
    const TGO tID = 4;
    RowMap tRowMap;
    tRowMap[tSphereID] = tRowDetail;
    const unsigned int tMaxSize = 4;
    {
        detail::add_weight_from_search_result_to_map(tRowMap, RowSphereGlobalID{tSphereID}, ColumnNodeGlobalID{tID},
                                                     Weight{tWeight}, tMaxSize);

        ASSERT_EQ(tRowMap[tSphereID].mColumnEntryWeights.size(), 4u);
        EXPECT_EQ(tRowMap[tSphereID].mColumnEntryWeights[3], tWeight);
        ASSERT_EQ(tRowMap[tSphereID].mNonzeroColumnGlobalIDs.size(), 4u);
        EXPECT_EQ(tRowMap[tSphereID].mNonzeroColumnGlobalIDs[3], tID);
        EXPECT_EQ(tRowMap[tSphereID].mColumnEntryWeights.capacity(), tMaxSize);
        EXPECT_EQ(tRowMap[tSphereID].mNonzeroColumnGlobalIDs.capacity(), tMaxSize);
    }
    {
        const TGO tSphereIDTwo{2};
        detail::add_weight_from_search_result_to_map(tRowMap, RowSphereGlobalID{tSphereIDTwo}, ColumnNodeGlobalID{tID},
                                                     Weight{tWeight}, tMaxSize);

        ASSERT_EQ(tRowMap[tSphereIDTwo].mColumnEntryWeights.size(), 1u);
        EXPECT_EQ(tRowMap[tSphereIDTwo].mColumnEntryWeights[0], tWeight);
        ASSERT_EQ(tRowMap[tSphereIDTwo].mNonzeroColumnGlobalIDs.size(), 1u);
        EXPECT_EQ(tRowMap[tSphereIDTwo].mNonzeroColumnGlobalIDs[0], tID);
        EXPECT_EQ(tRowMap[tSphereIDTwo].mColumnEntryWeights.capacity(), tMaxSize);
        EXPECT_EQ(tRowMap[tSphereIDTwo].mNonzeroColumnGlobalIDs.capacity(), tMaxSize);
    }
}

TEST(LinearMaskBuilderDetail, NormalizeRowsInMap)
{
    using TGO = third_party_integration::tpetra::TpetraGlobalOrdinal;
    const TGO tSphereIDOne{1};
    const RowDetail tRowDetailOne{{TGO{1}, TGO{2}, TGO{3}}, {1, 1, 1}, 3};
    const TGO tSphereIDTwo{2};
    const RowDetail tRowDetailTwo{{TGO{1}, TGO{3}}, {.25, .25}, .5};
    const TGO tSphereIDThree{3};
    const RowDetail tRowDetailThree{{}, {}, 0};

    RowMap tRowMap;
    tRowMap[tSphereIDOne] = tRowDetailOne;
    tRowMap[tSphereIDTwo] = tRowDetailTwo;
    {
        detail::normalize_rows_in_map(tRowMap);
        const std::vector<double> tGoldOne{1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0};
        const std::vector<double> tGoldTwo{1.0 / 2.0, 1.0 / 2.0};
        EXPECT_EQ(tRowMap[tSphereIDOne].mColumnEntryWeights, tGoldOne);
        EXPECT_EQ(tRowMap[tSphereIDTwo].mColumnEntryWeights, tGoldTwo);
    }
    {
        tRowMap[tSphereIDThree] = tRowDetailThree;
        EXPECT_THROW(detail::normalize_rows_in_map(tRowMap), utilities::Exception);
    }
}

TEST(LinearMaskBuilder, GenerateDistanceMapGivenCentroid)
{
    const std::vector<third_party_integration::common::Coordinate> tRelativeToCoordinate{{1, 0, 0}};
    const LinearMaskBuilder tLinearMaskBuilder =
        test_utilities::create_simple_linear_mask_builder(tRelativeToCoordinate);

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

TEST(LinearMaskBuilder, RadiusTooSmallError)
{
    namespace tpic = third_party_integration::common;
    const auto tCoordinates = std::vector<tpic::Coordinate>{{0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {3, 0, 0}};
    const auto tElemCent = std::vector<tpic::Coordinate>{{0.5, 0, 0}, {1.5, 0, 0}, {2.5, 0, 0}};

    EXPECT_THROW([[maybe_unused]] const auto tUnused =
                     LinearMaskBuilder(NodalVector{tCoordinates}, CenterVector{tElemCent}, SearchRadius{.10},
                                       test_utilities::kMaximumConnectivity, boost::mpi::communicator{}),
                 utilities::Exception);
}

TEST(LinearMaskBuilderDetail, NormalizeVector)
{
    const std::vector<double> tVector{1, 2, 3};
    auto tResult = tVector;
    EXPECT_THROW(detail::normalize_vector(tResult, 0), utilities::Exception);
    {
        constexpr double tNormalization = 3;

        detail::normalize_vector(tResult, tNormalization);
        EXPECT_EQ(tResult[0], tVector[0] / tNormalization);
        EXPECT_EQ(tResult[1], tVector[1] / tNormalization);
        EXPECT_EQ(tResult[2], tVector[2] / tNormalization);
    }
}

}  // namespace plato::filter::extension::unittest
