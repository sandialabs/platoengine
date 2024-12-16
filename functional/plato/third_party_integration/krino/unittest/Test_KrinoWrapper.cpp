#include <gtest/gtest.h>

#include <filesystem>
#include <numeric>
#include <string>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshOperators.hpp"
#include "plato/third_party_integration/krino/KrinoWrapper.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/krino/unittest/KrinoTestFixture.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/utilities/Enumerate.hpp"

namespace plato::third_party_integration::krino::unittest
{
namespace
{
constexpr int kNumDimensions = 3;

const auto kUnitBoundingBox = BoundingBox{{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}};

class TwoBlockMeshKrinoFixture : public stk_io::test_utilities::ThreeDTwoBlockTetMesh, public KrinoTestFixture
{
   public:
    void SetUp() override
    {
        KrinoTestFixture::SetUp();
        ThreeDTwoBlockTetMesh::SetUp();
    }

    void TearDown() override
    {
        KrinoTestFixture::TearDown();
        ThreeDTwoBlockTetMesh::TearDown();
    }

    auto analysisDomainMesh() const -> analysis::AnalysisDomainMesh
    {
        return analysis::AnalysisDomainMesh{mMeshFilePath, {{1U, mBlock1}, {2U, mBlock2}}};
    }

    const std::vector<analysis::ScalarFieldValue> mBlock1{{1, 0, 1.0}, {2, 1, 2.0}, {3, 2, 3.0}, {4, 3, 4.0},
                                                          {5, 4, 5.0}, {6, 5, 6.0}, {7, 6, 7.0}, {8, 7, 8.0}};
    const std::vector<analysis::ScalarFieldValue> mBlock2{{5, 4, 5.0}, {6, 5, 6.0},   {7, 6, 7.0},    {8, 7, 8.0},
                                                          {9, 8, 9.0}, {10, 9, 10.0}, {11, 10, 11.0}, {12, 11, 12.0}};
};

[[nodiscard]] auto number_of_tets_in_block(const KrinoWrapper &aKrinoWrapper, const std::string_view aBlockName)
    -> unsigned int
{
    const stk::mesh::Part *tPart = aKrinoWrapper.bulkData().mesh_meta_data().get_part(std::string{aBlockName});
    const stk::mesh::Selector tSelector(*tPart);
    const stk::mesh::BucketVector &tBuckets = aKrinoWrapper.bulkData().get_buckets(stk::topology::ELEM_RANK, tSelector);
    return std::accumulate(tBuckets.begin(), tBuckets.end(), 0U,
                           [](const unsigned int tTotal, const auto &tBucket) { return tTotal + tBucket->size(); });
}

[[nodiscard]] auto new_coordinates_based_on_perturbed_level_set_values(
    const KrinoWrapper &aKrinoWrapper,
    const std::unordered_map<unsigned int, stk::math::Vector3d> &aCoordVals,
    const double aPerturbation) -> std::unordered_map<unsigned int, stk::math::Vector3d>
{
    auto tPredictedCoordinateValues = std::unordered_map<unsigned int, stk::math::Vector3d>{};
    const auto &tSensitivities = aKrinoWrapper.sensitivities();
    tPredictedCoordinateValues.reserve(tSensitivities.size());
    for (const auto &[tInterfaceNodeID, tLevelSetJacobianColumn] : tSensitivities)
    {
        auto tCoord = stk::math::Vector3d{0.0, 0.0, 0.0};
        for (size_t i = 0; i < tLevelSetJacobianColumn.mBackgroundMeshNodeIDs.size(); ++i)
        {
            tCoord += aPerturbation * tLevelSetJacobianColumn.mNodalSensitivities[i];
        }
        tPredictedCoordinateValues[tInterfaceNodeID] = aCoordVals.at(tInterfaceNodeID) + tCoord;
    }
    // now add coords for nodes that weren't on the interface (ones
    // we don't have sensitivities for)
    for (auto &tCoordValue : aCoordVals)
    {
        if (tPredictedCoordinateValues.count(tCoordValue.first) == 0)
        {
            tPredictedCoordinateValues[tCoordValue.first] = tCoordValue.second;
        }
    }
    return tPredictedCoordinateValues;
}

}  // namespace

TEST_F(KrinoTestFixture, CoordinateValues)
{
    const auto tFilename = std::filesystem::path{"tmp.exo"};
    create_bounding_box_mesh(kUnitBoundingBox, 0.5, tFilename);
    constexpr auto tExcludeVoidRegion = VoidPhase::kExcludeFromMesh;
    const auto tKrinoWrapper = KrinoWrapper{tFilename, LevelSetPrimitives{}, tExcludeVoidRegion};

    const auto tNodalCoordinates = tKrinoWrapper.coordinates();
    {
        const auto tCoordinate = tNodalCoordinates.at(1);
        EXPECT_DOUBLE_EQ(tCoordinate[0], 0.0);
        EXPECT_DOUBLE_EQ(tCoordinate[1], 0.0);
        EXPECT_DOUBLE_EQ(tCoordinate[2], 0.0);
    }
    {
        const auto tCoordinate = tNodalCoordinates.at(68);
        EXPECT_DOUBLE_EQ(tCoordinate[0], 0.0);
        EXPECT_DOUBLE_EQ(tCoordinate[1], 0.75);
        EXPECT_DOUBLE_EQ(tCoordinate[2], 0.75);
    }
    {
        const auto tCoordinate = tNodalCoordinates.at(58);
        EXPECT_DOUBLE_EQ(tCoordinate[0], 0.75);
        EXPECT_DOUBLE_EQ(tCoordinate[1], 1.0);
        EXPECT_DOUBLE_EQ(tCoordinate[2], 0.25);
    }
    std::filesystem::remove(tFilename);
}

TEST_F(KrinoTestFixture, CutSphereOutOfBackgroundMesh)
{
    const auto tBackgroundFilename = std::filesystem::path{"background_mesh.exo"};
    const auto tCutFilename = std::filesystem::path{"swiss_cheese.exo"};
    const std::string tBlockName{"block_1"};
    create_bounding_box_mesh(kUnitBoundingBox, 0.333, tBackgroundFilename);

    const auto tSphere =
        LevelSetPrimitives{/*.mPlanes=*/{}, /*.mSpheres=*/{Sphere{/*.mCenter=*/{0.5, 0.5, 0.5}, /*.mRadius=*/0.3}}};
    auto tKrinoWrapper = KrinoWrapper{tBackgroundFilename, tSphere};
    tKrinoWrapper.writeMesh(tCutFilename);
    const unsigned int tNumSolidTets = number_of_tets_in_block(tKrinoWrapper, tBlockName);
    ASSERT_EQ(tNumSolidTets, 672u);

    std::filesystem::remove(tBackgroundFilename);
    std::filesystem::remove(tCutFilename);
}

TEST_F(KrinoTestFixture, GetSetLevelSetValues)
{
    const auto tFilename = std::filesystem::path{"tmp.exo"};
    create_bounding_box_mesh(kUnitBoundingBox, 1.0, tFilename);

    const auto tNumberOfNodes = stk_io::node_size(*stk_io::read_mesh_bulk_data(tFilename));
    auto tLevelSetField = std::vector(tNumberOfNodes, 0.0);
    std::iota(tLevelSetField.begin(), tLevelSetField.end(), 0.0);
    auto tKrinoWrapper = KrinoWrapper{tFilename, tLevelSetField};

    const auto tLevelSetValuesFromKrino = tKrinoWrapper.levelSetValues();

    EXPECT_EQ(tLevelSetValuesFromKrino, tLevelSetField);

    std::filesystem::remove(tFilename);
}

TEST_F(KrinoTestFixture, Redistance)
{
    const auto tFilename = std::filesystem::path{"tmp.exo"};
    create_bounding_box_mesh(BoundingBox{{0.0, 0.0, 0.0}, {2.0, 1.0, 1.0}}, 1.0, tFilename);

    const auto tPlane =
        LevelSetPrimitives{/*.mPlanes=*/{Plane{/*.mNormal=*/{1, 0, 0}, /*.mOffset=*/-0.25}}, /*.mSpheres=*/{}};
    auto tKrinoWrapper = KrinoWrapper{tFilename, tPlane};

    std::vector<double> tLevelSetValues = tKrinoWrapper.levelSetValues();
    const std::vector<double> tInitialGold = {-0.25, 0.75,  1.75, -0.25, 0.75, 1.75, -0.25, 0.75,
                                              1.75,  -0.25, 0.75, 1.75,  0.25, 1.25, 0.25,  1.25,
                                              -0.25, 0.25,  1.25, 1.75,  0.25, 1.25, 0.25,  1.25};
    EXPECT_EQ(tLevelSetValues, tInitialGold);

    // Perturb the nodes adjacent to the interface so that the interface moves slightly to the right.
    const auto tInterfaceNodes = std::vector{0, 1, 3, 4, 6, 7, 9, 10, 12, 14, 16, 17, 20, 22};
    constexpr double tDelta = .05;
    for (const auto tIndex : tInterfaceNodes)
    {
        tLevelSetValues.at(tIndex) -= tDelta;
    }

    tKrinoWrapper.setLevelSetValues(tLevelSetValues);

    const std::vector<double> tLevelSetValues2 = tKrinoWrapper.levelSetValues();
    EXPECT_EQ(tLevelSetValues, tLevelSetValues2);
    tKrinoWrapper.redistance();
    const std::vector<double> tLevelSetValues3 = tKrinoWrapper.levelSetValues();
    std::vector<double> tRedistancedGold = tInitialGold;
    for (auto &tCurVal : tRedistancedGold)
    {
        tCurVal -= tDelta;
    }
    for (size_t i = 0; i < tRedistancedGold.size(); ++i)
    {
        EXPECT_DOUBLE_EQ(tRedistancedGold[i], tLevelSetValues3[i]);
    }

    std::filesystem::remove(tFilename);
}

TEST_F(KrinoTestFixture, Sensitivities)
{
    const auto tFilename = std::filesystem::path{"tmp.exo"};
    create_bounding_box_mesh(kUnitBoundingBox, 1.0, tFilename);

    const auto tPlane =
        LevelSetPrimitives{/*.mPlanes=*/{Plane{/*.mNormal=*/{-1.0, 0.5, 0.35}, /*.mOffset=*/0.2}}, /*.mSpheres=*/{}};
    KrinoWrapper tKrinoWrapper{tFilename, tPlane};

    const auto tCurCoordinateValues = tKrinoWrapper.coordinates();
    constexpr auto tPerturbation = double{0.01};
    const auto tPredictedCoordValues =
        new_coordinates_based_on_perturbed_level_set_values(tKrinoWrapper, tCurCoordinateValues, tPerturbation);

    auto tPerturbedLevelSetValues = tKrinoWrapper.levelSetValues();
    for (auto &tCurLS : tPerturbedLevelSetValues)
    {
        tCurLS += tPerturbation;
    }
    const auto tNewCoordValues = KrinoWrapper{tFilename, tPerturbedLevelSetValues}.coordinates();
    for (auto tPredictedCoordValue : tPredictedCoordValues)
    {
        for (int i = 0; i < kNumDimensions; i++)
        {
            constexpr auto tTolerance = 1e-14;
            EXPECT_NEAR(tPredictedCoordValue.second[i], tNewCoordValues.at(tPredictedCoordValue.first)[i], tTolerance);
        }
    }

    std::filesystem::remove(tFilename);
}

TEST_F(TwoBlockMeshKrinoFixture, TwoBlockCtor)
{
    constexpr auto tFixedLevelSetValue = 1.0;
    const auto tKrinoWrapper = KrinoWrapper{analysisDomainMesh(), tFixedLevelSetValue};

    // Level-set values
    {
        const auto tLevelSetValues = tKrinoWrapper.levelSetValues();
        for (const auto &[tIndex, tValue] : utilities::enumerate(tLevelSetValues))
        {
            EXPECT_EQ(tIndex + 1, tValue);
        }
    }

    // Coordinates
    {
        const auto tCoordinates = tKrinoWrapper.coordinates();
        const auto tMeshBulkData = stk_io::read_mesh_bulk_data(mMeshFilePath);
        const auto tExpectedCoordinates = stk_io::nodal_coordinates(*tMeshBulkData);
        for (const auto &[tIndex, tCoordinate] : tKrinoWrapper.coordinates())
        {
            const auto &tExpectedCoordinate = tExpectedCoordinates.at(tIndex - 1);
            EXPECT_EQ(tExpectedCoordinate.x, tCoordinate[0]) << "Index: " << tIndex;
            EXPECT_EQ(tExpectedCoordinate.y, tCoordinate[1]) << "Index: " << tIndex;
            EXPECT_EQ(tExpectedCoordinate.z, tCoordinate[2]) << "Index: " << tIndex;
        }
    }
}

TEST_F(TwoBlockMeshKrinoFixture, OneFixedBlock)
{
    const auto tTestMesh = analysis::AnalysisDomainMesh{mMeshFilePath, {{1U, mBlock1}}};
    constexpr auto tFixedLevelSetValue = -1.0;
    const auto tKrinoWrapper = KrinoWrapper{tTestMesh, tFixedLevelSetValue};

    const auto tExpectedLevelSetValues = std::vector{1.0,
                                                     2.0,
                                                     3.0,
                                                     4.0,
                                                     5.0,
                                                     6.0,
                                                     7.0,
                                                     8.0,
                                                     tFixedLevelSetValue,
                                                     tFixedLevelSetValue,
                                                     tFixedLevelSetValue,
                                                     tFixedLevelSetValue};
    const auto tLevelSetValues = tKrinoWrapper.levelSetValues();
    EXPECT_EQ(tExpectedLevelSetValues, tLevelSetValues);
}

}  // namespace plato::third_party_integration::krino::unittest
