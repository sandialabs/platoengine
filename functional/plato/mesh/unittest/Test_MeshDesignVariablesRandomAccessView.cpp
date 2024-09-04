#include <gtest/gtest.h>

#include "plato/mesh/MeshDesignVariablesRandomAccessView.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::mesh::unittest
{
namespace
{
const auto kScalarField1 = std::vector{2.0, 4.0, 122.0};
const auto kIDs1 = std::vector<std::size_t>{2, 4, 122};
const auto kBlockScalarFieldVector1 = MeshDesignVariables::ScalarFieldVector{{kIDs1[0], kIDs1[0], kScalarField1[0]},
                                                                             {kIDs1[1], kIDs1[1], kScalarField1[1]},
                                                                             {kIDs1[2], kIDs1[2], kScalarField1[2]}};
const auto kScalarField2 = std::vector{4.0, 8.0};
const auto kIDs2 = std::vector<std::size_t>{4, 8};
const auto kBlockScalarFieldVector2 = MeshDesignVariables::ScalarFieldVector{{kIDs2[0], kIDs2[0], kScalarField2[0]},
                                                                             {kIDs2[1], kIDs2[1], kScalarField2[1]}};
const auto kScalarField3 = std::vector{1.0, 3.0};
const auto kIDs3 = std::vector<std::size_t>{1, 3};
const auto kBlockScalarFieldVector3 = MeshDesignVariables::ScalarFieldVector{{kIDs3[0], kIDs3[0], kScalarField3[0]},
                                                                             {kIDs3[1], kIDs3[1], kScalarField3[1]}};

const auto kFileName = std::filesystem::path{"not-a-file.exo"};
}  // namespace

TEST(MeshDesignVariablesRandomAccessView, OneBlockSize)
{
    const auto tBlockScalarField = MeshDesignVariables::BlockScalarField{{1, kBlockScalarFieldVector1}};
    const auto tMeshDesignVariables = MeshDesignVariables{kFileName, tBlockScalarField};
    const auto tRandomAccessView = MeshDesignVariablesRandomAccessView{tMeshDesignVariables};
    EXPECT_EQ(tRandomAccessView.size(), kScalarField1.size());
}

TEST(MeshDesignVariablesRandomAccessView, OneBlockAccess)
{
    const auto tBlockScalarField = MeshDesignVariables::BlockScalarField{{1, kBlockScalarFieldVector1}};
    const auto tMeshDesignVariables = MeshDesignVariables{kFileName, tBlockScalarField};
    const auto tRandomAccessView = MeshDesignVariablesRandomAccessView{tMeshDesignVariables};
    for (const auto tIndex : utilities::IndexRange{kScalarField1.size()})
    {
        const auto tResult = tRandomAccessView[kIDs1[tIndex]];
        ASSERT_TRUE(tResult);
        EXPECT_EQ(tResult->mValue, kScalarField1[tIndex]);
    }
    // Check some non-entries
    EXPECT_FALSE(tRandomAccessView[42]);
    EXPECT_FALSE(tRandomAccessView[100]);
    EXPECT_FALSE(tRandomAccessView[200]);
}

TEST(MeshDesignVariablesRandomAccessView, TwoBlockAccessNoOverlap)
{
    const auto tBlockScalarField =
        MeshDesignVariables::BlockScalarField{{1, kBlockScalarFieldVector1}, {3, kBlockScalarFieldVector3}};
    const auto tMeshDesignVariables = MeshDesignVariables{kFileName, tBlockScalarField};
    const auto tRandomAccessView = MeshDesignVariablesRandomAccessView{tMeshDesignVariables};
    for (const auto tIndex : utilities::IndexRange{kScalarField1.size()})
    {
        const auto tResult = tRandomAccessView[kIDs1[tIndex]];
        ASSERT_TRUE(tResult);
        EXPECT_EQ(tResult->mValue, kScalarField1[tIndex]);
    }
    for (const auto tIndex : utilities::IndexRange{kScalarField3.size()})
    {
        const auto tResult = tRandomAccessView[kIDs3[tIndex]];
        ASSERT_TRUE(tResult);
        EXPECT_EQ(tResult->mValue, kScalarField3[tIndex]);
    }  // Check some non-entries
    ASSERT_FALSE(tRandomAccessView[42]);
    ASSERT_FALSE(tRandomAccessView[100]);
    EXPECT_FALSE(tRandomAccessView[200]);
}

TEST(MeshDesignVariablesRandomAccessView, TwoBlockAccessHasOverlap)
{
    const auto tBlockScalarField =
        MeshDesignVariables::BlockScalarField{{1, kBlockScalarFieldVector1}, {2, kBlockScalarFieldVector2}};
    const auto tMeshDesignVariables = MeshDesignVariables{kFileName, tBlockScalarField};
    const auto tRandomAccessView = MeshDesignVariablesRandomAccessView{tMeshDesignVariables};
    for (const auto tIndex : utilities::IndexRange{kScalarField1.size()})
    {
        const auto tResult = tRandomAccessView[kIDs1[tIndex]];
        ASSERT_TRUE(tResult.has_value());
        EXPECT_EQ(tResult->mValue, kScalarField1[tIndex]);
    }
    for (const auto tIndex : utilities::IndexRange{kScalarField2.size()})
    {
        const auto tResult = tRandomAccessView[kIDs2[tIndex]];
        ASSERT_TRUE(tResult.has_value());
        EXPECT_EQ(tResult->mValue, kScalarField2[tIndex]);
    }
    ASSERT_FALSE(tRandomAccessView[42]);
    ASSERT_FALSE(tRandomAccessView[100]);
    EXPECT_FALSE(tRandomAccessView[200]);
}
}  // namespace plato::mesh::unittest
