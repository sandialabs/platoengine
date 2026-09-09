#include <gtest/gtest.h>
#include <mpi.h>

#include <array>
#include <boost/mpi/communicator.hpp>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <numbers>
#include <optional>
#include <ranges>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "plato/geometry/extension/KrinoWrapper.hpp"
#include "plato/geometry/extension/test_utilities/KrinoWrapperTestUtilities.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"
#include "plato/third_party_integration/krino/SensitivityMapUtilities.hpp"
#include "plato/third_party_integration/krino/SnappingParameters.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/utilities/DataFilePath.hpp"
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

constexpr double kCDFEMEdgeTolerance = 1.0e-3;

using tpik::test_utilities::KrinoTestFixture;

void initialize_krino()
{
    tpik::set_stk_communicator_for_krino(boost::mpi::communicator{MPI_COMM_WORLD, boost::mpi::comm_attach});
}

[[nodiscard]] auto create_krino_wrapper_test_command_generator() -> third_party_integration::stk_io::CommandGenerator
{
    const third_party_integration::stk_io::CommandBounds tLowerBounds{0, 0, 0};
    const third_party_integration::stk_io::CommandBounds tUpperBounds{1, 1, 1};
    const third_party_integration::stk_io::CommandNumberOfElements tElements{1, 1, 1};
    return third_party_integration::stk_io::CommandGenerator{tElements, tLowerBounds, tUpperBounds,
                                                             third_party_integration::stk_io::CommandElementType::kTet};
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

class FourTriTwoBlockKrinoFixture : public tpik::test_utilities::KrinoTestFixture
{
   public:
    FourTriTwoBlockKrinoFixture() : mFilePath(utilities::data_file_path("four_tri_two_block.cdf").value()) {}

    auto makeKrinoWrapperFromDownwardPlane(const double aPlaneOffset,
                                           const tpik::VoidPhase& aVoidPhase,
                                           const std::set<std::string>& aFixedBlocks,
                                           const tpik::SnappingParameters& aSnappingParameters) -> KrinoWrapper
    {
        const auto tDownwardPlaneNormal = third_party_integration::common::Vector3{.x = 0, .y = -1, .z = 0};
        const auto tPlane = tpik::LevelSetPrimitives{
            .mPlanes = {tpik::Plane{.mNormal = tDownwardPlaneNormal, .mOffset = aPlaneOffset}}, .mSpheres = {}};
        return test_utilities::make_krino_wrapper_from_level_set_primitives(mFilePath, tPlane, aVoidPhase, aFixedBlocks,
                                                                            aSnappingParameters);
    }

   private:
    std::filesystem::path mFilePath;
};

class TetBlock1x2x1KrinoFixture : public tpik::test_utilities::KrinoTestFixture
{
   public:
    void SetUp() override
    {
        KrinoTestFixture::SetUp();
        const auto tCommandGenerator = third_party_integration::stk_io::CommandGenerator{
            .mElements = {.mX = 1, .mY = 2, .mZ = 1},
            .mLowerBounds = {.x = -0.5, .y = -1, .z = -0.5},
            .mUpperBounds = {.x = 0.5, .y = 1, .z = 0.5},
            .mType = third_party_integration::stk_io::CommandElementType::kTet};
        third_party_integration::stk_io::write_mesh(std::string{kTemporaryMeshFile}, tCommandGenerator);
    }

    void TearDown() override
    {
        KrinoTestFixture::TearDown();
        std::filesystem::remove(std::string{kTemporaryMeshFile});
    }
};

void test_krino_sensitivities_against_expected(
    KrinoWrapper aWrapper,
    const std::unordered_map<std::size_t, std::vector<std::size_t>>& aExpectedParentNodes,
    const std::unordered_map<std::size_t, std::vector<third_party_integration::common::Vector3>>&
        aExpectedSensitivityValues,
    const plato::test_utilities::TestContext& aTestContext,
    const double aTolerance = 1.0e-15)
{
    const auto tCutFilename = std::filesystem::path{"cut_mesh.exo"};
    aWrapper.writeCutMesh(tCutFilename);
    const auto& tSensitivities = aWrapper.sensitivities();

    const auto tParentNodesToMap =
        tSensitivities |
        std::views::transform(
            [](const auto& aInterfaceNode)
            { return std::make_pair(aInterfaceNode.first, aInterfaceNode.second.mBackgroundMeshNodeIDs); });
    const auto tParentNodes =
        std::unordered_map<std::size_t, std::vector<std::size_t>>{tParentNodesToMap.begin(), tParentNodesToMap.end()};
    EXPECT_EQ(tParentNodes, aExpectedParentNodes) << aTestContext;

    const auto tSensitivityValuesToMap =
        tSensitivities | std::views::transform(
                             [](const auto& aInterfaceNode) {
                                 return std::make_pair(aInterfaceNode.first, aInterfaceNode.second.mNodalSensitivities);
                             });
    const auto tComputedSensitivityValues =
        std::unordered_map<std::size_t, std::vector<third_party_integration::common::Vector3>>{
            tSensitivityValuesToMap.begin(), tSensitivityValuesToMap.end()};
    for (const auto& [tCutMeshNode, tExpectedSensitivityVector] : aExpectedSensitivityValues)
    {
        const auto tComputedSensitivityValuesIterator = tComputedSensitivityValues.find(tCutMeshNode);
        ASSERT_NE(tComputedSensitivityValuesIterator, tComputedSensitivityValues.end());
        for (const auto& [tValue, tGold] :
             utilities::Zip{tComputedSensitivityValuesIterator->second, tExpectedSensitivityVector})
        {
            third_party_integration::common::test_utilities::test_near_equality_of_components(
                tValue, tGold, aTolerance, EXTEND_CONTEXT("sensitivity values", aTestContext));
        }
    }

    std::filesystem::remove(tCutFilename);
}

void test_expected_sensitivity_information_near_interface_above_cdfem_edge_tolerance(
    const double aOffset, KrinoWrapper aWrapper, const plato::test_utilities::TestContext& aTestContext)
{
    ASSERT_GE(aOffset, kCDFEMEdgeTolerance);
    const double tOneMinusOffset = 1.0 - aOffset;
    const auto tExpectedParentNodes =
        std::unordered_map<std::size_t, std::vector<std::size_t>>{{8, {4, 1}}, {9, {5, 1}}, {2, {2, 4}}};
    const auto tExpectedSensitivityValues =
        std::unordered_map<std::size_t, std::vector<third_party_integration::common::Vector3>>{
            {8, {{aOffset, aOffset, 0.0}, {tOneMinusOffset, tOneMinusOffset, 0.0}}},
            {9, {{0.0, aOffset, 0.0}, {0.0, tOneMinusOffset, 0.0}}},
            {2, {{0.0, tOneMinusOffset, 0.0}, {0.0, aOffset, 0.0}}}};
    test_krino_sensitivities_against_expected(std::move(aWrapper), tExpectedParentNodes, tExpectedSensitivityValues,
                                              aTestContext);
}

void test_expected_sensitivity_information_near_interface_below_cdfem_edge_tolerance(
    const double aOffset,
    KrinoWrapper aWrapper,
    const std::unordered_map<std::size_t, std::vector<std::size_t>>& aExpectedParentNodes,
    const plato::test_utilities::TestContext& aTestContext)
{
    ASSERT_LT(aOffset, kCDFEMEdgeTolerance);
    const double tOneMinusOffset = 1.0 - aOffset;
    const auto tExpectedSensitivityValues =
        std::unordered_map<std::size_t, std::vector<third_party_integration::common::Vector3>>{
            {1, {{0.0, 1.0, 0.0}, {0.0, 0.0, 0.0}}}, {2, {{0.0, tOneMinusOffset, 0.0}, {0.0, aOffset, 0.0}}}};
    test_krino_sensitivities_against_expected(std::move(aWrapper), aExpectedParentNodes, tExpectedSensitivityValues,
                                              aTestContext);
}

void test_expected_sensitivity_information_near_interface_below_cdfem_edge_tolerance_no_snapping(
    const double aOffset,
    KrinoWrapper aWrapper,
    const std::unordered_map<std::size_t, std::vector<std::size_t>>& aExpectedParentNodes,
    const plato::test_utilities::TestContext& aTestContext)
{
    ASSERT_LT(aOffset, kCDFEMEdgeTolerance);
    const auto tExpectedSensitivityValues =
        std::unordered_map<std::size_t, std::vector<third_party_integration::common::Vector3>>{
            {1, {{0.0, 1.0, 0.0}, {0.0, 0.0, 0.0}}}, {2, {{0.0, 1.0, 0.0}, {0.0, 0.0, 0.0}}}};
    test_krino_sensitivities_against_expected(std::move(aWrapper), aExpectedParentNodes, tExpectedSensitivityValues,
                                              aTestContext);
}

void test_expected_sensitivity_information_near_interface_3d(const double aOffset,
                                                             KrinoWrapper aWrapper,
                                                             const plato::test_utilities::TestContext& aTestContext,
                                                             const double aTolerance = 1.0e-15)
{
    const double tOneMinusOffset = 1.0 - aOffset;

    const auto tExpectedParentNodes =
        std::unordered_map<std::size_t, std::vector<std::size_t>>{{3, {3, 1}}, {4, {4, 2}}, {9, {9, 7}}, {10, {10, 8}}};
    const auto tExpectedSensitivityValues =
        std::unordered_map<std::size_t, std::vector<third_party_integration::common::Vector3>>{
            {3, {{0.0, -tOneMinusOffset, 0.0}, {0.0, -aOffset, 0.0}}},
            {4, {{0.0, -tOneMinusOffset, 0.0}, {0.0, -aOffset, 0.0}}},
            {9, {{0.0, -tOneMinusOffset, 0.0}, {0.0, -aOffset, 0.0}}},
            {10, {{0.0, -tOneMinusOffset, 0.0}, {0.0, -aOffset, 0.0}}}};

    test_krino_sensitivities_against_expected(std::move(aWrapper), tExpectedParentNodes, tExpectedSensitivityValues,
                                              aTestContext, aTolerance);
}

void test_expected_sensitivity_information_near_interface_no_snapping_above_cdfem_edge_tolerance(
    const double aOffset,
    KrinoWrapper aWrapper,
    const plato::test_utilities::TestContext& aTestContext,
    const double aTolerance = 1.0e-15)
{
    const double tOneMinusOffset = 1.0 - aOffset;

    const auto tExpectedParentNodes = std::unordered_map<std::size_t, std::vector<std::size_t>>{
        {15, {1, 10}}, {13, {1, 4}},  {14, {1, 3}},  {16, {1, 9}}, {17, {9, 7}},
        {18, {7, 10}}, {19, {8, 10}}, {20, {2, 10}}, {21, {2, 4}}};
    const auto tExpectedSensitivityValues =
        std::unordered_map<std::size_t, std::vector<third_party_integration::common::Vector3>>{
            {15, {{-aOffset, -aOffset, -aOffset}, {-tOneMinusOffset, -tOneMinusOffset, -tOneMinusOffset}}},
            {13, {{-aOffset, -aOffset, 0.0}, {-tOneMinusOffset, -tOneMinusOffset, 0.0}}},
            {14, {{0.0, -aOffset, 0.0}, {0.0, -tOneMinusOffset, 0.0}}},
            {16, {{0.0, -aOffset, -aOffset}, {0.0, -tOneMinusOffset, -tOneMinusOffset}}},
            {17, {{0.0, -tOneMinusOffset, 0.0}, {0.0, -aOffset, 0.0}}},
            {18, {{-aOffset, -aOffset, 0.0}, {-tOneMinusOffset, -tOneMinusOffset, 0.0}}},
            {19, {{0.0, -aOffset, 0.0}, {0.0, -tOneMinusOffset, 0.0}}},
            {20, {{0.0, -aOffset, -aOffset}, {0.0, -tOneMinusOffset, -tOneMinusOffset}}},
            {21, {{0.0, -aOffset, 0.0}, {0.0, -tOneMinusOffset, 0.0}}}};

    test_krino_sensitivities_against_expected(std::move(aWrapper), tExpectedParentNodes, tExpectedSensitivityValues,
                                              aTestContext, aTolerance);
}

void test_expected_sensitivity_information_near_interface_no_snapping_below_cdfem_edge_tolerance(
    KrinoWrapper aWrapper, const plato::test_utilities::TestContext& aTestContext, const double aTolerance = 1.0e-15)
{
    const auto tExpectedParentNodes =
        std::unordered_map<std::size_t, std::vector<std::size_t>>{{3, {3, 1}}, {4, {4, 1}}, {9, {9, 1}}, {10, {10, 1}}};
    const auto tExpectedSensitivityValues =
        std::unordered_map<std::size_t, std::vector<third_party_integration::common::Vector3>>{
            {3, {{0.0, -1.0, 0.0}, {0.0, 0.0, 0.0}}},
            {4, {{-1.0, -1.0, 0.0}, {0.0, 0.0, 0.0}}},
            {9, {{0.0, -1.0, -1.0}, {0.0, 0.0, 0.0}}},
            {10, {{-1.0, -1.0, -1.0}, {0.0, 0.0, 0.0}}}};

    test_krino_sensitivities_against_expected(std::move(aWrapper), tExpectedParentNodes, tExpectedSensitivityValues,
                                              aTestContext, aTolerance);
}

struct KrinoWrapperGoldValues
{
    unsigned int mSpatialDimensions;
    unsigned int mNumberBackgroundNodes;
    unsigned int mNumberCutNodes;
    unsigned int mSensitivityMapSize;
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
        test_utilities::InitialLevelSetValues{std::vector{-1.0, -1.0, -1.0, -1.0, 1.0, 1.0, 1.0, 1.0}},
        tpik::VoidPhase::kExcludeFromMesh);
};

