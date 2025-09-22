#include <gtest/gtest.h>
#include <mpi.h>

#include <filesystem>
#include <iomanip>
#include <numeric>
#include <string>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshOperators.hpp"
#include "plato/geometry/extension/KrinoWrapper.hpp"
#include "plato/geometry/extension/test_utilities/KrinoWrapperTestUtilities.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/utilities/DataFilePath.hpp"
#include "plato/utilities/Enumerate.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::geometry::extension::unittest
{

namespace
{
namespace tpik = third_party_integration::krino;

const auto kOneTriMeshFilePath = utilities::data_file_path("one_tri.cdf");
const auto kThreeQuarterOffsetXHatPlane = tpik::Plane{{-1, 0, 0}, 0.75};
const auto kPlaneNearBackgroundNode = tpik::Plane{{-1, -1, 0}, 1.0e-4};
const auto kFourTriTwoBlockMeshFilePath = utilities::data_file_path("four_tri_two_block.cdf");

constexpr auto kTemporaryMeshFile = std::string_view{"tmp.exo"};
const std::string_view kLogFile = "Krino_Test.txt";

using tpik::test_utilities::KrinoTestFixture;

void initialize_krino()
{
    static bool tFirstTime{true};
    if (tFirstTime)
    {
        tpik::initialize_environment_for_krino(kLogFile, MPI_COMM_SELF);
        tFirstTime = false;
    }
}

[[nodiscard]] auto create_krino_wrapper_test_command_generator() -> third_party_integration::stk_io::CommandGenerator
{
    const third_party_integration::stk_io::CommandBounds tLowerBounds{0, 0, 0};
    const third_party_integration::stk_io::CommandBounds tUpperBounds{1, 1, 1};
    const third_party_integration::stk_io::CommandNumberOfElements tElements{1, 1, 1};
    return third_party_integration::stk_io::CommandGenerator{tElements, tLowerBounds, tUpperBounds,
                                                             third_party_integration::stk_io::CommandElementType::Tet};
}

const auto kOneBlockMeshSensitivityMap = tpik::SensitivityMap{
    {17, tpik::LevelSetJacobianColumn{{2, 1}, {{0.750, -0.000, -0.000}, {0.250, -0.000, -0.000}}, {1, 0}}},
    {16, tpik::LevelSetJacobianColumn{{6, 5}, {{0.750, -0.000, -0.000}, {0.250, -0.000, -0.000}}, {5, 4}}},
    {15, tpik::LevelSetJacobianColumn{{1, 6}, {{0.250, 0.000, 0.250}, {0.750, 0.000, 0.750}}, {0, 5}}},
    {14, tpik::LevelSetJacobianColumn{{5, 8}, {{0.250, 0.250, 0.000}, {0.750, 0.750, 0.000}}, {4, 7}}},
    {13, tpik::LevelSetJacobianColumn{{7, 8}, {{0.250, 0.000, 0.000}, {0.750, 0.000, 0.000}}, {6, 7}}},
    {12, tpik::LevelSetJacobianColumn{{3, 8}, {{0.250, 0.000, 0.250}, {0.750, 0.000, 0.750}}, {2, 7}}},
    {10, tpik::LevelSetJacobianColumn{{4, 3}, {{0.750, -0.000, -0.000}, {0.250, -0.000, -0.000}}, {3, 2}}},
    {9, tpik::LevelSetJacobianColumn{{1, 4}, {{0.250, 0.250, 0.000}, {0.750, 0.750, 0.000}}, {0, 3}}},
    {11, tpik::LevelSetJacobianColumn{{1, 8}, {{0.250, 0.250, 0.250}, {0.750, 0.750, 0.750}}, {0, 7}}}};
}  // namespace

class OneBlockMeshKrinoFixture : public tpik::test_utilities::SensitivityTestKrinoFixture
{
   public:
    OneBlockMeshKrinoFixture()
        : SensitivityTestKrinoFixture(std::string{kTemporaryMeshFile}, kOneBlockMeshSensitivityMap)
    {
    }
    void SetUp() override
    {
        KrinoTestFixture::SetUp();
        third_party_integration::stk_io::write_mesh(std::string{kTemporaryMeshFile},
                                                    create_krino_wrapper_test_command_generator());
    }

    void TearDown() override
    {
        KrinoTestFixture::TearDown();
        std::filesystem::remove(std::string{kTemporaryMeshFile});
    }
};

struct KrinoWrapperGoldValues
{
    unsigned int mSpatialDimensions;
    unsigned int mNumberBackgroundNodes;
    unsigned int mNumberCutNodes;
    unsigned int mSensitivityMapSize;
    std::vector<double> mLevelSetValues;
    std::vector<double> mJacobianGold;
    std::vector<double> mAdjointJacobianGold;
};

struct KrinoWrapperTestFixtureInputs
{
    std::string mFileName;
    std::vector<double> mCutMeshRowVector;
    std::vector<double> mBackgroundRowVector;
    tpik::VoidPhase mVoidPhase;
};

namespace
{

const auto kUnitCubeLambda = []() -> KrinoWrapper
{
    initialize_krino();
    third_party_integration::stk_io::write_mesh(std::string{kTemporaryMeshFile},
                                                create_krino_wrapper_test_command_generator());
    return test_utilities::make_krino_wrapper_from_vector_values(
        kTemporaryMeshFile,
        test_utilities::InitialLevelSetValues{std::vector{-1.0, -1.0, -1.0, -1.0, 1.0, 1.0, 1.0, 1.0}});
};

const auto kUnitCubeInputs = KrinoWrapperTestFixtureInputs{/*mFileName=*/std::string{kTemporaryMeshFile},
                                                           /*mCutMeshRowVector=*/std::vector<double>(17U * 3U, 1.0),
                                                           /*mBackgroundRowVector=*/std::vector<double>(8U * 3U, 1.0),
                                                           /*mVoidPhase=*/tpik::VoidPhase::kExcludeFromMesh};

const auto kUnitCubeGoldValues = KrinoWrapperGoldValues{
    /*mSpatialDimensions=*/3U,
    /*mNumberBackgroundNodes=*/8U,
    /*mNumberCutNodes=*/13U,
    /*mSensitivityMapSize=*/9U,
    /*mLevelSetValues=*/{-1.0, -1.0, -1.0, -1.0, 1.0, 1.0, 1.0, 1.0},
    /*mJacobianGold=*/{-2.0, -0.75, -0.75, -0.25, -0.25, -0.75, -0.75, -2.0},
    /*mAdjointJacobianGold*/ std::vector<double>{0,    0, 0,    0,    0,    0,    0,    0,    0,    0,
                                                 0,    0, -0.5, -0.5, -0.5, 0,    0,    -0.5, -0.5, 0,
                                                 -0.5, 0, 0,    -0.5, 0,    -0.5, -0.5, 0,    0,    -0.5,
                                                 -0.5, 0, -0.5, 0,    0,    -0.5, 0,    -0.5, -0.5}};

const auto kOneTriLambda = []() -> KrinoWrapper
{
    initialize_krino();
    return test_utilities::make_krino_wrapper_from_level_set_primitives(
        kOneTriMeshFilePath.value(), tpik::LevelSetPrimitives{{kThreeQuarterOffsetXHatPlane}, {}});
};

const auto kOneTriInputs = KrinoWrapperTestFixtureInputs{/*mFileName=*/std::string{kOneTriMeshFilePath.value()},
                                                         /*mCutMeshRowVector=*/{1.0, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                                         /*mBackgroundRowVector=*/{1.0, 2, 3},
                                                         /*mVoidPhase=*/tpik::VoidPhase::kIncludeInMesh};

const auto kOneTriGoldValues =
    KrinoWrapperGoldValues{/*mSpatialDimensions=*/2U,
                           /*mNumberBackgroundNodes=*/3U,
                           /*mNumberCutNodes=*/5U,
                           /*mSensitivityMapSize=*/2U,
                           /*mLevelSetValues=*/{0.75, -0.25, -0.25},
                           /*mJacobianGold=*/{6.5, 5.25, 14.25},
                           /*mAdjointJacobianGold*/ std::vector<double>{0, 0, 0, 0, 0, 0, 1.75, 0, 2.5, 2.5}};

const auto kFourTriLambda = []() -> KrinoWrapper
{
    initialize_krino();
    const auto tFixedBlockNames = std::set<std::string>{"block_2"};
    return test_utilities::make_krino_wrapper_from_level_set_primitives(
        kFourTriTwoBlockMeshFilePath.value(), tpik::LevelSetPrimitives{{kThreeQuarterOffsetXHatPlane}, {}},
        tFixedBlockNames);
};

const auto kFourTriInputs = KrinoWrapperTestFixtureInputs{
    .mFileName = std::string{kFourTriTwoBlockMeshFilePath.value()},
    .mCutMeshRowVector = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0},
    .mBackgroundRowVector = {1.0, 2.0, 3.0, 4.0},
    .mVoidPhase = tpik::VoidPhase::kExcludeFromMesh};

const auto kFourTriFixedTwoGoldValues =
    KrinoWrapperGoldValues{.mSpatialDimensions = 2U,
                           .mNumberBackgroundNodes = 4U,
                           .mNumberCutNodes = 8U,
                           .mSensitivityMapSize = 3U,
                           .mLevelSetValues = {0.75, -0.25, -0.25, 0.75},
                           .mJacobianGold = {9.5, 31.5, 20.25, 7.75},
                           .mAdjointJacobianGold = std::vector{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.75,
                                                               0.0, 2.5, 2.5, 2.5, 2.5}};

const auto kFourTriInterfaceNearNodeLambda = []() -> KrinoWrapper
{
    initialize_krino();
    const auto tFixedBlockNames = std::set<std::string>{"block_2"};
    return test_utilities::make_krino_wrapper_from_level_set_primitives(
        kFourTriTwoBlockMeshFilePath.value(), tpik::LevelSetPrimitives{{kPlaneNearBackgroundNode}, {}},
        tFixedBlockNames);
};

const auto kFourTriInterfaceNearNodeInputs =
    KrinoWrapperTestFixtureInputs{/*mFileName=*/std::string{kFourTriTwoBlockMeshFilePath.value()},
                                  /*mCutMeshRowVector=*/{1.0, 2, 3, 4, 5, 6},  // three nodes in cut mesh
                                  /*mBackgroundRowVector=*/{1.0, 2, 3, 4},
                                  /*mVoidPhase=*/tpik::VoidPhase::kExcludeFromMesh};

const auto kFourTriInterfaceNearNodeGoldValues = KrinoWrapperGoldValues{
    /*mSpatialDimensions=*/2U,
    /*mNumberBackgroundNodes=*/4U,
    /*mNumberCutNodes=*/3U,
    /*mSensitivityMapSize=*/2U,
    /*mLevelSetValues=*/{0.75, -0.25, -0.25, 0.75},
    /*mJacobianGold=*/{2.1211703435596427, 3.8896372965260113, 0.0001500000000000, 3.8885372965260117},
    /*mAdjointJacobianGold=*/
    std::vector<double>{0.7072067811865476, 0.7072067811865476, 0, 0, 2.1212203435596426, 2.1212203435596426}};

}  // namespace

class KrinoWrapperTestFixtureBase : virtual public ::testing::Test, public KrinoTestFixture
{
   public:
    KrinoWrapperTestFixtureBase(const std::function<KrinoWrapper()>& aKrinoWrapperLambda,
                                const KrinoWrapperTestFixtureInputs& aKrinoWrapperTestFixtureInputs,
                                const KrinoWrapperGoldValues& aKrinoWrapperGoldValues)
        : mKrinoWrapper(aKrinoWrapperLambda()),
          mInputValues(aKrinoWrapperTestFixtureInputs),
          mGoldValues(aKrinoWrapperGoldValues)
    {
    }
    void TearDown() override
    {
        KrinoTestFixture::TearDown();
        std::filesystem::remove(std::string{kTemporaryMeshFile});
    }
    void runJacobianTest(const plato::test_utilities::TestContext& aTestContext)
    {
        EXPECT_EQ(mKrinoWrapper.value().sensitivities().size(), mGoldValues.mSensitivityMapSize) << aTestContext;
        const auto tJacobianRowVectorProduct =
            mKrinoWrapper.value().rowVectorJacobianProduct(mInputValues.mCutMeshRowVector, mInputValues.mVoidPhase);
        ASSERT_EQ(tJacobianRowVectorProduct.size(), mGoldValues.mNumberBackgroundNodes) << aTestContext;
        EXPECT_EQ(tJacobianRowVectorProduct, mGoldValues.mJacobianGold) << aTestContext;
    }

    void runAdjointJacobianTest(const plato::test_utilities::TestContext& aTestContext)
    {
        const auto tRowVectorAdjointJacobianProduct = mKrinoWrapper.value().rowVectorAdjointJacobianProduct(
            mInputValues.mBackgroundRowVector, mInputValues.mVoidPhase);
        ASSERT_EQ(tRowVectorAdjointJacobianProduct.size(), mGoldValues.mNumberCutNodes * mGoldValues.mSpatialDimensions)
            << aTestContext;
        EXPECT_EQ(tRowVectorAdjointJacobianProduct, mGoldValues.mAdjointJacobianGold) << aTestContext;
    }

    void runJacobianRegressionTest(const plato::test_utilities::TestContext& aTestContext)
    {
        EXPECT_EQ(mKrinoWrapper.value().sensitivities().size(), mGoldValues.mSensitivityMapSize) << aTestContext;
        const auto tJacobianRowVectorProduct =
            mKrinoWrapper.value().rowVectorJacobianProduct(mInputValues.mCutMeshRowVector, mInputValues.mVoidPhase);
        for (const auto [tJacobianRowVectorProductValue, tGoldValue] :
             utilities::Zip{tJacobianRowVectorProduct, mGoldValues.mJacobianGold})
        {
            EXPECT_DOUBLE_EQ(tJacobianRowVectorProductValue, tGoldValue);
        }
    }

   protected:
    std::optional<KrinoWrapper> mKrinoWrapper = std::nullopt;
    const KrinoWrapperTestFixtureInputs mInputValues;
    const KrinoWrapperGoldValues mGoldValues;
};

class KrinoWrapperTestFixtureUnitCube : public KrinoWrapperTestFixtureBase
{
   public:
    KrinoWrapperTestFixtureUnitCube()
        : KrinoWrapperTestFixtureBase(kUnitCubeLambda, kUnitCubeInputs, kUnitCubeGoldValues)
    {
    }
    void TearDown() override { KrinoWrapperTestFixtureBase::TearDown(); }
};

class KrinoWrapperTestFixtureOneTriMesh : public KrinoWrapperTestFixtureBase
{
   public:
    KrinoWrapperTestFixtureOneTriMesh() : KrinoWrapperTestFixtureBase(kOneTriLambda, kOneTriInputs, kOneTriGoldValues)
    {
    }
};

class KrinoWrapperTestFixtureFourTriMeshFixedBlockTwo : public KrinoWrapperTestFixtureBase
{
   public:
    KrinoWrapperTestFixtureFourTriMeshFixedBlockTwo()
        : KrinoWrapperTestFixtureBase(kFourTriLambda, kFourTriInputs, kFourTriFixedTwoGoldValues)
    {
    }
};

class KrinoWrapperTestFixtureFourTriInterfaceNearNode : public KrinoWrapperTestFixtureBase
{
   public:
    KrinoWrapperTestFixtureFourTriInterfaceNearNode()
        : KrinoWrapperTestFixtureBase(
              kFourTriInterfaceNearNodeLambda, kFourTriInterfaceNearNodeInputs, kFourTriInterfaceNearNodeGoldValues)
    {
    }
};

TEST_F(OneBlockMeshKrinoFixture, CutSphereOutOfBackgroundMesh)
{
    const auto tCutFilename = std::filesystem::path{"cut_mesh.exo"};

    constexpr double tRadius = .5;
    const double tDiagonalCoordinate = std::sqrt(3 * tRadius * tRadius) / 3.0;
    const double tFaceCoordinate = std::sqrt(2.0) / 2.0 * tRadius;

    const auto tSphere =
        tpik::LevelSetPrimitives{/*.mPlanes=*/{},
                                 /*.mSpheres=*/{tpik::Sphere{/*.mCenter=*/{0., 0., 0.}, /*.mRadius=*/tRadius}}};
    test_utilities::make_krino_wrapper_from_level_set_primitives(mFileName, tSphere)
        .writeCutMesh(tCutFilename, tpik::VoidPhase::kExcludeFromMesh);

    const auto tBulkData = third_party_integration::stk_io::read_mesh_bulk_data(tCutFilename);
    const auto tCoordinates = third_party_integration::stk_io::nodal_coordinates(*tBulkData);

    const auto tGoldCoordinates = std::vector<third_party_integration::common::Coordinate>{
        {1, 0, 0},
        {0, 1, 0},
        {1, 1, 0},
        {0, 0, 1},
        {1, 0, 1},
        {0, 1, 1},
        {1, 1, 1},
        {tFaceCoordinate, tFaceCoordinate, 0},
        {0, tRadius, 0},
        {tDiagonalCoordinate, tDiagonalCoordinate, tDiagonalCoordinate},
        {0, tFaceCoordinate, tFaceCoordinate},
        {0, 0, tRadius},
        {tFaceCoordinate, 0, tFaceCoordinate},
        {tRadius, 0, 0}};

    ASSERT_EQ(tCoordinates.size(), 14U);

    for (const auto [tResult, tGold] : utilities::Zip(tCoordinates, tGoldCoordinates))
    {
        third_party_integration::common::test_utilities::test_double_equality_of_components(
            tResult, tGold, TEST_CONTEXT("Coordinates of sphere cut out."));
    }
    std::filesystem::remove(tCutFilename);
}

TEST_F(OneBlockMeshKrinoFixture, CutPlaneBackgroundMesh)
{
    const auto tCutFilename = std::filesystem::path{"cut_mesh.exo"};
    constexpr double tOffset = -.5;
    const auto tPlane = tpik::LevelSetPrimitives{/*.mPlanes=*/{tpik::Plane{{0, 1, 0}, tOffset}}, /*.mSpheres=*/{}};
    test_utilities::make_krino_wrapper_from_level_set_primitives(mFileName, tPlane)
        .writeCutMesh(tCutFilename, tpik::VoidPhase::kIncludeInMesh);

    const auto tBulkData = third_party_integration::stk_io::read_mesh_bulk_data(tCutFilename);
    const auto tCoordinates = third_party_integration::stk_io::nodal_coordinates(*tBulkData);
    ASSERT_EQ(tCoordinates.size(), 17U);

    std::filesystem::remove(tCutFilename);
}

TEST_F(OneBlockMeshKrinoFixture, PlaneSensitivities)
{
    const auto tPlane = tpik::LevelSetPrimitives{/*.mPlanes=*/{kThreeQuarterOffsetXHatPlane},
                                                 /*.mSpheres=*/{}};
    const auto tSensitivities =
        test_utilities::make_krino_wrapper_from_level_set_primitives(mFileName, tPlane).sensitivities();
    ASSERT_EQ(tSensitivities.size(), 9U);

    compareMapAgainstBuiltInGold(tSensitivities, TEST_CONTEXT("One block mesh sensitivity check"));
}

TEST_F(KrinoWrapperTestFixtureUnitCube, Jacobian)
{
    runJacobianTest(TEST_CONTEXT("KrinoWrapperTestFixtureUnitCube Jacobian"));
}

TEST_F(KrinoWrapperTestFixtureOneTriMesh, Jacobian)
{
    runJacobianTest(TEST_CONTEXT("KrinoWrapperTestFixtureOneTriMesh Jacobian"));
}

TEST_F(KrinoWrapperTestFixtureFourTriMeshFixedBlockTwo, Jacobian)
{
    runJacobianTest(TEST_CONTEXT("KrinoWrapperTestFixtureFourTriMeshFixedBlockTwo Jacobian"));
}

TEST_F(KrinoWrapperTestFixtureFourTriInterfaceNearNode, JacobianRegression)
{
    runJacobianRegressionTest(TEST_CONTEXT("KrinoWrapperTestFixtureFourTriInterfaceNearNode JacobianRegression"));
}

TEST_F(KrinoWrapperTestFixtureUnitCube, AdjointJacobian)
{
    runAdjointJacobianTest(TEST_CONTEXT("KrinoWrapperTestFixtureUnitCube AdjointJacobian"));
}

TEST_F(KrinoWrapperTestFixtureOneTriMesh, AdjointJacobian)
{
    runAdjointJacobianTest(TEST_CONTEXT("KrinoWrapperTestFixtureOneTriMesh AdjointJacobian"));
}

TEST_F(KrinoWrapperTestFixtureFourTriMeshFixedBlockTwo, AdjointJacobian)
{
    runAdjointJacobianTest(TEST_CONTEXT("KrinoWrapperTestFixtureFourTriMeshFixedBlockTwo AdjointJacobian"));
}

}  // namespace plato::geometry::extension::unittest
