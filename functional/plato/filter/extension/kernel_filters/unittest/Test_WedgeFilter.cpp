#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/filter/extension/kernel_filters/KernelFilter.hpp"
#include "plato/filter/extension/kernel_filters/ReflectionUtilities.hpp"
#include "plato/filter/extension/kernel_filters/WedgeFilter.hpp"
#include "plato/filter/extension/kernel_filters/test_utilities/SymmetryFilterTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::filter::extension::kernel_filters::unittest
{
namespace
{
using WedgeValidationTet4MeshOnDisk = third_party_integration::stk_io::test_utilities::Tet4MeshOnDisk;
using WedgeValidationThirtyDegreeWedgeMesh = third_party_integration::stk_io::test_utilities::ThirtyDegreeWedgeMesh;
using WedgeValidationTwoDThreeBlockMesh = third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;
namespace tpi = third_party_integration;
const auto kRadius = .5;
const auto kNodeCentering = input_parser::KernelFilterCenteringTypes::kNodeCentered;
const auto kWedgeAngle = 30U;
const auto kSourcePoint = tpi::common::Coordinate{1, 0, 1};
const auto kTargetPoint = tpi::common::Coordinate{0, -1.1, 1};

const auto kWedgeInputNodeCentered =
    input_parser::wedge_filter{/*.filter_radius=*/kRadius,
                               /*.centering_type=*/kNodeCentering,
                               /*.use_relative_radius=*/boost::none,
                               /*.target_mesh_name=*/input_parser::FileName{"target.exo"},
                               /*.number_of_processors=*/boost::none,
                               /*.wedge_angle=*/kWedgeAngle,
                               /*.fixed_blocks=*/boost::none};

const auto kWedgeInputElementCentered =
    input_parser::wedge_filter{/*.filter_radius=*/kRadius,
                               /*.centering_type=*/input_parser::KernelFilterCenteringTypes::kElementCentered,
                               /*.use_relative_radius=*/boost::none,
                               /*.target_mesh_name=*/input_parser::FileName{"target.exo"},
                               /*.number_of_processors=*/boost::none,
                               /*.wedge_angle=*/kWedgeAngle,
                               /*.fixed_blocks=*/boost::none};

const auto kWedgeLambdaDetail = [](const double aRadius, const input_parser::KernelFilterCenteringTypes& aCentering)
{ return detail::make_wedge_filter_type(aRadius, aCentering, kWedgeAngle); };
const auto kWedgeLambda =
    [](const input_parser::wedge_filter& aInput, const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
{ return make_wedge_filter_type(aInput, aAnalysisDomainMesh); };
}  // namespace

TEST(WedgeFilter, MakeWedgeFilterTypeRadiusAndCentering)
{
    test_utilities::check_make_filter_type_radius_and_centering(kWedgeInputElementCentered, kWedgeLambdaDetail,
                                                                TEST_CONTEXT("Wedge input element centered."));
    test_utilities::check_make_filter_type_radius_and_centering(kWedgeInputNodeCentered, kWedgeLambdaDetail,
                                                                TEST_CONTEXT("Wedge input node centered."));
}

TEST(WedgeFilter, MakeWedgeFilterTypeSearchFunction)
{
    namespace tpic = third_party_integration::common;
    const auto tFilterType = detail::make_wedge_filter_type(kRadius, kNodeCentering, kWedgeAngle);

    const std::vector<tpic::Coordinate> tSourceCoordinates{kSourcePoint};
    const std::vector<tpic::Coordinate> tTargetCoordinates{kSourcePoint, kTargetPoint};

    const auto tWedge = positive_quadrant_wedge(kWedgeAngle);
    const auto tSearchResultsGold = distribute_search_vectors_and_stk_search_with_spheres(
        TargetRowVector{tTargetCoordinates}, SourceColumnVector{tSourceCoordinates}, kRadius,
        boost::mpi::communicator{}, tWedge);

    const auto tFilterInfoSearchResults = tFilterType.mSearchFunction(
        TargetRowVector{tTargetCoordinates}, SourceColumnVector{tSourceCoordinates}, boost::mpi::communicator{});
    EXPECT_EQ(tSearchResultsGold, tFilterInfoSearchResults);
}

TEST(WedgeFilter, MakeWedgeFilterTypeUnnormalizedWeightFunction)
{
    const auto tFilterType = detail::make_wedge_filter_type(kRadius, kNodeCentering, kWedgeAngle);

    const auto tWedge = positive_quadrant_wedge(kWedgeAngle);
    const double tDistance =
        minimum_distance_reflected_points_wedge(SourcePoint{kSourcePoint}, TargetPoint{kTargetPoint}, tWedge);
    const auto tUnnormalizedWeightGold = detail::linear_ramp_weight(Distance{tDistance}, SearchRadius{kRadius});

    const auto tFilterInfoUnnormalizedWeight =
        tFilterType.mUnnormalizedWeightFunction(SourcePoint{kSourcePoint}, TargetPoint{kTargetPoint});
    EXPECT_EQ(tUnnormalizedWeightGold, tFilterInfoUnnormalizedWeight);
}

TEST(WedgeFilter, MakeWedgeFilterTypeTargetMeshFunction)
{
    test_utilities::check_target_mesh_function(kWedgeInputElementCentered, kWedgeLambda,
                                               TEST_CONTEXT("Wedge filter target mesh name check."));
}

TEST(WedgeFilter, MakeWedgeFilterTypeTetSource)
{
    test_utilities::check_make_function_from_input_against_parameter_function(
        kWedgeInputNodeCentered, kWedgeLambdaDetail, kWedgeLambda,
        TEST_CONTEXT("Parameter make function and non-detail for wedge filter"));
}

TEST(WedgeFilterDetail, ValidateWedgeAngleCommensurateWith360)
{
    auto tInput = kWedgeInputElementCentered;
    {
        const auto tErrorMessage = detail::validate_wedge_angle_commensurate_with_360(tInput);
        EXPECT_FALSE(tErrorMessage.has_value()) << "Valid angle specified.";
    }
    {
        tInput.wedge_angle = 23;
        const auto tErrorMessage = detail::validate_wedge_angle_commensurate_with_360(tInput);
        EXPECT_TRUE(tErrorMessage.has_value()) << "Invalid angle specified, does not evenly divide 360.";
    }
    {
        tInput.wedge_angle = 45 / 2.0;
        const auto tErrorMessage = detail::validate_wedge_angle_commensurate_with_360(tInput);
        EXPECT_FALSE(tErrorMessage.has_value()) << "Valid angle specified, does rationally divide 360.";
    }
    {
        tInput.wedge_angle = boost::none;
        const auto tErrorMessage = detail::validate_wedge_angle_commensurate_with_360(tInput);
        EXPECT_TRUE(tErrorMessage.has_value()) << "No angle specified.";
    }
}
TEST(WedgeFilterDetail, ValidateWedgeAngleEvenDihedral)
{
    auto tInput = kWedgeInputElementCentered;
    {
        const auto tErrorMessage = detail::validate_wedge_angle_even_dihedral(tInput);
        EXPECT_FALSE(tErrorMessage.has_value()) << "Valid angle specified.";
    }
    {
        tInput.wedge_angle = 360.0 / 5.0;
        const auto tErrorMessage = detail::validate_wedge_angle_even_dihedral(tInput);
        EXPECT_TRUE(tErrorMessage.has_value()) << "Invalid angle specified, does not create even number of wedges.";
    }
    {
        tInput.wedge_angle = 360 / 10.0;
        const auto tErrorMessage = detail::validate_wedge_angle_even_dihedral(tInput);
        EXPECT_FALSE(tErrorMessage.has_value()) << "Valid angle specified, does create even number of wedges.";
    }
}

TEST_F(WedgeValidationThirtyDegreeWedgeMesh, ValidateSourceMeshMatchesWedgeAngleValid)
{
    const auto tErrorMessage =
        detail::validate_source_mesh_matches_wedge_angle(kWedgeInputElementCentered, mMeshFilePath);
    EXPECT_FALSE(tErrorMessage.has_value()) << "Valid source mesh. " << tErrorMessage.value();
}

TEST_F(WedgeValidationTet4MeshOnDisk, ValidateSourceMeshMatchesWedgeAngleInvalid)
{
    const auto tErrorMessage =
        detail::validate_source_mesh_matches_wedge_angle(kWedgeInputElementCentered, mMeshFilePath);
    EXPECT_TRUE(tErrorMessage.has_value()) << "Source mesh wedge angle does not match input angle specified.";
}

TEST_F(WedgeValidationTwoDThreeBlockMesh, ValidateSourceMeshWedgePositiveYInvalid)
{
    const auto tErrorMessage = detail::validate_source_mesh_wedge_positive_y(kWedgeInputElementCentered, mMeshFilePath);
    EXPECT_TRUE(tErrorMessage.has_value()) << "Source mesh has negative y values in the wedge.";
}

TEST_F(WedgeValidationThirtyDegreeWedgeMesh, ValidateSourceMeshWedgePositiveYValid)
{
    const auto tErrorMessage = detail::validate_source_mesh_wedge_positive_y(kWedgeInputElementCentered, mMeshFilePath);
    EXPECT_FALSE(tErrorMessage.has_value()) << "Valid source mesh.";
}

}  // namespace plato::filter::extension::kernel_filters::unittest