constexpr auto kNumberOfCutMeshNodes = std::size_t{17};
constexpr auto kNumberOfBackgroundMeshNodes = std::size_t{8};

const auto kUnitCubeInputs =
    KrinoWrapperTestFixtureInputs{.mFileName = std::string{kTemporaryMeshFile},
                                  .mCutMeshRowVector = std::vector<double>(kNumberOfCutMeshNodes * 3U, 1.0),
                                  .mBackgroundRowVector = std::vector<double>(kNumberOfBackgroundMeshNodes * 3U, 1.0),
                                  .mVoidPhase = tpik::VoidPhase::kExcludeFromMesh};

const auto kUnitCubeGoldValues = KrinoWrapperGoldValues{
    .mSpatialDimensions = 3U,
    .mNumberBackgroundNodes = 8U,
    .mNumberCutNodes = 13U,
    .mSensitivityMapSize = 9U,
    .mJacobianGold = {-2.0, -0.75, -0.75, -0.25, -0.25, -0.75, -0.75, -2.0},
    .mAdjointJacobianGold = std::vector<double>{
        0,    0, 0, 0,    0, 0,    0,    0, 0, 0,    0,    0, -0.5, -0.5, -0.5, 0,    0, -0.5, -0.5, 0,
        -0.5, 0, 0, -0.5, 0, -0.5, -0.5, 0, 0, -0.5, -0.5, 0, -0.5, 0,    0,    -0.5, 0, -0.5, -0.5}};

