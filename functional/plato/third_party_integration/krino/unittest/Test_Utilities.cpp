#include <gtest/gtest.h>

#include <Akri_Phase_Support.hpp>
#include <Akri_TriangleWithSensitivities.hpp>
#include <algorithm>
#include <filesystem>
#include <string_view>

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/GradientChecker.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/krino/SnappingParameters.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/third_party_integration/stk_io/BlockUtilities.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/utilities/DataFilePath.hpp"
#include "plato/utilities/Enumerate.hpp"

namespace plato::third_party_integration::krino::unittest
{

namespace
{
using krino::test_utilities::KrinoTestFixture;
constexpr auto kMeshName = std::string_view{"simple_mesh.exo"};
constexpr auto kWriteMeshName = std::string_view{"levelset_mesh.exo"};
const auto kOneTriMeshFilePath = utilities::data_file_path("one_tri.cdf");
const auto kFourTriTwoBlockMeshFilePath = utilities::data_file_path("four_tri_two_block.cdf");

const auto kGoldCutMeshWithVoidNodes = std::map<std::size_t, double>{
    {1, -1.0}, {2, -1.0}, {3, -1.0}, {4, -1.0}, {5, 1.0},  {6, 1.0},  {7, 1.0},  {8, 1.0}, {9, 0.0},
    {10, 0.0}, {11, 0.0}, {12, 0.0}, {13, 0.0}, {14, 0.0}, {15, 0.0}, {16, 0.0}, {17, 0.0}};

void create_mesh()
{
    const stk_io::CommandBounds tLowerBounds{0, 0, 0};
    const stk_io::CommandBounds tUpperBounds{1, 1, 1};
    const stk_io::CommandNumberOfElements tElements{1, 1, 1};

    stk_io::write_mesh(std::filesystem::path{kMeshName}, stk_io::CommandGenerator{tElements, tLowerBounds, tUpperBounds,
                                                                                  stk_io::CommandElementType::Tet});
}

[[nodiscard]] auto make_test_level_set_field(::krino::MeshInterface& aKrinoMesh) -> std::vector<::krino::LS_Field>
{
    std::vector<::krino::LS_Field> tField = ::krino::Phase_Support::get_levelset_fields(aKrinoMesh.meta_data());
    const auto tNodes = node_entities_in_mesh(aKrinoMesh, tField);

    std::vector<double> tVector(tNodes.size(), 1);
    auto tGenerator = [N = tNodes.size(), i = 0UL]() mutable -> double { return (i++ < N / 2) ? -1.0 : 1.0; };
    std::generate_n(tVector.begin(), tNodes.size(), tGenerator);

    for (const auto [tNode, tValue] : utilities::Zip(tNodes, tVector))
    {
        level_set_value(tField, tNode) = tValue;
    }
    return tField;
}

[[nodiscard]] auto read_coordinates_and_level_sets(const std::filesystem::path& aFilePath)
{
    return stk_io::read_nodal_field(aFilePath, get_level_set_field_name());
}

void run_cut_and_write_test(const VoidPhase& aVoidPhase,
                            const std::map<std::size_t, double>& aGold,
                            const plato::test_utilities::TestContext& aTestContext)
{
    create_mesh();
    const auto tMesh = read_and_setup_for_decomposition(kMeshName);
    const auto tLevelSetField = make_test_level_set_field(*tMesh);
    cut_mesh(tMesh->bulk_data(), tLevelSetField, SnappingParameters{});
    write_mesh(tMesh->bulk_data(), kWriteMeshName, aVoidPhase);

    const auto tCutLevelSetField = read_coordinates_and_level_sets(kWriteMeshName);
    EXPECT_EQ(tCutLevelSetField, aGold) << aTestContext;

    plato::test_utilities::test_for_existence_and_remove({kWriteMeshName}, TEST_CONTEXT("Write Mesh file"));
}

}  // namespace

TEST_F(KrinoTestFixture, ReadAndSetupForDecomposition)
{
    create_mesh();
    const auto tMesh = read_and_setup_for_decomposition(kMeshName);

    write_mesh(tMesh->bulk_data(), kWriteMeshName, VoidPhase::kIncludeInMesh);

    const auto tFieldNames = stk_io::nodal_field_names(kWriteMeshName);

    const auto tFieldNameGold = std::vector<std::string>{"coordinates",
                                                         "CDFEM_SNAP_DISPLACEMENTS",
                                                         "CDFEM_SNAP_DISPLACEMENTS_STKFS_O",
                                                         "CDFEM_UP_4_PARENT_NODE_IDS",
                                                         "CDFEM_UP_4_PARENT_NODE_WTS",
                                                         "DistanceCorrectionDenominator",
                                                         "DistanceCorrectionNumerator",
                                                         "LEVEL_SET",
                                                         "LEVEL_SET_STASH",
                                                         "LS"};
    EXPECT_EQ(tFieldNameGold, tFieldNames);
    std::filesystem::remove(kMeshName);
    plato::test_utilities::test_for_existence_and_remove({kWriteMeshName}, TEST_CONTEXT("Write Mesh file"));
}

TEST_F(KrinoTestFixture, CutMeshIncludeVoid)
{
    run_cut_and_write_test(VoidPhase::kIncludeInMesh, kGoldCutMeshWithVoidNodes, TEST_CONTEXT("Void Included"));
}

TEST_F(KrinoTestFixture, CutMeshExcludeVoid)
{
    auto tGold = kGoldCutMeshWithVoidNodes;
    tGold.erase(tGold.find(1), std::next(tGold.find(4)));

    run_cut_and_write_test(VoidPhase::kExcludeFromMesh, tGold, TEST_CONTEXT("Void not Included"));
}

TEST_F(KrinoTestFixture, BackgroundNodeIds)
{
    const auto tMesh = read_and_setup_for_decomposition(kOneTriMeshFilePath.value());
    const auto tLevelSetField = test_utilities::make_level_set_field_from_vector(*tMesh, {.75, -.25, -.25});
    cut_mesh(tMesh->bulk_data(), tLevelSetField, SnappingParameters{});

    const auto tResult = background_node_ids(*tMesh, tLevelSetField);
    const auto tGold = std::vector<stk::mesh::EntityId>{1, 2, 4};
    EXPECT_EQ(tGold, tResult);
}

TEST_F(KrinoTestFixture, BackgroundNodeIdsFourTri)
{
    test_utilities::four_tri_test_on_background_node_ids(TEST_CONTEXT("Four tri background nodes in serial."));
}

TEST_F(KrinoTestFixture, CutMeshNodeIds)
{
    const auto tMesh = read_and_setup_for_decomposition(kOneTriMeshFilePath.value());
    const auto tLevelSetField = test_utilities::make_level_set_field_from_vector(*tMesh, {.75, -.25, -.25});
    cut_mesh(tMesh->bulk_data(), tLevelSetField, SnappingParameters{});
    {
        const auto tResult = cut_mesh_node_ids(*tMesh, VoidPhase::kIncludeInMesh);
        const auto tGold = std::vector<stk::mesh::EntityId>{1, 2, 4, 5, 6};
        EXPECT_EQ(tGold, tResult) << "Include void.";
    }
    {
        const auto tResult = cut_mesh_node_ids(*tMesh, VoidPhase::kExcludeFromMesh);
        const auto tGold = std::vector<stk::mesh::EntityId>{1, 5, 6};
        EXPECT_EQ(tGold, tResult) << "Exclude void.";
    }
}

TEST_F(KrinoTestFixture, CutMeshNodeIdsFourTri)
{
    const auto tMesh = read_and_setup_for_decomposition(kFourTriTwoBlockMeshFilePath.value());
    const auto tLevelSetField =
        test_utilities::make_level_set_field_from_vector(*tMesh, {.75, -.25, -.25, 0.75, 0.75, 0.75});
    cut_mesh(tMesh->bulk_data(), tLevelSetField, SnappingParameters{});
    {
        const auto tResult = cut_mesh_node_ids(*tMesh, VoidPhase::kIncludeInMesh);
        const auto tGold = std::vector<stk::mesh::EntityId>{1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12};
        EXPECT_EQ(tGold, tResult) << "Include void.";
    }
    {
        const auto tResult = cut_mesh_node_ids(*tMesh, VoidPhase::kExcludeFromMesh);
        const auto tGold = std::vector<stk::mesh::EntityId>{1, 5, 6, 7, 8, 9, 10, 11, 12};
        EXPECT_EQ(tGold, tResult) << "Exclude void.";
    }
}

TEST_F(KrinoTestFixture, GetLevelSetValuesMakeFromFixed)
{
    constexpr auto tFixedValue = double{1.234};
    create_mesh();
    const auto tMesh = read_and_setup_for_decomposition(kMeshName);
    const auto tLevelSetFields = make_level_set_field_from_fixed_value(*tMesh, tFixedValue);
    const auto tLevelSetValues = get_level_set_values(*tMesh, tLevelSetFields);
    for (const auto& [tId, tValue] : tLevelSetValues)
    {
        EXPECT_EQ(tValue, tFixedValue);
    }

    std::filesystem::remove(kMeshName);
}

TEST_F(KrinoTestFixture, GetLevelSetValuesMakeFromVector)
{
    create_mesh();
    const auto tMesh = read_and_setup_for_decomposition(kMeshName);

    const auto tVector = std::vector<double>{1., 2, 3, 4, 5, 6, 7, 8};
    const auto tLevelSetFields = test_utilities::make_level_set_field_from_vector(*tMesh, tVector);
    const auto tLevelSetValues = get_level_set_values(*tMesh, tLevelSetFields);
    for (const auto& [tId, tValue] : tLevelSetValues)
    {
        EXPECT_EQ(tValue, tId);
    }

    std::filesystem::remove(kMeshName);
}

TEST(KrinoUtilities, GetInterfaceTriangles)
{
    constexpr double tTolerance{1e-14};
    constexpr double tGoldArea{0.5};
    constexpr size_t tNumTris{2};
    const std::vector<common::Vector3> tGoldNormals{{0, 0, 1}, {0, 0, -1}};
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
        const std::vector<stk_io::Triangle> tTriangles = get_interface_triangles(
            *tBulkData, "my_ss", PartReferenceVector{std::cref(*tBulkData->mesh_meta_data().get_part(tBlockName))});
        EXPECT_EQ(tTriangles.size(), tNumTris);
        for (size_t i = 0; i < tNumTris; ++i)
        {
            EXPECT_NEAR(tTriangles[i].volume(), tGoldArea, tTolerance);
            const common::Vector3 tNormal = tTriangles[i].normal();
            EXPECT_NEAR(tNormal.x, tGoldNormal.x, tTolerance);
            EXPECT_NEAR(tNormal.y, tGoldNormal.y, tTolerance);
            EXPECT_NEAR(tNormal.z, tGoldNormal.z, tTolerance);
        }
    }
    std::filesystem::remove(tMeshPath);
}

