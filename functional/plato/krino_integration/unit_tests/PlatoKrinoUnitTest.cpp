/*
 * PlatoKrinoUnitTests.cpp
 *
 *  Created on: Nov 1, 2023
 */

#include <gtest/gtest.h>  // for AssertHelper, TEST, etc

#include <cstdio>

#include "plato/krino_integration/PlatoKrinoInterface.hpp"

namespace Plato::Krino
{

class PlatoTestKrino : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        static bool tFirstTime{true};
        if (tFirstTime)
        {
            Plato::Krino::initializeSTKEnvironment(MPI_COMM_WORLD);
            Plato::Krino::initializeKrinoLogging();
            tFirstTime = false;
        }
    }
};

TEST_F(PlatoTestKrino, create_bounding_box_background_mesh)
{
    PlatoKrinoInterface tInterface;
    tInterface.createBoundingBoxMesh(0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.5);
    double x, y, z;
    tInterface.getNodalCoordinates(1, x, y, z);
    EXPECT_FLOAT_EQ(x, 0.0);
    EXPECT_FLOAT_EQ(y, 0.0);
    EXPECT_FLOAT_EQ(z, 0.0);
    tInterface.getNodalCoordinates(68, x, y, z);
    EXPECT_FLOAT_EQ(x, 0.0);
    EXPECT_FLOAT_EQ(y, 0.75);
    EXPECT_FLOAT_EQ(z, 0.75);
    tInterface.getNodalCoordinates(58, x, y, z);
    EXPECT_FLOAT_EQ(x, 0.75);
    EXPECT_FLOAT_EQ(y, 1.0);
    EXPECT_FLOAT_EQ(z, 0.25);
}

TEST_F(PlatoTestKrino, create_write_read_bbox_background_mesh)
{
    PlatoKrinoInterface tInterface;
    tInterface.createAndWriteBoundingBoxMesh({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}, 0.5, "background_mesh.exo");
    tInterface.readAndSetupMeshForDecomposition("background_mesh.exo");
    double x, y, z;
    tInterface.getNodalCoordinates(1, x, y, z);
    EXPECT_FLOAT_EQ(x, 0.0);
    EXPECT_FLOAT_EQ(y, 0.0);
    EXPECT_FLOAT_EQ(z, 0.0);
    tInterface.getNodalCoordinates(68, x, y, z);
    EXPECT_FLOAT_EQ(x, 0.0);
    EXPECT_FLOAT_EQ(y, 0.75);
    EXPECT_FLOAT_EQ(z, 0.75);
    tInterface.getNodalCoordinates(58, x, y, z);
    EXPECT_FLOAT_EQ(x, 0.75);
    EXPECT_FLOAT_EQ(y, 1.0);
    EXPECT_FLOAT_EQ(z, 0.25);
    std::remove("background_mesh.exo");
}

TEST_F(PlatoTestKrino, cut_sphere_out_of_background_mesh)
{
    PlatoKrinoInterface tInterface;
    tInterface.createAndWriteBoundingBoxMesh({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}, 0.333, "background_mesh.exo");
    tInterface.readAndSetupMeshForDecomposition("background_mesh.exo");
    const std::vector<std::pair<stk::math::Vector3d, double>> spheres{{{0.5, 0.5, 0.5}, 0.3}};
    tInterface.initializeSphereLevelset(spheres);
    tInterface.cutMesh();
    tInterface.includeVoidRegion(false);
    tInterface.writeMesh("swiss_cheese.exo");
    unsigned int tNumSolidTets = tInterface.getNumTetsInNamedBlock("block_1");
    ASSERT_EQ(tNumSolidTets, 672u);
    std::remove("background_mesh.exo");
    std::remove("swiss_cheese.exo");
}