const auto kOneTriLambda = []() -> KrinoWrapper
{
    initialize_krino();
    return test_utilities::make_krino_wrapper_from_level_set_primitives(
        kOneTriMeshFilePath.value(), tpik::LevelSetPrimitives{{kThreeQuarterOffsetXHatPlane}, {}},
        tpik::VoidPhase::kIncludeInMesh);
};

const auto kOneTriInputs = KrinoWrapperTestFixtureInputs{.mFileName = std::string{kOneTriMeshFilePath.value()},
                                                         .mCutMeshRowVector = {1.0, 2, 3, 4, 5, 6, 7, 8, 9, 10},
                                                         .mBackgroundRowVector = {1.0, 2, 3},
                                                         .mVoidPhase = tpik::VoidPhase::kIncludeInMesh};

const auto kOneTriGoldValues =
    KrinoWrapperGoldValues{.mSpatialDimensions = 2U,
                           .mNumberBackgroundNodes = 3U,
                           .mNumberCutNodes = 5U,
                           .mSensitivityMapSize = 2U,
                           .mJacobianGold = {6.5, 5.25, 14.25},
                           .mAdjointJacobianGold = std::vector<double>{0, 0, 0, 0, 0, 0, 1.75, 0, 2.5, 2.5}};

const auto kFourTriLambda = []() -> KrinoWrapper
{
    initialize_krino();
    const auto tFixedBlockNames = std::set<std::string>{"block_2"};
    return test_utilities::make_krino_wrapper_from_level_set_primitives(
        kFourTriTwoBlockMeshFilePath.value(), tpik::LevelSetPrimitives{{kThreeQuarterOffsetXHatPlane}, {}},
        tpik::VoidPhase::kExcludeFromMesh, tFixedBlockNames);
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
                           .mJacobianGold = {9.5, 31.5, 20.25, 7.75},
                           .mAdjointJacobianGold = std::vector{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.75,
                                                               0.0, 2.5, 2.5, 2.5, 2.5}};

