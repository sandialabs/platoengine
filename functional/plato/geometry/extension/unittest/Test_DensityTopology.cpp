#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>
#include <numeric>
#include <stk_mesh/base/Comm.hpp>
#include <stk_util/parallel/Parallel.hpp>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/linear_algebra/JacobianColumnEvaluator.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/IOUtilities.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::geometry::extension::unittest
{
namespace
{

const auto kDensityInput = plato::test_utilities::create_valid_density_topology_geometry();

constexpr unsigned int kExpectedDensitySize = 8;  // Based on mesh generation command below (1x1x1)

void create_small_mesh(const std::string& aFileName)
{
    ASSERT_EQ(stk::parallel_machine_size(MPI_COMM_WORLD), 1);
    const third_party_integration::stk_io::CommandGenerator tCommandGenerator{
        {1, 1, 1}, {-1, -2, -1}, {2, 1, 2}, third_party_integration::stk_io::CommandElementType::Hex};

    third_party_integration::stk_io::write_mesh(aFileName, tCommandGenerator);
}
}  // namespace

TEST(DensityTopology, Jacobian)
{
    create_small_mesh(kDensityInput.mesh_name->mToken);

    const DensityTopology tDensityTopology(kDensityInput, filter::extension::make_identity_filter_function());

    const std::vector<double> tDesignVars = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8};
    const linear_algebra::DynamicVector<double> tDesignVec(tDesignVars);
    const int tNumDesignParameters = tDesignVec.size();

    const linear_algebra::JacobianMultiplier tJacobian = tDensityTopology.jacobian(tDesignVec);

    std::vector<double> tRowVec(tNumDesignParameters, 0.0);
    std::iota(tRowVec.begin(), tRowVec.end(), 1.0);
    const linear_algebra::DynamicVector<double> tRolVec(tRowVec);

    // Jacobian is identity matrix
    const std::vector<double> tGold = tRowVec;
    const linear_algebra::DynamicVector<double> tRes = tRolVec * tJacobian;
    EXPECT_EQ(tRes.stdVector(), tGold);

    EXPECT_TRUE(std::filesystem::remove(kDensityInput.mesh_name->mToken));
}

TEST(DensityTopology, GenerateMesh)
{
    create_small_mesh(kDensityInput.mesh_name->mToken);

    const DensityTopology tDensityTopology(kDensityInput, filter::extension::make_identity_filter_function());

    const std::vector<double> tDesignVars = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8};
    const linear_algebra::DynamicVector<double> tDesignVec(tDesignVars);

    const auto tAnalysisDomainMesh = tDensityTopology.generateMesh(tDesignVec);
    const auto tDensities =
        analysis::mesh_analysis_to_vector(analysis::AnalysisDomainMeshSequentialView{tAnalysisDomainMesh});
    const auto [tDensityValues, tIDMap] = analysis::split_scalar_field_values(tDensities);
    EXPECT_EQ(tDensityValues, tDesignVars);

    EXPECT_TRUE(std::filesystem::remove(kDensityInput.mesh_name->mToken));
}

TEST(DensityTopology, InitialGuess)
{
    create_small_mesh(kDensityInput.mesh_name->mToken);

    const linear_algebra::DynamicVector<double> tInitialGuess = DensityTopology::initialGuess(kDensityInput);

    EXPECT_EQ(tInitialGuess.size(), kExpectedDensitySize);

    for (const double val : tInitialGuess.stdVector())
    {
        EXPECT_EQ(val, 0.5);
    }

    EXPECT_TRUE(std::filesystem::remove(kDensityInput.mesh_name->mToken));
}

TEST(DensityTopology, Bounds)
{
    create_small_mesh(kDensityInput.mesh_name->mToken);

    const auto [tLowerBounds, tUpperBounds] = DensityTopology::bounds(kDensityInput);

    EXPECT_EQ(tLowerBounds.size(), kExpectedDensitySize);
    EXPECT_EQ(tUpperBounds.size(), kExpectedDensitySize);

    EXPECT_TRUE(std::all_of(tLowerBounds.cbegin(), tLowerBounds.cend(), [](const double aVal) { return aVal == 0.0; }));
    EXPECT_TRUE(std::all_of(tUpperBounds.cbegin(), tUpperBounds.cend(), [](const double aVal) { return aVal == 1.0; }));

    EXPECT_TRUE(std::filesystem::remove(kDensityInput.mesh_name->mToken));
}

