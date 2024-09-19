#include <gtest/gtest.h>

#include "plato/analysis/AnalysisDomainMeshRandomAccessView.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/Enumerate.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::analysis::unittest
{
namespace
{
const auto kScalarField1 = std::vector{2.0, 4.0, 122.0};
const auto kIDs1 = std::vector<std::size_t>{2, 4, 122};

const auto kScalarField2 = std::vector{4.0, 8.0};
const auto kIDs2 = std::vector<std::size_t>{4, 8};

const auto kScalarField3 = std::vector{1.0, 3.0};
const auto kIDs3 = std::vector<std::size_t>{1, 3};

const auto kFileName = std::filesystem::path{"not-a-file.exo"};

auto scalar_field_vector(const std::vector<std::size_t>& aGlobalIDs, const std::vector<double>& aScalarField)
    -> AnalysisDomainMesh::ScalarFieldVector
{
    auto tBlockScalarField = AnalysisDomainMesh::ScalarFieldVector{};
    tBlockScalarField.reserve(aGlobalIDs.size());
    for (const auto [tIndex, tGlobalID, tFieldValue] : utilities::enumerate(aGlobalIDs, aScalarField))
    {
        tBlockScalarField.push_back(ScalarFieldValue{tGlobalID, tIndex, tFieldValue});
    }
    return tBlockScalarField;
}

void check_view_vs_vector(const AnalysisDomainMeshRandomAccessView aAnalysisDomainMeshView,
                          const std::vector<std::size_t>& aIDs,
                          const std::vector<double>& aValues,
                          const test_utilities::TestContext& aTestContext)
{
    for (const auto tIndex : utilities::IndexRange{aIDs.size()})
    {
        const auto tResult = aAnalysisDomainMeshView[aIDs[tIndex]];
        ASSERT_TRUE(tResult) << aTestContext;
        EXPECT_EQ(tResult->mValue, aValues[tIndex]) << aTestContext;
    }
}
void check_nonexistent_entries(const AnalysisDomainMeshRandomAccessView aAnalysisDomainMeshView,
                               const std::vector<std::size_t>& aIDs,
                               const test_utilities::TestContext& aTestContext)
{
    for (const auto tID : aIDs)
    {
        EXPECT_FALSE(aAnalysisDomainMeshView[tID]) << aTestContext;
    }
}
}  // namespace

TEST(AnalysisDomainMeshRandomAccessView, OneBlockSize)
{
    const auto tBlockScalarField = AnalysisDomainMesh::BlockScalarField{{1, scalar_field_vector(kIDs1, kScalarField1)}};
    const auto tAnalysisDomainMesh = AnalysisDomainMesh{kFileName, tBlockScalarField};
    const auto tRandomAccessView = AnalysisDomainMeshRandomAccessView{tAnalysisDomainMesh};
    EXPECT_EQ(tRandomAccessView.size(), kScalarField1.size());
}

TEST(AnalysisDomainMeshRandomAccessView, OneBlockAccess)
{
    const auto tBlockScalarField = AnalysisDomainMesh::BlockScalarField{{1, scalar_field_vector(kIDs1, kScalarField1)}};
    const auto tAnalysisDomainMesh = AnalysisDomainMesh{kFileName, tBlockScalarField};
    const auto tRandomAccessView = AnalysisDomainMeshRandomAccessView{tAnalysisDomainMesh};

    check_view_vs_vector(tRandomAccessView, kIDs1, kScalarField1, TEST_CONTEXT("Field 1"));
    const auto tNonExistentIDs = std::vector<std::size_t>{42, 100, 200};
    check_nonexistent_entries(tRandomAccessView, tNonExistentIDs, TEST_CONTEXT("Non-existent entries"));
}

TEST(AnalysisDomainMeshRandomAccessView, TwoBlockAccessNoOverlap)
{
    const auto tBlockScalarField = AnalysisDomainMesh::BlockScalarField{{1, scalar_field_vector(kIDs1, kScalarField1)},
                                                                        {3, scalar_field_vector(kIDs3, kScalarField3)}};
    const auto tAnalysisDomainMesh = AnalysisDomainMesh{kFileName, tBlockScalarField};
    const auto tRandomAccessView = AnalysisDomainMeshRandomAccessView{tAnalysisDomainMesh};

    check_view_vs_vector(tRandomAccessView, kIDs1, kScalarField1, TEST_CONTEXT("Field 1"));
    check_view_vs_vector(tRandomAccessView, kIDs3, kScalarField3, TEST_CONTEXT("Field 3"));
    const auto tNonExistentIDs = std::vector<std::size_t>{42, 100, 200};
    check_nonexistent_entries(tRandomAccessView, tNonExistentIDs, TEST_CONTEXT("Non-existent entries"));
}

TEST(AnalysisDomainMeshRandomAccessView, TwoBlockAccessHasOverlap)
{
    const auto tBlockScalarField = AnalysisDomainMesh::BlockScalarField{{1, scalar_field_vector(kIDs1, kScalarField1)},
                                                                        {2, scalar_field_vector(kIDs2, kScalarField2)}};
    const auto tAnalysisDomainMesh = AnalysisDomainMesh{kFileName, tBlockScalarField};
    const auto tRandomAccessView = AnalysisDomainMeshRandomAccessView{tAnalysisDomainMesh};

    check_view_vs_vector(tRandomAccessView, kIDs1, kScalarField1, TEST_CONTEXT("Field 1"));
    check_view_vs_vector(tRandomAccessView, kIDs2, kScalarField2, TEST_CONTEXT("Field 2"));
    const auto tNonExistentIDs = std::vector<std::size_t>{42, 100, 200};
    check_nonexistent_entries(tRandomAccessView, tNonExistentIDs, TEST_CONTEXT("Non-existent entries"));
}
}  // namespace plato::analysis::unittest
