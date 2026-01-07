#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <numeric>
#include <vector>

#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/filter/extension/KernelFilter.hpp"
#include "plato/filter/library/FilterFactory.hpp"
#include "plato/filter/test_utilities/FilterFunction.hpp"
#include "plato/geometry/extension/LevelSetTopology.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/unittest/LevelSetTopologyTestFixtures.hpp"
#include "plato/linear_algebra/JacobianColumnEvaluator.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/Containers.hpp"
#include "plato/test_utilities/GradientChecker.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshIOHelpers.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshWithFieldWriter.hpp"

namespace plato::geometry::extension::unittest
{
using NodalDensityMesh = third_party_integration::stk_io::test_utilities::MeshWithNodalDensities;

namespace
{
const auto kLevelSetInput = geometry::extension::test_utilities::create_valid_level_set_topology_geometry_input();

[[nodiscard]] auto create_background_mesh() -> third_party_integration::stk_io::CommandGenerator
{
    namespace tskio = third_party_integration::stk_io;
    const tskio::CommandBounds tLowerBounds{0, 0, 0};
    const tskio::CommandBounds tUpperBounds{1, 1, 1};
    const tskio::CommandNumberOfElements tElements{2, 2, 2};
    return tskio::CommandGenerator{tElements, tLowerBounds, tUpperBounds, tskio::CommandElementType::Tet};
}

auto make_kernel_filter_test_function(const filter::extension::FilterRadius aRadius,
                                      const std::filesystem::path& aMeshFilePath,
                                      const std::set<std::string>& aFixedBlocks = {}) -> filter::library::FilterFunction
{
    const auto tKernelFilter = std::make_shared<filter::extension::KernelFilter>(
        mesh::Mesh{aMeshFilePath, aFixedBlocks}, aRadius, input_parser::KernelFilterCenteringTypes::kNodeCentered,
        boost::mpi::communicator{});

    return filter::test_utilities::make_filter_function(tKernelFilter);
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
    third_party_integration::stk_io::write_mesh(kLevelSetInput.mesh_name->mToken, create_background_mesh());

    const auto tLevelSetTopology = LevelSetTopology{kLevelSetInput};
    const auto tInitialGuess = tLevelSetTopology.initialGuess();
    const auto tCutMesh = tLevelSetTopology.generateMesh(tInitialGuess);
    ASSERT_TRUE(std::filesystem::exists(tCutMesh.mFileName));
    const linear_algebra::JacobianMultiplier tJacobian = tLevelSetTopology.jacobian(tInitialGuess);

    const unsigned int tRowVectorSize =
        mNumDimensions * mesh::EntityCounts{mesh::Mesh{tCutMesh.mFileName}}.numberOfNodes();
    const auto tRowVector = linear_algebra::DynamicVector(tRowVectorSize, 1.0);

    const auto tResult = tRowVector * tJacobian;

    const auto tGold = std::vector<double>{0.500, 0.500, 0.000, 0.500, 0.500,  0.000,  0.000, 0.000,  0.000,
                                           0.500, 0.500, 0.000, 0.500, -0.000, -0.500, 0.000, -0.500, -0.500,
                                           0.000, 0.000, 0.000, 0.000, -0.500, -0.500, 0.000, -0.500, -0.500};
    constexpr double tTolerance = 1e-6;
    plato::test_utilities::expect_container_entries_near(tResult.stdVector(), tGold, tTolerance,
                                                         TEST_CONTEXT("LevelSet Jacobian entries"));
}

TEST_F(LevelSetTopologyMeshFixture, JacobianRegression)
{
    auto tInput = singleSphereInput();
    tInput.max_edge_length_percentage_for_snapping = 0.0;  // turn snapping off for regression

    const auto tOnesVectorJacobianProductSum = [this](const library::GeometryFunction& aLevelSetTopologyFunction,
                                                      const linear_algebra::DynamicVector<double>& aArgument)
    {
        const auto tJacobian = aLevelSetTopologyFunction.evaluate<core::evaluation::kFirstDerivative>(aArgument);
        const auto tCutMesh = aLevelSetTopologyFunction.evaluate<core::evaluation::kFunction>(aArgument);
        const auto tNumberOfNodes = mesh::EntityCounts{mesh::Mesh{tCutMesh.mFileName}}.numberOfNodes();
        return ones_vector_times_jacobian(mNumDimensions * tNumberOfNodes, tJacobian);
    };
    const auto tLevelSetTopology = std::make_shared<LevelSetTopology>(tInput);
    const auto tInitialGuess = tLevelSetTopology->initialGuess();
    // No filter
    {
        const auto tLevelSetFunction =
            make_level_set_geometry(tLevelSetTopology, filter::extension::make_identity_filter_function());
        const auto tComputedSum = tOnesVectorJacobianProductSum(tLevelSetFunction, tInitialGuess);
        // This is just a regression test, but this mesh has a non-trivial node map, so the test will fail if
        // the node map is not used.
        EXPECT_DOUBLE_EQ(mExpectedJacobianSum, tComputedSum);
    }
    // Kernel filter
    {
        const auto tLevelSetFunction = make_level_set_geometry(
            tLevelSetTopology,
            make_kernel_filter_test_function(filter::extension::FilterRadius{1.0}, Tet4MeshOnDisk::mMeshFilePath));
        const auto tComputedSum = tOnesVectorJacobianProductSum(tLevelSetFunction, tInitialGuess);
        // This is just a regression test, but it is expected to be different from the Jacobian computed with the
        // identity filter
        EXPECT_DOUBLE_EQ(mExpectedFilteredJacobianSum, tComputedSum);
    }
}

TEST_F(LevelSetTopologyFixture, JacobianTransposeRegression)
{
    third_party_integration::stk_io::write_mesh(kLevelSetInput.mesh_name->mToken, create_background_mesh());

    const auto tLevelSetTopology = LevelSetTopology{kLevelSetInput};
    const auto tInitialGuess = tLevelSetTopology.initialGuess();
    const auto tAdjointJacobian = tLevelSetTopology.adjointJacobian(tInitialGuess);

    const unsigned int tRowVectorSize =
        mesh::EntityCounts{mesh::Mesh{kLevelSetInput.mesh_name->mToken}}.numberOfNodes();
    const auto tRowVector = linear_algebra::DynamicVector(tRowVectorSize, 1.0);

    const auto tResult = tRowVector * tAdjointJacobian;

    // Regression result computed by outputting the entire Jacobian matrix using the `jacobian` function, and
    // performing the transpose matrix vector multiplication in Matlab.
    // clang-format off
    auto tExpected = std::vector(78,0.0);
    const auto tValuesInExpected = std::vector{0.5773502691896257, 0.5773502691896257, 0.5773502691896257, 0.0, 0.0, 1.0000000000000000, 
                                                                 0.7071067811865475, 0.0, 0.7071067811865475, 1.0000000000000000, 0.0, 0.0, 
                                                                 0.7071067811865475, 0.7071067811865475, 0.0, 0.0, 1.0000000000000000, 0.0, 
                                                                 0.0, 0.7071067811865475, 0.7071067811865475, -1.0000000000000000, 0.0, 0.0, 
                                                                 0.0, -1.0000000000000000, 0.0, -0.7071067811865475, -0.7071067811865475, 
                                                                 0.0, 0.0, 0.0, -1.0000000000000000, -0.7071067811865475, 0.0, 
                                                                 -0.7071067811865475, 0.0, -0.7071067811865475, -0.7071067811865475, 
                                                                 -0.5773502691896257, -0.5773502691896257, -0.5773502691896257};

    tExpected.insert(tExpected.end(), tValuesInExpected.begin(), tValuesInExpected.end());
    // clang-format on
    constexpr double tTol = 1e-14;
    plato::test_utilities::expect_container_entries_near(tResult.stdVector(), tExpected, tTol,
                                                         TEST_CONTEXT("LevelSet adjoint Jacobian entries"));
}

TEST_F(LevelSetTopologyMeshFixture, JacobianTransposeRegression)
{
    auto tInput = singleSphereInput();
    tInput.max_edge_length_percentage_for_snapping = 0.0;  // turn snapping off for regression

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
    const auto tInitialGuess = tLevelSetTopology->initialGuess();
    constexpr auto tTolerance = 1e-13;
    // No filter
    {
        const auto tLevelSetFunction =
            make_level_set_geometry(tLevelSetTopology, filter::extension::make_identity_filter_function());
        const auto tComputedSum = tOnesVectorJacobianProductSum(tLevelSetFunction, tInitialGuess);
        EXPECT_NEAR(mExpectedJacobianSum, tComputedSum, tTolerance);
    }
    // Kernel filter
    {
        const auto tLevelSetFunction = make_level_set_geometry(
            tLevelSetTopology,
            make_kernel_filter_test_function(filter::extension::FilterRadius{1.0}, tInput.mesh_name->mToken));
        const auto tComputedSum = tOnesVectorJacobianProductSum(tLevelSetFunction, tInitialGuess);
        EXPECT_NEAR(mExpectedFilteredJacobianSum, tComputedSum, tTolerance);
    }
}

TEST_F(LevelSetTopologyFixture, GenerateMeshRegression)
{
    third_party_integration::stk_io::write_mesh(kLevelSetInput.mesh_name->mToken, create_background_mesh());
    auto tLevelSetTopology = std::make_unique<LevelSetTopology>(kLevelSetInput);
    const auto tInitialGuess = tLevelSetTopology->initialGuess();

    const auto tMeshRegressionChecks = [](const std::size_t aExpectedNumberOfNodes,
                                          const std::filesystem::path& aFilePath,
                                          const plato::test_utilities::TestContext& aTestContext)
    {
        EXPECT_TRUE(std::filesystem::exists(aFilePath)) << aTestContext;
        const auto tNumberOfNodes = mesh::EntityCounts{mesh::Mesh{aFilePath}}.numberOfNodes();
        EXPECT_EQ(tNumberOfNodes, aExpectedNumberOfNodes) << aTestContext;
    };
    {
        const auto tAnalysisMesh = tLevelSetTopology->generateMesh(tInitialGuess);
        constexpr auto tExpectedNumberOfNodes = 40U;
        tMeshRegressionChecks(tExpectedNumberOfNodes, tAnalysisMesh.mFileName,
                              TEST_CONTEXT("No filter, direct construction"));
    }
    {
        const auto tLevelSetFunction = make_level_set_geometry(
            std::move(tLevelSetTopology),
            make_kernel_filter_test_function(filter::extension::FilterRadius{1.0}, kLevelSetInput.mesh_name->mToken));

        const auto tAnalysisMesh = tLevelSetFunction.evaluate<core::evaluation::kFunction>(tInitialGuess);
        constexpr auto tExpectedNumberOfNodes = 27U;
        tMeshRegressionChecks(tExpectedNumberOfNodes, tAnalysisMesh.mFileName,
                              TEST_CONTEXT("Kernel filter, using function wrapper"));
    }
}

TEST_F(LevelSetTopologyFixture, InitialGuessRegression)
{
    third_party_integration::stk_io::write_mesh(kLevelSetInput.mesh_name->mToken, create_background_mesh());

    const auto tLevelSetTopology = LevelSetTopology{kLevelSetInput};
    const auto tInitialGuess = tLevelSetTopology.initialGuess();

    const auto tExpectedInitialGuess = std::vector<double>{
        0.6160254037844386, 0.4571067811865476, 0.6160254037844386, 0.4571067811865476,  0.2500000000000000,
        0.4571067811865476, 0.6160254037844386, 0.4571067811865476, 0.6160254037844386,  0.4571067811865476,
        0.2500000000000000, 0.4571067811865476, 0.2500000000000000, -0.2500000000000000, 0.2500000000000000,
        0.4571067811865476, 0.2500000000000000, 0.4571067811865476, 0.6160254037844386,  0.4571067811865476,
        0.6160254037844386, 0.4571067811865476, 0.2500000000000000, 0.4571067811865476,  0.6160254037844386,
        0.4571067811865476, 0.6160254037844386};
    ASSERT_EQ(tInitialGuess.size(), mExpectedBackgroundLevelSetSize);
    constexpr auto tTolerance = 1e-14;
    plato::test_utilities::expect_container_entries_near(tExpectedInitialGuess, tInitialGuess.stdVector(), tTolerance,
                                                         TEST_CONTEXT("Initial guess"));
}

TEST_F(LevelSetTopologyTwoBlockFixture, InitialGuessOneBlockResultSize)
{
    const auto tLevelSetTopology = levelSetTopologyWithFixedBlocks({mBlockNames[1]});
    const auto tInitialGuess = tLevelSetTopology.initialGuess();
    constexpr auto tExpectedNumberOfDesignVariables = 8U;
    EXPECT_EQ(tInitialGuess.size(), tExpectedNumberOfDesignVariables);
}

TEST_F(LevelSetTopologyTwoBlockFixture, BoundsOneBlockResultSize)
{
    const auto tLevelSetTopology = levelSetTopologyWithFixedBlocks({mBlockNames[0]});
    const auto tBounds = tLevelSetTopology.bounds();
    constexpr auto tExpectedNumberOfDesignVariables = 8U;
    EXPECT_EQ(tBounds.first.size(), tExpectedNumberOfDesignVariables);
    EXPECT_EQ(tBounds.second.size(), tExpectedNumberOfDesignVariables);
}

TEST_F(LevelSetTopologyTwoBlockFixture, GenerateMeshSize)
{
    const auto tLevelSetTopology = levelSetTopologyWithFixedBlocks({mBlockNames[0]});
    const auto tInitialGuess = tLevelSetTopology.initialGuess();
    const auto tAnalysisMesh = tLevelSetTopology.generateMesh(tInitialGuess);
    EXPECT_TRUE(std::filesystem::exists(tAnalysisMesh.mFileName));
    const auto tNumberOfNodes = mesh::EntityCounts{mesh::Mesh{tAnalysisMesh.mFileName}}.numberOfNodes();
    EXPECT_EQ(tNumberOfNodes, mExpectedNumberOfNodes);
}

TEST_F(LevelSetTopologyTwoBlockFixture, JacobianOneBlockResultSize)
{
    const auto tCheckJacobianProductSize =
        [](const LevelSetTopology& aLevelSetTopology, const plato::test_utilities::TestContext& aTestContext)
    {
        const auto tInitialGuess = aLevelSetTopology.initialGuess();
        const auto tAnalysisMesh = aLevelSetTopology.generateMesh(tInitialGuess);
        const auto tNumberOfNodes = mesh::EntityCounts{mesh::Mesh{tAnalysisMesh.mFileName}}.numberOfNodes();
        const auto tNumberOfCoordinateComponents = 3U;
        const auto tVector = linear_algebra::DynamicVector<double>(tNumberOfCoordinateComponents * tNumberOfNodes, 1.0);

        const auto tResult = tVector * aLevelSetTopology.jacobian(tInitialGuess);
        EXPECT_EQ(tResult.size(), mExpectedNumberOfNodesInBlock1) << aTestContext;
    };
    tCheckJacobianProductSize(levelSetTopologyWithFixedBlocks({mBlockNames[1]}),
                              TEST_CONTEXT("Cut mesh and background mesh have matching global IDs"));

    constexpr auto tSphereRadius = 10.0;
    tCheckJacobianProductSize(levelSetTopologyWithRadiusAndFixedBlocks(tSphereRadius, {mBlockNames[0]}),
                              TEST_CONTEXT("Cut mesh has more global IDs than are in background mesh"));
}

TEST_F(LevelSetTopologyTwoBlockFixture, JacobianTransposeOneBlockResultSize)
{
    const auto tCheckTransposeJacobianProductSize =
        [](const LevelSetTopology& aLevelSetTopology, const plato::test_utilities::TestContext& aTestContext)
    {
        const auto tInitialGuess = aLevelSetTopology.initialGuess();
        const auto tVector = linear_algebra::DynamicVector<double>(mExpectedNumberOfNodesInBlock2, 1.0);

        const auto tAnalysisMesh = aLevelSetTopology.generateMesh(tInitialGuess);
        const auto tNumberOfNodes = mesh::EntityCounts{mesh::Mesh{tAnalysisMesh.mFileName}}.numberOfNodes();
        const auto tNumberOfCoordinateComponents = 3U;

        const auto tResult = tVector * aLevelSetTopology.adjointJacobian(tInitialGuess);
        EXPECT_EQ(tResult.size(), tNumberOfCoordinateComponents * tNumberOfNodes) << aTestContext;
    };

    tCheckTransposeJacobianProductSize(levelSetTopologyWithFixedBlocks({mBlockNames[0]}),
                                       TEST_CONTEXT("Cut mesh and background mesh have matching global IDs"));
    constexpr auto tSphereRadius = 10.0;
    tCheckTransposeJacobianProductSize(levelSetTopologyWithRadiusAndFixedBlocks(tSphereRadius, {mBlockNames[1]}),
                                       TEST_CONTEXT("Cut mesh has more global IDs than are in background mesh"));
}

TEST_F(LevelSetTopologyFixture, Bounds)
{
    third_party_integration::stk_io::write_mesh(kLevelSetInput.mesh_name->mToken, create_background_mesh());
    const auto tLevelSetTopology = LevelSetTopology{kLevelSetInput};
    const auto [tLowerBounds, tUpperBounds] = tLevelSetTopology.bounds();

    ASSERT_EQ(tLowerBounds.size(), mExpectedBackgroundLevelSetSize);
    ASSERT_EQ(tUpperBounds.size(), mExpectedBackgroundLevelSetSize);

    EXPECT_TRUE(
        std::all_of(tLowerBounds.cbegin(), tLowerBounds.cend(), [](const double aVal) { return aVal == -1.0; }));
    EXPECT_TRUE(std::all_of(tUpperBounds.cbegin(), tUpperBounds.cend(), [](const double aVal) { return aVal == 1.0; }));
}

TEST_F(LevelSetTopologyTwoBlockFixture, OutputRoundTrip)
{
    const auto tInput = levelSetTopologyInputWithFixedBlocks({mBlockNames[0]});
    const auto tDesignVariablesForOutput = linear_algebra::DynamicVector<double>(mExpectedNumberOfNodesInBlock2, 0.0);
    LevelSetTopology::output(tInput, filter::extension::make_identity_filter_function(), tDesignVariablesForOutput,
                             output::kOverwriteInfo);

    const auto tOutputMeshName = std::filesystem::path{tInput.output_name->mToken};
    const auto tRestartOutputMeshName = restart_file_name(tInput);

    EXPECT_TRUE(std::filesystem::exists(tOutputMeshName));
    EXPECT_TRUE(std::filesystem::exists(tRestartOutputMeshName));

    constexpr auto tFieldName = level_set_mesh_field_name();
    const auto tReadDesignVariables =
        third_party_integration::stk_io::test_utilities::read_nodal_field_as_vector(tRestartOutputMeshName, tFieldName);

    EXPECT_EQ(tReadDesignVariables.size(), mExpectedNumberOfNodes);
    // Block 1 is fixed, so we expect 0's for the first 8 entries, and 1's for the rest
    const auto tExpectedDesignVariables = []()
    {
        auto tDesignVariables = std::vector<double>(mExpectedNumberOfNodes, 1.0);
        const auto tNumberOfFixedNodes = 4U;
        std::fill_n(std::next(tDesignVariables.begin(), tNumberOfFixedNodes),
                    mExpectedNumberOfNodes - tNumberOfFixedNodes, 0.0);
        return tDesignVariables;
    }();

    EXPECT_EQ(tExpectedDesignVariables, tReadDesignVariables);

    std::filesystem::remove(tOutputMeshName);
    std::filesystem::remove(tRestartOutputMeshName);
}

TEST_F(LevelSetTopologyTwoBlockFixture, FilteredOutputRoundTrip)
{
    const auto tFixedBlock = std::string{mBlockNames[1]};
    const auto tInput = levelSetTopologyInputWithFixedBlocks({tFixedBlock});
    auto tDesignVariableVector = std::vector<double>(mExpectedNumberOfNodesInBlock1, 0.0);
    tDesignVariableVector.front() = 1.0;
    const auto tDesignVariables = linear_algebra::DynamicVector<double>(std::move(tDesignVariableVector));

    LevelSetTopology::output(
        tInput,
        make_kernel_filter_test_function(filter::extension::FilterRadius{2.0}, tInput.mesh_name->mToken, {tFixedBlock}),
        tDesignVariables, output::kOverwriteInfo);

    const auto tFieldOutputMeshName = restart_file_name(tInput);
    constexpr auto tFieldName = filtered_level_set_mesh_field_name();
    const auto tReadDesignVariables =
        third_party_integration::stk_io::test_utilities::read_nodal_field_as_vector(tFieldOutputMeshName, tFieldName);
    const auto tNormalization = 3.0 - 0.5 * std::sqrt(2.0);  // Sum of filter values: 1, 0.5, 0.5, 1 - sqrt(2)/2
    const auto tExpected = std::vector{1.0 / tNormalization,
                                       0.5 / tNormalization,
                                       0.5 / tNormalization,
                                       (1.0 - 0.5 * std::sqrt(2.0)) / tNormalization,
                                       0.0,
                                       0.0,
                                       0.0,
                                       0.0,
                                       1.0,
                                       1.0,
                                       1.0,
                                       1.0};

    constexpr auto tTolerance = 1e-14;
    plato::test_utilities::expect_container_entries_near(tExpected, tReadDesignVariables, tTolerance,
                                                         TEST_CONTEXT("Filtered output round trip"));

    std::filesystem::remove(tInput.output_name->mToken);
    std::filesystem::remove(restart_file_name(tInput));
}

TEST(LevelSetTopology, OutputFileNames)
{
    const auto tRestartPrefix = std::string{"restart_"};
    const auto tRestartOutputMeshName = std::filesystem::path{tRestartPrefix + kLevelSetInput.output_name->mToken};
    EXPECT_EQ(tRestartOutputMeshName, restart_file_name(kLevelSetInput));
}

TEST(LevelSetTopology, DetailAffineTransformation)
{
    const auto tVector = std::vector<double>{1.0, 1.5, 2.0};
    const auto tResult = detail::affine_transformation(tVector, detail::StartingLimits{std::make_pair(1.0, 2.0)},
                                                       detail::EndingLimits{std::make_pair(-1.0, 1.0)});

    ASSERT_EQ(tResult.size(), 3U);
    EXPECT_DOUBLE_EQ(tResult.at(0), -1.0);
    EXPECT_DOUBLE_EQ(tResult.at(1), .0);
    EXPECT_DOUBLE_EQ(tResult.at(2), 1.0);
}

TEST_F(NodalDensityMesh, InitialGuessFromField)
{
    auto tInput = test_utilities::create_valid_level_set_topology_geometry_initialize_from_field_input();
    tInput.mesh_name = input_parser::FileName{mMeshName};
    tInput.initial_field_name = input_parser::IdentifierString{mFieldName};

    const auto tLevelSetGeometry = LevelSetTopology(tInput);
    const auto tInitialGuess = tLevelSetGeometry.initialGuess();
    const auto tGoldVector =
        linear_algebra::DynamicVector<double>{-30, -26, -22, -18, -14, -10, -6, -2, 2, 6, 10, 14, 18, 22, 26, 30} *
        (1.0 / 30.0);

    plato::test_utilities::expect_container_entries_near(tInitialGuess.stdVector(), tGoldVector.stdVector(), 1e-15,
                                                         TEST_CONTEXT("Initial guess affine transformation"));
}

namespace
{

[[nodiscard]] auto retrieve_flattened_cut_node_coordinates(const std::filesystem::path& aCutMeshFilePath,
                                                           const unsigned int aDimension) -> std::vector<double>
{
    auto tCutCoordinates = third_party_integration::stk_io::nodal_coordinates(
        *third_party_integration::stk_io::read_mesh_bulk_data(aCutMeshFilePath));

    std::vector<double> tFlattenedCoordinates;
    tFlattenedCoordinates.reserve(tCutCoordinates.size() * aDimension);
    for (const auto& tCutCoordinate : tCutCoordinates)
    {
        tFlattenedCoordinates.push_back(tCutCoordinate.x);
        tFlattenedCoordinates.push_back(tCutCoordinate.y);
        if (aDimension == 3)
        {
            tFlattenedCoordinates.push_back(tCutCoordinate.z);
        }
    }

    return tFlattenedCoordinates;
}

void check_coordinate_sum_gradient_for_small_level_set_perturbations(const input_parser::level_set_topology& aInput,
                                                                     const unsigned int aNumDimensions)
{
    constexpr auto tTolerance = 1e-11;

    const auto tF = [aNumDimensions, aInput](const linear_algebra::DynamicVector<double>& aX) -> double
    {
        const auto tLevelSetTopology = LevelSetTopology{aInput};
        const auto tCutMesh = tLevelSetTopology.generateMesh(aX);
        const auto tCoordinates = retrieve_flattened_cut_node_coordinates(tCutMesh.mFileName, aNumDimensions);

        return std::accumulate(tCoordinates.begin(), tCoordinates.end(), 0.0);
    };
    const auto tDf = [aNumDimensions, aInput](const linear_algebra::DynamicVector<double>& aX,
                                              const linear_algebra::DynamicVector<double>& aV) -> double
    {
        const auto tLevelSetTopology = LevelSetTopology{aInput};
        const linear_algebra::JacobianMultiplier tJacobian = tLevelSetTopology.jacobian(aX);

        const auto tCutMesh = tLevelSetTopology.generateMesh(aX);
        const auto tCoordinates = retrieve_flattened_cut_node_coordinates(tCutMesh.mFileName, aNumDimensions);
        const auto tRowVectorSize = tCoordinates.size();
        const auto tOnes = linear_algebra::DynamicVector<double>(tRowVectorSize, 1.0);

        const auto tResult = tOnes * tJacobian;

        return tResult.dot(aV);
    };

    const auto tGradientCheckParameters = plato::test_utilities::GradientCheckParameters{
        0.5, 8,
        0.0125};  // set the initial step size such that mesh topological changes are not by the level set perturbation
    const auto tGradientCheck = plato::test_utilities::GradientChecker{tF, tDf};

    const auto tLevelSetTopology = LevelSetTopology{aInput};
    const auto tInitialGuess = tLevelSetTopology.initialGuess();
    const auto tDirection = linear_algebra::DynamicVector(tInitialGuess.size(), 1.0);

    const auto tErrors = tGradientCheck.finiteDifferenceErrors(tInitialGuess, tDirection, tGradientCheckParameters);
    ASSERT_FALSE(tErrors.empty());

    EXPECT_TRUE(std::all_of(tErrors.begin(), tErrors.end(),
                            [tTolerance](const auto aStepAndValue) { return aStepAndValue.mValue < tTolerance; }))
        << tGradientCheck.table(tInitialGuess, tDirection, tGradientCheckParameters);
}

}  // namespace

TEST_F(LevelSetTopology2DFixture, JacobianWithoutTopologicalChanges)
{
    const auto tInput = levelSetTopologyInputFor2D();
    check_coordinate_sum_gradient_for_small_level_set_perturbations(tInput, mNumDimensions);
}

TEST_F(LevelSetTopologyLargeSphereFixture, JacobianWithoutTopologicalChanges)
{
    const auto tInput = levelSetTopologyInputForLargeSphere();
    check_coordinate_sum_gradient_for_small_level_set_perturbations(tInput, mNumDimensions);
}

namespace
{

void check_jacobian_adjoint_jacobian_consistency(const input_parser::level_set_topology& aInput,
                                                 const unsigned int aNumDimensions,
                                                 const double aTolerance)
{
    const auto tLevelSetTopology = LevelSetTopology{aInput};
    const auto tInitialGuess = tLevelSetTopology.initialGuess();
    const auto tJacobian = tLevelSetTopology.jacobian(tInitialGuess);
    const auto tAdjointJacobian = tLevelSetTopology.adjointJacobian(tInitialGuess);

    const auto tCutMesh = tLevelSetTopology.generateMesh(tInitialGuess);
    const auto tCoordinates = retrieve_flattened_cut_node_coordinates(tCutMesh.mFileName, aNumDimensions);
    const auto tRowVectorSize = tCoordinates.size();
    const auto tRowVectorForJacobian = linear_algebra::DynamicVector<double>(tRowVectorSize, 1.0);

    const auto tRowVectorForAdjointJacobian = linear_algebra::DynamicVector<double>(
        mesh::EntityCounts{mesh::Mesh{aInput.mesh_name.value().mToken}}.numberOfDesignDomainNodes(), 1.0);

    const auto tRowVectorTimesJacobianDotRowVectorForAdjointJacobian =
        (tRowVectorForJacobian * tJacobian).dot(tRowVectorForAdjointJacobian);
    const auto tRowVectorForAdjointJacobianTimesAdjointDotRowVectorForJacobian =
        (tRowVectorForAdjointJacobian * tAdjointJacobian).dot(tRowVectorForJacobian);

    const auto tResult = std::abs(tRowVectorForAdjointJacobianTimesAdjointDotRowVectorForJacobian -
                                  tRowVectorTimesJacobianDotRowVectorForAdjointJacobian);
    EXPECT_NEAR(tResult, 0.0, aTolerance);
}

}  // namespace

TEST_F(LevelSetTopology2DFixture, JacobianAdjointJacobianConsistency)
{
    const auto tInput = levelSetTopologyInputFor2D();
    const auto tTolerance = 5e-15;
    check_jacobian_adjoint_jacobian_consistency(tInput, mNumDimensions, tTolerance);
}

TEST_F(LevelSetTopologyMeshFixture, JacobianAdjointJacobianConsistency)
{
    const auto tInput = singleSphereInput();
    const auto tTolerance = 8e-14;
    check_jacobian_adjoint_jacobian_consistency(tInput, mNumDimensions, tTolerance);
}

TEST(LevelSetTopology, Registration) { EXPECT_TRUE(library::is_geometry_function_registered("level_set_topology")); }

}  // namespace plato::geometry::extension::unittest
