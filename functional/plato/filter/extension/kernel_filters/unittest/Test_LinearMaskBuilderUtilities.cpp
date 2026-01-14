#include <gtest/gtest.h>

#include "plato/filter/extension/kernel_filters/LinearMaskBuilderUtilities.hpp"

namespace plato::filter::extension::kernel_filters::unittest
{

TEST(LinearMaskBuilderUtilitiesDetail, NormalizeVector)
{
    const std::vector<double> tVector{1, 2, 3};
    auto tResult = tVector;
    {
        constexpr double tNormalization = 3;

        detail::normalize_vector(tResult, tNormalization);
        EXPECT_EQ(tResult[0], tVector[0] / tNormalization);
        EXPECT_EQ(tResult[1], tVector[1] / tNormalization);
        EXPECT_EQ(tResult[2], tVector[2] / tNormalization);
    }
}

TEST(LinearMaskBuilderUtilitiesDetail, NormalizeRowsInMap)
{
    using TpetraGlobalOrdinal = third_party_integration::tpetra::TpetraGlobalOrdinal;
    const TpetraGlobalOrdinal tSphereIDOne{1};
    const RowDetail tRowDetailOne{
        .mNonzeroColumnGlobalIDs = {TpetraGlobalOrdinal{1}, TpetraGlobalOrdinal{2}, TpetraGlobalOrdinal{3}},
        .mColumnEntryWeights = {1, 1, 1},
        .mRowSum = 3};
    const TpetraGlobalOrdinal tSphereIDTwo{2};
    const RowDetail tRowDetailTwo{.mNonzeroColumnGlobalIDs = {TpetraGlobalOrdinal{1}, TpetraGlobalOrdinal{3}},
                                  .mColumnEntryWeights = {.25, .25},
                                  .mRowSum = .5};

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
}

namespace
{
const third_party_integration::tpetra::TpetraGlobalOrdinal kSphereID{1};
[[nodiscard]] auto create_test_row_map() -> RowMap
{
    using TpetraGlobalOrdinal = third_party_integration::tpetra::TpetraGlobalOrdinal;
    const RowDetail tRowDetail{
        .mNonzeroColumnGlobalIDs = {TpetraGlobalOrdinal{1}, TpetraGlobalOrdinal{2}, TpetraGlobalOrdinal{3}},
        .mColumnEntryWeights = {1, 1, 1},
        .mRowSum = 3};
    RowMap tRowMap;
    tRowMap[kSphereID] = tRowDetail;
    return tRowMap;
}

}  // namespace

TEST(LinearMaskBuilderUtilitiesDetail, AddWeightFromSearchResultToMap)
{
    using TpetraGlobalOrdinal = third_party_integration::tpetra::TpetraGlobalOrdinal;
    auto tRowMap = create_test_row_map();

    constexpr double tWeight = 0.4;
    const TpetraGlobalOrdinal tID = 4;
    const unsigned int tMaxSize = 4;
    {
        detail::add_weight_from_search_result_to_map(tRowMap, RowSearchEntityGlobalID{kSphereID},
                                                     ColumnNodeGlobalID{tID}, Weight{tWeight}, tMaxSize);

        ASSERT_EQ(tRowMap[kSphereID].mColumnEntryWeights.size(), 4u);
        EXPECT_EQ(tRowMap[kSphereID].mColumnEntryWeights[3], tWeight);
        ASSERT_EQ(tRowMap[kSphereID].mNonzeroColumnGlobalIDs.size(), 4u);
        EXPECT_EQ(tRowMap[kSphereID].mNonzeroColumnGlobalIDs[3], tID);
        EXPECT_EQ(tRowMap[kSphereID].mColumnEntryWeights.capacity(), tMaxSize);
        EXPECT_EQ(tRowMap[kSphereID].mNonzeroColumnGlobalIDs.capacity(), tMaxSize);
    }
    {
        const TpetraGlobalOrdinal tSphereIDTwo{2};
        detail::add_weight_from_search_result_to_map(tRowMap, RowSearchEntityGlobalID{tSphereIDTwo},
                                                     ColumnNodeGlobalID{tID}, Weight{tWeight}, tMaxSize);

        ASSERT_EQ(tRowMap[tSphereIDTwo].mColumnEntryWeights.size(), 1u);
        EXPECT_EQ(tRowMap[tSphereIDTwo].mColumnEntryWeights[0], tWeight);
        ASSERT_EQ(tRowMap[tSphereIDTwo].mNonzeroColumnGlobalIDs.size(), 1u);
        EXPECT_EQ(tRowMap[tSphereIDTwo].mNonzeroColumnGlobalIDs[0], tID);
        EXPECT_EQ(tRowMap[tSphereIDTwo].mColumnEntryWeights.capacity(), tMaxSize);
        EXPECT_EQ(tRowMap[tSphereIDTwo].mNonzeroColumnGlobalIDs.capacity(), tMaxSize);
    }
}

TEST(LinearMaskBuilderUtilitiesDetail, AddWeightFromSearchResultToMapDuplicateGlobalIDs)
{
    using TpetraGlobalOrdinal = third_party_integration::tpetra::TpetraGlobalOrdinal;
    auto tRowMap = create_test_row_map();
    constexpr double tWeight = 10;
    const TpetraGlobalOrdinal tRepeatedID = 1;
    const unsigned int tMaxSize = 3;
    {
        detail::add_weight_from_search_result_to_map(tRowMap, RowSearchEntityGlobalID{kSphereID},
                                                     ColumnNodeGlobalID{tRepeatedID}, Weight{tWeight}, tMaxSize);

        EXPECT_EQ(tRowMap[kSphereID].mColumnEntryWeights.size(), 3u);
        EXPECT_EQ(tRowMap[kSphereID].mRowSum, 3);
    }
}

TEST(LinearMaskBuilderUtilitiesDetail, NumberOfColumnEntriesInRowMap)
{
    namespace tpitp = third_party_integration::tpetra;

    const auto tRowDetailWithOneColumn =
        RowDetail{.mNonzeroColumnGlobalIDs = {0}, .mColumnEntryWeights = {0.0}, .mRowSum = 0.0};
    const auto tRowDetailWithTwoColumns =
        RowDetail{.mNonzeroColumnGlobalIDs = {0, 1}, .mColumnEntryWeights = {0.0, 0.0}, .mRowSum = 0.0};
    const auto tRowDetailWithThreeColumns =
        RowDetail{.mNonzeroColumnGlobalIDs = {0, 1, 2}, .mColumnEntryWeights = {0.0, 0.0, 0.0}, .mRowSum = 0.0};

    const auto tTestRowMap = RowMap{{tpitp::TpetraGlobalOrdinal{0}, tRowDetailWithOneColumn},
                                    {tpitp::TpetraGlobalOrdinal{1}, tRowDetailWithTwoColumns},
                                    {tpitp::TpetraGlobalOrdinal{2}, tRowDetailWithThreeColumns},
                                    {tpitp::TpetraGlobalOrdinal{3}, tRowDetailWithOneColumn}};

    const auto tComputed = detail::number_of_column_entries_per_row(tTestRowMap);
    const auto tExpected = std::vector<std::size_t>{1, 2, 3, 1};
    EXPECT_EQ(tComputed, tExpected);
}

}  // namespace plato::filter::extension::kernel_filters::unittest
