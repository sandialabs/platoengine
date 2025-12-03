#include <gtest/gtest.h>

#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/MetaData.hpp>

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/GradientChecker.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/krino/TriangleUtilities.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::third_party_integration::krino::unittest
{

TEST(KrinoTriangleUtilities, GetInterfaceTriangles)
{
    constexpr double tTolerance{1e-14};
    constexpr double tGoldArea{0.5};
    constexpr size_t tNumTris{2};
    const std::vector<common::Vector3> tGoldNormals{{.x = 0, .y = 0, .z = 1}, {.x = 0, .y = 0, .z = -1}};
    const std::vector<std::string> tReferenceBlockNames{"block_1", "block_2"};
    const auto tMeshPath = std::filesystem::path{"temp_mesh_save.exo"};
    constexpr auto tMesh = std::string_view{
        "textmesh:"
        "0,1,TET_4,5,1,2,3,block_1\n"
        "0,2,TET_4,6,5,2,3,block_1\n"
        "0,3,TET_4,6,7,5,3,block_1\n"
        "0,4,TET_4,6,4,7,3,block_1\n"
        "0,5,TET_4,6,2,4,3,block_1\n"
        "0,6,TET_4,6,8,7,4,block_1\n"
        "0,7,TET_4,9,5,6,7,block_2\n"
        "0,8,TET_4,10,9,6,7,block_2\n"
        "0,9,TET_4,10,11,9,7,block_2\n"
        "0,10,TET_4,10,8,11,7,block_2\n"
        "0,11,TET_4,10,6,8,7,block_2\n"
        "0,12,TET_4,10,12,11,8,block_2\n"
        "|coordinates: 0,-1,-1,0,0,-1,1,-1,-1,1,0,-1,0,-1,1,0,0,1,1,-1,1,1,0,1,0,-1,3,0,0,3,1,-1,3,1,0,3"
        "|dimension:3|sideset:name=my_ss;data=7,2,11,2"};  // data=<tet_id>,<side_id>,<tet_id>,<side_id>...
    stk_io::write_mesh(tMeshPath, tMesh);
    const auto tBulkData = stk_io::read_mesh_bulk_data(tMeshPath);
    for (const auto [tBlockName, tGoldNormal] : utilities::Zip(tReferenceBlockNames, tGoldNormals))
    {
        const std::vector<SensitivityTriangle> tTriangles = detail::get_interface_triangles(
            *tBulkData, "my_ss", PartReferenceVector{std::cref(*tBulkData->mesh_meta_data().get_part(tBlockName))});
        EXPECT_EQ(tTriangles.size(), tNumTris);
        for (size_t i = 0; i < tNumTris; ++i)
        {
            EXPECT_NEAR(tTriangles[i].area(), tGoldArea, tTolerance);
            const common::UnitVector3 tNormal = tTriangles[i].normal();
            EXPECT_NEAR(tNormal.x, tGoldNormal.x, tTolerance);
            EXPECT_NEAR(tNormal.y, tGoldNormal.y, tTolerance);
            EXPECT_NEAR(tNormal.z, tGoldNormal.z, tTolerance);
        }
    }
    std::filesystem::remove(tMeshPath);
}

void test_d_func_d_coords(const double aAbsoluteError, const auto& aF, const auto& aDF)
{
    const auto tGradientCheckParameters =
        plato::test_utilities::GradientCheckParameters{.mStepDelta = 0.5, .mNumSteps = 5, .mInitialStepSize = .000125};
    const auto tGradientCheck = plato::test_utilities::GradientChecker{aF, aDF};
    const auto tNodalCoordinates = linear_algebra::DynamicVector<double>{0.1, -0.9, .8, -.3, -.3, -.3, .5, .67, .1};
    const auto tDirection = linear_algebra::DynamicVector<double>{.10, -.10, 0.05, 0.03, -0.09, 0.2, -.04, -.3, .07};

    EXPECT_NEAR(tGradientCheck.maxFirstOrderTruncationError(tNodalCoordinates, tDirection, tGradientCheckParameters),
                0.0, aAbsoluteError)
        << tGradientCheck.table(tNodalCoordinates, tDirection, tGradientCheckParameters);
}

SensitivityTriangle create_sensitivity_triangle_from_coords(const std::vector<double>& aCoords)
{
    const common::Coordinate tNode1{.x = aCoords[0], .y = aCoords[1], .z = aCoords[2]};
    const common::Coordinate tNode2{.x = aCoords[3], .y = aCoords[4], .z = aCoords[5]};
    const common::Coordinate tNode3{.x = aCoords[6], .y = aCoords[7], .z = aCoords[8]};
    return SensitivityTriangle{NodeIDCoordsPair{1, tNode1}, NodeIDCoordsPair{2, tNode2}, NodeIDCoordsPair{3, tNode3}};
}

TEST(KrinoTriangleUtilities, dAreadCoords)
{
    constexpr auto tAbsoluteError = 2e-4;
    const auto tF = [](const linear_algebra::DynamicVector<double>& aX)
    {
        SensitivityTriangle tTriangle = create_sensitivity_triangle_from_coords(aX.stdVector());
        return tTriangle.area();
    };
    const auto tDf =
        [](const linear_algebra::DynamicVector<double>& aX, const linear_algebra::DynamicVector<double>& aV)
    {
        const auto tGradient =
            linear_algebra::DynamicVector<double>{detail::get_d_area_d_nodal_coords_from_tri_coords(aX.stdVector())};
        return tGradient.dot(aV);
    };

    test_d_func_d_coords(tAbsoluteError, tF, tDf);
}

namespace
{

constexpr size_t kXComponent{0};
constexpr size_t kYComponent{1};
constexpr size_t kZComponent{2};
constexpr double kDNormalAbsoluteError{1e-4};

[[nodiscard]] double calculate_d_normal(const linear_algebra::DynamicVector<double>& aX,
                                        const linear_algebra::DynamicVector<double>& aV,
                                        const size_t aDimensionIndex)
{
    constexpr size_t tNumDimensions = 3;
    constexpr size_t tNumNumSensComponents = 9;
    const auto tGradient =
        linear_algebra::DynamicVector<double>{detail::get_d_normal_d_nodal_coords_from_tri_coords(aX.stdVector())};
    double tDot = 0.0;
    for (size_t i = 0; i < tNumNumSensComponents; ++i)
    {
        tDot += tGradient[tNumDimensions * i + aDimensionIndex] * aV[i];
    }
    return tDot;
}

}  // namespace

TEST(KrinoTriangleUtilities, dNormaldCoordsX)
{
    const auto tF = [](const linear_algebra::DynamicVector<double>& aX)
    {
        SensitivityTriangle tTriangle = create_sensitivity_triangle_from_coords(aX.stdVector());
        return tTriangle.normal().x;
    };
    const auto tDf =
        [](const linear_algebra::DynamicVector<double>& aX, const linear_algebra::DynamicVector<double>& aV)
    { return calculate_d_normal(aX, aV, kXComponent); };

    test_d_func_d_coords(kDNormalAbsoluteError, tF, tDf);
}

TEST(KrinoTriangleUtilities, dNormaldCoordsY)
{
    const auto tF = [](const linear_algebra::DynamicVector<double>& aX)
    {
        SensitivityTriangle tTriangle = create_sensitivity_triangle_from_coords(aX.stdVector());
        return tTriangle.normal().y;
    };
    const auto tDf =
        [](const linear_algebra::DynamicVector<double>& aX, const linear_algebra::DynamicVector<double>& aV)
    { return calculate_d_normal(aX, aV, kYComponent); };

    test_d_func_d_coords(kDNormalAbsoluteError, tF, tDf);
}

TEST(KrinoTriangleUtilities, dNormaldCoordsZ)
{
    const auto tF = [](const linear_algebra::DynamicVector<double>& aX)
    {
        SensitivityTriangle tTriangle = create_sensitivity_triangle_from_coords(aX.stdVector());
        return tTriangle.normal().z;
    };
    const auto tDf =
        [](const linear_algebra::DynamicVector<double>& aX, const linear_algebra::DynamicVector<double>& aV)
    { return calculate_d_normal(aX, aV, kZComponent); };

    test_d_func_d_coords(kDNormalAbsoluteError, tF, tDf);
}

}  // namespace plato::third_party_integration::krino::unittest
