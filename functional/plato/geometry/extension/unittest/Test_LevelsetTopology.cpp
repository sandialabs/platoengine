#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <numeric>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Comm.hpp>
#include <stk_util/environment/Env.hpp>
#include <stk_util/parallel/Parallel.hpp>
#include <string_view>
#include <vector>

#include "plato/geometry/extension/LevelsetTopology.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/linear_algebra/JacobianColumnEvaluator.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/Containers.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"

namespace plato::geometry::extension::unittest
{

namespace
{
constexpr int kNumDimensions = 3;
const auto kLevelsetInput = plato::test_utilities::create_valid_levelset_topology_geometry();

constexpr unsigned int kExpectedBackgroundLevelsetSize = 59;  // Based on mesh generation command below

void create_background_mesh(const std::string& aFileName, const double aMeshSize)
{
    ASSERT_EQ(stk::parallel_machine_size(MPI_COMM_WORLD), 1);
    third_party_integration::krino::create_bounding_box_mesh({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}, aMeshSize, aFileName);
}

class LevelsetTopologyFixture : public ::testing::Test
{
   protected:
    void SetUp() override
    {
        static bool tFirstTime{true};
        if (tFirstTime)
        {
            third_party_integration::krino::initialize_environment_for_krino(MPI_COMM_WORLD);
            tFirstTime = false;
        }
    }
    void TearDown() override
    {
        std::filesystem::remove(kLevelsetInput.background_mesh_name->mToken);
        std::filesystem::remove(kLevelsetInput.cut_mesh_name->mToken);
    }
};

}  // namespace

TEST_F(LevelsetTopologyFixture, Jacobian)
{
    create_background_mesh(kLevelsetInput.background_mesh_name->mToken, 1.0);

    const auto tLevelsetTopology = LevelsetTopology{kLevelsetInput};
    const auto tInitialGuess = tLevelsetTopology.initialGuess(kLevelsetInput.background_mesh_name->mToken);
    const linear_algebra::JacobianMultiplier tJacobian = tLevelsetTopology.jacobian(tInitialGuess);

    const unsigned int tDFDXSize =
        kNumDimensions * mesh::EntityCounts{mesh::Mesh{kLevelsetInput.cut_mesh_name->mToken}}.numberOfNodes();
    const auto tDFDX = linear_algebra::DynamicVector(tDFDXSize, 1.0);

    const auto tRes = tDFDX * tJacobian;

    const auto tGold = std::vector<double>{0.5, 0.166667, 0.166667, -0.166667,   0.166667, -0.166667, -0.166667, -0.5,
                                           0.5, 0.5,      0.5,      1.11022e-16, -0.5,     -0.5,      -0.5};
    constexpr double tTol = 1e-6;
    test_utilities::expect_container_entries_near(tRes.stdVector(), tGold, tTol,
                                                  TEST_CONTEXT("Levelset Jacobian entries"));
}

TEST_F(LevelsetTopologyFixture, JacobianTranspose)
{
    create_background_mesh(kLevelsetInput.background_mesh_name->mToken, 1.0);

    const auto tLevelsetTopology = LevelsetTopology{kLevelsetInput};
    const auto tInitialGuess = tLevelsetTopology.initialGuess(kLevelsetInput.background_mesh_name->mToken);
    const auto tAdjointJacobian = tLevelsetTopology.adjointJacobian(tInitialGuess);

    const unsigned int tDFDXSize =
        mesh::EntityCounts{mesh::Mesh{kLevelsetInput.background_mesh_name->mToken}}.numberOfNodes();
    const auto tDFDX = linear_algebra::DynamicVector(tDFDXSize, 1.0);

    const auto tResult = tDFDX * tAdjointJacobian;

    // Regression result computed by outputting the entire Jacobian matrix using the `jacobian` function, and performing
    // the transpose matrix vector multiplication in Matlab.
    // clang-format off
    const auto tExpected = std::vector{
        0.0,  0.0,  0.0,  0.0,  0.0, 0.0,  0.0,  0.0,  0.0,  0.0, 0.0,  0.0,  
        0.0,  0.0,  0.0, 0.0,  0.0,  0.0,  0.0, 0.0, 0.0,  0.0,  0.0,  0.0,  0.0, 
        0.0,  0.0,  0.0,  0.0,  0.0, 0.0,  0.0,  0.0,  0.0,  0.0, 0.0,  0.0,  0.0,  0.0,  0.0, 0.0,  0.0,  1.0,  0.0,  0.0,
        0.5773502691896258,  0.5773502691896258,  0.5773502691896258,  0.5773502691896258,  0.5773502691896258,
        -0.5773502691896258, 0.5773502691896258,  -0.5773502691896258, -0.5773502691896258, 0.5773502691896258,
        -0.5773502691896258, 0.5773502691896258,  -1.0, 0.0,  0.0,
        -0.5773502691896258, 0.5773502691896258,  0.5773502691896258,  -0.5773502691896258, -0.5773502691896258,
        0.5773502691896258,  -0.5773502691896258, -0.5773502691896258, -0.5773502691896258, -0.5773502691896258,
        0.5773502691896258,  -0.5773502691896258, 0.0,  1.0,  0.0, 0.0,  -1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, -1.0};
    // clang-format on
    constexpr double tTol = 1e-14;
    test_utilities::expect_container_entries_near(tResult.stdVector(), tExpected, tTol,
                                                  TEST_CONTEXT("Levelset adjoint Jacobian entries"));
}

TEST_F(LevelsetTopologyFixture, GenerateMesh)
{
    create_background_mesh(kLevelsetInput.background_mesh_name->mToken, 0.5);
    const LevelsetTopology tLevelsetTopology(kLevelsetInput);
    // Calling initialGuess() will create an initial cut mesh with
    // the default sphere pattern defined in kLevelsetInput
    const linear_algebra::DynamicVector<double> tInitialGuess =
        tLevelsetTopology.initialGuess(kLevelsetInput.background_mesh_name->mToken);
    const std::vector<third_party_integration::common::Coordinate> tNodalCoords1 =
        mesh::EntityRetrieval{mesh::Mesh{kLevelsetInput.cut_mesh_name->mToken}}.nodalCoordinates();
    // Call generateMesh to write out the cut mesh based on the passed in params (levelset values
    // from the initial mesh). We should get the exact same mesh.
    const auto tMeshDesignVariables = tLevelsetTopology.generateMesh(tInitialGuess);
    ASSERT_TRUE(std::filesystem::exists(kLevelsetInput.cut_mesh_name->mToken));
    const std::vector<third_party_integration::common::Coordinate> tNodalCoords2 =
        mesh::EntityRetrieval{mesh::Mesh{kLevelsetInput.cut_mesh_name->mToken}}.nodalCoordinates();

    ASSERT_EQ(tNodalCoords1.size(), tNodalCoords2.size());
    for (size_t i = 0; i < tNodalCoords1.size(); ++i)
    {
        EXPECT_DOUBLE_EQ(tNodalCoords1[i].x, tNodalCoords2[i].x);
        EXPECT_DOUBLE_EQ(tNodalCoords1[i].y, tNodalCoords2[i].y);
        EXPECT_DOUBLE_EQ(tNodalCoords1[i].z, tNodalCoords2[i].z);
    }
}

TEST_F(LevelsetTopologyFixture, InitialGuess)
{
    create_background_mesh(kLevelsetInput.background_mesh_name->mToken, 0.5);

    const LevelsetTopology tLevelsetTopology(kLevelsetInput);
    const linear_algebra::DynamicVector<double> tInitialGuess =
        tLevelsetTopology.initialGuess(kLevelsetInput.background_mesh_name->mToken);

    const std::vector<double> tGoldValues{
        0.616025, 0.457107, 0.616025, 0.457107, 0.25,     0.457107, 0.616025, 0.457107, 0.616025, 0.457107,
        0.25,     0.457107, 0.25,     -0.25,    0.25,     0.457107, 0.25,     0.457107, 0.616025, 0.457107,
        0.616025, 0.457107, 0.25,     0.457107, 0.616025, 0.457107, 0.616025, 0.362372, 0.362372, 0.362372,
        0.362372, 0.362372, 0.362372, 0.362372, 0.183013, 0.183013, 0.362372, 0.362372, 0.183013, 0.183013,
        0.362372, 0.362372, 0.362372, 0.362372, 0.362372, 0.362372, 0.183013, 0.183013, 0.362372, 0.362372,
        0.183013, 0.183013, 0.362372, 0.362372, 0.362372, 0.362372, 0.362372, 0.362372, 0.362372};
    ASSERT_EQ(tInitialGuess.size(), kExpectedBackgroundLevelsetSize);
    for (size_t i = 0; i < tInitialGuess.size(); ++i)
    {
        EXPECT_DOUBLE_EQ(tInitialGuess[i], tInitialGuess.stdVector()[i]);
    }
}

TEST_F(LevelsetTopologyFixture, Bounds)
{
    create_background_mesh(kLevelsetInput.background_mesh_name->mToken, 0.5);
    const LevelsetTopology tLevelsetTopology(kLevelsetInput);
    const auto [tLowerBounds, tUpperBounds] = tLevelsetTopology.bounds(kLevelsetInput.background_mesh_name->mToken);

    ASSERT_EQ(tLowerBounds.size(), kExpectedBackgroundLevelsetSize);
    ASSERT_EQ(tUpperBounds.size(), kExpectedBackgroundLevelsetSize);

    EXPECT_TRUE(
        std::all_of(tLowerBounds.cbegin(), tLowerBounds.cend(), [](const double aVal) { return aVal == -1.0; }));
    EXPECT_TRUE(std::all_of(tUpperBounds.cbegin(), tUpperBounds.cend(), [](const double aVal) { return aVal == 1.0; }));
}

}  // namespace plato::geometry::extension::unittest
