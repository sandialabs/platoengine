
#include <gtest/gtest.h>  // for AssertHelper, TEST, etc

#include <cstdio>

#include "plato/third_party_integration/krino/KrinoWrapper.hpp"

namespace plato::third_party_integration::krino::unit_tests
{

class PlatoTestKrino : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        static bool tFirstTime{true};
        if (tFirstTime)
        {
            initialize_environment_for_krino(MPI_COMM_WORLD);
            tFirstTime = false;
        }
    }
};

TEST_F(PlatoTestKrino, create_bounding_box_background_mesh)
{
    const std::string tFilename{"tmp.exo"};
    KrinoWrapper tKrinoWrapper{stk::math::Vector3d{0.0, 0.0, 0.0}, stk::math::Vector3d{1.0, 1.0, 1.0}, 0.5, tFilename};
    double x, y, z;
    tKrinoWrapper.getNodalCoordinates(1, x, y, z);
    EXPECT_FLOAT_EQ(x, 0.0);
    EXPECT_FLOAT_EQ(y, 0.0);
    EXPECT_FLOAT_EQ(z, 0.0);
    tKrinoWrapper.getNodalCoordinates(68, x, y, z);
    EXPECT_FLOAT_EQ(x, 0.0);
    EXPECT_FLOAT_EQ(y, 0.75);
    EXPECT_FLOAT_EQ(z, 0.75);
    tKrinoWrapper.getNodalCoordinates(58, x, y, z);
    EXPECT_FLOAT_EQ(x, 0.75);
    EXPECT_FLOAT_EQ(y, 1.0);
    EXPECT_FLOAT_EQ(z, 0.25);
    remove(tFilename.c_str());
}

TEST_F(PlatoTestKrino, cut_sphere_out_of_background_mesh)
{
    const std::string tBackgroundFilename{"background_mesh.exo"};
    const std::string tCutFilename{"swiss_cheese.exo"};
    const std::string tBlockName{"block_1"};
    constexpr bool tIncludeVoidRegion = false;

    KrinoWrapper tKrinoWrapper{{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}, 0.333, tBackgroundFilename, tIncludeVoidRegion};
    const std::vector<std::pair<stk::math::Vector3d, double>> spheres{{{0.5, 0.5, 0.5}, 0.3}};
    tKrinoWrapper.initializeSphereLevelset(spheres);
    tKrinoWrapper.cutMesh();
    tKrinoWrapper.writeMesh(tCutFilename);
    const unsigned int tNumSolidTets = tKrinoWrapper.getNumTetsInNamedBlock(tBlockName);
    ASSERT_EQ(tNumSolidTets, 672u);
    remove(tBackgroundFilename.c_str());
    remove(tCutFilename.c_str());
}

TEST_F(PlatoTestKrino, calculate_dFdLS)
{
    const std::unordered_map<unsigned int, stk::math::Vector3d> tDFDX_values{
        {1, {.6, .5, .4}}, {2, {.2, -.1, -.9}}, {3, {.45, -.03, -.5}}};
    const std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP> tDXDP{
        {3, InterfaceNodeDXDP{{7, 12, 19}, {{.5, .5, .5}, {.4, .4, .4}, {-.1, .1, -.1}}}},
        {1, InterfaceNodeDXDP{{34, 22, 2}, {{.1, .1, .1}, {.2, .2, .2}, {-.1, -.1, -.1}}}},
        {2, InterfaceNodeDXDP{{19, 10}, {{.3, .3, .3}, {-.2, -.2, -.2}}}},
    };

    const std::vector<unsigned int> tBackgroundNodemap{7, 12, 19, 34, 22, 2, 10};
    const std::unordered_map<unsigned int, double> tDFDLS = calculate_dfdls(tDFDX_values, tDXDP, tBackgroundNodemap);

    EXPECT_FLOAT_EQ(tDFDLS.at(34), .15);
    EXPECT_FLOAT_EQ(tDFDLS.at(22), .3);
    EXPECT_FLOAT_EQ(tDFDLS.at(2), -.15);
    EXPECT_FLOAT_EQ(tDFDLS.at(19), -.238);
    EXPECT_FLOAT_EQ(tDFDLS.at(10), .16);
    EXPECT_FLOAT_EQ(tDFDLS.at(7), -.04);
    EXPECT_FLOAT_EQ(tDFDLS.at(12), -.032);
}

