#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/filter/extension/kernel_filters/CylindricalMaskUtilities.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilter.hpp"
#include "plato/filter/extension/kernel_filters/SweptFilter.hpp"
#include "plato/filter/extension/kernel_filters/test_utilities/SymmetryFilterTestUtilities.hpp"
#include "plato/third_party_integration/common/BoundingBox.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::filter::extension::kernel_filters::unittest
{
namespace
{

using third_party_integration::stk_io::test_utilities::ThirtyDegreeWedgeMesh;
using third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;
namespace tpi = third_party_integration;
const auto kRadius = .5;
const auto kNodeCentering = input_parser::KernelFilterCenteringTypes::kNodeCentered;
const auto kDirection = third_party_integration::common::Vector3{0, 0, 1};
const auto kZerothCoordinate = tpi::common::Coordinate{1, 0, 0};
const auto kSecondCoordinate = tpi::common::Coordinate{1.1, 0, 10};

const auto kSweptInputNodeCentered =
    test_utilities::make_filter_input<input_parser::z_swept_filter>(kRadius, kNodeCentering);

const auto kSweptInputElementCentered = test_utilities::make_filter_input<input_parser::z_swept_filter>(
    kRadius, input_parser::KernelFilterCenteringTypes::kElementCentered);

const auto kSweptLambdaDetail = [](const double aRadius, const input_parser::KernelFilterCenteringTypes& aCentering)
{ return detail::make_z_swept_filter_type(aRadius, aCentering, kDirection); };
const auto kSweptLambda =
    [](const input_parser::z_swept_filter& aInput, const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
{ return make_z_swept_filter_type(aInput, aAnalysisDomainMesh); };
}  // namespace

TEST(SweptFilter, MakeSweptFilterTypeRadiusAndCentering)
{
    test_utilities::check_make_filter_type_radius_and_centering(kSweptInputElementCentered, kSweptLambdaDetail,
                                                                TEST_CONTEXT("Swept input element centered."));
    test_utilities::check_make_filter_type_radius_and_centering(kSweptInputNodeCentered, kSweptLambdaDetail,
                                                                TEST_CONTEXT("Swept input node centered."));
}

TEST(SweptFilter, MakeSweptFilterTypeSearchFunction)
{
    namespace tpic = third_party_integration::common;
    const auto tFilterType = detail::make_z_swept_filter_type(kRadius, kNodeCentering, kDirection);

    const std::vector<tpic::Coordinate> tCoordinates{kZerothCoordinate, {2, 0, 0}, kSecondCoordinate};
    const auto tBoundingBox = tpic::bounding_box(tpic::bounding_box(tCoordinates), tpic::bounding_box(tCoordinates));
    const auto tDimension = tpic::dot(tBoundingBox.second - tBoundingBox.first, kDirection);

    const auto tSearchResultsGold = detail::distribute_search_vectors_and_stk_search_with_cylinders(
        TargetRowVector{tCoordinates}, SourceColumnVector{tCoordinates}, CylinderAxis{kDirection * tDimension},
        CylinderRadius{kRadius}, boost::mpi::communicator{});

    const auto tFilterInfoSearchResults = tFilterType.mSearchFunction(
        TargetRowVector{tCoordinates}, SourceColumnVector{tCoordinates}, boost::mpi::communicator{});
    EXPECT_EQ(tSearchResultsGold, tFilterInfoSearchResults);
}

TEST(SweptFilter, MakeSweptFilterTypeUnnormalizedWeightFunction)
{
    const auto tFilterType = detail::make_z_swept_filter_type(kRadius, kNodeCentering, kDirection);

    const auto tUnnormalizedWeightGold =
        detail::cylinder_ramp_weight(detail::CylinderCenter{kSecondCoordinate}, CylinderAxis{kDirection},
                                     CylinderRadius{kRadius}, detail::QueryLocation{kZerothCoordinate});

    const auto tFilterInfoUnnormalizedWeight =
        tFilterType.mUnnormalizedWeightFunction(SourcePoint{kZerothCoordinate}, TargetPoint{kSecondCoordinate});
    EXPECT_EQ(tUnnormalizedWeightGold, tFilterInfoUnnormalizedWeight);
}

TEST(SweptFilter, MakeSweptFilterTypeTargetMeshFunction)
{
    test_utilities::check_target_mesh_function(kSweptInputElementCentered, kSweptLambda,
                                               TEST_CONTEXT("Swept filter target mesh name check."));
}

TEST(SweptFilter, MakeSweptFilterType)
{
    test_utilities::check_make_function_from_input_against_parameter_function(
        kSweptInputNodeCentered, kSweptLambdaDetail, kSweptLambda,
        TEST_CONTEXT("Parameter make function and non-detail for swept filter"));
}

TEST_F(TwoDThreeBlockMesh, ValidateSourceMeshInXYPlaneValid)
{
    const auto tErrorMessage = detail::validate_source_mesh_in_xy_plane(input_parser::z_swept_filter{}, mMeshFilePath);
    EXPECT_FALSE(tErrorMessage.has_value()) << "Valid source mesh";
}

TEST_F(ThirtyDegreeWedgeMesh, ValidateSourceMeshInXYPlaneInvalid)
{
    const auto tErrorMessage = detail::validate_source_mesh_in_xy_plane(input_parser::z_swept_filter{}, mMeshFilePath);
    EXPECT_TRUE(tErrorMessage.has_value()) << "Invalid source mesh, has extents in z.";
}

}  // namespace plato::filter::extension::kernel_filters::unittest
