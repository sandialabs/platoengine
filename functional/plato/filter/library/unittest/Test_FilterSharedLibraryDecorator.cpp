#include <gtest/gtest.h>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshOperators.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/filter/library/FilterSharedLibraryDecorator.hpp"
#include "plato/services/SharedLibrarySetupTeardown.hpp"

namespace plato::filter::library::unittest
{
namespace
{
const auto kSharedLibraryPath = std::filesystem::path{"libPlatoConstantValueFilter.so"};
constexpr auto kMeshName = std::string_view{"empty.exo"};
constexpr auto kBlockID = 1U;

using FilterFunctionSignature = std::unique_ptr<FilterInterface>(const FilterParameters&);

auto test_analysis_domain_mesh() -> analysis::AnalysisDomainMesh
{
    const auto tDensities = std::vector<double>{-1.0, 0.0, 1.0};
    const auto tIDs = std::vector<std::size_t>{1, 2, 3};
    return analysis::AnalysisDomainMesh{
        kMeshName, analysis::AnalysisDomainMesh::BlockScalarField{
                       {kBlockID, analysis::combine_scalar_field_values_and_ids(tDensities, tIDs)}}};
}
}  // namespace

TEST(FilterSharedLibraryDecorator, Filter)
{
    auto tSharedLibrary = services::SharedLibrarySetupTeardown{kSharedLibraryPath};
    auto tFilter = tSharedLibrary.call<FilterFunctionSignature>(kCreateFilterFunctionName, library::FilterParameters{});
    const auto tSharedLibraryFilter = std::make_unique<FilterSharedLibraryDecorator>(
        services::SharedLibraryObject{std::move(tSharedLibrary), std::move(tFilter)});

    const auto tAnalysisDomainMesh = test_analysis_domain_mesh();
    const auto tResult = tSharedLibraryFilter->filter(tAnalysisDomainMesh);

    ASSERT_EQ(tResult.mBlockScalarField.count(kBlockID), 1U);
    EXPECT_EQ(tResult.mBlockScalarField.at(kBlockID), tAnalysisDomainMesh.mBlockScalarField.at(kBlockID));
}

TEST(FilterSharedLibraryDecorator, Jacobians)
{
    const auto tSharedLibraryFilter =
        std::make_unique<FilterSharedLibraryDecorator>(services::make_shared_library_object<FilterFunctionSignature>(
            kSharedLibraryPath, kCreateFilterFunctionName, library::FilterParameters{}));

    const auto tAnalysisDomainMesh = test_analysis_domain_mesh();
    const auto tRowVector = linear_algebra::DynamicVector<double>{};
    {
        const auto tResult = tSharedLibraryFilter->rowVectorTimesJacobian(tAnalysisDomainMesh, tRowVector);
        const auto tExpected = std::vector{10.0, 11.0, 12.0};
        EXPECT_EQ(tResult.stdVector(), tExpected);
    }
    {
        const auto tResult = tSharedLibraryFilter->rowVectorTimesAdjointJacobian(tAnalysisDomainMesh, tRowVector);
        const auto tExpected = std::vector{9.0, 8.0, 7.0};
        EXPECT_EQ(tResult.stdVector(), tExpected);
    }
}

}  // namespace plato::filter::library::unittest