TEST_F(PlatoTestKrino, get_set_levelset_values)
{
    const std::string tFilename{"tmp.exo"};
    KrinoWrapper tKrinoWrapper{stk::math::Vector3d{0.0, 0.0, 0.0}, stk::math::Vector3d{1.0, 1.0, 1.0}, 1.0, tFilename};
    tKrinoWrapper.initializePlaneLevelset(-1, .5, .35, .2);
    tKrinoWrapper.cutMesh();
    std::vector<double> tCurLevelsetValues1 = tKrinoWrapper.getLevelsetValues();
    tKrinoWrapper.setLevelsetValues(tCurLevelsetValues1);
    const std::vector<double> tCurLevelsetValues2 = tKrinoWrapper.getLevelsetValues();
    for (size_t i = 0; i < tCurLevelsetValues1.size(); ++i)
    {
        EXPECT_FLOAT_EQ(tCurLevelsetValues1[i], tCurLevelsetValues2[i]);
        tCurLevelsetValues1[i] += 1.0;
    }
    tKrinoWrapper.setLevelsetValues(tCurLevelsetValues1);
    const std::vector<double> tCurLevelsetValues3 = tKrinoWrapper.getLevelsetValues();
    for (size_t i = 0; i < tCurLevelsetValues1.size(); ++i)
    {
        EXPECT_FLOAT_EQ(tCurLevelsetValues3[i], tCurLevelsetValues2[i] + 1.0);
        EXPECT_FLOAT_EQ(tCurLevelsetValues3[i], tCurLevelsetValues1[i]);
    }
    remove(tFilename.c_str());
}

TEST_F(PlatoTestKrino, redistance)
{
    const std::string tFilename{"tmp.exo"};
    KrinoWrapper tKrinoWrapper{stk::math::Vector3d{0.0, 0.0, 0.0}, stk::math::Vector3d{2.0, 1.0, 1.0}, 1.0, tFilename};
    tKrinoWrapper.initializePlaneLevelset(1, 0, 0, -.25);
    tKrinoWrapper.cutMesh();
    std::vector<double> tLevelsetValues = tKrinoWrapper.getLevelsetValues();
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
        EXPECT_FLOAT_EQ(tRedistancedGold[i], tLevelsetValues3[i]);
    }
    remove(tFilename.c_str());
}

TEST_F(PlatoTestKrino, test_krino_sensitivities)
{
    const std::string tFilename{"tmp.exo"};
    KrinoWrapper tKrinoWrapper{stk::math::Vector3d{0.0, 0.0, 0.0}, stk::math::Vector3d{1.0, 1.0, 1.0}, 1.0, tFilename};
    tKrinoWrapper.initializePlaneLevelset(-1, .5, .35, .2);
    std::vector<double> tOriginalLevelsetValues = tKrinoWrapper.getLevelsetValues();
    tKrinoWrapper.cutMesh();
    tKrinoWrapper.getSensitivities();
    const std::unordered_map<unsigned int, stk::math::Vector3d> tCurCoordinateValues =
        tKrinoWrapper.getCoordinateValues();
    const std::unordered_map<unsigned int, stk::math::Vector3d> tPredictedCoordValues =
        tKrinoWrapper.predictNewCoordinatesBasedOnPerturbedLevelsetValues(tCurCoordinateValues, .01);
    tKrinoWrapper.resetMesh();
    // Add .01 to all level set values
    for (auto &tCurLS : tOriginalLevelsetValues)
    {
        tCurLS += .01;
    }
    tKrinoWrapper.setLevelsetValues(tOriginalLevelsetValues);
    tKrinoWrapper.cutMesh();
    const std::unordered_map<unsigned int, stk::math::Vector3d> tNewCoordValues = tKrinoWrapper.getCoordinateValues();
    for (auto tPredictedCoordValue : tPredictedCoordValues)
    {
        for (int i = 0; i < 3; i++)
        {
            EXPECT_FLOAT_EQ(tPredictedCoordValue.second[i], tNewCoordValues.at(tPredictedCoordValue.first)[i]);
        }
    }
    remove(tFilename.c_str());
}