const auto kFourTriInterfaceNearNodeLambda = []() -> KrinoWrapper
{
    initialize_krino();
    const auto tFixedBlockNames = std::set<std::string>{"block_2"};
    return test_utilities::make_krino_wrapper_from_level_set_primitives(
        kFourTriTwoBlockMeshFilePath.value(), tpik::LevelSetPrimitives{{kPlaneNearBackgroundNode}, {}},
        tpik::VoidPhase::kExcludeFromMesh, tFixedBlockNames);
};

const auto kFourTriInterfaceNearNodeInputs =
    KrinoWrapperTestFixtureInputs{.mFileName = std::string{kFourTriTwoBlockMeshFilePath.value()},
                                  .mCutMeshRowVector = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0},
                                  .mBackgroundRowVector = {1.0, 2.0, 3.0, 4.0},
                                  .mVoidPhase = tpik::VoidPhase::kExcludeFromMesh};

const auto kFourTriInterfaceNearNodeGoldValues =
    KrinoWrapperGoldValues{.mSpatialDimensions = 2U,
                           .mNumberBackgroundNodes = 4U,
                           .mNumberCutNodes = 3U,
                           .mSensitivityMapSize = 2U,
                           .mJacobianGold = {2.1213203435596428, 3.8896372965260113, 0.0, 3.8885372965260117},
                           .mAdjointJacobianGold = std::vector<double>{0.7072067811865476, 0.7072067811865476, 0, 0,
                                                                       2.1212203435596426, 2.1212203435596426}};

}  // namespace