TEST_F(PlatoTestKrino, calculate_dFdLS)
{
    std::map<unsigned int, stk::math::Vector3d> tDFDX_values{
        {1, {.6, .5, .4}}, {2, {.2, -.1, -.9}}, {3, {.45, -.03, -.5}}};
    const std::map<stk::mesh::EntityId, Plato::Krino::InterfaceNode_DXDP> tDXDP{
        {3, Plato::Krino::InterfaceNode_DXDP{{7, 12, 19}, {{.5, .5, .5}, {.4, .4, .4}, {-.1, .1, -.1}}}},
        {1, Plato::Krino::InterfaceNode_DXDP{{34, 22, 2}, {{.1, .1, .1}, {.2, .2, .2}, {-.1, -.1, -.1}}}},
        {2, Plato::Krino::InterfaceNode_DXDP{{19, 10}, {{.3, .3, .3}, {-.2, -.2, -.2}}}},
    };

    //    const std::vector<std::pair<unsigned int, std::vector<std::pair<unsigned int, stk::math::Vector3d>>>>
    //        tSensitivities{{3, {{7, {.5, .5, .5}}, {12, {.4, .4, .4}}, {19, {-.1, .1, -.1}}}},
    //                       {1, {{34, {.1, .1, .1}}, {22, {.2, .2, .2}}, {2, {-.1, -.1, -.1}}}},
    //                       {2, {{19, {.3, .3, .3}}, {10, {-.2, -.2, -.2}}}}};
    //    PlatoKrinoInterface tInterface;
    //    tInterface.setSensitivities(tSensitivities);
    //    tInterface.setUncutBackgroundMeshSize(34);
    const std::vector<unsigned int> tBackgroundNodemap{7, 12, 19, 34, 22, 2, 10};
    std::map<unsigned int, double> tDFDLS = calculateDFDLS(tDFDX_values, tDXDP, tBackgroundNodemap);

    EXPECT_FLOAT_EQ(tDFDLS[34], .15);
    EXPECT_FLOAT_EQ(tDFDLS[22], .3);
    EXPECT_FLOAT_EQ(tDFDLS[2], -.15);
    EXPECT_FLOAT_EQ(tDFDLS[19], -.238);
    EXPECT_FLOAT_EQ(tDFDLS[10], .16);
    EXPECT_FLOAT_EQ(tDFDLS[7], -.04);
    EXPECT_FLOAT_EQ(tDFDLS[12], -.032);
}

TEST_F(PlatoTestKrino, get_set_levelset_values)
{
    PlatoKrinoInterface tInterface;
    tInterface.createBoundingBoxMesh(0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0);
    tInterface.initializePlaneLevelset(-1, .5, .35, .2);
    tInterface.cutMesh();
    std::vector<double> tCurLevelsetValues1 = tInterface.getLevelsetValues();
    tInterface.setLevelsetValues(tCurLevelsetValues1);
    std::vector<double> tCurLevelsetValues2 = tInterface.getLevelsetValues();
    for (size_t i = 0; i < tCurLevelsetValues1.size(); ++i)
    {
        EXPECT_FLOAT_EQ(tCurLevelsetValues1[i], tCurLevelsetValues2[i]);
        tCurLevelsetValues1[i] += 1.0;
    }
    tInterface.setLevelsetValues(tCurLevelsetValues1);
    std::vector<double> tCurLevelsetValues3 = tInterface.getLevelsetValues();
    for (size_t i = 0; i < tCurLevelsetValues1.size(); ++i)
    {
        EXPECT_FLOAT_EQ(tCurLevelsetValues3[i], tCurLevelsetValues2[i] + 1.0);
        EXPECT_FLOAT_EQ(tCurLevelsetValues3[i], tCurLevelsetValues1[i]);
    }
}

