#include <gtest/gtest.h>

#include <boost/core/ignore_unused.hpp>
#include <numeric>

#include "plato/mesh/MeshDesignVariables.hpp"
#include "plato/mesh/MeshDesignVariablesSequentialView.hpp"
#include "plato/mesh/unittest/Utilities.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/NamedType.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::mesh::unittest
{
namespace
{
const auto kScalarField1 = std::vector{1.0, 0.0, 4.0};
const auto kIDs1 = std::vector<std::size_t>{0, 1, 2};
const auto kBlockScalarFieldVector1 = MeshDesignVariables::ScalarFieldVector{{kIDs1[0], kIDs1[0], kScalarField1[0]},
                                                                             {kIDs1[1], kIDs1[1], kScalarField1[1]},
                                                                             {kIDs1[2], kIDs1[2], kScalarField1[2]}};

const auto kScalarField2 = std::vector{-1.0, 10.0};
const auto kIDs2 = std::vector<std::size_t>{0, 1};
const auto kBlockScalarFieldVector2WithOverlap = MeshDesignVariables::ScalarFieldVector{
    {kIDs2[0], kIDs2[0], kScalarField2[0]}, {kIDs2[1], kIDs2[1], kScalarField2[1]}};

const auto kScalarField3 = std::vector{-2.0, 11.0};
const auto kIDs3 = std::vector<std::size_t>{3, 4};
const auto kBlockScalarFieldVector2NoOverlap = MeshDesignVariables::ScalarFieldVector{
    {kIDs3[0], kIDs3[0], kScalarField3[0]}, {kIDs3[1], kIDs3[1], kScalarField3[1]}};

const auto kSingleBlockScalarField = MeshDesignVariables::BlockScalarField{{0, kBlockScalarFieldVector1}};
const auto kTwoBlockScalarFieldOverlap =
    MeshDesignVariables::BlockScalarField{{0, kBlockScalarFieldVector1}, {1, kBlockScalarFieldVector2WithOverlap}};

const auto kTwoBlockScalarFieldNoOverlap =
    MeshDesignVariables::BlockScalarField{{0, kBlockScalarFieldVector1}, {1, kBlockScalarFieldVector2NoOverlap}};

using NewScalarField = utilities::NamedType<std::vector<ScalarFieldValue>, struct NewScalarFieldTag>;
using Block1ScalarField = utilities::NamedType<std::vector<ScalarFieldValue>, struct NewScalarFieldTag>;
using Block2ScalarField = utilities::NamedType<std::vector<ScalarFieldValue>, struct NewScalarFieldTag>;

void check_mutable_view(MeshDesignVariables& aMeshDesignVariables,
                        const NewScalarField& aNewScalarField,
                        const Block1ScalarField& aExpectedScalarFieldBlock1,
                        const Block2ScalarField& aExpectedScalarFieldBlock2,
                        const plato::test_utilities::TestContext& aTestContext)
{
    const auto tMeshView = MeshDesignVariablesMutableSequentialView{aMeshDesignVariables};
    std::copy(aNewScalarField.mValue.cbegin(), aNewScalarField.mValue.cend(), tMeshView.begin());
    EXPECT_EQ(aExpectedScalarFieldBlock1.mValue, aMeshDesignVariables.mBlockScalarField.at(0)) << aTestContext;
    EXPECT_EQ(aExpectedScalarFieldBlock2.mValue, aMeshDesignVariables.mBlockScalarField.at(1)) << aTestContext;
}
}  // namespace

TEST(MeshDesignVariablesViews, SizeSingleBlock)
{
    const auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"mars.exo", /*.mBlockScalarField=*/kSingleBlockScalarField};
    ASSERT_EQ(MeshDesignVariablesSequentialView{tMeshDesignVariables}.size(), kScalarField1.size());
}

TEST(MeshDesignVariablesViews, SizeTwoBlockOverlap)
{
    const auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"ceres.exo", /*.mBlockScalarField=*/kTwoBlockScalarFieldOverlap};
    ASSERT_EQ(MeshDesignVariablesSequentialView{tMeshDesignVariables}.size(), kScalarField1.size());
}

TEST(MeshDesignVariablesViews, SizeTwoBlockNoOverlap)
{
    const auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"io.exo", /*.mBlockScalarField=*/kTwoBlockScalarFieldNoOverlap};
    ASSERT_EQ(MeshDesignVariablesSequentialView{tMeshDesignVariables}.size(),
              kScalarField1.size() + kScalarField2.size());
}

