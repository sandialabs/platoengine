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

const auto kNonExistentIDs = std::vector<std::size_t>{42, 100, 200};

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

void check_vectors(const std::vector<ScalarFieldValue>& aScalarFieldValues,
                   const std::vector<double>& aExpectedValues,
                   const test_utilities::TestContext& aTestContext)
{
    for (const auto& [tScalarFieldValue, tExpectedValue] : utilities::Zip{aScalarFieldValues, aExpectedValues})
    {
        EXPECT_EQ(tScalarFieldValue.mValue, tExpectedValue) << aTestContext;
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

void check_nonexistent_entries(const AnalysisDomainMeshMutableRandomAccessView aAnalysisDomainMeshView,
                               const std::vector<std::size_t>& aIDs,
                               const test_utilities::TestContext& aTestContext)
{
    for (const auto tID : aIDs)
    {
        EXPECT_TRUE(aAnalysisDomainMeshView[tID].mIterators.empty()) << aTestContext;
    }
}

}  // namespace

TEST(AnalysisDomainMeshRandomAccessView, OneBlockSize)
{
    const auto tBlockScalarField = AnalysisDomainMesh::BlockScalarField{{1, scalar_field_vector(kIDs1, kScalarField1)}};
    const auto tAnalysisDomainMesh = AnalysisDomainMesh{kFileName, tBlockScalarField};
    const auto tRandomAccessView = AnalysisDomainMeshRandomAccessView{tAnalysisDomainMesh};
    EXPECT_EQ(tRandomAccessView.size(), kScalarField1.size());

    auto tNonConstAnalysisDomainMesh = AnalysisDomainMesh{kFileName, tBlockScalarField};
    const auto tMutableRandomAccessView = AnalysisDomainMeshMutableRandomAccessView{tNonConstAnalysisDomainMesh};
    EXPECT_EQ(tMutableRandomAccessView.size(), kScalarField1.size());
}

TEST(AnalysisDomainMeshRandomAccessView, OneBlockAccess)
{
    const auto tBlockScalarField = AnalysisDomainMesh::BlockScalarField{{1, scalar_field_vector(kIDs1, kScalarField1)}};
    auto tAnalysisDomainMesh = AnalysisDomainMesh{kFileName, tBlockScalarField};

    // Const view
    {
        const auto tRandomAccessView = AnalysisDomainMeshRandomAccessView{tAnalysisDomainMesh};
        check_view_vs_vector(tRandomAccessView, kIDs1, kScalarField1, TEST_CONTEXT("Field 1"));
        check_nonexistent_entries(tRandomAccessView, kNonExistentIDs, TEST_CONTEXT("Non-existent entries"));
    }
    // Mutable view
    {
        const auto tRandomAccessMutableView = AnalysisDomainMeshMutableRandomAccessView{tAnalysisDomainMesh};
        constexpr auto tNewValue = double{42.0};
        tRandomAccessMutableView[kIDs1.front()] = tNewValue;

        auto tExpectedFieldValues = kScalarField1;
        tExpectedFieldValues.front() = tNewValue;

        check_vectors(tAnalysisDomainMesh.mBlockScalarField.at(1), tExpectedFieldValues,
                      TEST_CONTEXT("Modified field 1"));
        check_nonexistent_entries(tRandomAccessMutableView, kNonExistentIDs,
                                  TEST_CONTEXT("Non-existent entries in mutable view"));
    }
}

TEST(AnalysisDomainMeshRandomAccessView, TwoBlockAccessNoOverlap)
{
    const auto tBlockScalarField = AnalysisDomainMesh::BlockScalarField{{1, scalar_field_vector(kIDs1, kScalarField1)},
                                                                        {3, scalar_field_vector(kIDs3, kScalarField3)}};
    auto tAnalysisDomainMesh = AnalysisDomainMesh{kFileName, tBlockScalarField};

    // Const view
    {
        const auto tRandomAccessView = AnalysisDomainMeshRandomAccessView{tAnalysisDomainMesh};
        check_view_vs_vector(tRandomAccessView, kIDs1, kScalarField1, TEST_CONTEXT("Field 1"));
        check_view_vs_vector(tRandomAccessView, kIDs3, kScalarField3, TEST_CONTEXT("Field 3"));
        check_nonexistent_entries(tRandomAccessView, kNonExistentIDs, TEST_CONTEXT("Non-existent entries"));
    }
    // Mutable view
    {
        const auto tRandomAccessMutableView = AnalysisDomainMeshMutableRandomAccessView{tAnalysisDomainMesh};
        constexpr auto tNewValue = double{42.0};
        tRandomAccessMutableView[kIDs1.back()] = tNewValue;
        tRandomAccessMutableView[kIDs3.front()] = tNewValue;

        auto tExpectedFieldValues1 = kScalarField1;
        tExpectedFieldValues1.back() = tNewValue;
        check_vectors(tAnalysisDomainMesh.mBlockScalarField.at(1), tExpectedFieldValues1,
                      TEST_CONTEXT("Modified field 1"));

        auto tExpectedFieldValues3 = kScalarField3;
        tExpectedFieldValues3.front() = tNewValue;
        check_vectors(tAnalysisDomainMesh.mBlockScalarField.at(3), tExpectedFieldValues3,
                      TEST_CONTEXT("Modified field 3"));
        check_nonexistent_entries(tRandomAccessMutableView, kNonExistentIDs,
                                  TEST_CONTEXT("Non-existent entries in mutable view"));
    }
}

TEST(AnalysisDomainMeshRandomAccessView, TwoBlockAccessHasOverlap)
{
    const auto tBlockScalarField = AnalysisDomainMesh::BlockScalarField{{1, scalar_field_vector(kIDs1, kScalarField1)},
                                                                        {2, scalar_field_vector(kIDs2, kScalarField2)}};
    auto tAnalysisDomainMesh = AnalysisDomainMesh{kFileName, tBlockScalarField};

    // Const view
    {
        const auto tRandomAccessView = AnalysisDomainMeshRandomAccessView{tAnalysisDomainMesh};
        check_view_vs_vector(tRandomAccessView, kIDs1, kScalarField1, TEST_CONTEXT("Field 1"));
        check_view_vs_vector(tRandomAccessView, kIDs2, kScalarField2, TEST_CONTEXT("Field 2"));
        check_nonexistent_entries(tRandomAccessView, kNonExistentIDs, TEST_CONTEXT("Non-existent entries"));
    }
    // Mutable view
    {
        ASSERT_EQ(kIDs1.at(1), kIDs2.front());  // Make sure we're checking a shared ID

        const auto tRandomAccessMutableView = AnalysisDomainMeshMutableRandomAccessView{tAnalysisDomainMesh};
        constexpr auto tNewValue = double{42.0};
        tRandomAccessMutableView[kIDs1.at(1)] = tNewValue;  // One assignment, should modify both values

        auto tExpectedFieldValues1 = kScalarField1;
        tExpectedFieldValues1.at(1) = tNewValue;
        check_vectors(tAnalysisDomainMesh.mBlockScalarField.at(1), tExpectedFieldValues1,
                      TEST_CONTEXT("Modified field 1"));

        auto tExpectedFieldValues2 = kScalarField2;
        tExpectedFieldValues2.front() = tNewValue;
        check_vectors(tAnalysisDomainMesh.mBlockScalarField.at(2), tExpectedFieldValues2,
                      TEST_CONTEXT("Modified field 2"));
        check_nonexistent_entries(tRandomAccessMutableView, kNonExistentIDs,
                                  TEST_CONTEXT("Non-existent entries in mutable view"));
    }
}

}  // namespace plato::analysis::unittest