TEST(DensityTopology, UniqueFixedBlockNames)
{
    const auto tBlockName1 = std::string{"block_1"};
    const auto tBlockName2 = std::string{"some-other-block"};
    {
        auto tDensityInputWithFixedBlocks = kDensityInput;
        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tBlockName1}};
        const auto tUniqueFixedBlocks = detail::fixed_blocks(tDensityInputWithFixedBlocks);
        EXPECT_EQ(tUniqueFixedBlocks.count(tBlockName1), 1U);
    }
    {
        auto tDensityInputWithFixedBlocks = kDensityInput;
        tDensityInputWithFixedBlocks.fixed_blocks =
            input_parser::FixedBlockList{{tBlockName1, tBlockName2, tBlockName1, tBlockName2}};
        const auto tUniqueFixedBlocks = detail::fixed_blocks(tDensityInputWithFixedBlocks);
        EXPECT_EQ(tUniqueFixedBlocks.count(tBlockName1), 1U);
        EXPECT_EQ(tUniqueFixedBlocks.count(tBlockName2), 1U);
    }
}

TEST_F(TwoDThreeBlockMesh, MeshFromInput)
{
    struct ExpectedSizes
    {
        std::size_t mNumberOfFixedBlocks = 0U;
        std::size_t mNumberOfDesignBlocks = 0U;
        std::size_t mNumberOfDesignDomainNodes = 0U;
        std::size_t mNumberOfDesignDomainElements = 0U;
    };

    const auto tTestFunction = [](const input_parser::density_topology& tDensityInput,
                                  const ExpectedSizes& aExpectedSizes, const test_utilities::TestContext& aTestContext)
    {
        const auto tMesh = detail::mesh_from_input(tDensityInput);
        EXPECT_EQ(tMesh.fixedBlockOrdinals().size(), aExpectedSizes.mNumberOfFixedBlocks) << aTestContext;
        EXPECT_EQ(tMesh.designBlockOrdinals().size(), aExpectedSizes.mNumberOfDesignBlocks) << aTestContext;
        EXPECT_EQ(mesh::EntityCounts{tMesh}.numberOfDesignDomainNodes(), aExpectedSizes.mNumberOfDesignDomainNodes)
            << aTestContext;
        EXPECT_EQ(mesh::EntityCounts{tMesh}.numberOfDesignDomainElements(),
                  aExpectedSizes.mNumberOfDesignDomainElements)
            << aTestContext;
    };

    const auto tNoFixedBlocksContext = TEST_CONTEXT("No fixed blocks");
    {
        auto tDensityInputNoFixedBlocks = kDensityInput;
        tDensityInputNoFixedBlocks.mesh_name = input_parser::FileName{mMeshFilePath.string()};
        constexpr auto tExpectedSizes = ExpectedSizes{/*.mNumberOfFixedBlocks=*/0U, /*.mNumberOfFixedBlocks=*/3U,
                                                      /*.mNumberOfDesignDomainNodes=*/mExpectedNumberOfNodes,
                                                      /*.mNumberOfDesignDomainElements=*/mExpectedNumberOfElements};
        tTestFunction(tDensityInputNoFixedBlocks, tExpectedSizes, tNoFixedBlocksContext);
    }
    const auto tBlockThreeFixedContext = TEST_CONTEXT("Block 3 fixed");
    {
        auto tDensityInputWithFixedBlocks = kDensityInput;
        tDensityInputWithFixedBlocks.mesh_name = input_parser::FileName{mMeshFilePath.string()};
        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{std::vector<std::string>{"block_3"}};
        constexpr auto tExpectedSizes = ExpectedSizes{
            /*.mNumberOfFixedBlocks=*/1U, /*.mNumberOfFixedBlocks=*/2U,
            /*.mNumberOfDesignDomainNodes=*/8U,
            /*.mNumberOfDesignDomainElements=*/mExpectedNumberOfElementsInBlock1 + mExpectedNumberOfElementsInBlock2};
        tTestFunction(tDensityInputWithFixedBlocks, tExpectedSizes, tBlockThreeFixedContext);
    }
}