class KrinoWrapperTestFixtureBase : virtual public ::testing::Test, public KrinoTestFixture
{
   public:
    KrinoWrapperTestFixtureBase(const std::function<KrinoWrapper()>& aKrinoWrapperLambda,
                                KrinoWrapperTestFixtureInputs aKrinoWrapperTestFixtureInputs,
                                KrinoWrapperGoldValues aKrinoWrapperGoldValues)
        : mKrinoWrapper(aKrinoWrapperLambda()),
          mInputValues(std::move(aKrinoWrapperTestFixtureInputs)),
          mGoldValues(std::move(aKrinoWrapperGoldValues))
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
            mKrinoWrapper.value().rowVectorJacobianProduct(mInputValues.mCutMeshRowVector);
        ASSERT_EQ(tJacobianRowVectorProduct.size(), mGoldValues.mNumberBackgroundNodes) << aTestContext;
        EXPECT_EQ(tJacobianRowVectorProduct, mGoldValues.mJacobianGold) << aTestContext;
    }

    void runAdjointJacobianTest(const plato::test_utilities::TestContext& aTestContext)
    {
        const auto tRowVectorAdjointJacobianProduct =
            mKrinoWrapper.value().rowVectorAdjointJacobianProduct(mInputValues.mBackgroundRowVector);
        ASSERT_EQ(tRowVectorAdjointJacobianProduct.size(), mGoldValues.mNumberCutNodes * mGoldValues.mSpatialDimensions)
            << aTestContext;
        EXPECT_EQ(tRowVectorAdjointJacobianProduct, mGoldValues.mAdjointJacobianGold) << aTestContext;
    }

    void runJacobianRegressionTest(const plato::test_utilities::TestContext& aTestContext)
    {
        EXPECT_EQ(mKrinoWrapper.value().sensitivities().size(), mGoldValues.mSensitivityMapSize) << aTestContext;

        const auto tJacobianRowVectorProduct =
            mKrinoWrapper.value().rowVectorJacobianProduct(mInputValues.mCutMeshRowVector);
        ASSERT_EQ(tJacobianRowVectorProduct.size(), mGoldValues.mJacobianGold.size());
        for (const auto [tJacobianRowVectorProductValue, tGoldValue] :
             utilities::Zip{tJacobianRowVectorProduct, mGoldValues.mJacobianGold})
        {
            EXPECT_DOUBLE_EQ(tJacobianRowVectorProductValue, tGoldValue);
        }
    }

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
    const double tFaceCoordinate = std::numbers::sqrt2 / 2.0 * tRadius;

    const auto tSphere = tpik::LevelSetPrimitives{
        .mPlanes = {}, .mSpheres = {tpik::Sphere{.mCenter = {0., 0., 0.}, .mRadius = tRadius}}};
    test_utilities::make_krino_wrapper_from_level_set_primitives(mFileName, tSphere, tpik::VoidPhase::kExcludeFromMesh)
        .writeCutMesh(tCutFilename);

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
    const auto tPlane = tpik::LevelSetPrimitives{.mPlanes = {tpik::Plane{{0, 1, 0}, tOffset}}, .mSpheres = {}};
    test_utilities::make_krino_wrapper_from_level_set_primitives(mFileName, tPlane, tpik::VoidPhase::kIncludeInMesh)
        .writeCutMesh(tCutFilename);

    const auto tBulkData = third_party_integration::stk_io::read_mesh_bulk_data(tCutFilename);
    const auto tCoordinates = third_party_integration::stk_io::nodal_coordinates(*tBulkData);
    ASSERT_EQ(tCoordinates.size(), 17U);

    std::filesystem::remove(tCutFilename);
}

