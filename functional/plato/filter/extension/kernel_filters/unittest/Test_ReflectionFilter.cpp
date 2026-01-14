#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/filter/extension/kernel_filters/KernelFilter.hpp"
#include "plato/filter/extension/kernel_filters/ReflectFilter.hpp"
#include "plato/filter/extension/kernel_filters/ReflectionUtilities.hpp"
#include "plato/filter/extension/kernel_filters/test_utilities/SymmetryFilterTestUtilities.hpp"
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
const auto kPositiveXPlane = Plane{{0, 0, 0}, {1, 0, 0}};
const auto kPositiveYPlane = Plane{{0, 0, 0}, {0, 1, 0}};
const auto kPositiveZPlane = Plane{{0, 0, 0}, {0, 0, 1}};
const auto kSourcePoint = tpi::common::Coordinate{1, 1, 1};
const auto kTargetPoint = tpi::common::Coordinate{1, -1.1, 1};

const auto kReflectInputNodeCentered =
    test_utilities::make_filter_input<input_parser::positive_octant_reflect_filter>(kRadius, kNodeCentering);

const auto kReflectInputElementCentered =
    test_utilities::make_filter_input<input_parser::positive_octant_reflect_filter>(
        kRadius, input_parser::KernelFilterCenteringTypes::kElementCentered);

const auto kReflectLambdaDetail = [](const double aRadius, const input_parser::KernelFilterCenteringTypes& aCentering)
{ return detail::make_positive_octant_reflect_filter_type(aRadius, aCentering); };
const auto kReflectLambda = [](const input_parser::positive_octant_reflect_filter& aInput,
                               const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
{ return make_positive_octant_reflect_filter_type(aInput, aAnalysisDomainMesh); };

}  // namespace

TEST(ReflectFilter, MakeReflectFilterTypeRadiusAndCentering)
{
    test_utilities::check_make_filter_type_radius_and_centering(kReflectInputElementCentered, kReflectLambdaDetail,
                                                                TEST_CONTEXT("Reflect input element centered."));
    test_utilities::check_make_filter_type_radius_and_centering(kReflectInputNodeCentered, kReflectLambdaDetail,
                                                                TEST_CONTEXT("Reflect input node centered."));
}

TEST(ReflectFilter, MakeReflectFilterTypeSearchFunction)
{
    namespace tpic = third_party_integration::common;
    const auto tFilterType = detail::make_positive_octant_reflect_filter_type(kRadius, kNodeCentering);

    const std::vector<tpic::Coordinate> tSourceCoordinates{kSourcePoint};
    const std::vector<tpic::Coordinate> tTargetCoordinates{kSourcePoint, kTargetPoint};

    const auto tSearchResultsGold = distribute_search_vectors_and_stk_search_with_spheres(
        TargetRowVector{tTargetCoordinates}, SourceColumnVector{tSourceCoordinates}, kRadius,
        boost::mpi::communicator{}, kPositiveXPlane, kPositiveYPlane, kPositiveZPlane);

    const auto tFilterInfoSearchResults = tFilterType.mSearchFunction(
        TargetRowVector{tTargetCoordinates}, SourceColumnVector{tSourceCoordinates}, boost::mpi::communicator{});
    EXPECT_EQ(tSearchResultsGold, tFilterInfoSearchResults);
}

TEST(ReflectFilter, MakeReflectFilterTypeUnnormalizedWeightFunction)
{
    const auto tFilterType = detail::make_positive_octant_reflect_filter_type(kRadius, kNodeCentering);
    const double tDistance = minimum_distance_reflected_points_planes_list(
        SourcePoint{kSourcePoint}, TargetPoint{kTargetPoint}, kPositiveXPlane, kPositiveYPlane, kPositiveZPlane);
    const auto tUnnormalizedWeightGold = detail::linear_ramp_weight(Distance{tDistance}, SearchRadius{kRadius});
    const auto tFilterInfoUnnormalizedWeight =
        tFilterType.mUnnormalizedWeightFunction(SourcePoint{kSourcePoint}, TargetPoint{kTargetPoint});
    EXPECT_EQ(tUnnormalizedWeightGold, tFilterInfoUnnormalizedWeight);
}

TEST(ReflectFilter, MakeReflectFilterTypeTargetMeshFunction)
{
    test_utilities::check_target_mesh_function(kReflectInputElementCentered, kReflectLambda,
                                               TEST_CONTEXT("Reflect filter target mesh name check."));
}

TEST(ReflectFilter, MakeReflectFilterType)
{
    test_utilities::check_make_function_from_input_against_parameter_function(
        kReflectInputNodeCentered, kReflectLambdaDetail, kReflectLambda,
        TEST_CONTEXT("Parameter make function and non-detail for reflect filter"));
}

TEST_F(ThirtyDegreeWedgeMesh, ValidateSourceMeshInPositiveOctantValid)
{
    const auto tErrorMessage =
        detail::validate_source_mesh_in_positive_octant(input_parser::positive_octant_reflect_filter{}, mMeshFilePath);
    EXPECT_FALSE(tErrorMessage.has_value()) << "Valid source mesh";
}

TEST_F(TwoDThreeBlockMesh, ValidateSourceMeshInPositiveOctantInvalid)
{
    const auto tErrorMessage =
        detail::validate_source_mesh_in_positive_octant(input_parser::positive_octant_reflect_filter{}, mMeshFilePath);
    EXPECT_TRUE(tErrorMessage.has_value()) << "Invalid source mesh, has extents in z.";
}

}  // namespace plato::filter::extension::kernel_filters::unittest