TEST(MeshDesignVariablesViews, BeginEnd)
{
    const auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"jupiter.exo", /*.mBlockScalarField=*/kSingleBlockScalarField};
    const auto tMeshView = MeshDesignVariablesSequentialView{tMeshDesignVariables};

    const auto tBeginIterator = tMeshView.begin();
    constexpr auto tExpectedIteratorVectorSize = 1U;
    ASSERT_EQ(tBeginIterator.mCurrentIterators.size(), tExpectedIteratorVectorSize);
    EXPECT_EQ(tBeginIterator.mCurrentIterators.front(),
              tMeshDesignVariables.mBlockScalarField.cbegin()->second.cbegin());
    ASSERT_EQ(tBeginIterator.mEndIterators.size(), tExpectedIteratorVectorSize);
    EXPECT_EQ(tBeginIterator.mEndIterators.front(), tMeshDesignVariables.mBlockScalarField.cbegin()->second.cend());

    const auto tEndIterator = tMeshView.end();
    ASSERT_EQ(tEndIterator.mCurrentIterators.size(), tExpectedIteratorVectorSize);
    EXPECT_EQ(tEndIterator.mCurrentIterators.front(), tMeshDesignVariables.mBlockScalarField.cbegin()->second.cend());
    ASSERT_EQ(tEndIterator.mEndIterators.size(), tExpectedIteratorVectorSize);
    EXPECT_EQ(tEndIterator.mEndIterators.front(), tMeshDesignVariables.mBlockScalarField.cbegin()->second.cend());

    // Increment begin until it's at the end and check that they're equal
    auto tIncrementedIterator = tMeshView.begin();
    for (const auto tScalarField : kScalarField1)
    {
        boost::ignore_unused(tScalarField);
        ++tIncrementedIterator;
    }
    EXPECT_EQ(tIncrementedIterator, tEndIterator);
}

TEST(MeshDesignVariablesViews, RangeBasedFor)
{
    const auto tCheckScalarField = [](const MeshDesignVariablesSequentialView aMeshView,
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
        const auto tMeshDesignVariables =
            MeshDesignVariables{/*.mFileName=*/"pluto.exo", /*.mBlockScalarField=*/kTwoBlockScalarFieldNoOverlap};
        const auto tMeshView = MeshDesignVariablesSequentialView{tMeshDesignVariables};

        auto tExpectedScalarField = kBlockScalarFieldVector1;
        std::copy(kBlockScalarFieldVector2NoOverlap.cbegin(), kBlockScalarFieldVector2NoOverlap.cend(),
                  std::back_inserter(tExpectedScalarField));
        tCheckScalarField(tMeshView, tExpectedScalarField, TEST_CONTEXT("No overlap"));
    }
    // Overlap
    {
        const auto tMeshDesignVariables =
            MeshDesignVariables{/*.mFileName=*/"deimos.exo", /*.mBlockScalarField=*/kTwoBlockScalarFieldOverlap};
        const auto tMeshView = MeshDesignVariablesSequentialView{tMeshDesignVariables};

        auto tExpectedScalarField = kBlockScalarFieldVector1;
        tCheckScalarField(tMeshView, tExpectedScalarField, TEST_CONTEXT("Full overlap"));
    }
    // One block
    {
        const auto tMeshDesignVariables =
            MeshDesignVariables{/*.mFileName=*/"venus.exo", /*.mBlockScalarField=*/kSingleBlockScalarField};
        const auto tMeshView = MeshDesignVariablesSequentialView{tMeshDesignVariables};

        auto tExpectedScalarField = kBlockScalarFieldVector1;
        tCheckScalarField(tMeshView, tExpectedScalarField, TEST_CONTEXT("One block"));
    }
}

TEST(MeshDesignVariablesViews, MutableView)
{
    auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"uranus.exo", /*.mBlockScalarField=*/kSingleBlockScalarField};

    static_assert(!kIsConstIterator<std::vector<double>::iterator>);
    static_assert(kIsConstIterator<std::vector<double>::const_iterator>);

    const auto tMeshView = MeshDesignVariablesMutableSequentialView{tMeshDesignVariables};

    const auto tNewScalarField =
        std::vector{ScalarFieldValue{0, 0, 0.0}, ScalarFieldValue{1, 1, 1.0}, ScalarFieldValue{2, 2, 2.0}};
    std::copy(tNewScalarField.cbegin(), tNewScalarField.cend(), tMeshView.begin());

    const auto tVectorFromView =
        mesh_design_variables_to_vector(MeshDesignVariablesSequentialView{tMeshDesignVariables});
    EXPECT_EQ(tVectorFromView, tNewScalarField);
}