TEST_F(OneBlockMeshKrinoFixture, PlaneSensitivities)
{
    const auto tPlane = tpik::LevelSetPrimitives{.mPlanes = {kThreeQuarterOffsetXHatPlane}, .mSpheres = {}};
    const auto tSensitivities = test_utilities::make_krino_wrapper_from_level_set_primitives(
                                    mFileName, tPlane, tpik::VoidPhase::kExcludeFromMesh)
                                    .sensitivities();
    ASSERT_EQ(tSensitivities.size(), 9U);

    compareMapAgainstBuiltInGold(tSensitivities, TEST_CONTEXT("One block mesh sensitivity check"));
}

TEST_F(FourTriTwoBlockKrinoFixture, PlaneInterfaceSensitivityParentNodes)
{
    const auto tFixedBlocks = std::set<std::string>{};
    const auto tDefaultSnappingParameters = tpik::SnappingParameters{};
    const auto tVoidPhase = third_party_integration::krino::VoidPhase::kIncludeInMesh;
    {
        constexpr double tOffset = 1.0e-1;
        test_expected_sensitivity_information_near_interface_above_cdfem_edge_tolerance(
            tOffset, makeKrinoWrapperFromDownwardPlane(tOffset, tVoidPhase, tFixedBlocks, tDefaultSnappingParameters),
            TEST_CONTEXT("Offset 1E-1 - Here the level-set value is above the cdfem edge tolerance, so there is no "
                         "special treatment of sensitivities."));
    }

    {
        constexpr double tOffset = 1.0e-3;
        test_expected_sensitivity_information_near_interface_above_cdfem_edge_tolerance(
            tOffset, makeKrinoWrapperFromDownwardPlane(tOffset, tVoidPhase, tFixedBlocks, tDefaultSnappingParameters),
            TEST_CONTEXT("Offset 1E-3 - Here the level-set value is above the cdfem edge tolerance, so there is no "
                         "special treatment of sensitivities."));
    }

    const auto tExpectedParentNodesInterfaceBelowTolerance =
        std::unordered_map<std::size_t, std::vector<std::size_t>>{{1, {1, 5}}, {2, {2, 4}}};
    {
        constexpr double tOffset = 1.0e-4;
        test_expected_sensitivity_information_near_interface_below_cdfem_edge_tolerance(
            tOffset, makeKrinoWrapperFromDownwardPlane(tOffset, tVoidPhase, tFixedBlocks, tDefaultSnappingParameters),
            tExpectedParentNodesInterfaceBelowTolerance,
            TEST_CONTEXT("Offset 1e-4 - Here the level-set value is below the cdfem edge tolerance, meaning that "
                         "cutting is disabled and nodes that cannot snap are treated as if they are exactly on the "
                         "interface. Since background mesh node 1 is on a fixed block, it cannot be snapped "
                         "and is treated as if it lies on the interface. Background mesh node 2 can be snapped, so it "
                         "has the expected sensitivity values."));
    }

    {
        constexpr double tOffset = 1.0e-9;
        test_expected_sensitivity_information_near_interface_below_cdfem_edge_tolerance(
            tOffset, makeKrinoWrapperFromDownwardPlane(tOffset, tVoidPhase, tFixedBlocks, tDefaultSnappingParameters),
            tExpectedParentNodesInterfaceBelowTolerance,
            TEST_CONTEXT("Offset 1e-9 - This is the same case as with offset 1e-4, except it checks that there is no "
                         "invisible 1e-8 snapping tolerance like there used to be."));
    }

    {
        constexpr double tOffset = 1.0e-15;
        test_expected_sensitivity_information_near_interface_below_cdfem_edge_tolerance(
            tOffset, makeKrinoWrapperFromDownwardPlane(tOffset, tVoidPhase, tFixedBlocks, tDefaultSnappingParameters),
            tExpectedParentNodesInterfaceBelowTolerance,
            TEST_CONTEXT(
                "Offset 1e-15 - This is the same case as with offset 1e-4, except the offset is right about the 4 ULPs "
                "tolerance value for EXPECT_DOUBLE_EQUALITY which is being used to test for equality."));
    }

    {
        constexpr double tOffset = 1.0e-4;
        const auto tSnappingOffParameters = tpik::SnappingParameters{.mMaxSnappingEdgeLength = 0.0};
        test_expected_sensitivity_information_near_interface_below_cdfem_edge_tolerance_no_snapping(
            tOffset, makeKrinoWrapperFromDownwardPlane(tOffset, tVoidPhase, tFixedBlocks, tSnappingOffParameters),
            tExpectedParentNodesInterfaceBelowTolerance,
            TEST_CONTEXT("Offset 1e-4 with snapping disabled - Here the level-set value is below the cdfem edge "
                         "tolerance, meaning that cutting is disabled and both background nodes are treated as if they "
                         "are exactly on the interface."));
    }
}

