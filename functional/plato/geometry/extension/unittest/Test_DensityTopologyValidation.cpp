#include <gtest/gtest.h>

#include <filesystem>

#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/MeshDesignVariablesSequentialView.hpp"
#include "plato/test_utilities/FileCreatingTestFixture.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::geometry::extension::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;
using third_party_integration::stk_io::test_utilities::TwoDTwoBlockMesh;

const auto kDensityTopology = plato::test_utilities::create_valid_density_topology_geometry();

struct DensityTopologyValidationFileFixture : public test_utilities::FileCreatingTestFixture
{
    DensityTopologyValidationFileFixture() : FileCreatingTestFixture{kDensityTopology.mesh_name.value().mToken} {}
};

}  // namespace

TEST(DensityTopologyValidation, ValidateMeshName)
{
    auto tDensityTopology = kDensityTopology;
    EXPECT_FALSE(library::detail::validate_mesh_name(tDensityTopology).has_value());
    tDensityTopology.mesh_name = boost::none;
    EXPECT_TRUE(library::detail::validate_mesh_name(tDensityTopology).has_value());
}

TEST(DensityTopologyValidation, ValidateOutputName)
{
    auto tDensityTopology = kDensityTopology;
    EXPECT_FALSE(detail::validate_output_name(tDensityTopology).has_value());
    tDensityTopology.output_name = boost::none;
    EXPECT_TRUE(detail::validate_output_name(tDensityTopology).has_value());
}

TEST_F(DensityTopologyValidationFileFixture, ValidDensityTopologyInput)
{
    auto tInput = input_parser::ParsedInput{};
    tInput.mDensityTopology = kDensityTopology;

    std::vector<std::string> tMessages;
    tMessages = library::validate_geometry(tInput, std::move(tMessages));
    EXPECT_TRUE(tMessages.empty());
}

TEST(DensityTopology, UniqueFixedBlockNames)
{
    const auto tBlockName1 = std::string{"block_1"};
    const auto tBlockName2 = std::string{"some-other-block"};
    {
        auto tDensityInputWithFixedBlocks = kDensityTopology;
        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tBlockName1}};
        const auto tUniqueFixedBlocks = detail::fixed_blocks(tDensityInputWithFixedBlocks);
        EXPECT_EQ(tUniqueFixedBlocks.count(tBlockName1), 1U);
    }
    {
        auto tDensityInputWithFixedBlocks = kDensityTopology;
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
        auto tDensityInputNoFixedBlocks = kDensityTopology;
        tDensityInputNoFixedBlocks.mesh_name = input_parser::FileName{mMeshFilePath.string()};
        constexpr auto tExpectedSizes = ExpectedSizes{/*.mNumberOfFixedBlocks=*/0U, /*.mNumberOfFixedBlocks=*/3U,
                                                      /*.mNumberOfDesignDomainNodes=*/mExpectedNumberOfNodes,
                                                      /*.mNumberOfDesignDomainElements=*/mExpectedNumberOfElements};
        tTestFunction(tDensityInputNoFixedBlocks, tExpectedSizes, tNoFixedBlocksContext);
    }
    const auto tBlockThreeFixedContext = TEST_CONTEXT("Block 3 fixed");
    {
        auto tDensityInputWithFixedBlocks = kDensityTopology;
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
    auto tDensityInputWithFixedBlocks = kDensityTopology;
    tDensityInputWithFixedBlocks.mesh_name = input_parser::FileName{mMeshFilePath.string()};
    tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{std::vector<std::string>{"block_1"}};

    const auto tInitialGuess = DensityTopology::initialGuess(tDensityInputWithFixedBlocks);
    constexpr auto tExpectedNumberOfDesignVariables = 6U;
    EXPECT_EQ(tInitialGuess.size(), tExpectedNumberOfDesignVariables);

    const auto tDensityTopology =
        DensityTopology{tDensityInputWithFixedBlocks, filter::extension::make_identity_filter_function()};
    const auto tMeshDesignVariables = tDensityTopology.generateMesh(tInitialGuess);
    constexpr auto tExpectedNumberOfBlocks = 2U;
    EXPECT_EQ(tMeshDesignVariables.mBlockScalarField.size(), tExpectedNumberOfBlocks);

    const auto tMeshDesignVariablesView = mesh::MeshDesignVariablesSequentialView{tMeshDesignVariables};
    EXPECT_EQ(tMeshDesignVariablesView.size(), tExpectedNumberOfDesignVariables);
}

TEST(DensityTopology, ValidateUniqueBlockNames)
{
    const auto tBlockName1 = std::string{"fixed-block-1"};
    const auto tBlockName2 = std::string{"fixed-block-2"};
    const auto tBlockName3 = std::string{"design-block"};
    // Valid cases
    {
        auto tDensityInputWithFixedBlocks = kDensityTopology;
        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{}};
        EXPECT_FALSE(detail::validate_unique_fixed_block_names(tDensityInputWithFixedBlocks).has_value());

        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tBlockName1}};
        EXPECT_FALSE(detail::validate_unique_fixed_block_names(tDensityInputWithFixedBlocks).has_value());

        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tBlockName1, tBlockName2}};
        EXPECT_FALSE(detail::validate_unique_fixed_block_names(tDensityInputWithFixedBlocks).has_value());
    }
    // Invalid
    {
        auto tDensityInputWithFixedBlocks = kDensityTopology;
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

    auto tDensityInputWithFixedBlocks = kDensityTopology;
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

TEST_F(TwoDTwoBlockMesh, ValidateAtLeastOneDesignBlock)
{
    const auto tFixedBlockName = std::string{"fixed"};
    const auto tDesignBlockName = std::string{"design"};
    auto tDensityInputWithFixedBlocks = kDensityTopology;
    tDensityInputWithFixedBlocks.mesh_name = input_parser::FileName{mMeshFilePath.string()};

    // Valid
    {
        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{}};
        const auto tErrorMessageForNoFixedBlocks =
            detail::validate_at_least_one_design_block(tDensityInputWithFixedBlocks);
        EXPECT_FALSE(tErrorMessageForNoFixedBlocks.has_value()) << tErrorMessageForNoFixedBlocks.value();

        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tFixedBlockName}};
        const auto tErrorMessageForOneFixedBlock =
            detail::validate_at_least_one_design_block(tDensityInputWithFixedBlocks);
        EXPECT_FALSE(tErrorMessageForOneFixedBlock.has_value()) << tErrorMessageForOneFixedBlock.value();
    }
    // Invalid
    {
        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::FixedBlockList{{tFixedBlockName, tDesignBlockName}};
        const auto tErrorMessageForAllFixedBlock =
            detail::validate_at_least_one_design_block(tDensityInputWithFixedBlocks);
        EXPECT_TRUE(tErrorMessageForAllFixedBlock.has_value()) << tErrorMessageForAllFixedBlock.value();
    }
}

}  // namespace plato::geometry::extension::unittest
