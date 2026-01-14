#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/extension/kernel_filters/CanonicalKernelFilter.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilter.hpp"
#include "plato/filter/extension/kernel_filters/SphericalMaskUtilities.hpp"
#include "plato/filter/extension/kernel_filters/test_utilities/SymmetryFilterTestUtilities.hpp"

namespace plato::filter::extension::kernel_filters::unittest
{

namespace
{
namespace tpi = third_party_integration;
constexpr std::string_view kMeshFile = "mesh.exo";
const auto kRadius = 1.5;
const auto kNodeCentering = input_parser::KernelFilterCenteringTypes::kNodeCentered;

const auto kZerothCoordinate = tpi::common::Coordinate{1, 0, 0};
const auto kSecondCoordinate = tpi::common::Coordinate{1.5, 0, 0};

const auto kKernelInputNodeCentered =
    input_parser::kernel_filter{/*.filter_radius=*/kRadius, /*.centering_type=*/kNodeCentering,
                                /*.use_relative_radius=*/boost::none,
                                /*.number_of_processors=*/boost::none};

const auto kKernelInputElementCentered = input_parser::kernel_filter{
    /*.filter_radius=*/kRadius, /*.centering_type=*/input_parser::KernelFilterCenteringTypes::kElementCentered,
    /*.use_relative_radius=*/boost::none,
    /*.number_of_processors=*/boost::none};

const auto kKernelLambdaDetail = [](const double aRadius, const input_parser::KernelFilterCenteringTypes& aCentering)
{ return detail::make_kernel_filter_type(aRadius, aCentering, mesh::Mesh{kMeshFile}); };

}  // namespace

TEST(CanonicalKernelFilter, MakeKernelFilterTypeRadiusAndCentering)
{
    test_utilities::check_make_filter_type_radius_and_centering(kKernelInputElementCentered, kKernelLambdaDetail,
                                                                TEST_CONTEXT("Kernel input element centered."));
    test_utilities::check_make_filter_type_radius_and_centering(kKernelInputNodeCentered, kKernelLambdaDetail,
                                                                TEST_CONTEXT("Kernel input node centered."));
}

TEST(CanonicalKernelFilter, MakeKernelFilterTypeSearchFunction)
{
    namespace tpic = third_party_integration::common;
    const auto tFilterType = detail::make_kernel_filter_type(kRadius, kNodeCentering, mesh::Mesh{kMeshFile});

    const std::vector<tpic::Coordinate> tCoordinates{kZerothCoordinate, {2, 0, 0}, kSecondCoordinate};
    const auto tSearchResultsGold = distribute_search_vectors_and_stk_search_with_spheres(
        TargetRowVector{tCoordinates}, SourceColumnVector{tCoordinates}, kRadius, boost::mpi::communicator{});
    const auto tFilterInfoSearchResults = tFilterType.mSearchFunction(
        TargetRowVector{tCoordinates}, SourceColumnVector{tCoordinates}, boost::mpi::communicator{});
    EXPECT_EQ(tSearchResultsGold, tFilterInfoSearchResults);
}

TEST(CanonicalKernelFilter, MakeKernelFilterTypeUnnormalizedWeightFunction)
{
    namespace tpic = third_party_integration::common;
    const auto tFilterType = detail::make_kernel_filter_type(kRadius, kNodeCentering, mesh::Mesh{kMeshFile});

    const auto tDistance = tpic::magnitude(kZerothCoordinate - kSecondCoordinate);
    const auto tUnnormalizedWeightGold = detail::linear_ramp_weight(Distance{tDistance}, SearchRadius{kRadius});
    const auto tFilterInfoUnnormalizedWeight =
        tFilterType.mUnnormalizedWeightFunction(SourcePoint{kZerothCoordinate}, TargetPoint{kSecondCoordinate});
    EXPECT_EQ(tUnnormalizedWeightGold, tFilterInfoUnnormalizedWeight);
}

TEST(CanonicalKernelFilter, MakeKernelFilterTypeTargetMeshFunction)
{
    const auto tGoldMesh = mesh::Mesh{kMeshFile};
    const auto tFilterType = detail::make_kernel_filter_type(kRadius, kNodeCentering, tGoldMesh);
    const auto tResultTargetMesh = tFilterType.mTargetMeshFunction(input_parser::kernel_filter{});
    EXPECT_EQ(tResultTargetMesh.filePath(), kMeshFile);
}

TEST(CanonicalKernelFilter, MakeKernelFilterType)
{
    const auto tFilterTypeFromParameters =
        detail::make_kernel_filter_type(kRadius, kNodeCentering, mesh::Mesh{kMeshFile});
    const auto tFilterTypeFromInputAndAnalysisDomainMesh =
        make_kernel_filter_type(kKernelInputNodeCentered, analysis::AnalysisDomainMesh{kMeshFile, {}});
    EXPECT_EQ(tFilterTypeFromParameters.mRadius, tFilterTypeFromInputAndAnalysisDomainMesh.mRadius);
    EXPECT_EQ(tFilterTypeFromParameters.mFilterCentering, tFilterTypeFromInputAndAnalysisDomainMesh.mFilterCentering);
    EXPECT_EQ(tFilterTypeFromParameters.mTargetMeshFunction(kKernelInputNodeCentered).filePath(),
              tFilterTypeFromInputAndAnalysisDomainMesh.mTargetMeshFunction(kKernelInputNodeCentered).filePath());
}

}  // namespace plato::filter::extension::kernel_filters::unittest
