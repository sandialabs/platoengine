#include <gtest/gtest.h>

#include <boost/none.hpp>
#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/extension/MeshValidationUtilities.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/library/GeometryValidation.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/test_utilities/FileCreatingTestFixture.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshWithFieldWriter.hpp"

namespace plato::geometry::extension::unittest
{
namespace
{
using DensityTopologyValidationTwoDThreeBlockMesh = third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;

using NodalDensityMesh = third_party_integration::stk_io::test_utilities::MeshWithNodalDensities;

const auto kDensityTopology = geometry::extension::test_utilities::create_valid_density_topology_geometry_input();

struct DensityTopologyValidationFileFixture : public plato::test_utilities::FileCreatingTestFixture
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
    constexpr auto tValidInRangeInitialDensity = 0.5;
    tDensityTopology.initial_density_value = tValidInRangeInitialDensity;
    EXPECT_FALSE(detail::validate_initial_density_value(tDensityTopology).has_value()) << "0.5 value is valid";
    constexpr auto tValidOnBoundsInitialDensity = 1.0;
    tDensityTopology.initial_density_value = tValidOnBoundsInitialDensity;
    EXPECT_FALSE(detail::validate_initial_density_value(tDensityTopology).has_value()) << "1 value is valid";
    constexpr auto tInvalidInitialDensity = 1.5;
    tDensityTopology.initial_density_value = tInvalidInitialDensity;
    EXPECT_TRUE(detail::validate_initial_density_value(tDensityTopology).has_value()) << "1.5 value is invalid";
}

TEST(DensityTopologyValidation, ValidateMeshIsTrisOrTetsIfCutting)
{
    auto tDensityTopology = kDensityTopology;
    namespace tpis = third_party_integration::stk_io;
    tpis::write_mesh(tDensityTopology.mesh_name->mToken, tpis::CommandGenerator{});

    EXPECT_FALSE(detail::validate_mesh_is_tet_or_tri_when_cutting(tDensityTopology).has_value())
        << "No cutting, doesn't matter the type.";
    tDensityTopology.cut_mesh_name = input_parser::FileName{"cut_mesh.exo"};
    EXPECT_TRUE(detail::validate_mesh_is_tet_or_tri_when_cutting(tDensityTopology).has_value())
        << "When cutting, mesh must be tet or tri.";

    tpis::write_mesh(tDensityTopology.mesh_name->mToken,
                     tpis::CommandGenerator{.mType = tpis::CommandElementType::kTet});
    EXPECT_FALSE(detail::validate_mesh_is_tet_or_tri_when_cutting(tDensityTopology).has_value())
        << "When cutting, mesh must be tet or tri.";
    std::filesystem::remove(tDensityTopology.mesh_name->mToken);
}

TEST(DensityTopologyValidation, ValidateExactlyOneInitialTopologySpecifier)
{
    auto tDensityTopology = input_parser::density_topology{};
    EXPECT_TRUE(detail::validate_exactly_one_initial_topology_specifier(tDensityTopology).has_value())
        << "Missing both specifiers";

    constexpr auto tValidInRangeInitialDensity = 0.5;
    tDensityTopology.initial_density_value = tValidInRangeInitialDensity;
    EXPECT_FALSE(detail::validate_exactly_one_initial_topology_specifier(tDensityTopology).has_value())
        << "Valid, has initial_density_value";

    tDensityTopology.initial_field_name = input_parser::IdentifierString{"bogus"};
    EXPECT_TRUE(detail::validate_exactly_one_initial_topology_specifier(tDensityTopology).has_value())
        << "Both specifiers used, Invalid.";

    tDensityTopology.initial_density_value = boost::none;
    EXPECT_FALSE(detail::validate_exactly_one_initial_topology_specifier(tDensityTopology).has_value())
        << "Only field specifier is used, valid.";
}

TEST_F(DensityTopologyValidationTwoDThreeBlockMesh, MeshFromInput)
{
    struct ExpectedSizes
    {
        std::size_t mNumberOfFixedBlocks = 0U;
        std::size_t mNumberOfDesignBlocks = 0U;
        std::size_t mNumberOfDesignDomainNodes = 0U;
        std::size_t mNumberOfDesignDomainElements = 0U;
    };

    const auto tTestFunction = [](const input_parser::density_topology& aDensityInput,
                                  const ExpectedSizes& aExpectedSizes,
                                  const plato::test_utilities::TestContext& aTestContext)
    {
        const auto tMesh = mesh_from_input(aDensityInput);
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
        tDensityInputWithFixedBlocks.fixed_blocks = input_parser::BlockList{std::vector<std::string>{mBlockNames[2]}};
        constexpr auto tExpectedSizes = ExpectedSizes{
            /*.mNumberOfFixedBlocks=*/1U, /*.mNumberOfFixedBlocks=*/2U,
            /*.mNumberOfDesignDomainNodes=*/8U,
            /*.mNumberOfDesignDomainElements=*/mExpectedNumberOfElementsInBlock1 + mExpectedNumberOfElementsInBlock2};
        tTestFunction(tDensityInputWithFixedBlocks, tExpectedSizes, tBlockThreeFixedContext);
    }
}

TEST_F(DensityTopologyValidationTwoDThreeBlockMesh, NumberOfDesignVariablesWithFixedBlocks)
{
    auto tDensityInputWithFixedBlocks = kDensityTopology;
    tDensityInputWithFixedBlocks.mesh_name = input_parser::FileName{mMeshFilePath.string()};
    tDensityInputWithFixedBlocks.fixed_blocks = input_parser::BlockList{std::vector<std::string>{mBlockNames[0]}};

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
