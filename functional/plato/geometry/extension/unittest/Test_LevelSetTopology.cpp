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

#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/filter/extension/KernelFilter.hpp"
#include "plato/filter/library/FilterFactory.hpp"
#include "plato/geometry/extension/LevelSetTopology.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/linear_algebra/JacobianColumnEvaluator.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/Containers.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::geometry::extension::unittest
{

namespace
{
constexpr auto kExpectedJacobianSum = -43.3185837663019484;
constexpr auto kExpectedAdjointJacobianSum = -72.780945645414576;
constexpr auto kNumDimensions = std::size_t{3};
const auto kLevelSetInput = plato::test_utilities::create_valid_level_set_topology_geometry();
constexpr unsigned int kExpectedBackgroundLevelSetSize = 59;  // Based on mesh generation command below
const auto kKrinoLogFileName = std::filesystem::path{"Krino_Output.txt"};

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
            third_party_integration::krino::initialize_environment_for_krino(kKrinoLogFileName, MPI_COMM_WORLD);
            tFirstTime = false;
        }
    }
    void TearDown() override
    {
        std::filesystem::remove(kLevelSetInput.background_mesh_name->mToken);
        std::filesystem::remove(kKrinoLogFileName);
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

    auto singleSphereInput() -> input_parser::level_set_topology
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
        return tInput;
    }
};

auto make_kernel_filter_test_function(const std::filesystem::path& aMeshFilePath) -> filter::library::FilterFunction
{
    constexpr auto tFilterRadius = filter::extension::FilterRadius{1.0};
    const auto tKernelFilter = std::make_shared<filter::extension::KernelFilter>(
        mesh::Mesh{aMeshFilePath}, tFilterRadius, input_parser::KernelFilterCenteringTypes::kNodeCentered,
        boost::mpi::communicator{});

    return filter::library::make_filter_function(tKernelFilter);
}

auto ones_vector_times_jacobian(const std::size_t aNumberOfNodes, const linear_algebra::JacobianMultiplier& aJacobian)
    -> double
{
    const auto tOnesVector = linear_algebra::DynamicVector(aNumberOfNodes, 1.0);
    const auto tResult = tOnesVector * aJacobian;
    return std::accumulate(tResult.stdVector().begin(), tResult.stdVector().end(), 0.0);
}

}  // namespace

TEST_F(LevelSetTopologyFixture, JacobianRegression)
{
    create_background_mesh(kLevelSetInput.background_mesh_name->mToken, 1.0);

    const auto tLevelSetTopology = LevelSetTopology{kLevelSetInput};
    const auto tInitialGuess = tLevelSetTopology.initialGuess(kLevelSetInput.background_mesh_name->mToken);
    const auto tCutMesh = tLevelSetTopology.generateMesh(tInitialGuess);
    ASSERT_TRUE(std::filesystem::exists(tCutMesh.mFileName));
    const linear_algebra::JacobianMultiplier tJacobian = tLevelSetTopology.jacobian(tInitialGuess);

    const unsigned int tRowVectorSize =
        kNumDimensions * mesh::EntityCounts{mesh::Mesh{tCutMesh.mFileName}}.numberOfNodes();
    const auto tRowVector = linear_algebra::DynamicVector(tRowVectorSize, 1.0);

    const auto tRes = tRowVector * tJacobian;

    const auto tGold = std::vector<double>{0.5, 0.166667, 0.166667, -0.166667,   0.166667, -0.166667, -0.166667, -0.5,
                                           0.5, 0.5,      0.5,      1.11022e-16, -0.5,     -0.5,      -0.5};
    constexpr double tTol = 1e-6;
    test_utilities::expect_container_entries_near(tRes.stdVector(), tGold, tTol,
                                                  TEST_CONTEXT("LevelSet Jacobian entries"));
}

TEST_F(LevelSetTopologyMeshFixture, JacobianRegression)
{
    const auto tInput = singleSphereInput();

    const auto tOnesVectorJacobianProductSum = [](const library::GeometryFunction& aLevelSetTopologyFunction,
                                                  const linear_algebra::DynamicVector<double>& aArgument)
    {
        const auto tJacobian = aLevelSetTopologyFunction.evaluate<core::evaluation::kFirstDerivative>(aArgument);
        const auto tCutMesh = aLevelSetTopologyFunction.evaluate<core::evaluation::kFunction>(aArgument);
        const auto tNumberOfNodes = mesh::EntityCounts{mesh::Mesh{tCutMesh.mFileName}}.numberOfNodes();
        return ones_vector_times_jacobian(kNumDimensions * tNumberOfNodes, tJacobian);
    };
    const auto tLevelSetTopology = std::make_shared<LevelSetTopology>(tInput);
    const auto tInitialGuess = tLevelSetTopology->initialGuess(tInput.background_mesh_name->mToken);
    // No filter
    {
        const auto tLevelSetFunction =
            make_level_set_geometry(tLevelSetTopology, filter::extension::make_identity_filter_function());
        const auto tComputedSum = tOnesVectorJacobianProductSum(tLevelSetFunction, tInitialGuess);
        // This is just a regression test, but this mesh has a non-trivial node map, so the test will fail if
        // the node map is not used.
        EXPECT_DOUBLE_EQ(kExpectedJacobianSum, tComputedSum);
    }
    // Kernel filter
    {
        const auto tLevelSetFunction =
            make_level_set_geometry(tLevelSetTopology, make_kernel_filter_test_function(Tet4MeshOnDisk::mMeshFilePath));
        const auto tComputedSum = tOnesVectorJacobianProductSum(tLevelSetFunction, tInitialGuess);
        // This is just a regression test, but it is expected to be different from the Jacobian computed with the
        // identity filter
        EXPECT_DOUBLE_EQ(kExpectedAdjointJacobianSum, tComputedSum);
    }
}