TEST_F(PlatoTestKrino, redistance)
{
    PlatoKrinoInterface tInterface;
    tInterface.createBoundingBoxMesh(0.0, 0.0, 0.0, 2.0, 1.0, 1.0, 1.0);
    tInterface.initializePlaneLevelset(1, 0, 0, -.25);
    tInterface.cutMesh();
    std::vector<double> tLevelsetValues = tInterface.getLevelsetValues();
    const std::vector<double> tInitialGold = {-0.25, 0.75,  1.75, -0.25, 0.75, 1.75, -0.25, 0.75,
                                              1.75,  -0.25, 0.75, 1.75,  0.25, 1.25, 0.25,  1.25,
                                              -0.25, 0.25,  1.25, 1.75,  0.25, 1.25, 0.25,  1.25};
    EXPECT_EQ(tLevelsetValues, tInitialGold);

    // Perturb the nodes adjacent to the interface so that the interface moves slightly to the right.
    const double tDelta = .05;
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

    tInterface.resetMesh();
    tInterface.setLevelsetValues(tLevelsetValues);
    tInterface.cutMesh();

    std::vector<double> tLevelsetValues2 = tInterface.getLevelsetValues();
    EXPECT_EQ(tLevelsetValues, tLevelsetValues2);
    tInterface.redistance();
    std::vector<double> tLevelsetValues3 = tInterface.getLevelsetValues();
    std::vector<double> tRedistancedGold = tInitialGold;
    for (auto &tCurVal : tRedistancedGold)
    {
        tCurVal -= tDelta;
    }
    for (size_t i = 0; i < tRedistancedGold.size(); ++i)
    {
        EXPECT_FLOAT_EQ(tRedistancedGold[i], tLevelsetValues3[i]);
    }
}

TEST_F(PlatoTestKrino, test_krino_sensitivities)
{
    PlatoKrinoInterface tInterface;
    tInterface.createBoundingBoxMesh(0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0);
    tInterface.initializePlaneLevelset(-1, .5, .35, .2);
    std::vector<double> tOriginalLevelsetValues = tInterface.getLevelsetValues();
    tInterface.cutMesh();
    tInterface.getSensitivities();
    std::unordered_map<unsigned int, stk::math::Vector3d> tCurCoordinateValues = tInterface.getCoordinateValues();
    std::unordered_map<unsigned int, stk::math::Vector3d> tPredictedCoordValues =
        tInterface.predictNewCoordinatesBasedOnPerturbedLevelsetValues(tCurCoordinateValues, .01);
    tInterface.resetMesh();
    // Add .01 to all level set values
    for (auto &tCurLS : tOriginalLevelsetValues)
    {
        tCurLS += .01;
    }
    tInterface.setLevelsetValues(tOriginalLevelsetValues);
    tInterface.cutMesh();
    std::unordered_map<unsigned int, stk::math::Vector3d> tNewCoordValues = tInterface.getCoordinateValues();
    for (auto tPredictedCoordValue : tPredictedCoordValues)
    {
        for (int i = 0; i < 3; i++)
        {
            EXPECT_FLOAT_EQ(tPredictedCoordValue.second[i], tNewCoordValues[tPredictedCoordValue.first][i]);
        }
    }
}

TEST_F(PlatoTestKrino, test_generateSpheres_success)
{
    SpherePatternData tData;
    tData.mCoordMins = {1.0, -2.0, 3.0};
    tData.mCoordMaxes = {10.0, 20.0, 30.0};
    tData.mNumSpheres = {3, 1, 4};
    tData.mSphereRadius = 1.5;
    tData.mSpheresCanOverlapBoundingBox = false;
    const std::vector<Sphere> tSpheres = generateSpheres(tData);

    EXPECT_FLOAT_EQ(tSpheres[0].mCenterX, 3.25);
    EXPECT_FLOAT_EQ(tSpheres[0].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[0].mCenterZ, 8.4);
    EXPECT_FLOAT_EQ(tSpheres[0].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[1].mCenterX, 3.25);
    EXPECT_FLOAT_EQ(tSpheres[1].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[1].mCenterZ, 13.8);
    EXPECT_FLOAT_EQ(tSpheres[1].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[2].mCenterX, 3.25);
    EXPECT_FLOAT_EQ(tSpheres[2].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[2].mCenterZ, 19.2);
    EXPECT_FLOAT_EQ(tSpheres[2].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[3].mCenterX, 3.25);
    EXPECT_FLOAT_EQ(tSpheres[3].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[3].mCenterZ, 24.6);
    EXPECT_FLOAT_EQ(tSpheres[3].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[4].mCenterX, 5.5);
    EXPECT_FLOAT_EQ(tSpheres[4].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[4].mCenterZ, 8.4);
    EXPECT_FLOAT_EQ(tSpheres[4].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[5].mCenterX, 5.5);
    EXPECT_FLOAT_EQ(tSpheres[5].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[5].mCenterZ, 13.8);
    EXPECT_FLOAT_EQ(tSpheres[5].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[6].mCenterX, 5.5);
    EXPECT_FLOAT_EQ(tSpheres[6].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[6].mCenterZ, 19.2);
    EXPECT_FLOAT_EQ(tSpheres[6].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[7].mCenterX, 5.5);
    EXPECT_FLOAT_EQ(tSpheres[7].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[7].mCenterZ, 24.6);
    EXPECT_FLOAT_EQ(tSpheres[7].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[8].mCenterX, 7.75);
    EXPECT_FLOAT_EQ(tSpheres[8].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[8].mCenterZ, 8.4);
    EXPECT_FLOAT_EQ(tSpheres[8].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[9].mCenterX, 7.75);
    EXPECT_FLOAT_EQ(tSpheres[9].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[9].mCenterZ, 13.8);
    EXPECT_FLOAT_EQ(tSpheres[9].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[10].mCenterX, 7.75);
    EXPECT_FLOAT_EQ(tSpheres[10].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[10].mCenterZ, 19.2);
    EXPECT_FLOAT_EQ(tSpheres[10].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[11].mCenterX, 7.75);
    EXPECT_FLOAT_EQ(tSpheres[11].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[11].mCenterZ, 24.6);
    EXPECT_FLOAT_EQ(tSpheres[11].mRadius, 1.5);
}

