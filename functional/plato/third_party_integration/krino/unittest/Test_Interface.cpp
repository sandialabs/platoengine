
#include <gtest/gtest.h>  // for AssertHelper, TEST, etc

#include <cstdio>

#include "plato/third_party_integration/krino/KrinoWrapper.hpp"

namespace plato::third_party_integration::krino::unittest
{

namespace
{
constexpr int kNumDimensions = 3;
}
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
    const auto tDFDX = std::unordered_map<unsigned int, stk::math::Vector3d>{
        {1, {2.0, 0.5, 0.25}}, {2, {0.25, -0.125, -1.0}}, {3, {0.5, -0.0625, -0.125}}};

    const auto tDXDP = std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP>{
        {3, InterfaceNodeDXDP{{7, 12, 19}, {{0.5, 0.5, -0.5}, {-0.25, 0.25, 0.25}, {-0.125, 0.125, -0.125}}}},
        {1, InterfaceNodeDXDP{{34, 22, 2}, {{-0.125, -0.125, 0.125}, {0.25, 0.25, -0.25}, {0.75, -0.75, -0.75}}}},
        {2, InterfaceNodeDXDP{{19, 10}, {{1.5, -1.5, 1.5}, {0.25, -0.25, -0.25}}}},
    };

    const auto tBackgroundNodemap = std::vector<unsigned int>{7, 12, 19, 34, 22, 2, 10};
    const auto tDFDLS = calculate_dfdls(tDFDX, tDXDP, tBackgroundNodemap);

    const auto tExpected = std::unordered_map<KrinoGlobalNodeID, double>{
        {2, 0.9375}, {7, 0.28125}, {10, 0.34375}, {12, -0.171875}, {19, -0.9921875}, {22, 0.5625}, {34, -0.28125}};

    EXPECT_EQ(tDFDLS, tExpected);
}

TEST_F(PlatoTestKrino, CalculateAdjointDFDLS)
{
    const auto tDXDP = std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP>{
        {3, InterfaceNodeDXDP{{7, 12, 19}, {{0.5, 0.5, -0.5}, {-0.25, 0.25, 0.25}, {-0.125, 0.125, -0.125}}}},
        {1, InterfaceNodeDXDP{{34, 22, 2}, {{-0.125, -0.125, 0.125}, {0.25, 0.25, -0.25}, {0.75, -0.75, -0.75}}}},
        {2, InterfaceNodeDXDP{{19, 10}, {{1.5, -1.5, 1.5}, {0.25, -0.25, -0.25}}}},
    };
    const auto tBackgroundLevelSetValues = std::unordered_map<unsigned int, double>{
        {7, 2.0}, {12, 4.0}, {19, 5.0}, {34, 7.0}, {22, 6.0}, {2, 1.0}, {10, 3.0}};

    const auto tDFDLS = calculate_adjoint_dfdls(tBackgroundLevelSetValues, tDXDP);

    auto tExpected =
        std::unordered_map<unsigned int, stk::math::Vector3d>{{1, stk::math::Vector3d{1.375, -0.125, -1.375}},
                                                              {2, stk::math::Vector3d{8.25, -8.25, 6.75}},
                                                              {3, stk::math::Vector3d{-0.625, 2.625, -0.625}}};

    EXPECT_EQ(tExpected, tDFDLS);
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
        for (int i = 0; i < kNumDimensions; i++)
        {
            EXPECT_FLOAT_EQ(tPredictedCoordValue.second[i], tNewCoordValues.at(tPredictedCoordValue.first)[i]);
        }
    }
    remove(tFilename.c_str());
}

}  // namespace plato::third_party_integration::krino::unittest