TEST_F(PlatoTestKrino, test_generate_spheres_success)
{
    SpherePatternData tData;
    tData.mBoundingBoxMinXYZ = {1.0, -2.0, 3.0};
    tData.mBoundingBoxMaxXYZ = {10.0, 20.0, 30.0};
    tData.mNumSpheres = {3, 1, 4};
    tData.mSphereRadius = 1.5;
    tData.mSpheresCanOverlapBoundingBox = false;
    const std::vector<Sphere> tSpheres = generate_spheres(tData);

    EXPECT_FLOAT_EQ(tSpheres[0].mCenter.x, 3.25);
    EXPECT_FLOAT_EQ(tSpheres[0].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[0].mCenter.z, 8.4);
    EXPECT_FLOAT_EQ(tSpheres[0].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[1].mCenter.x, 3.25);
    EXPECT_FLOAT_EQ(tSpheres[1].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[1].mCenter.z, 13.8);
    EXPECT_FLOAT_EQ(tSpheres[1].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[2].mCenter.x, 3.25);
    EXPECT_FLOAT_EQ(tSpheres[2].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[2].mCenter.z, 19.2);
    EXPECT_FLOAT_EQ(tSpheres[2].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[3].mCenter.x, 3.25);
    EXPECT_FLOAT_EQ(tSpheres[3].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[3].mCenter.z, 24.6);
    EXPECT_FLOAT_EQ(tSpheres[3].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[4].mCenter.x, 5.5);
    EXPECT_FLOAT_EQ(tSpheres[4].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[4].mCenter.z, 8.4);
    EXPECT_FLOAT_EQ(tSpheres[4].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[5].mCenter.x, 5.5);
    EXPECT_FLOAT_EQ(tSpheres[5].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[5].mCenter.z, 13.8);
    EXPECT_FLOAT_EQ(tSpheres[5].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[6].mCenter.x, 5.5);
    EXPECT_FLOAT_EQ(tSpheres[6].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[6].mCenter.z, 19.2);
    EXPECT_FLOAT_EQ(tSpheres[6].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[7].mCenter.x, 5.5);
    EXPECT_FLOAT_EQ(tSpheres[7].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[7].mCenter.z, 24.6);
    EXPECT_FLOAT_EQ(tSpheres[7].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[8].mCenter.x, 7.75);
    EXPECT_FLOAT_EQ(tSpheres[8].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[8].mCenter.z, 8.4);
    EXPECT_FLOAT_EQ(tSpheres[8].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[9].mCenter.x, 7.75);
    EXPECT_FLOAT_EQ(tSpheres[9].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[9].mCenter.z, 13.8);
    EXPECT_FLOAT_EQ(tSpheres[9].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[10].mCenter.x, 7.75);
    EXPECT_FLOAT_EQ(tSpheres[10].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[10].mCenter.z, 19.2);
    EXPECT_FLOAT_EQ(tSpheres[10].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[11].mCenter.x, 7.75);
    EXPECT_FLOAT_EQ(tSpheres[11].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[11].mCenter.z, 24.6);
    EXPECT_FLOAT_EQ(tSpheres[11].mRadius, 1.5);
}