TEST_F(PlatoTestKrino, test_generateSpheres_success_with_overlap)
{
    SpherePatternData tData;
    tData.mCoordMins = {1.0, -2.0, 3.0};
    tData.mCoordMaxes = {10.0, 20.0, 30.0};
    tData.mNumSpheres = {3, 1, 4};
    tData.mSphereRadius = 1.5;
    tData.mSpheresCanOverlapBoundingBox = true;
    std::vector<Sphere> tSpheres = generateSpheres(tData);

    EXPECT_FLOAT_EQ(tSpheres[0].mCenterX, 1);
    EXPECT_FLOAT_EQ(tSpheres[0].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[0].mCenterZ, 3);
    EXPECT_FLOAT_EQ(tSpheres[0].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[1].mCenterX, 1);
    EXPECT_FLOAT_EQ(tSpheres[1].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[1].mCenterZ, 12);
    EXPECT_FLOAT_EQ(tSpheres[1].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[2].mCenterX, 1);
    EXPECT_FLOAT_EQ(tSpheres[2].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[2].mCenterZ, 21);
    EXPECT_FLOAT_EQ(tSpheres[2].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[3].mCenterX, 1);
    EXPECT_FLOAT_EQ(tSpheres[3].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[3].mCenterZ, 30);
    EXPECT_FLOAT_EQ(tSpheres[3].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[4].mCenterX, 5.5);
    EXPECT_FLOAT_EQ(tSpheres[4].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[4].mCenterZ, 3);
    EXPECT_FLOAT_EQ(tSpheres[4].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[5].mCenterX, 5.5);
    EXPECT_FLOAT_EQ(tSpheres[5].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[5].mCenterZ, 12);
    EXPECT_FLOAT_EQ(tSpheres[5].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[6].mCenterX, 5.5);
    EXPECT_FLOAT_EQ(tSpheres[6].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[6].mCenterZ, 21);
    EXPECT_FLOAT_EQ(tSpheres[6].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[7].mCenterX, 5.5);
    EXPECT_FLOAT_EQ(tSpheres[7].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[7].mCenterZ, 30);
    EXPECT_FLOAT_EQ(tSpheres[7].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[8].mCenterX, 10);
    EXPECT_FLOAT_EQ(tSpheres[8].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[8].mCenterZ, 3);
    EXPECT_FLOAT_EQ(tSpheres[8].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[9].mCenterX, 10);
    EXPECT_FLOAT_EQ(tSpheres[9].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[9].mCenterZ, 12);
    EXPECT_FLOAT_EQ(tSpheres[9].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[10].mCenterX, 10);
    EXPECT_FLOAT_EQ(tSpheres[10].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[10].mCenterZ, 21);
    EXPECT_FLOAT_EQ(tSpheres[10].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[11].mCenterX, 10);
    EXPECT_FLOAT_EQ(tSpheres[11].mCenterY, 9);
    EXPECT_FLOAT_EQ(tSpheres[11].mCenterZ, 30);
    EXPECT_FLOAT_EQ(tSpheres[11].mRadius, 1.5);
}

