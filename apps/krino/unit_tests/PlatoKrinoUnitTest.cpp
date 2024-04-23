/*
 * PlatoKrinoUnitTests.cpp
 *
 *  Created on: Nov 1, 2023
 */

#include <cstdio> 
#include <gtest/gtest.h>                // for AssertHelper, TEST, etc
#include <PlatoKrinoInterface.hpp>
#include "Plato_InputData.hpp"
#include "Plato_Parser.hpp"

namespace Plato::Krino
{

TEST(PlatoKrinoUnitTests, create_bounding_box_background_mesh)
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

TEST(PlatoKrinoUnitTests, create_write_read_bbox_background_mesh)
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

TEST(PlatoKrinoUnitTests, cut_sphere_out_of_background_mesh)
{
    PlatoKrinoInterface tInterface;
    tInterface.createAndWriteBoundingBoxMesh({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}, 0.333, "background_mesh.exo");
    tInterface.readAndSetupMeshForDecomposition("background_mesh.exo");
    const std::vector<std::pair<stk::math::Vector3d,double>> spheres
    {
      { {0.5,0.5,0.5}, 0.3 }
    };
    tInterface.initializeSphereLevelset(spheres);
    tInterface.cutMesh();
    tInterface.includeVoidRegion(false);
    tInterface.writeMesh("swiss_cheese.exo");
    unsigned int tNumSolidTets = tInterface.getNumTetsInNamedBlock("block_1");
    ASSERT_EQ(tNumSolidTets, 672u);
    std::remove("background_mesh.exo");
    std::remove("swiss_cheese.exo");
}

TEST(PlatoKrinoUnitTests, calculate_dFdLS)
{
    std::map<unsigned int, stk::math::Vector3d> tDFDX_values
    { 
        {1, {.6, .5, .4}     },
        {2, {.2, -.1, -.9}   },
        {3, {.45, -.03, -.5} }
    };
    const std::vector<std::pair<unsigned int, std::vector<std::pair<unsigned int, stk::math::Vector3d>>>> tSensitivities
    { 
        {3, {{7, {.5, .5, .5}}, {12, {.4, .4, .4}}, {19, {-.1, .1, -.1}}}  },
        {1, {{34, {.1, .1, .1}}, {22, {.2, .2, .2}}, {2, {-.1, -.1, -.1}}} },
        {2, {{19, {.3, .3, .3}}, {10, {-.2, -.2, -.2}}}                    }
    };
    PlatoKrinoInterface tInterface;
    tInterface.setSensitivities(tSensitivities);
    tInterface.setUncutBackgroundMeshSize(34);
    std::map<unsigned int, double> tDFDLS = tInterface.calculateDFDLS(tDFDX_values);
    EXPECT_FLOAT_EQ(tDFDLS[34], .15);
    EXPECT_FLOAT_EQ(tDFDLS[22], .3);
    EXPECT_FLOAT_EQ(tDFDLS[2], -.15);
    EXPECT_FLOAT_EQ(tDFDLS[19], -.238);
    EXPECT_FLOAT_EQ(tDFDLS[10], .16);
    EXPECT_FLOAT_EQ(tDFDLS[7], -.04);
    EXPECT_FLOAT_EQ(tDFDLS[12], -.032);
    
}

TEST(PlatoKrinoUnitTests, get_set_levelset_values)
{
    PlatoKrinoInterface tInterface;
    tInterface.createBoundingBoxMesh(0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0);
    tInterface.initializePlaneLevelset(-1, .5, .35, .2);
    tInterface.cutMesh();
    std::vector<double> tCurLevelsetValues1 = tInterface.getLevelsetValues();
    tInterface.setLevelsetValues(tCurLevelsetValues1);
    std::vector<double> tCurLevelsetValues2 = tInterface.getLevelsetValues();
    for(size_t i=0; i<tCurLevelsetValues1.size(); ++i)
    {
        EXPECT_FLOAT_EQ(tCurLevelsetValues1[i], tCurLevelsetValues2[i]); 
        tCurLevelsetValues1[i] += 1.0;
    }
    tInterface.setLevelsetValues(tCurLevelsetValues1);
    std::vector<double> tCurLevelsetValues3 = tInterface.getLevelsetValues();
    for(size_t i=0; i<tCurLevelsetValues1.size(); ++i)
    {
        EXPECT_FLOAT_EQ(tCurLevelsetValues3[i], tCurLevelsetValues2[i]+1.0); 
        EXPECT_FLOAT_EQ(tCurLevelsetValues3[i], tCurLevelsetValues1[i]); 
    }
}

TEST(PlatoKrinoUnitTests, redistance)
{
    PlatoKrinoInterface tInterface;
    tInterface.createBoundingBoxMesh(0.0, 0.0, 0.0, 2.0, 1.0, 1.0, 1.0);
    tInterface.initializePlaneLevelset(1, 0, 0, -.25);
    tInterface.cutMesh();
    std::vector<double> tLevelsetValues = tInterface.getLevelsetValues();
    const std::vector<double> tInitialGold = {-0.25,0.75,1.75,-0.25,0.75,1.75,-0.25,0.75,1.75,-0.25,
                                              0.75,1.75,0.25,1.25,0.25,1.25,-0.25,0.25,1.25,1.75,0.25,
                                              1.25,0.25,1.25};
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
    for(auto &tCurVal : tRedistancedGold)
    {
        tCurVal -= tDelta;
    }
    for(size_t i=0; i<tRedistancedGold.size(); ++i)
    {
        EXPECT_FLOAT_EQ(tRedistancedGold[i], tLevelsetValues3[i]); 
    }
}

TEST(PlatoKrinoUnitTests, test_krino_sensitivities)
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
    for(auto &tCurLS : tOriginalLevelsetValues)
    {
        tCurLS += .01;
    } 
    tInterface.setLevelsetValues(tOriginalLevelsetValues);
    tInterface.cutMesh();
    std::unordered_map<unsigned int, stk::math::Vector3d> tNewCoordValues = tInterface.getCoordinateValues();
    for(auto tPredictedCoordValue : tPredictedCoordValues)
    {
        for(int i=0; i<3; i++)
        {
            EXPECT_FLOAT_EQ(tPredictedCoordValue.second[i], tNewCoordValues[tPredictedCoordValue.first][i]); 
        }
    }
}