TEST_F(PlatoTestKrino, test_generate_spheres_success_with_overlap)
{
    SpherePatternData tData;
    tData.mBoundingBoxMinXYZ = {1.0, -2.0, 3.0};
    tData.mBoundingBoxMaxXYZ = {10.0, 20.0, 30.0};
    tData.mNumSpheres = {3, 1, 4};
    tData.mSphereRadius = 1.5;
    tData.mSpheresCanOverlapBoundingBox = true;
    const std::vector<Sphere> tSpheres = generate_spheres(tData);

    EXPECT_FLOAT_EQ(tSpheres[0].mCenter.x, 1);
    EXPECT_FLOAT_EQ(tSpheres[0].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[0].mCenter.z, 3);
    EXPECT_FLOAT_EQ(tSpheres[0].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[1].mCenter.x, 1);
    EXPECT_FLOAT_EQ(tSpheres[1].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[1].mCenter.z, 12);
    EXPECT_FLOAT_EQ(tSpheres[1].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[2].mCenter.x, 1);
    EXPECT_FLOAT_EQ(tSpheres[2].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[2].mCenter.z, 21);
    EXPECT_FLOAT_EQ(tSpheres[2].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[3].mCenter.x, 1);
    EXPECT_FLOAT_EQ(tSpheres[3].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[3].mCenter.z, 30);
    EXPECT_FLOAT_EQ(tSpheres[3].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[4].mCenter.x, 5.5);
    EXPECT_FLOAT_EQ(tSpheres[4].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[4].mCenter.z, 3);
    EXPECT_FLOAT_EQ(tSpheres[4].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[5].mCenter.x, 5.5);
    EXPECT_FLOAT_EQ(tSpheres[5].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[5].mCenter.z, 12);
    EXPECT_FLOAT_EQ(tSpheres[5].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[6].mCenter.x, 5.5);
    EXPECT_FLOAT_EQ(tSpheres[6].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[6].mCenter.z, 21);
    EXPECT_FLOAT_EQ(tSpheres[6].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[7].mCenter.x, 5.5);
    EXPECT_FLOAT_EQ(tSpheres[7].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[7].mCenter.z, 30);
    EXPECT_FLOAT_EQ(tSpheres[7].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[8].mCenter.x, 10);
    EXPECT_FLOAT_EQ(tSpheres[8].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[8].mCenter.z, 3);
    EXPECT_FLOAT_EQ(tSpheres[8].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[9].mCenter.x, 10);
    EXPECT_FLOAT_EQ(tSpheres[9].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[9].mCenter.z, 12);
    EXPECT_FLOAT_EQ(tSpheres[9].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[10].mCenter.x, 10);
    EXPECT_FLOAT_EQ(tSpheres[10].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[10].mCenter.z, 21);
    EXPECT_FLOAT_EQ(tSpheres[10].mRadius, 1.5);

    EXPECT_FLOAT_EQ(tSpheres[11].mCenter.x, 10);
    EXPECT_FLOAT_EQ(tSpheres[11].mCenter.y, 9);
    EXPECT_FLOAT_EQ(tSpheres[11].mCenter.z, 30);
    EXPECT_FLOAT_EQ(tSpheres[11].mRadius, 1.5);
}

TEST_F(PlatoTestKrino, calculate_sphere_starts_and_spacing_succeed_with_bbox_overlap)
{
    SpherePatternData tData;
    tData.mBoundingBoxMinXYZ = {-3.0, -2.0, 10.0};
    tData.mBoundingBoxMaxXYZ = {-1.0, 5.0, 10.5};
    tData.mNumSpheres = {1, 5, 2};
    tData.mSphereRadius = 1.5;
    tData.mSpheresCanOverlapBoundingBox = true;
    const SphereLocatorData tLocatorData = calculate_sphere_starts_and_spacing(tData);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[0].first, -3.0);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[1].first, -3.75);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[2].first, 9.5);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[0].second, 1.0);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[1].second, 1.75);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[2].second, 0.5);
}

TEST_F(PlatoTestKrino, calculate_sphere_starts_and_spacing_succeed_without_bbox_overlap)
{
    SpherePatternData tData;
    tData.mBoundingBoxMinXYZ = {-3.0, -2.0, 10.0};
    tData.mBoundingBoxMaxXYZ = {-1.0, 5.0, 10.5};
    tData.mNumSpheres = {1, 5, 2};
    tData.mSphereRadius = 1.5;
    tData.mSpheresCanOverlapBoundingBox = false;
    const SphereLocatorData tLocatorData = calculate_sphere_starts_and_spacing(tData);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[0].first, -3.0);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[1].first, -2.0);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[2].first, 10.0);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[0].second, 1.0);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[1].second, 7.0 / 6.0);
    EXPECT_FLOAT_EQ(tLocatorData.mStartAndSpacing[2].second, 1.0 / 6.0);
}

}  // namespace plato::third_party_integration::krino::unit_tests