TEST_F(PlatoTestKrino, calculateSphereStartsAndSpacing_succeed_with_bbox_overlap)
{
    SpherePatternData tData;
    tData.mCoordMins = {-3.0, -2.0, 10.0};
    tData.mCoordMaxes = {-1.0, 5.0, 10.5};
    tData.mNumSpheres = {1, 5, 2};
    tData.mSphereRadius = 1.5;
    tData.mSpheresCanOverlapBoundingBox = true;
    SphereLocatorData tLocatorData = calculateSphereStartsAndSpacing(tData);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[0].first, -3.0);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[1].first, -3.75);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[2].first, 9.5);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[0].second, 1.0);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[1].second, 1.75);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[2].second, 0.5);
}

TEST_F(PlatoTestKrino, calculateSphereStartsAndSpacing_succeed_without_bbox_overlap)
{
    SpherePatternData tData;
    tData.mCoordMins = {-3.0, -2.0, 10.0};
    tData.mCoordMaxes = {-1.0, 5.0, 10.5};
    tData.mNumSpheres = {1, 5, 2};
    tData.mSphereRadius = 1.5;
    tData.mSpheresCanOverlapBoundingBox = false;
    SphereLocatorData tLocatorData = calculateSphereStartsAndSpacing(tData);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[0].first, -3.0);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[1].first, -2.0);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[2].first, 10.0);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[0].second, 1.0);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[1].second, 7.0 / 6.0);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[2].second, 1.0 / 6.0);
}

TEST_F(PlatoTestKrino, checkForReasonableSpherePatternDefinition_success)
{
    SpherePatternData tData;
    tData.mCoordMins = {-3.0, -2.0, 10.0};
    tData.mCoordMaxes = {-1.0, 5.0, 10.5};
    tData.mNumSpheres = {1, 5, 2};
    tData.mSphereRadius = 1.5;
    tData.mSpheresCanOverlapBoundingBox = false;
    checkForReasonableSpherePatternDefinition(tData);
}

TEST_F(PlatoTestKrino, checkForReasonableSpherePatternDefinition_fail_invalid_bbox)
{
    SpherePatternData tData;
    tData.mCoordMins = {-0.5, -2.0, 10.0};
    tData.mCoordMaxes = {-1.0, 5.0, 10.5};
    tData.mNumSpheres = {1, 5, 2};
    tData.mSphereRadius = 1.5;
    tData.mSpheresCanOverlapBoundingBox = false;
    EXPECT_THROW(checkForReasonableSpherePatternDefinition(tData), std::runtime_error);
}

TEST_F(PlatoTestKrino, checkForReasonableSpherePatternDefinition_fail_invalid_count)
{
    SpherePatternData tData;
    tData.mCoordMins = {-5, -2.0, 10.0};
    tData.mCoordMaxes = {-1.0, 5.0, 10.5};
    tData.mNumSpheres = {1, -1, 2};
    tData.mSphereRadius = 1.5;
    tData.mSpheresCanOverlapBoundingBox = false;
    EXPECT_THROW(checkForReasonableSpherePatternDefinition(tData), std::runtime_error);
}

TEST_F(PlatoTestKrino, checkForReasonableSpherePatternDefinition_fail_invalid_radius)
{
    SpherePatternData tData;
    tData.mCoordMins = {-5, -2.0, 10.0};
    tData.mCoordMaxes = {-1.0, 5.0, 10.5};
    tData.mNumSpheres = {1, 6, 2};
    tData.mSphereRadius = -1.5;
    tData.mSpheresCanOverlapBoundingBox = false;
    EXPECT_THROW(checkForReasonableSpherePatternDefinition(tData), std::runtime_error);
}

}  // namespace Plato::Krino