void testDFuncDCoords(const double aAbsoluteError, const auto& aF, const auto& aDF)
{
    const auto tGradientCheckParameters = plato::test_utilities::GradientCheckParameters{0.5, 10, .001};
    const auto tGradientCheck = plato::test_utilities::GradientChecker{aF, aDF};
    const auto tNodalCoordinates = linear_algebra::DynamicVector<double>{0.1, -0.9, .8, -.3, -.3, -.3, .5, .67, .1};
    const auto tDirection = linear_algebra::DynamicVector<double>{.10, -.10, 0.05, 0.03, -0.09, 0.2, -.04, -.3, .07};

    EXPECT_NEAR(tGradientCheck.maxFirstOrderTruncationError(tNodalCoordinates, tDirection, tGradientCheckParameters),
                0.0, aAbsoluteError)
        << tGradientCheck.table(tNodalCoordinates, tDirection, tGradientCheckParameters);
}

TEST(KrinoUtilities, dAreadCoords)
{
    constexpr auto tAbsoluteError = 4e-3;
    const auto tF = [](const linear_algebra::DynamicVector<double>& aX)
    { return get_tri_area_from_nodal_coords(aX.stdVector()); };
    const auto tDf =
        [](const linear_algebra::DynamicVector<double>& aX, const linear_algebra::DynamicVector<double>& aV)
    {
        const auto tGradient =
            linear_algebra::DynamicVector<double>{get_d_area_d_nodal_coords_from_tri_coords(aX.stdVector())};
        return tGradient.dot(aV);
    };

    testDFuncDCoords(tAbsoluteError, tF, tDf);
}