TEST(PlatoKrinoUnitTests, readSpherePatternData_succeed)
{
    const std::string tInput =
      "<SpherePattern>\n"
      "  <bbox_xmin>1.0</bbox_xmin>\n"
      "  <bbox_ymin>-2.0</bbox_ymin>\n"
      "  <bbox_zmin>3.0</bbox_zmin>\n"
      "  <bbox_xmax>1</bbox_xmax>\n"
      "  <bbox_ymax>.5</bbox_ymax>\n"
      "  <bbox_zmax>.05</bbox_zmax>\n"
      "  <num_spheres_x>5</num_spheres_x>\n"
      "  <num_spheres_y>4</num_spheres_y>\n"
      "  <num_spheres_z>1</num_spheres_z>\n"
      "  <radius>.075</radius>\n"
      "  <overlap_bbox>true</overlap_bbox>\n"
      "</SpherePattern>\n";

    const Plato::PugiParser tParser;
    const Plato::InputData tInputData = tParser.parseString(tInput);
    Plato::InputData tSpherePatternNode = tInputData.get<Plato::InputData>("SpherePattern");

    SpherePatternData tData;
    tData = readSpherePatternData(tSpherePatternNode);
    EXPECT_FLOAT_EQ(tData.mCoordMins[0], 1.0); 
    EXPECT_FLOAT_EQ(tData.mCoordMins[1], -2.0); 
    EXPECT_FLOAT_EQ(tData.mCoordMins[2], 3.0); 
    EXPECT_FLOAT_EQ(tData.mCoordMaxes[0], 1.0); 
    EXPECT_FLOAT_EQ(tData.mCoordMaxes[1], .5); 
    EXPECT_FLOAT_EQ(tData.mCoordMaxes[2], .05); 
    EXPECT_EQ(tData.mNumSpheres[0], 5); 
    EXPECT_EQ(tData.mNumSpheres[1], 4); 
    EXPECT_EQ(tData.mNumSpheres[2], 1); 
    EXPECT_FLOAT_EQ(tData.mSphereRadius, .075); 
    EXPECT_EQ(tData.mSpheresCanOverlapBoundingBox, true); 
}

