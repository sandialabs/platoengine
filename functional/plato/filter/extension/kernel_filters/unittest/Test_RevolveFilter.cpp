#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <filesystem>

#include "plato/filter/extension/kernel_filters/CylindricalMaskUtilities.hpp"
#include "plato/filter/extension/kernel_filters/KernelFilter.hpp"
#include "plato/filter/extension/kernel_filters/RevolveFilter.hpp"
#include "plato/filter/extension/kernel_filters/test_utilities/SymmetryFilterTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::filter::extension::kernel_filters::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::XZPlaneMesh;
namespace tpi = third_party_integration;
const auto kRadius = .25;
const auto kNodeCentering = input_parser::KernelFilterCenteringTypes::kNodeCentered;
const auto kThetaDirection = third_party_integration::common::Vector3{0, 1, 0};
const auto kTestPoint = tpi::common::Coordinate{0.75, 0.1, 0};
const auto kTargetZerothCoordinate = tpi::common::Coordinate{1, 0, 0};
const auto kTargetFirstCoordinate = tpi::common::Coordinate{0, 1, 0};

const auto kRevolveInputNodeCentered =
    test_utilities::make_filter_input<input_parser::y_axis_revolve_filter>(kRadius, kNodeCentering);

const auto kRevolveInputElementCentered = test_utilities::make_filter_input<input_parser::y_axis_revolve_filter>(
    kRadius, input_parser::KernelFilterCenteringTypes::kElementCentered);

const auto kRevolveLambdaDetail = [](const double aRadius, const input_parser::KernelFilterCenteringTypes& aCentering)
{ return detail::make_y_axis_revolve_filter_type(aRadius, aCentering); };
const auto kRevolveLambda =
    [](const input_parser::y_axis_revolve_filter& aInput, const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
{ return make_y_axis_revolve_filter_type(aInput, aAnalysisDomainMesh); };

}  // namespace

TEST(RevolveFilter, MakeRevolveFilterTypeRadiusAndCentering)
{
    test_utilities::check_make_filter_type_radius_and_centering(kRevolveInputElementCentered, kRevolveLambdaDetail,
                                                                TEST_CONTEXT("Revolve input element centered."));
    test_utilities::check_make_filter_type_radius_and_centering(kRevolveInputNodeCentered, kRevolveLambdaDetail,
                                                                TEST_CONTEXT("Revolve input node centered."));
}

TEST(RevolveFilter, MakeRevolveFilterTypeSearchFunction)
{
    namespace tpic = third_party_integration::common;
    const auto tFilterType = detail::make_y_axis_revolve_filter_type(kRadius, kNodeCentering);

    const std::vector<tpic::Coordinate> tSourceCoordinates{kTestPoint};
    const std::vector<tpic::Coordinate> tTargetCoordinates{kTargetZerothCoordinate, kTargetFirstCoordinate};

    const auto tCylindricalPolarSources =
        SourceColumnVector{detail::cylindrical_polar_coordinates(tSourceCoordinates).mValue};
    const auto tCylindricalPolarTargets =
        TargetRowVector{detail::cylindrical_polar_coordinates(tTargetCoordinates).mValue};

    const auto tDimension = double{2 * std::numbers::pi};
    const auto tSearchResultsGold = detail::distribute_search_vectors_and_stk_search_with_cylinders(
        tCylindricalPolarTargets, tCylindricalPolarSources, CylinderAxis{kThetaDirection * tDimension},
        CylinderRadius{kRadius}, boost::mpi::communicator{});

    const auto tFilterInfoSearchResults = tFilterType.mSearchFunction(
        TargetRowVector{tTargetCoordinates}, SourceColumnVector{tSourceCoordinates}, boost::mpi::communicator{});
    EXPECT_EQ(tSearchResultsGold, tFilterInfoSearchResults);
}

TEST(RevolveFilter, MakeRevolveFilterTypeUnnormalizedWeightFunction)
{
    const auto tFilterType = detail::make_y_axis_revolve_filter_type(kRadius, kNodeCentering);

    const auto tUnnormalizedWeightGold = detail::cylinder_ramp_weight(
        detail::CylinderCenter{detail::cylindrical_polar_coordinates(kTargetFirstCoordinate).mValue},
        CylinderAxis{kThetaDirection}, CylinderRadius{kRadius},
        detail::QueryLocation{detail::cylindrical_polar_coordinates(kTestPoint).mValue});

    const auto tFilterInfoUnnormalizedWeight =
        tFilterType.mUnnormalizedWeightFunction(SourcePoint{kTestPoint}, TargetPoint{kTargetFirstCoordinate});
    EXPECT_EQ(tUnnormalizedWeightGold, tFilterInfoUnnormalizedWeight);
}

TEST(RevolveFilter, MakeRevolveFilterTypeTargetMeshFunction)
{
    test_utilities::check_target_mesh_function(kRevolveInputElementCentered, kRevolveLambda,
                                               TEST_CONTEXT("Revolve filter target mesh name check."));
}

TEST(RevolveFilter, MakeRevolveFilterType)
{
    test_utilities::check_make_function_from_input_against_parameter_function(
        kRevolveInputNodeCentered, kRevolveLambdaDetail, kRevolveLambda,
        TEST_CONTEXT("Parameter make function and non-detail for revolve filter"));
}

TEST(RevolveFilterDetail, ValidateSourceMeshInXZPlane)
{
    const auto t2DXYSourceFilePath = utilities::data_file_path("four_tri_two_block.cdf");
    ASSERT_TRUE(t2DXYSourceFilePath.has_value());
    {
        const auto tErrorMessage = detail::validate_source_mesh_in_xz_plane(input_parser::y_axis_revolve_filter{},
                                                                            t2DXYSourceFilePath.value());
        EXPECT_TRUE(tErrorMessage.has_value()) << "Four tri is in XY plane but revolve needs XZ plane only";
    }
    {
        const auto tThreeDFilePath = std::filesystem::path{"three_d.exo"};
        namespace tpis = third_party_integration::stk_io;
        const auto tCommandGenerator =
            tpis::CommandGenerator{{10, 10, 10}, {-5, -5, -5}, {5, 5, 5}, tpis::CommandElementType::Tet};
        tpis::write_mesh(tThreeDFilePath, tCommandGenerator.toString());
        const auto tErrorMessage =
            detail::validate_source_mesh_in_xz_plane(input_parser::y_axis_revolve_filter{}, tThreeDFilePath);
        EXPECT_TRUE(tErrorMessage.has_value()) << "3D Mesh but revolve needs XZ plane only";
        std::filesystem::remove(tThreeDFilePath);
    }
}

TEST_F(XZPlaneMesh, ValidateSourceMeshInXZPlane)
{
    const auto tErrorMessage =
        detail::validate_source_mesh_in_xz_plane(input_parser::y_axis_revolve_filter{}, mMeshFilePath);
    EXPECT_FALSE(tErrorMessage.has_value()) << "Valid source mesh";
}

}  // namespace plato::filter::extension::kernel_filters::unittest