TEST(KrinoUtilities, dNormaldCoordsX)
{
    constexpr auto tAbsoluteError = 2e-3;
    constexpr size_t tNumDimensions = 3;
    constexpr size_t tNumNumSensComponents = 9;
    constexpr size_t tDimensionIndex = 0;
    const auto tF = [](const linear_algebra::DynamicVector<double>& aX)
    {
        const auto tNormal = linear_algebra::DynamicVector<double>{get_tri_normal_from_nodal_coords(aX.stdVector())};
        return tNormal[tDimensionIndex];
    };
    const auto tDf =
        [](const linear_algebra::DynamicVector<double>& aX, const linear_algebra::DynamicVector<double>& aV)
    {
        const auto tGradient =
            linear_algebra::DynamicVector<double>{get_d_normal_d_nodal_coords_from_tri_coords(aX.stdVector())};
        double tDot = 0.0;
        for (size_t i = 0; i < tNumNumSensComponents; ++i)
        {
            tDot += tGradient[tNumDimensions * i] * aV[i];
        }
        return tDot;
    };

    testDFuncDCoords(tAbsoluteError, tF, tDf);
}

TEST(KrinoUtilities, dNormaldCoordsY)
{
    constexpr auto tAbsoluteError = 2e-3;
    constexpr size_t tNumDimensions = 3;
    constexpr size_t tNumNumSensComponents = 9;
    constexpr size_t tDimensionIndex = 1;
    const auto tF = [](const linear_algebra::DynamicVector<double>& aX)
    {
        const auto tNormal = linear_algebra::DynamicVector<double>{get_tri_normal_from_nodal_coords(aX.stdVector())};
        return tNormal[tDimensionIndex];
    };
    const auto tDf =
        [](const linear_algebra::DynamicVector<double>& aX, const linear_algebra::DynamicVector<double>& aV)
    {
        const auto tGradient =
            linear_algebra::DynamicVector<double>{get_d_normal_d_nodal_coords_from_tri_coords(aX.stdVector())};
        double tDot = 0.0;
        for (size_t i = 0; i < tNumNumSensComponents; ++i)
        {
            tDot += tGradient[tNumDimensions * i + tDimensionIndex] * aV[i];
        }
        return tDot;
    };

    testDFuncDCoords(tAbsoluteError, tF, tDf);
}

TEST(KrinoUtilities, dNormaldCoordsZ)
{
    constexpr auto tAbsoluteError = 4e-3;
    constexpr size_t tNumDimensions = 3;
    constexpr size_t tNumNumSensComponents = 9;
    constexpr size_t tDimensionIndex = 2;
    const auto tF = [](const linear_algebra::DynamicVector<double>& aX)
    {
        const auto tNormal = linear_algebra::DynamicVector<double>{get_tri_normal_from_nodal_coords(aX.stdVector())};
        return tNormal[tDimensionIndex];
    };
    const auto tDf =
        [](const linear_algebra::DynamicVector<double>& aX, const linear_algebra::DynamicVector<double>& aV)
    {
        const auto tGradient =
            linear_algebra::DynamicVector<double>{get_d_normal_d_nodal_coords_from_tri_coords(aX.stdVector())};
        double tDot = 0.0;
        for (size_t i = 0; i < tNumNumSensComponents; ++i)
        {
            tDot += tGradient[tNumDimensions * i + tDimensionIndex] * aV[i];
        }
        return tDot;
    };

    testDFuncDCoords(tAbsoluteError, tF, tDf);
}

}  // namespace plato::third_party_integration::krino::unittest
