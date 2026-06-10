#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <boost/none.hpp>
#include <cmath>
#include <filesystem>
#include <numbers>
#include <optional>
#include <string>
#include <vector>

#include "plato/filter/extension/kernel_filters/GeneralPlaneReflectFilter.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilterCenteringTypes.hpp"
#include "plato/filter/extension/kernel_filters/Plane.hpp"
#include "plato/filter/extension/kernel_filters/ReflectionUtilities.hpp"
#include "plato/filter/extension/kernel_filters/ReliableMeshZeroValue.hpp"
#include "plato/filter/extension/kernel_filters/SphericalMaskUtilities.hpp"
#include "plato/filter/extension/kernel_filters/test_utilities/ReflectionMeshTestUtility.hpp"
#include "plato/filter/extension/kernel_filters/test_utilities/SymmetryFilterTestUtilities.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/test_utilities/ValidationTestUtilities.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::filter::extension::kernel_filters::unittest
{
namespace
{
using GeneralReflectWedgeFixture = third_party_integration::stk_io::test_utilities::ThirtyDegreeWedgeMesh;

namespace tpi = third_party_integration;
constexpr auto kRadius = .5;
constexpr auto kNodeCentering = input_parser::KernelFilterCenteringTypes::kNodeCentered;
const auto kGeneralPlane = Plane{{1, 0, 0}, {1, 0, 0}};
constexpr auto kSourcePoint = tpi::common::Coordinate{1.1, 1, 1};
constexpr auto kTargetPoint = tpi::common::Coordinate{0.9, 1, 1};

const auto kGeneralReflectInputBase = input_parser::general_plane_reflect_filter{
    kRadius,
    kNodeCentering,
    /*.use_relative_radius=*/boost::none,
    /*.target_mesh_name=*/input_parser::FileName{"target_mesh.exo"},
    /*.number_of_processors=*/boost::none,
    /*.fixed_blocks=*/boost::none,
    /*.plane=*/input_parser::GeneralPlane{input_parser::Point{0, 0, 0}, input_parser::Point{1, 0, 0}}};

[[nodiscard]] auto general_reflect_filter_node_centered() -> input_parser::general_plane_reflect_filter
{
    auto tInput = kGeneralReflectInputBase;
    tInput.plane = input_parser::GeneralPlane{input_parser::Point{1, 0, 0}, input_parser::Point{1, 0, 0}};
    return tInput;
}

[[nodiscard]] auto general_reflect_filter_element_centered() -> input_parser::general_plane_reflect_filter
{
    auto tInput = kGeneralReflectInputBase;
    tInput.centering_type = input_parser::KernelFilterCenteringTypes::kElementCentered;
    tInput.plane = input_parser::GeneralPlane{input_parser::Point{1, 0, 0}, input_parser::Point{1, 0, 0}};
    return tInput;
}

const auto kReflectLambdaDetail = [](const double aRadius, const input_parser::KernelFilterCenteringTypes& aCentering)
{ return detail::make_general_plane_reflect_filter_type(aRadius, aCentering, kGeneralPlane); };
const auto kReflectLambda = [](const input_parser::general_plane_reflect_filter& aInput,
                               const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
{ return make_general_plane_reflect_filter_type(aInput, aAnalysisDomainMesh); };

}  // namespace

TEST(GeneralReflectFilter, MakeReflectFilterTypeRadiusAndCentering)
{
    test_utilities::check_make_filter_type_radius_and_centering(general_reflect_filter_element_centered(),
                                                                kReflectLambdaDetail,
                                                                TEST_CONTEXT("Reflect input element centered."));
    test_utilities::check_make_filter_type_radius_and_centering(
        general_reflect_filter_node_centered(), kReflectLambdaDetail, TEST_CONTEXT("Reflect input node centered."));
}

TEST(GeneralReflectFilter, MakeReflectFilterTypeSearchFunction)
{
    namespace tpic = third_party_integration::common;
    const auto tFilterType = detail::make_general_plane_reflect_filter_type(kRadius, kNodeCentering, kGeneralPlane);

    const std::vector<tpic::Coordinate> tSourceCoordinates{kSourcePoint};
    const std::vector<tpic::Coordinate> tTargetCoordinates{kSourcePoint, kTargetPoint};

    const auto tSearchResultsGold = distribute_search_vectors_and_stk_search_with_spheres(
        TargetRowVector{tTargetCoordinates}, SourceColumnVector{tSourceCoordinates}, kRadius,
        boost::mpi::communicator{}, kGeneralPlane);

    const auto tFilterInfoSearchResults = tFilterType.mKernelFilterFunctions.mSearchFunction(
        TargetRowVector{tTargetCoordinates}, SourceColumnVector{tSourceCoordinates}, boost::mpi::communicator{});
    EXPECT_EQ(tSearchResultsGold, tFilterInfoSearchResults);
}

TEST(GeneralReflectFilter, MakeReflectFilterTypeUnnormalizedWeightFunction)
{
    const auto tFilterType = detail::make_general_plane_reflect_filter_type(kRadius, kNodeCentering, kGeneralPlane);
    const double tDistance = minimum_distance_reflected_points_planes_list(SourcePoint{kSourcePoint},
                                                                           TargetPoint{kTargetPoint}, kGeneralPlane);
    const auto tUnnormalizedWeightGold = detail::linear_ramp_weight(Distance{tDistance}, SearchRadius{kRadius});
    const auto tFilterInfoUnnormalizedWeight = tFilterType.mKernelFilterFunctions.mUnnormalizedWeightFunction(
        SourcePoint{kSourcePoint}, TargetPoint{kTargetPoint});
    EXPECT_EQ(tUnnormalizedWeightGold, tFilterInfoUnnormalizedWeight);
}

TEST(GeneralReflectFilter, MakeReflectFilterTypeTargetMeshFunction)
{
    test_utilities::check_target_mesh_function(general_reflect_filter_element_centered(), kReflectLambda,
                                               TEST_CONTEXT("Reflect filter target mesh name check."));
}

TEST(GeneralReflectFilter, MakeReflectFilterType)
{
    test_utilities::check_make_function_from_input_against_parameter_function(
        general_reflect_filter_node_centered(), kReflectLambdaDetail, kReflectLambda,
        TEST_CONTEXT("Parameter make function and non-detail for reflect filter"));
}

TEST(GeneralReflectFilter, ValidationMessages)
{
    const auto tValidatePlane = [](const auto& aInput) { return detail::validate_required_plane(aInput); };
    auto tInput = input_parser::general_plane_reflect_filter{};
    plato::test_utilities::expect_input_is_invalid(TEST_CONTEXT("Missing plane definition is invalid."), tValidatePlane,
                                                   tInput);
    tInput.plane = input_parser::GeneralPlane{input_parser::Point{0, 0, 0}, input_parser::Point{0, 0, 0}};
    plato::test_utilities::expect_input_is_valid(
        TEST_CONTEXT("An invalid plane is defined is ok for this validation function."), tValidatePlane, tInput);
}

TEST(GeneralReflectFilter, Registration)
{
    EXPECT_TRUE(library::is_filter_function_registered("general_plane_reflect_filter"));
}

TEST(GeneralReflectFilter, ValidateRequiredPlaneNormalIsNonZero)
{
    const auto tValidatePlaneNormal = [](const auto& aInput)
    { return detail::validate_required_plane_normal_is_non_zero_magnitude(aInput); };

    auto tInput = input_parser::general_plane_reflect_filter{};
    tInput.plane = input_parser::GeneralPlane{input_parser::Point{0, 0, 0}, input_parser::Point{0, 0, 0}};
    plato::test_utilities::expect_input_is_invalid(TEST_CONTEXT("Zero normal vector is invalid."), tValidatePlaneNormal,
                                                   tInput);
    tInput.plane = input_parser::GeneralPlane{input_parser::Point{0, 0, 0}, input_parser::Point{1, 1, 1}};
    plato::test_utilities::expect_input_is_valid(TEST_CONTEXT("Unnormalized normal vector is valid."),
                                                 tValidatePlaneNormal, tInput);
}

TEST(GeneralReflectFilter, ValidateSourceMeshOnPositiveSideOfNormal)
{
    constexpr auto tAcceptableSmallNegative = -.9 * reliable_mesh_zero_value();
    constexpr auto tYCenter = 1.0;
    const auto tMeshFilePath = std::filesystem::path{"candidate_mesh.exo"};
    auto tInput = input_parser::general_plane_reflect_filter{};
    tInput.plane = input_parser::GeneralPlane{input_parser::Point{0, tYCenter, 0}, input_parser::Point{0, 1, 0}};

    const auto tValidateSourceMesh =
        [tMeshFilePath](const input_parser::general_plane_reflect_filter& aInput) -> std::optional<std::string>
    { return detail::validate_source_mesh_on_outward_normal_side_of_plane(aInput, tMeshFilePath); };

    test_utilities::create_mesh_for_reflection_filter(tMeshFilePath, /*aLowerBounds=*/{0.0, 0.0, 0.0});
    plato::test_utilities::expect_input_is_invalid(TEST_CONTEXT("Way below 1 source is not ok"), tValidateSourceMesh,
                                                   tInput);
    test_utilities::create_mesh_for_reflection_filter(tMeshFilePath,
                                                      /*aLowerBounds=*/{0.0, tYCenter + tAcceptableSmallNegative, 0.0});
    plato::test_utilities::expect_input_is_valid(TEST_CONTEXT("Slightly less than 1 source is ok"), tValidateSourceMesh,
                                                 tInput);
    test_utilities::create_mesh_for_reflection_filter(tMeshFilePath, /*aLowerBounds=*/{0.0, tYCenter, 1.0});
    plato::test_utilities::expect_input_is_valid(TEST_CONTEXT("Min 1 source is ok"), tValidateSourceMesh, tInput);
    test_utilities::create_mesh_for_reflection_filter(tMeshFilePath, /*aLowerBounds=*/{0.0, 2 * tYCenter, 1.0});
    plato::test_utilities::expect_input_is_valid(TEST_CONTEXT("Positive 2 in source is ok"), tValidateSourceMesh,
                                                 tInput);
    std::filesystem::remove(tMeshFilePath);
}

TEST_F(GeneralReflectWedgeFixture, ValidateSourceMeshOnPositiveSideOfNormal30DegreePlane)
{
    const auto tThirtyDegreesInRadians = std::numbers::pi / 6.0;
    auto tInput = input_parser::general_plane_reflect_filter{};
    tInput.plane = input_parser::GeneralPlane{
        /*.center=*/input_parser::Point{0, 0, 0},
        /*.normal=*/input_parser::Point{std::sin(tThirtyDegreesInRadians), std::cos(tThirtyDegreesInRadians), 0}};
    const auto tValidateSourceMesh =
        [this](const input_parser::general_plane_reflect_filter& aInput) -> std::optional<std::string>
    { return detail::validate_source_mesh_on_outward_normal_side_of_plane(aInput, mMeshFilePath); };
    plato::test_utilities::expect_input_is_valid(TEST_CONTEXT("Wedge source and plane definition valid"),
                                                 tValidateSourceMesh, tInput);
}

}  // namespace plato::filter::extension::kernel_filters::unittest
