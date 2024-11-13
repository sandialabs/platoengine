
#include <gtest/gtest.h>  // for AssertHelper, TEST, etc

#include <filesystem>

#include "plato/third_party_integration/krino/KrinoWrapper.hpp"
#include "plato/third_party_integration/krino/unittest/KrinoTestFixture.hpp"

namespace plato::third_party_integration::krino::unittest
{

namespace
{
constexpr int kNumDimensions = 3;

auto predict_new_coordinates_based_on_perturbed_levelset_values(
    const KrinoWrapper &aKrinoWrapper,
    const std::unordered_map<unsigned int, stk::math::Vector3d> &aCoordVals,
    const double aPerturbation) -> std::unordered_map<unsigned int, stk::math::Vector3d>
{
    auto tPredictedCoordinateValues = std::unordered_map<unsigned int, stk::math::Vector3d>{};
    const auto &tSensitivities = aKrinoWrapper.getSensitivities();
    tPredictedCoordinateValues.reserve(tSensitivities.size());
    for (const auto &[tInterfaceNodeID, tInterfaceNodeDXDP] : tSensitivities)
    {
        auto tCoord = stk::math::Vector3d{0.0, 0.0, 0.0};
        for (size_t i = 0; i < tInterfaceNodeDXDP.mParentNodeIds.size(); ++i)
        {
            tCoord += aPerturbation * tInterfaceNodeDXDP.mParentDXDP[i];
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

TEST_F(KrinoTestFixture, CreateBoundingBoxBackgroundMesh)
{
    const auto tFilename = std::filesystem::path{"tmp.exo"};
    const auto tKrinoWrapper =
        KrinoWrapper{stk::math::Vector3d{0.0, 0.0, 0.0}, stk::math::Vector3d{1.0, 1.0, 1.0}, 0.5, tFilename};

    const auto tNodalCoordinates = tKrinoWrapper.getCoordinateValues();
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
    constexpr bool tIncludeVoidRegion = false;

    KrinoWrapper tKrinoWrapper{{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}, 0.333, tBackgroundFilename, tIncludeVoidRegion};
    const std::vector<std::pair<stk::math::Vector3d, double>> spheres{{{0.5, 0.5, 0.5}, 0.3}};
    tKrinoWrapper.initializeSphereLevelset(spheres);
    tKrinoWrapper.cutMesh();
    tKrinoWrapper.writeMesh(tCutFilename);
    const unsigned int tNumSolidTets = tKrinoWrapper.getNumTetsInNamedBlock(tBlockName);
    ASSERT_EQ(tNumSolidTets, 672u);

    std::filesystem::remove(tBackgroundFilename);
    std::filesystem::remove(tCutFilename);
}

TEST_F(KrinoTestFixture, GetSetLevelsetValues)
{
    const auto tFilename = std::filesystem::path{"tmp.exo"};
    KrinoWrapper tKrinoWrapper{stk::math::Vector3d{0.0, 0.0, 0.0}, stk::math::Vector3d{1.0, 1.0, 1.0}, 1.0, tFilename};
    tKrinoWrapper.initializePlaneLevelset(-1, .5, .35, .2);
    tKrinoWrapper.cutMesh();
    std::vector<double> tCurLevelsetValues1 = tKrinoWrapper.getLevelsetValues();
    tKrinoWrapper.setLevelsetValues(tCurLevelsetValues1);
    const std::vector<double> tCurLevelsetValues2 = tKrinoWrapper.getLevelsetValues();
    for (size_t i = 0; i < tCurLevelsetValues1.size(); ++i)
    {
        EXPECT_DOUBLE_EQ(tCurLevelsetValues1[i], tCurLevelsetValues2[i]);
        tCurLevelsetValues1[i] += 1.0;
    }
    tKrinoWrapper.setLevelsetValues(tCurLevelsetValues1);
    const std::vector<double> tCurLevelsetValues3 = tKrinoWrapper.getLevelsetValues();
    for (size_t i = 0; i < tCurLevelsetValues1.size(); ++i)
    {
        EXPECT_DOUBLE_EQ(tCurLevelsetValues3[i], tCurLevelsetValues2[i] + 1.0);
        EXPECT_DOUBLE_EQ(tCurLevelsetValues3[i], tCurLevelsetValues1[i]);
    }

    std::filesystem::remove(tFilename);
}

TEST_F(KrinoTestFixture, Redistance)
{
    const auto tFilename = std::filesystem::path{"tmp.exo"};
    KrinoWrapper tKrinoWrapper{stk::math::Vector3d{0.0, 0.0, 0.0}, stk::math::Vector3d{2.0, 1.0, 1.0}, 1.0, tFilename};
    tKrinoWrapper.initializePlaneLevelset(1, 0, 0, -.25);
    tKrinoWrapper.cutMesh();
    std::vector<double> tLevelsetValues = tKrinoWrapper.getLevelsetValues();
    const std::vector<double> tInitialGold = {-0.25, 0.75,  1.75, -0.25, 0.75, 1.75, -0.25, 0.75,
                                              1.75,  -0.25, 0.75, 1.75,  0.25, 1.25, 0.25,  1.25,
                                              -0.25, 0.25,  1.25, 1.75,  0.25, 1.25, 0.25,  1.25};
    EXPECT_EQ(tLevelsetValues, tInitialGold);

    // Perturb the nodes adjacent to the interface so that the interface moves slightly to the right.
    constexpr double tDelta = .05;
    tLevelsetValues[0] -= tDelta;
    tLevelsetValues[1] -= tDelta;
    tLevelsetValues[3] -= tDelta;
    tLevelsetValues[4] -= tDelta;
    tLevelsetValues[6] -= tDelta;
    tLevelsetValues[7] -= tDelta;
    tLevelsetValues[9] -= tDelta;
    tLevelsetValues[10] -= tDelta;
    tLevelsetValues[12] -= tDelta;
    tLevelsetValues[14] -= tDelta;
    tLevelsetValues[16] -= tDelta;
    tLevelsetValues[17] -= tDelta;
    tLevelsetValues[20] -= tDelta;
    tLevelsetValues[22] -= tDelta;

    tKrinoWrapper.resetMesh();
    tKrinoWrapper.setLevelsetValues(tLevelsetValues);
    tKrinoWrapper.cutMesh();

    const std::vector<double> tLevelsetValues2 = tKrinoWrapper.getLevelsetValues();
    EXPECT_EQ(tLevelsetValues, tLevelsetValues2);
    tKrinoWrapper.redistance();
    const std::vector<double> tLevelsetValues3 = tKrinoWrapper.getLevelsetValues();
    std::vector<double> tRedistancedGold = tInitialGold;
    for (auto &tCurVal : tRedistancedGold)
    {
        tCurVal -= tDelta;
    }
    for (size_t i = 0; i < tRedistancedGold.size(); ++i)
    {
        EXPECT_DOUBLE_EQ(tRedistancedGold[i], tLevelsetValues3[i]);
    }

    std::filesystem::remove(tFilename);
}

TEST_F(KrinoTestFixture, Sensitivities)
{
    const auto tFilename = std::filesystem::path{"tmp.exo"};

    KrinoWrapper tKrinoWrapper{stk::math::Vector3d{0.0, 0.0, 0.0}, stk::math::Vector3d{1.0, 1.0, 1.0}, 1.0, tFilename};
    tKrinoWrapper.initializePlaneLevelset(-1, .5, .35, .2);
    std::vector<double> tOriginalLevelsetValues = tKrinoWrapper.getLevelsetValues();
    tKrinoWrapper.cutMesh();
    const std::unordered_map<unsigned int, stk::math::Vector3d> tCurCoordinateValues =
        tKrinoWrapper.getCoordinateValues();
    constexpr auto tPerturbation = double{0.01};
    const auto tPredictedCoordValues =
        predict_new_coordinates_based_on_perturbed_levelset_values(tKrinoWrapper, tCurCoordinateValues, tPerturbation);
    tKrinoWrapper.resetMesh();
    // Add .01 to all level set values
    for (auto &tCurLS : tOriginalLevelsetValues)
    {
        tCurLS += tPerturbation;
    }
    tKrinoWrapper.setLevelsetValues(tOriginalLevelsetValues);
    tKrinoWrapper.cutMesh();
    const std::unordered_map<unsigned int, stk::math::Vector3d> tNewCoordValues = tKrinoWrapper.getCoordinateValues();
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

}  // namespace plato::third_party_integration::krino::unittest