TEST(MeshDesignVariablesViews, MutableViewTwoBlocksNoOverlap)
{
    auto tMeshDesignVariables = MeshDesignVariables{/*.mFileName*/ "mercury.exo", kTwoBlockScalarFieldNoOverlap};
    const auto tNewScalarField =
        std::vector<ScalarFieldValue>{{0, 0, 0.0}, {1, 1, 1.0}, {2, 2, 2.0}, {3, 3, 3.0}, {4, 4, 4.0}};
    const auto tExpectedScalarFieldBlock1 = std::vector<ScalarFieldValue>{{0, 0, 0.0}, {1, 1, 1.0}, {2, 2, 2.0}};
    const auto tExpectedScalarFieldBlock2 = std::vector<ScalarFieldValue>{{3, 3, 3.0}, {4, 4, 4.0}};
    check_mutable_view(tMeshDesignVariables, NewScalarField{tNewScalarField},
                       Block1ScalarField{tExpectedScalarFieldBlock1}, Block2ScalarField{tExpectedScalarFieldBlock2},
                       TEST_CONTEXT("Blocks do not overlap"));
}

TEST(MeshDesignVariablesViews, MutableViewTwoBlocksWithOverlap)
{
    auto tMeshDesignVariables = MeshDesignVariables{/*.mFileName*/ "earth.exo", kTwoBlockScalarFieldOverlap};
    const auto tNewScalarField = std::vector<ScalarFieldValue>{{0, 0, 0.0}, {1, 1, 1.0}, {2, 2, 2.0}};
    const auto tExpectedScalarFieldBlock1 = std::vector<ScalarFieldValue>{{0, 0, 0.0}, {1, 1, 1.0}, {2, 2, 2.0}};
    const auto tExpectedScalarFieldBlock2 = std::vector<ScalarFieldValue>{{0, 0, 0.0}, {1, 1, 1.0}};
    check_mutable_view(tMeshDesignVariables, NewScalarField{tNewScalarField},
                       Block1ScalarField{tExpectedScalarFieldBlock1}, Block2ScalarField{tExpectedScalarFieldBlock2},
                       TEST_CONTEXT("Blocks overlap"));
}

TEST(MeshDesignVariablesViews, ToVector)
{
    const auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"phobos.exo", /*.mBlockScalarField=*/kSingleBlockScalarField};
    const auto tVectorFromView =
        mesh_design_variables_to_vector(MeshDesignVariablesSequentialView{tMeshDesignVariables});
    EXPECT_EQ(tVectorFromView, kBlockScalarFieldVector1);
}

TEST(MeshDesignVariablesViews, ToVectorTwoBlocksOverlap)
{
    const auto tMeshDesignVariables =
        MeshDesignVariables{/*.mFileName=*/"saturn.exo", /*.mBlockScalarField=*/kTwoBlockScalarFieldOverlap};
    const auto tVectorFromView =
        mesh_design_variables_to_vector(MeshDesignVariablesSequentialView{tMeshDesignVariables});
    EXPECT_EQ(tVectorFromView, kBlockScalarFieldVector1);
}

TEST(MeshDesignVariablesViews, CombineScalarField)
{
    const auto tScalarField = detail::combine_scalar_field_values_and_ids(kScalarField1, kIDs1);
    for (const auto& [tResult, tExpected] : utilities::Zip{tScalarField, kBlockScalarFieldVector1})
    {
        EXPECT_EQ(tResult.mValue, tExpected.mValue);
        EXPECT_EQ(tResult.mGlobalMeshEntityID, tExpected.mGlobalMeshEntityID);
    }
}

TEST(MeshDesignVariablesViews, SplitScalarField)
{
    const auto [tScalarFieldValues, tIDMap] = detail::split_scalar_field_values(kBlockScalarFieldVector1);

    EXPECT_EQ(tScalarFieldValues, kScalarField1);
    EXPECT_EQ(tIDMap, kIDs1);
}
}  // namespace plato::mesh::unittest