TEST(PlatoKrinoUnitTests, readSpherePatternData_fail_missing_parameter)
{
    const std::string tInput =
      "<SpherePattern>\n"
      "  <bbox_xmin>1.0</bbox_xmin>\n"
      "  <bbox_zmin>3.0</bbox_zmin>\n"
      "  <bbox_xmax>1</bbox_xmax>\n"
      "  <bbox_ymax>.5</bbox_ymax>\n"
      "  <bbox_zmax>.05</bbox_zmax>\n"
      "  <num_spheres_x>5</num_spheres_x>\n"
      "  <num_spheres_y>4</num_spheres_y>\n"
      "  <num_spheres_z>1</num_spheres_z>\n"
      "  <radius>.075</radius>\n"
      "  <overlap_bbox>true</overlap_bbox>\n"
      "</SpherePattern>\n";

    const Plato::PugiParser tParser;
    const Plato::InputData tInputData = tParser.parseString(tInput);
    Plato::InputData tSpherePatternNode = tInputData.get<Plato::InputData>("SpherePattern");
    EXPECT_THROW(readSpherePatternData(tSpherePatternNode), std::runtime_error);
}

TEST(PlatoKrinoUnitTests, readSphereData_succeed)
{
    const std::string tInput =
      "<Sphere>\n"
      "  <center_x>1.0</center_x>\n"
      "  <center_y>2.0</center_y>\n"
      "  <center_z>3.0</center_z>\n"
      "  <radius>2.0</radius>\n"
      "</Sphere>\n";

    const Plato::PugiParser tParser;
    const Plato::InputData tInputData = tParser.parseString(tInput);
    Plato::InputData tSphereNode = tInputData.get<Plato::InputData>("Sphere");

    Sphere tSphere = readSphereData(tSphereNode);
    EXPECT_FLOAT_EQ(tSphere.mCenterX, 1.0); 
    EXPECT_FLOAT_EQ(tSphere.mCenterY, 2.0); 
    EXPECT_FLOAT_EQ(tSphere.mCenterZ, 3.0); 
    EXPECT_FLOAT_EQ(tSphere.mRadius, 2.0); 
}

TEST(PlatoKrinoUnitTests, readSphereData_fail_missing_param)
{
    const std::string tInput =
      "<Sphere>\n"
      "  <center_y>2.0</center_y>\n"
      "  <center_z>3.0</center_z>\n"
      "  <radius>2.0</radius>\n"
      "</Sphere>\n";

    const Plato::PugiParser tParser;
    const Plato::InputData tInputData = tParser.parseString(tInput);
    Plato::InputData tSphereNode = tInputData.get<Plato::InputData>("Sphere");

    EXPECT_THROW(readSphereData(tSphereNode), std::runtime_error);
}