TEST_F(LevelSetTopologyFixture, JacobianTransposeRegression)
{
    create_background_mesh(kLevelSetInput.background_mesh_name->mToken, 1.0);

    const auto tLevelSetTopology = LevelSetTopology{kLevelSetInput};
    const auto tInitialGuess = tLevelSetTopology.initialGuess(kLevelSetInput.background_mesh_name->mToken);
    const auto tAdjointJacobian = tLevelSetTopology.adjointJacobian(tInitialGuess);

    const unsigned int tRowVectorSize =
        mesh::EntityCounts{mesh::Mesh{kLevelSetInput.background_mesh_name->mToken}}.numberOfNodes();
    const auto tRowVector = linear_algebra::DynamicVector(tRowVectorSize, 1.0);

    const auto tResult = tRowVector * tAdjointJacobian;

    // Regression result computed by outputting the entire Jacobian matrix using the `jacobian` function, and
    // performing the transpose matrix vector multiplication in Matlab.
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

TEST_F(LevelSetTopologyMeshFixture, JacobianTranspose)
{
    const auto tInput = singleSphereInput();

    const auto tOnesVectorJacobianProductSum = [](const library::GeometryFunction& aLevelSetTopology,
                                                  const linear_algebra::DynamicVector<double>& aDesignVariables)
    {
        const auto tJacobian =
            aLevelSetTopology.evaluate<core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>(
                aDesignVariables);
        const auto tNumberOfNodes = aDesignVariables.size();
        return ones_vector_times_jacobian(tNumberOfNodes, tJacobian.mValue);
    };
    const auto tLevelSetTopology = std::make_shared<LevelSetTopology>(tInput);
    const auto tInitialGuess = tLevelSetTopology->initialGuess(tInput.background_mesh_name->mToken);
    constexpr auto tTolerance = 1e-13;
    // No filter
    {
        const auto tLevelSetFunction =
            make_level_set_geometry(tLevelSetTopology, filter::extension::make_identity_filter_function());
        const auto tComputedSum = tOnesVectorJacobianProductSum(tLevelSetFunction, tInitialGuess);
        EXPECT_NEAR(kExpectedJacobianSum, tComputedSum, tTolerance);
    }
    // Kernel filter
    {
        const auto tLevelSetFunction = make_level_set_geometry(
            tLevelSetTopology, make_kernel_filter_test_function(tInput.background_mesh_name->mToken));
        const auto tComputedSum = tOnesVectorJacobianProductSum(tLevelSetFunction, tInitialGuess);
        EXPECT_NEAR(kExpectedAdjointJacobianSum, tComputedSum, tTolerance);
    }
}

TEST_F(LevelSetTopologyFixture, GenerateMesh)
{
    create_background_mesh(kLevelSetInput.background_mesh_name->mToken, 0.5);
    auto tLevelSetTopology = std::make_unique<LevelSetTopology>(kLevelSetInput);
    const auto tInitialGuess = tLevelSetTopology->initialGuess(kLevelSetInput.background_mesh_name->mToken);

    const auto tMeshRegressionChecks = [](const std::size_t aExpectedNumberOfNodes,
                                          const std::filesystem::path& aFilePath,
                                          const test_utilities::TestContext& aTestContext)
    {
        EXPECT_TRUE(std::filesystem::exists(aFilePath)) << aTestContext;
        const auto tNumberOfNodes = mesh::EntityCounts{mesh::Mesh{aFilePath}}.numberOfNodes();
        EXPECT_EQ(tNumberOfNodes, aExpectedNumberOfNodes) << aTestContext;
    };
    {
        const auto tAnalysisMesh = tLevelSetTopology->generateMesh(tInitialGuess);
        constexpr auto tExpectedNumberOfNodes = 72U;
        tMeshRegressionChecks(tExpectedNumberOfNodes, tAnalysisMesh.mFileName,
                              TEST_CONTEXT("No filter, direct construction"));
    }
    {
        const auto tLevelSetFunction =
            make_level_set_geometry(std::move(tLevelSetTopology),
                                    make_kernel_filter_test_function(kLevelSetInput.background_mesh_name->mToken));

        const auto tAnalysisMesh = tLevelSetFunction.evaluate<core::evaluation::kFunction>(tInitialGuess);
        constexpr auto tExpectedNumberOfNodes = 59U;
        tMeshRegressionChecks(tExpectedNumberOfNodes, tAnalysisMesh.mFileName,
                              TEST_CONTEXT("Kernel filter, using function wrapper"));
    }
}

TEST_F(LevelSetTopologyFixture, InitialGuessRegression)
{
    create_background_mesh(kLevelSetInput.background_mesh_name->mToken, 0.5);

    const auto tLevelSetTopology = LevelSetTopology{kLevelSetInput};
    const auto tInitialGuess = tLevelSetTopology.initialGuess(kLevelSetInput.background_mesh_name->mToken);

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
    const auto tLevelSetTopology = LevelSetTopology{kLevelSetInput};
    const auto [tLowerBounds, tUpperBounds] = tLevelSetTopology.bounds(kLevelSetInput.background_mesh_name->mToken);

    ASSERT_EQ(tLowerBounds.size(), kExpectedBackgroundLevelSetSize);
    ASSERT_EQ(tUpperBounds.size(), kExpectedBackgroundLevelSetSize);

    EXPECT_TRUE(
        std::all_of(tLowerBounds.cbegin(), tLowerBounds.cend(), [](const double aVal) { return aVal == -1.0; }));
    EXPECT_TRUE(std::all_of(tUpperBounds.cbegin(), tUpperBounds.cend(), [](const double aVal) { return aVal == 1.0; }));
}

}  // namespace plato::geometry::extension::unittest