TEST_F(TetBlock1x2x1KrinoFixture, PlaneInterfaceNearBackground3D)
{
    const auto tMakeWrapperWithPlane =
        [](const double aPlaneOffset, const tpik::SnappingParameters& aSnappingParameters)
    {
        const auto tXZPlaneNormal = third_party_integration::common::Vector3{.x = 0, .y = 1, .z = 0};
        const auto tPlane = tpik::LevelSetPrimitives{
            .mPlanes = {tpik::Plane{.mNormal = tXZPlaneNormal, .mOffset = aPlaneOffset}}, .mSpheres = {}};
        return test_utilities::make_krino_wrapper_from_level_set_primitives(
            kTemporaryMeshFile, tPlane, tpik::VoidPhase::kIncludeInMesh, {}, aSnappingParameters);
    };

    {
        const auto tOffsets = std::array{1.0e-1, 1.0e-4, 1.0e-15};
        const auto tContextStrings = std::array{
            std::string{"Offset 1E-1 - Sensitivity values should match to 4 ULPs (~9E-16)."},
            std::string{"Offset 1E-4 - Sensitivity values may not match to 4 ULPs (~5E-20) but should match to 1E-16."},
            std::string{
                "Offset 1E-15 - Sensitivity values may not match to 4 ULPs (~7E-31) but should match to 1E-16."}};

        constexpr double tTolerance = 1.0e-16;
        for (const auto& [tOffset, tContextString] : utilities::Zip{tOffsets, tContextStrings})
        {
            test_expected_sensitivity_information_near_interface_3d(
                tOffset, tMakeWrapperWithPlane(tOffset, tpik::SnappingParameters{}), TEST_CONTEXT(tContextString),
                tTolerance);
        }
    }

    {
        const auto tOffsets = std::array{1.0e-1, 1.001e-3};
        const auto tContextStrings = std::array{
            std::string{"Offset 1E-1 no snapping - Sensitivity values should exist for all edges and match to 4 ULPs "
                        "(~9E-16)."},
            std::string{"Offset 1.001E-3 no snapping - Offset is just above cdfem edge tolerance. Sensitivity values "
                        "should exist for all edges and should match to 1E-15."}};
        constexpr double tTolerance = 1.0e-15;
        const auto tSnappingParameters = tpik::SnappingParameters{.mMaxSnappingEdgeLength = 0.0};
        for (const auto& [tOffset, tContextString] : utilities::Zip{tOffsets, tContextStrings})
        {
            test_expected_sensitivity_information_near_interface_no_snapping_above_cdfem_edge_tolerance(
                tOffset, tMakeWrapperWithPlane(tOffset, tSnappingParameters), TEST_CONTEXT(tContextString), tTolerance);
        }
    }

    {
        const auto tOffsets = std::array{9.999e-4, 1.0e-15};
        const auto tContextStrings = std::array{
            std::string{"Offset 9.999E-4 no snapping - Offset is just below cdfem edge tolerance. Sensitivity values "
                        "should only exist for nodes on the interface with a value of 1 and have a value of 0 for the "
                        "other parent nodes. Curiously, the second parent node is always set to node 1 for each of "
                        "the nodes on the interface. The sensitivity values along dimensions not aligned with the "
                        "plane also have values of 1, which could be problematic."},
            std::string{"Offset 1.0E-15 no snapping - Offset is below cdfem edge tolerance. Sensitivity values "
                        "should only exist for nodes on the interface with a value of 1 and have a value of 0 for the "
                        "other parent nodes. Curiously, the second parent node is always set to node 1 for each of "
                        "the nodes on the interface. The sensitivity values along dimensions not aligned with the "
                        "plane also have values of 1, which could be problematic."}};
        constexpr double tTolerance = 1.0e-16;
        const auto tSnappingParameters = tpik::SnappingParameters{.mMaxSnappingEdgeLength = 0.0};
        for (const auto& [tOffset, tContextString] : utilities::Zip{tOffsets, tContextStrings})
        {
            test_expected_sensitivity_information_near_interface_no_snapping_below_cdfem_edge_tolerance(
                tMakeWrapperWithPlane(tOffset, tSnappingParameters), TEST_CONTEXT(tContextString), tTolerance);
        }
    }
}

