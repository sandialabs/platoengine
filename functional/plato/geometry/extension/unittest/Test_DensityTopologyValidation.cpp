#include <gtest/gtest.h>

#include <filesystem>

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/extension/MeshValidationUtilities.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/test_utilities/FileCreatingTestFixture.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshWithFieldWriter.hpp"

namespace plato::geometry::extension::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;
using third_party_integration::stk_io::test_utilities::TwoDTwoBlockMesh;

using NodalDensityMesh = third_party_integration::stk_io::test_utilities::MeshWithNodalDensities;

const auto kDensityTopology = create_valid_density_topology_geometry_input();

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
    EXPECT_FALSE(library::detail::validate_output_name(tDensityTopology).has_value());
    tDensityTopology.output_name = boost::none;
    EXPECT_TRUE(library::detail::validate_output_name(tDensityTopology).has_value());
}

TEST(DensityTopologyValidation, ValidateInitialDensity)
{
    auto tDensityTopology = kDensityTopology;
    EXPECT_FALSE(detail::validate_initial_density_value(tDensityTopology).has_value());
    tDensityTopology.initial_density_value = boost::none;
    EXPECT_FALSE(detail::validate_initial_density_value(tDensityTopology).has_value()) << "Empty value is valid";
    tDensityTopology.initial_density_value = -1;
    EXPECT_TRUE(detail::validate_initial_density_value(tDensityTopology).has_value()) << "-1 value is invalid";
    tDensityTopology.initial_density_value = 0;
    EXPECT_TRUE(detail::validate_initial_density_value(tDensityTopology).has_value()) << "0 value is invalid";
    tDensityTopology.initial_density_value = 0.5;
    EXPECT_FALSE(detail::validate_initial_density_value(tDensityTopology).has_value()) << "0.5 value is valid";
    tDensityTopology.initial_density_value = 1.0;
    EXPECT_FALSE(detail::validate_initial_density_value(tDensityTopology).has_value()) << "1 value is valid";
    tDensityTopology.initial_density_value = 1.5;
    EXPECT_TRUE(detail::validate_initial_density_value(tDensityTopology).has_value()) << "1.5 value is invalid";
}

TEST(DensityTopologyValidation, ValidateExactlyOneInitialTopologySpecifier)
{
    auto tDensityTopology = input_parser::density_topology{};
    EXPECT_TRUE(detail::validate_exactly_one_initial_topology_specifier(tDensityTopology).has_value())
        << "Missing both specifiers";

    tDensityTopology.initial_density_value = 0.5;
    EXPECT_FALSE(detail::validate_exactly_one_initial_topology_specifier(tDensityTopology).has_value())
        << "Valid, has initial_density_value";

    tDensityTopology.initial_field_name = input_parser::IdentifierString{"bogus"};
    EXPECT_TRUE(detail::validate_exactly_one_initial_topology_specifier(tDensityTopology).has_value())
        << "Both specifiers used, Invalid.";

    tDensityTopology.initial_density_value = boost::none;
    EXPECT_FALSE(detail::validate_exactly_one_initial_topology_specifier(tDensityTopology).has_value())
        << "Only field specifier is used, valid.";
}

TEST_F(DensityTopologyValidationFileFixture, ValidDensityTopologyInput)
{
    //    const auto tInput = input_parser::ParsedInput{} | kDensityTopology;

    //    std::vector<std::string> tMessages;
    //    tMessages = library::validate_geometry(tInput, std::move(tMessages));
    //    EXPECT_TRUE(tMessages.empty());
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
        const auto tMesh = mesh_from_input(tDensityInput);
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

    const auto tMeshDesignVariablesView = analysis::AnalysisDomainMeshSequentialView{tMeshDesignVariables};
    EXPECT_EQ(tMeshDesignVariablesView.size(), tExpectedNumberOfDesignVariables);
}

TEST_F(NodalDensityMesh, ValidateInitialTopologySource)
{
    auto tDensityTopology = kDensityTopology;
    tDensityTopology.mesh_name = input_parser::FileName{mMeshName};
    tDensityTopology.initial_field_name = input_parser::IdentifierString{mFieldName};

    EXPECT_FALSE(validate_initial_field_source(tDensityTopology).has_value());

    tDensityTopology.initial_field_name = input_parser::IdentifierString{"bogus-field"};
    EXPECT_TRUE(validate_initial_field_source(tDensityTopology).has_value());
}

}  // namespace plato::geometry::extension::unittest