TEST(PlatoKrinoUnitTests, readPlaneData_succeed)
{
    const std::string tInput =
      "<Plane>\n"
      "  <normal_x>0.0</normal_x>\n"
      "  <normal_y>0.5</normal_y>\n"
      "  <normal_z>3.0</normal_z>\n"
      "  <offset>-2.2</offset>\n"
      "</Plane>\n";

    const Plato::PugiParser tParser;
    const Plato::InputData tInputData = tParser.parseString(tInput);
    Plato::InputData tPlaneNode = tInputData.get<Plato::InputData>("Plane");

    Plane tPlane = readPlaneData(tPlaneNode);
    EXPECT_FLOAT_EQ(tPlane.mNormalX, 0.0); 
    EXPECT_FLOAT_EQ(tPlane.mNormalY, 0.5); 
    EXPECT_FLOAT_EQ(tPlane.mNormalZ, 3.0); 
    EXPECT_FLOAT_EQ(tPlane.mOffset, -2.2); 
}

TEST(PlatoKrinoUnitTests, readPlaneData_fail_missing_param)
{
    const std::string tInput =
      "<Plane>\n"
      "  <normal_x>0.0</normal_x>\n"
      "  <normal_y>0.5</normal_y>\n"
      "  <offset>-2.2</offset>\n"
      "</Plane>\n";

    const Plato::PugiParser tParser;
    const Plato::InputData tInputData = tParser.parseString(tInput);
    Plato::InputData tPlaneNode = tInputData.get<Plato::InputData>("Plane");

    EXPECT_THROW(readPlaneData(tPlaneNode), std::runtime_error);
}

TEST(PlatoKrinoUnitTests, test_generateSpheres_success)
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

TEST(PlatoKrinoUnitTests, test_generateSpheres_success_with_overlap)
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

TEST(PlatoKrinoUnitTests, calculateSphereStartsAndSpacing_succeed_with_bbox_overlap)
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

TEST(PlatoKrinoUnitTests, calculateSphereStartsAndSpacing_succeed_without_bbox_overlap)
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
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[1].second, 7.0/6.0); 
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[2].second, 1.0/6.0); 
}

TEST(PlatoKrinoUnitTests, checkForReasonableSpherePatternDefinition_success)
{
    SpherePatternData tData;
    tData.mCoordMins = {-3.0, -2.0, 10.0};
    tData.mCoordMaxes = {-1.0, 5.0, 10.5};
    tData.mNumSpheres = {1, 5, 2};
    tData.mSphereRadius = 1.5;
    tData.mSpheresCanOverlapBoundingBox = false;
    checkForReasonableSpherePatternDefinition(tData);
}

TEST(PlatoKrinoUnitTests, checkForReasonableSpherePatternDefinition_fail_invalid_bbox)
{
    SpherePatternData tData;
    tData.mCoordMins = {-0.5, -2.0, 10.0};
    tData.mCoordMaxes = {-1.0, 5.0, 10.5};
    tData.mNumSpheres = {1, 5, 2};
    tData.mSphereRadius = 1.5;
    tData.mSpheresCanOverlapBoundingBox = false;
    EXPECT_THROW(checkForReasonableSpherePatternDefinition(tData), std::runtime_error);
}

TEST(PlatoKrinoUnitTests, checkForReasonableSpherePatternDefinition_fail_invalid_count)
{
    SpherePatternData tData;
    tData.mCoordMins = {-5, -2.0, 10.0};
    tData.mCoordMaxes = {-1.0, 5.0, 10.5};
    tData.mNumSpheres = {1, -1, 2};
    tData.mSphereRadius = 1.5;
    tData.mSpheresCanOverlapBoundingBox = false;
    EXPECT_THROW(checkForReasonableSpherePatternDefinition(tData), std::runtime_error);
}

TEST(PlatoKrinoUnitTests, checkForReasonableSpherePatternDefinition_fail_invalid_radius)
{
    SpherePatternData tData;
    tData.mCoordMins = {-5, -2.0, 10.0};
    tData.mCoordMaxes = {-1.0, 5.0, 10.5};
    tData.mNumSpheres = {1, 6, 2};
    tData.mSphereRadius = -1.5;
    tData.mSpheresCanOverlapBoundingBox = false;
    EXPECT_THROW(checkForReasonableSpherePatternDefinition(tData), std::runtime_error);
}


}
