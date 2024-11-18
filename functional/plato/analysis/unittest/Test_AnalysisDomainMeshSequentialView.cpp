#include <gtest/gtest.h>

#include <boost/core/ignore_unused.hpp>
#include <numeric>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshOperators.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/NamedType.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::analysis::unittest
{
namespace
{
const auto kScalarField1 = std::vector{1.0, 0.0, 4.0};
const auto kIDs1 = std::vector<std::size_t>{0, 1, 2};
const auto kBlockScalarFieldVector1 = AnalysisDomainMesh::ScalarFieldVector{{kIDs1[0], kIDs1[0], kScalarField1[0]},
                                                                            {kIDs1[1], kIDs1[1], kScalarField1[1]},
                                                                            {kIDs1[2], kIDs1[2], kScalarField1[2]}};

const auto kScalarField2 = std::vector{-1.0, 10.0};
const auto kIDs2 = std::vector<std::size_t>{0, 1};
const auto kBlockScalarFieldVector2WithOverlap = AnalysisDomainMesh::ScalarFieldVector{
    {kIDs2[0], kIDs2[0], kScalarField2[0]}, {kIDs2[1], kIDs2[1], kScalarField2[1]}};

const auto kScalarField3 = std::vector{-2.0, 11.0};
const auto kIDs3 = std::vector<std::size_t>{3, 4};
const auto kBlockScalarFieldVector2NoOverlap = AnalysisDomainMesh::ScalarFieldVector{
    {kIDs3[0], kIDs3[0], kScalarField3[0]}, {kIDs3[1], kIDs3[1], kScalarField3[1]}};

const auto kSingleBlockScalarField = AnalysisDomainMesh::BlockScalarField{{0, kBlockScalarFieldVector1}};
const auto kTwoBlockScalarFieldOverlap =
    AnalysisDomainMesh::BlockScalarField{{0, kBlockScalarFieldVector1}, {1, kBlockScalarFieldVector2WithOverlap}};

const auto kTwoBlockScalarFieldNoOverlap =
    AnalysisDomainMesh::BlockScalarField{{0, kBlockScalarFieldVector1}, {1, kBlockScalarFieldVector2NoOverlap}};

using NewScalarField = utilities::NamedType<std::vector<ScalarFieldValue>, struct NewScalarFieldTag>;
using Block1ScalarField = utilities::NamedType<std::vector<ScalarFieldValue>, struct NewScalarFieldTag>;
using Block2ScalarField = utilities::NamedType<std::vector<ScalarFieldValue>, struct NewScalarFieldTag>;

void check_mutable_view(AnalysisDomainMesh& aAnalysisDomainMesh,
                        const NewScalarField& aNewScalarField,
                        const Block1ScalarField& aExpectedScalarFieldBlock1,
                        const Block2ScalarField& aExpectedScalarFieldBlock2,
                        const plato::test_utilities::TestContext& aTestContext)
{
    const auto tMeshView = AnalysisDomainMeshMutableSequentialView{aAnalysisDomainMesh};
    std::copy(aNewScalarField.mValue.cbegin(), aNewScalarField.mValue.cend(), tMeshView.begin());
    EXPECT_EQ(aExpectedScalarFieldBlock1.mValue, aAnalysisDomainMesh.mBlockScalarField.at(0)) << aTestContext;
    EXPECT_EQ(aExpectedScalarFieldBlock2.mValue, aAnalysisDomainMesh.mBlockScalarField.at(1)) << aTestContext;
}
}  // namespace

TEST(AnalysisDomainMeshViews, SizeSingleBlock)
{
    const auto tAnalysisDomainMesh =
        AnalysisDomainMesh{/*.mFileName=*/"mars.exo", /*.mBlockScalarField=*/kSingleBlockScalarField};
    ASSERT_EQ(AnalysisDomainMeshSequentialView{tAnalysisDomainMesh}.size(), kScalarField1.size());
}

TEST(AnalysisDomainMeshViews, SizeTwoBlockOverlap)
{
    const auto tAnalysisDomainMesh =
        AnalysisDomainMesh{/*.mFileName=*/"ceres.exo", /*.mBlockScalarField=*/kTwoBlockScalarFieldOverlap};
    ASSERT_EQ(AnalysisDomainMeshSequentialView{tAnalysisDomainMesh}.size(), kScalarField1.size());
}

TEST(AnalysisDomainMeshViews, SizeTwoBlockNoOverlap)
{
    const auto tAnalysisDomainMesh =
        AnalysisDomainMesh{/*.mFileName=*/"io.exo", /*.mBlockScalarField=*/kTwoBlockScalarFieldNoOverlap};
    ASSERT_EQ(AnalysisDomainMeshSequentialView{tAnalysisDomainMesh}.size(),
              kScalarField1.size() + kScalarField2.size());
}

TEST(AnalysisDomainMeshViews, BeginEnd)
{
    const auto tAnalysisDomainMesh =
        AnalysisDomainMesh{/*.mFileName=*/"jupiter.exo", /*.mBlockScalarField=*/kSingleBlockScalarField};
    const auto tMeshView = AnalysisDomainMeshSequentialView{tAnalysisDomainMesh};

    const auto tBeginIterator = tMeshView.begin();
    constexpr auto tExpectedIteratorVectorSize = 1U;
    ASSERT_EQ(tBeginIterator.mCurrentIterators.size(), tExpectedIteratorVectorSize);
    EXPECT_EQ(tBeginIterator.mCurrentIterators.front(),
              tAnalysisDomainMesh.mBlockScalarField.cbegin()->second.cbegin());
    ASSERT_EQ(tBeginIterator.mEndIterators.size(), tExpectedIteratorVectorSize);
    EXPECT_EQ(tBeginIterator.mEndIterators.front(), tAnalysisDomainMesh.mBlockScalarField.cbegin()->second.cend());

    const auto tEndIterator = tMeshView.end();
    ASSERT_EQ(tEndIterator.mCurrentIterators.size(), tExpectedIteratorVectorSize);
    EXPECT_EQ(tEndIterator.mCurrentIterators.front(), tAnalysisDomainMesh.mBlockScalarField.cbegin()->second.cend());
    ASSERT_EQ(tEndIterator.mEndIterators.size(), tExpectedIteratorVectorSize);
    EXPECT_EQ(tEndIterator.mEndIterators.front(), tAnalysisDomainMesh.mBlockScalarField.cbegin()->second.cend());

    // Increment begin until it's at the end and check that they're equal
    auto tIncrementedIterator = tMeshView.begin();
    for (const auto tScalarField : kScalarField1)
    {
        boost::ignore_unused(tScalarField);
        ++tIncrementedIterator;
    }
    EXPECT_EQ(tIncrementedIterator, tEndIterator);
}

TEST(AnalysisDomainMeshViews, RangeBasedFor)
{
    const auto tCheckScalarField = [](const AnalysisDomainMeshSequentialView aMeshView,
                                      const std::vector<ScalarFieldValue>& aExpectedScalarField,
                                      const test_utilities::TestContext& aTestContext)
    {
        ASSERT_EQ(aExpectedScalarField.size(), aMeshView.size());
        auto tExpectedScalarFieldIterator = aExpectedScalarField.cbegin();
        for (const ScalarFieldValue tScalarFieldValue : aMeshView)
        {
            EXPECT_EQ(tScalarFieldValue.mValue, tExpectedScalarFieldIterator->mValue) << aTestContext;
            EXPECT_EQ(tScalarFieldValue.mGlobalMeshEntityID, tExpectedScalarFieldIterator->mGlobalMeshEntityID)
                << aTestContext;
            ++tExpectedScalarFieldIterator;
        }
    };

    // No overlap
    {
        const auto tAnalysisDomainMesh =
            AnalysisDomainMesh{/*.mFileName=*/"pluto.exo", /*.mBlockScalarField=*/kTwoBlockScalarFieldNoOverlap};
        const auto tMeshView = AnalysisDomainMeshSequentialView{tAnalysisDomainMesh};

        auto tExpectedScalarField = kBlockScalarFieldVector1;
        std::copy(kBlockScalarFieldVector2NoOverlap.cbegin(), kBlockScalarFieldVector2NoOverlap.cend(),
                  std::back_inserter(tExpectedScalarField));
        tCheckScalarField(tMeshView, tExpectedScalarField, TEST_CONTEXT("No overlap"));
    }
    // Overlap
    {
        const auto tAnalysisDomainMesh =
            AnalysisDomainMesh{/*.mFileName=*/"deimos.exo", /*.mBlockScalarField=*/kTwoBlockScalarFieldOverlap};
        const auto tMeshView = AnalysisDomainMeshSequentialView{tAnalysisDomainMesh};

        auto tExpectedScalarField = kBlockScalarFieldVector1;
        tCheckScalarField(tMeshView, tExpectedScalarField, TEST_CONTEXT("Full overlap"));
    }
    // One block
    {
        const auto tAnalysisDomainMesh =
            AnalysisDomainMesh{/*.mFileName=*/"venus.exo", /*.mBlockScalarField=*/kSingleBlockScalarField};
        const auto tMeshView = AnalysisDomainMeshSequentialView{tAnalysisDomainMesh};

        auto tExpectedScalarField = kBlockScalarFieldVector1;
        tCheckScalarField(tMeshView, tExpectedScalarField, TEST_CONTEXT("One block"));
    }
}

TEST(AnalysisDomainMeshViews, MutableView)
{
    auto tAnalysisDomainMesh =
        AnalysisDomainMesh{/*.mFileName=*/"uranus.exo", /*.mBlockScalarField=*/kSingleBlockScalarField};

    static_assert(!kIsConstIterator<std::vector<double>::iterator>);
    static_assert(kIsConstIterator<std::vector<double>::const_iterator>);

    const auto tMeshView = AnalysisDomainMeshMutableSequentialView{tAnalysisDomainMesh};

    const auto tNewScalarField =
        std::vector{ScalarFieldValue{0, 0, 0.0}, ScalarFieldValue{1, 1, 1.0}, ScalarFieldValue{2, 2, 2.0}};
    std::copy(tNewScalarField.cbegin(), tNewScalarField.cend(), tMeshView.begin());

    const auto tVectorFromView = mesh_analysis_to_vector(AnalysisDomainMeshSequentialView{tAnalysisDomainMesh});
    EXPECT_EQ(tVectorFromView, tNewScalarField);
}

TEST(AnalysisDomainMeshViews, MutableViewTwoBlocksNoOverlap)
{
    auto tAnalysisDomainMesh = AnalysisDomainMesh{/*.mFileName*/ "mercury.exo", kTwoBlockScalarFieldNoOverlap};
    const auto tNewScalarField =
        std::vector<ScalarFieldValue>{{0, 0, 0.0}, {1, 1, 1.0}, {2, 2, 2.0}, {3, 3, 3.0}, {4, 4, 4.0}};
    const auto tExpectedScalarFieldBlock1 = std::vector<ScalarFieldValue>{{0, 0, 0.0}, {1, 1, 1.0}, {2, 2, 2.0}};
    const auto tExpectedScalarFieldBlock2 = std::vector<ScalarFieldValue>{{3, 3, 3.0}, {4, 4, 4.0}};
    check_mutable_view(tAnalysisDomainMesh, NewScalarField{tNewScalarField},
                       Block1ScalarField{tExpectedScalarFieldBlock1}, Block2ScalarField{tExpectedScalarFieldBlock2},
                       TEST_CONTEXT("Blocks do not overlap"));
}

TEST(AnalysisDomainMeshViews, MutableViewTwoBlocksWithOverlap)
{
    auto tAnalysisDomainMesh = AnalysisDomainMesh{/*.mFileName*/ "earth.exo", kTwoBlockScalarFieldOverlap};
    const auto tNewScalarField = std::vector<ScalarFieldValue>{{0, 0, 0.0}, {1, 1, 1.0}, {2, 2, 2.0}};
    const auto tExpectedScalarFieldBlock1 = std::vector<ScalarFieldValue>{{0, 0, 0.0}, {1, 1, 1.0}, {2, 2, 2.0}};
    const auto tExpectedScalarFieldBlock2 = std::vector<ScalarFieldValue>{{0, 0, 0.0}, {1, 1, 1.0}};
    check_mutable_view(tAnalysisDomainMesh, NewScalarField{tNewScalarField},
                       Block1ScalarField{tExpectedScalarFieldBlock1}, Block2ScalarField{tExpectedScalarFieldBlock2},
                       TEST_CONTEXT("Blocks overlap"));
}

TEST(AnalysisDomainMeshViews, ToVector)
{
    const auto tAnalysisDomainMesh =
        AnalysisDomainMesh{/*.mFileName=*/"phobos.exo", /*.mBlockScalarField=*/kSingleBlockScalarField};
    const auto tVectorFromView = mesh_analysis_to_vector(AnalysisDomainMeshSequentialView{tAnalysisDomainMesh});
    EXPECT_EQ(tVectorFromView, kBlockScalarFieldVector1);
}

TEST(AnalysisDomainMeshViews, ToVectorTwoBlocksOverlap)
{
    const auto tAnalysisDomainMesh =
        AnalysisDomainMesh{/*.mFileName=*/"saturn.exo", /*.mBlockScalarField=*/kTwoBlockScalarFieldOverlap};
    const auto tVectorFromView = mesh_analysis_to_vector(AnalysisDomainMeshSequentialView{tAnalysisDomainMesh});
    EXPECT_EQ(tVectorFromView, kBlockScalarFieldVector1);
}

TEST(AnalysisDomainMeshViews, CombineScalarField)
{
    const auto tScalarField = combine_scalar_field_values_and_ids(kScalarField1, kIDs1);
    for (const auto& [tResult, tExpected] : utilities::Zip{tScalarField, kBlockScalarFieldVector1})
    {
        EXPECT_EQ(tResult.mValue, tExpected.mValue);
        EXPECT_EQ(tResult.mGlobalMeshEntityID, tExpected.mGlobalMeshEntityID);
    }
}

TEST(AnalysisDomainMeshViews, SplitScalarField)
{
    const auto [tScalarFieldValues, tIDMap] = split_scalar_field_values(kBlockScalarFieldVector1);

    EXPECT_EQ(tScalarFieldValues, kScalarField1);
    EXPECT_EQ(tIDMap, kIDs1);
}
}  // namespace plato::analysis::unittest