TEST_F(KrinoWrapperTestFixtureUnitCube, Jacobian)
{
    runJacobianTest(TEST_CONTEXT("KrinoWrapperTestFixtureUnitCube Jacobian"));
}

TEST_F(KrinoWrapperTestFixtureOneTriMesh, Jacobian)
{
    runJacobianRegressionTest(TEST_CONTEXT("KrinoWrapperTestFixtureOneTriMesh Jacobian"));
}

TEST_F(KrinoWrapperTestFixtureFourTriMeshFixedBlockTwo, Jacobian)
{
    runJacobianTest(TEST_CONTEXT("KrinoWrapperTestFixtureFourTriMeshFixedBlockTwo Jacobian"));
}

TEST_F(KrinoWrapperTestFixtureFourTriInterfaceNearNode, JacobianRegression)
{
    runJacobianRegressionTest(TEST_CONTEXT(
        "Jacobian regression for case when interface is within cdfem edge tolerance so cutting will not occur. "
        "Test "
        "captures the current state of what we believe to be the correct sensitivity values in such a case."));
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

TEST(KrinoWrapper, DetailClampToBounds)
{
    const auto tVector = std::vector<double>{-2.0, -1.0, -0.5, 0.0, 0.5, 1.0, 2.0};

    {
        const auto tBounds = LevelSetBounds{std::pair{-2.0, 2.0}};
        EXPECT_EQ(detail::clamp_to_bounds(tVector, tBounds), tVector) << "within bounds - no change";
    }

    {
        const auto tBounds = LevelSetBounds{std::pair{-1.0, 1.0}};
        const auto tGoldClamped = std::vector<double>{-1.0, -1.0, -0.5, 0.0, 0.5, 1.0, 1.0};
        EXPECT_EQ(detail::clamp_to_bounds(tVector, tBounds), tGoldClamped) << "outside of bounds - clamped";
    }
}

}  // namespace plato::geometry::extension::unittest
