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

#include "plato/geometry/extension/LevelSetTopology.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/linear_algebra/JacobianColumnEvaluator.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/Containers.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::geometry::extension::unittest
{

namespace
{
const std::string_view kLogFile = "Krino_Test.txt";

constexpr int kNumDimensions = 3;
const auto kLevelSetInput = plato::test_utilities::create_valid_level_set_topology_geometry();

constexpr unsigned int kExpectedBackgroundLevelSetSize = 59;  // Based on mesh generation command below

void create_background_mesh(const std::string& aFileName, const double aMeshSize)
{
    ASSERT_EQ(stk::parallel_machine_size(MPI_COMM_WORLD), 1);
    const auto tBoundingBox = third_party_integration::krino::BoundingBox{{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}};
    third_party_integration::krino::create_bounding_box_mesh(tBoundingBox, aMeshSize, aFileName);
}

class LevelSetTopologyFixture : virtual public ::testing::Test
{
   protected:
    void SetUp() override
    {
        static bool tFirstTime{true};
        if (tFirstTime)
        {
            third_party_integration::krino::initialize_environment_for_krino(kLogFile, MPI_COMM_WORLD);
            tFirstTime = false;
        }
    }
    void TearDown() override
    {
        std::filesystem::remove(kLevelSetInput.background_mesh_name->mToken);
        std::filesystem::remove(std::filesystem::path{kLogFile});
    }
};

/// @brief The purpose of this fixture is to provide a mesh with a non-trivial node map.
class LevelSetTopologyMeshFixture : public LevelSetTopologyFixture,
                                    public third_party_integration::stk_io::test_utilities::Tet4MeshOnDisk
{
   protected:
    void SetUp() override
    {
        LevelSetTopologyFixture::SetUp();
        Tet4MeshOnDisk::SetUp();
    }
    void TearDown() override
    {
        LevelSetTopologyFixture::TearDown();
        Tet4MeshOnDisk::TearDown();
    }
};

}  // namespace

TEST_F(LevelSetTopologyFixture, Jacobian)
{
    create_background_mesh(kLevelSetInput.background_mesh_name->mToken, 1.0);

    const auto tLevelSetTopology = LevelSetTopology{kLevelSetInput};
    const auto tInitialGuess = tLevelSetTopology.initialGuess(kLevelSetInput.background_mesh_name->mToken);
    const auto tCutMesh = tLevelSetTopology.generateMesh(tInitialGuess);
    ASSERT_TRUE(std::filesystem::exists(tCutMesh.mFileName));
    const linear_algebra::JacobianMultiplier tJacobian = tLevelSetTopology.jacobian(tInitialGuess);

    const unsigned int tDFDXSize = kNumDimensions * mesh::EntityCounts{mesh::Mesh{tCutMesh.mFileName}}.numberOfNodes();
    const auto tDFDX = linear_algebra::DynamicVector(tDFDXSize, 1.0);

    const auto tRes = tDFDX * tJacobian;

    const auto tGold = std::vector<double>{0.5, 0.166667, 0.166667, -0.166667,   0.166667, -0.166667, -0.166667, -0.5,
                                           0.5, 0.5,      0.5,      1.11022e-16, -0.5,     -0.5,      -0.5};
    constexpr double tTol = 1e-6;
    test_utilities::expect_container_entries_near(tRes.stdVector(), tGold, tTol,
                                                  TEST_CONTEXT("LevelSet Jacobian entries"));
}

TEST_F(LevelSetTopologyMeshFixture, JacobianRegression)
{
    auto tInput = kLevelSetInput;
    tInput.background_mesh_name = input_parser::FileName{Tet4MeshOnDisk::mMeshFilePath};
    tInput.sphere_pattern_bbox_min_x = 0.0;
    tInput.sphere_pattern_bbox_max_x = 0.0;
    tInput.sphere_pattern_bbox_min_y = 0.0;
    tInput.sphere_pattern_bbox_max_y = 0.0;
    tInput.sphere_pattern_bbox_min_z = 0.0;
    tInput.sphere_pattern_bbox_max_z = 0.0;
    tInput.sphere_pattern_radius = 1.0;
    const auto tLevelSetTopology = LevelSetTopology{tInput};

    const auto tInitialGuess = tLevelSetTopology.initialGuess(tInput.background_mesh_name->mToken);
    const auto tJacobian = tLevelSetTopology.jacobian(tInitialGuess);

    const auto tCutMesh = tLevelSetTopology.generateMesh(tInitialGuess);
    ASSERT_TRUE(std::filesystem::exists(tCutMesh.mFileName));
    const auto tNumberOfNodes = mesh::EntityCounts{mesh::Mesh{tCutMesh.mFileName}}.numberOfNodes();
    const auto tDFDX = linear_algebra::DynamicVector(static_cast<std::size_t>(kNumDimensions * tNumberOfNodes), 1.0);

    const auto tResult = tDFDX * tJacobian;

    ASSERT_EQ(tResult.size(), Tet4MeshOnDisk::mExpectedNumberOfNodes);

    const auto tSum = std::accumulate(tResult.stdVector().begin(), tResult.stdVector().end(), 0.0);
    // This is just a regression test, but this mesh has a non-trivial node map, so the test will fail if the node map
    // is not used.
    constexpr auto tExpectedJacobianSum = -43.3185837663019484;  // Regression value
    EXPECT_DOUBLE_EQ(tExpectedJacobianSum, tSum);
}

TEST_F(LevelSetTopologyFixture, JacobianTransposeRegression)
{
    create_background_mesh(kLevelSetInput.background_mesh_name->mToken, 1.0);

    const auto tLevelSetTopology = LevelSetTopology{kLevelSetInput};
    const auto tInitialGuess = tLevelSetTopology.initialGuess(kLevelSetInput.background_mesh_name->mToken);
    const auto tAdjointJacobian = tLevelSetTopology.adjointJacobian(tInitialGuess);

    const unsigned int tDFDXSize =
        mesh::EntityCounts{mesh::Mesh{kLevelSetInput.background_mesh_name->mToken}}.numberOfNodes();
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
                                                  TEST_CONTEXT("LevelSet adjoint Jacobian entries"));
}