TEST_F(TwoDThreeBlockMesh, NumberOfDesignVariablesWithFixedBlocks)
{
    auto tDensityInputWithFixedBlocks = kDensityInput;
    tDensityInputWithFixedBlocks.mesh_name = input_parser::FileName{mMeshFilePath.string()};
    tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{std::vector<std::string>{"block_1"}};

    const auto tInitialGuess = DensityTopology::initialGuess(tDensityInputWithFixedBlocks);
    constexpr auto tExpectedNumberOfDesignVariables = 6U;
    EXPECT_EQ(tInitialGuess.size(), tExpectedNumberOfDesignVariables);

    const auto tDensityTopology =
        DensityTopology{tDensityInputWithFixedBlocks, filter::extension::make_identity_filter_function()};
    const auto tAnalysisDomainMesh = tDensityTopology.generateMesh(tInitialGuess);
    constexpr auto tExpectedNumberOfBlocks = 2U;
    EXPECT_EQ(tAnalysisDomainMesh.mBlockScalarField.size(), tExpectedNumberOfBlocks);

    const auto tAnalysisDomainMeshView = analysis::AnalysisDomainMeshSequentialView{tAnalysisDomainMesh};
    EXPECT_EQ(tAnalysisDomainMeshView.size(), tExpectedNumberOfDesignVariables);
}

TEST(DensityTopology, ValidateUniqueBlockNames)
{
    const auto tBlockName1 = std::string{"fixed-block-1"};
    const auto tBlockName2 = std::string{"fixed-block-2"};
    const auto tBlockName3 = std::string{"design-block"};
    // Valid cases
    {
        auto tDensityInputWithFixedBlocks = kDensityInput;
        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{}};
        EXPECT_FALSE(detail::validate_unique_fixed_block_names(tDensityInputWithFixedBlocks).has_value());

        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tBlockName1}};
        EXPECT_FALSE(detail::validate_unique_fixed_block_names(tDensityInputWithFixedBlocks).has_value());

        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tBlockName1, tBlockName2}};
        EXPECT_FALSE(detail::validate_unique_fixed_block_names(tDensityInputWithFixedBlocks).has_value());
    }
    // Invalid
    {
        auto tDensityInputWithFixedBlocks = kDensityInput;
        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tBlockName1, tBlockName1}};
        EXPECT_TRUE(detail::validate_unique_fixed_block_names(tDensityInputWithFixedBlocks).has_value());

        tDensityInputWithFixedBlocks.fixed_blocks =
            input_parser::FixedBlockList{{tBlockName1, tBlockName2, tBlockName1}};
        EXPECT_TRUE(detail::validate_unique_fixed_block_names(tDensityInputWithFixedBlocks).has_value());

        tDensityInputWithFixedBlocks.fixed_blocks =
            input_parser::FixedBlockList{{tBlockName2, tBlockName2, tBlockName1}};
        EXPECT_TRUE(detail::validate_unique_fixed_block_names(tDensityInputWithFixedBlocks).has_value());
    }
}

TEST_F(TwoDTwoBlockMesh, ValidateBlockNamesExist)
{
    const auto tFixedBlockName = std::string{"fixed"};
    const auto tDesignBlockName = std::string{"design"};
    const auto tBogusBlockName = std::string{"bogus"};

    auto tDensityInputWithFixedBlocks = kDensityInput;
    tDensityInputWithFixedBlocks.mesh_name = input_parser::FileName{mMeshFilePath.string()};

    // Valid
    {
        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{}};
        const auto tErrorMessageForNoFixedBlocks =
            detail::validate_fixed_block_names_exist(tDensityInputWithFixedBlocks);
        EXPECT_FALSE(tErrorMessageForNoFixedBlocks.has_value()) << tErrorMessageForNoFixedBlocks.value();

        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tFixedBlockName}};
        const auto tErrorMessageForOneFixedBlock =
            detail::validate_fixed_block_names_exist(tDensityInputWithFixedBlocks);
        EXPECT_FALSE(tErrorMessageForOneFixedBlock.has_value()) << tErrorMessageForOneFixedBlock.value();

        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tFixedBlockName, tDesignBlockName}};
        const auto tErrorMessageForTwoFixedBlocks =
            detail::validate_fixed_block_names_exist(tDensityInputWithFixedBlocks);
        EXPECT_FALSE(tErrorMessageForTwoFixedBlocks.has_value()) << tErrorMessageForTwoFixedBlocks.value();
    }
    // Invalid
    {
        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tBogusBlockName}};
        EXPECT_TRUE(detail::validate_fixed_block_names_exist(tDensityInputWithFixedBlocks).has_value());
    }
}

}  // namespace plato::geometry::extension::unittest