TEST_F(LevelSetTopologyFixture, GenerateMeshRegression)
{
    create_background_mesh(kLevelSetInput.background_mesh_name->mToken, 0.5);
    const auto tLevelSetTopology = LevelSetTopology{kLevelSetInput};
    const auto tInitialGuess = tLevelSetTopology.initialGuess(kLevelSetInput.background_mesh_name->mToken);
    const auto tAnalysisMesh = tLevelSetTopology.generateMesh(tInitialGuess);
    ASSERT_TRUE(std::filesystem::exists(tAnalysisMesh.mFileName));

    // Regression, just checks number of nodes
    const auto tNumberOfNodes = mesh::EntityCounts{mesh::Mesh{tAnalysisMesh.mFileName}}.numberOfNodes();
    constexpr auto tExpectedNumberOfNodes = 72U;
    EXPECT_EQ(tNumberOfNodes, tExpectedNumberOfNodes);
}

TEST_F(LevelSetTopologyFixture, InitialGuessRegression)
{
    create_background_mesh(kLevelSetInput.background_mesh_name->mToken, 0.5);

    const LevelSetTopology tLevelSetTopology(kLevelSetInput);
    const linear_algebra::DynamicVector<double> tInitialGuess =
        tLevelSetTopology.initialGuess(kLevelSetInput.background_mesh_name->mToken);

    const auto tExpectedInitialGuess = std::vector<double>{
        0.6160254037844386, 0.4571067811865476, 0.6160254037844386, 0.4571067811865476,  0.2500000000000000,
        0.4571067811865476, 0.6160254037844386, 0.4571067811865476, 0.6160254037844386,  0.4571067811865476,
        0.2500000000000000, 0.4571067811865476, 0.2500000000000000, -0.2500000000000000, 0.2500000000000000,
        0.4571067811865476, 0.2500000000000000, 0.4571067811865476, 0.6160254037844386,  0.4571067811865476,
        0.6160254037844386, 0.4571067811865476, 0.2500000000000000, 0.4571067811865476,  0.6160254037844386,
        0.4571067811865476, 0.6160254037844386, 0.3623724356957945, 0.3623724356957945,  0.3623724356957945,
        0.3623724356957945, 0.3623724356957945, 0.3623724356957945, 0.3623724356957945,  0.1830127018922193,
        0.1830127018922193, 0.3623724356957945, 0.3623724356957945, 0.1830127018922193,  0.1830127018922193,
        0.3623724356957945, 0.3623724356957945, 0.3623724356957945, 0.3623724356957945,  0.3623724356957945,
        0.3623724356957945, 0.1830127018922193, 0.1830127018922193, 0.3623724356957945,  0.3623724356957945,
        0.1830127018922193, 0.1830127018922193, 0.3623724356957945, 0.3623724356957945,  0.3623724356957945,
        0.3623724356957945, 0.3623724356957945, 0.3623724356957945, 0.3623724356957945};
    ASSERT_EQ(tInitialGuess.size(), kExpectedBackgroundLevelSetSize);
    constexpr auto tTolerance = 1e-14;
    test_utilities::expect_container_entries_near(tExpectedInitialGuess, tInitialGuess.stdVector(), tTolerance,
                                                  TEST_CONTEXT("Initial guess"));
}

TEST_F(LevelSetTopologyFixture, Bounds)
{
    create_background_mesh(kLevelSetInput.background_mesh_name->mToken, 0.5);
    const LevelSetTopology tLevelSetTopology(kLevelSetInput);
    const auto [tLowerBounds, tUpperBounds] = tLevelSetTopology.bounds(kLevelSetInput.background_mesh_name->mToken);

    ASSERT_EQ(tLowerBounds.size(), kExpectedBackgroundLevelSetSize);
    ASSERT_EQ(tUpperBounds.size(), kExpectedBackgroundLevelSetSize);

    EXPECT_TRUE(
        std::all_of(tLowerBounds.cbegin(), tLowerBounds.cend(), [](const double aVal) { return aVal == -1.0; }));
    EXPECT_TRUE(std::all_of(tUpperBounds.cbegin(), tUpperBounds.cend(), [](const double aVal) { return aVal == 1.0; }));
}

}  // namespace plato::geometry::extension::unittest
