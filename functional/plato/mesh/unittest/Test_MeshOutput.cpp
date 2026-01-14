#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <vector>

#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/MeshFieldAppender.hpp"
#include "plato/mesh/MeshFieldWriter.hpp"
#include "plato/mesh/MeshOutput.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::mesh::unittest
{
namespace
{
const auto kFileToWrite = std::filesystem::path{"test-file.exo"};
const auto kFixedBlocks = std::set<std::string>{};
const auto kNumberOfNodes = 16U;

class MeshOutputTest : public third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh
{
    void TearDown() override
    {
        third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh::TearDown();
        std::filesystem::remove(kFileToWrite);
    }
};

[[nodiscard]] auto make_analysis_domain_mesh(const std::filesystem::path& aPath, const double aValues)
    -> analysis::AnalysisDomainMesh
{
    const auto tValues = std::vector<double>(kNumberOfNodes, aValues);
    return DesignVariablesConversion{mesh::Mesh{aPath, {}}}.nodalFieldToAnalysisDomainMesh(
        NodalFieldVectorReference{tValues});
}

}  // namespace

TEST_F(MeshOutputTest, AppendMeshOutput)
{
    const auto tAnalysisDomainMesh = make_analysis_domain_mesh(mMeshFilePath, 1.0);
    {
        const auto tMeshOutput = mesh_output(OutputMode::kAppend, tAnalysisDomainMesh, kFileToWrite, 1.0);
        EXPECT_NE(dynamic_cast<const MeshFieldWriter*>(tMeshOutput.get()), nullptr)
            << "Attempt to append an nonexistent mesh is a MeshFieldWriter.";
    }
    {
        const auto tMeshOutput = mesh_output(OutputMode::kAppend, tAnalysisDomainMesh, kFileToWrite, 2.0);
        EXPECT_NE(dynamic_cast<const MeshFieldAppender*>(tMeshOutput.get()), nullptr)
            << "Append an existing mesh is MeshFieldAppender.";
    }
    {
        const auto tMeshOutput = detail::append_mesh_output(tAnalysisDomainMesh, 3.0);
        EXPECT_NE(dynamic_cast<const MeshFieldAppender*>(tMeshOutput.get()), nullptr);
    }
}

TEST_F(MeshOutputTest, OverwriteMeshOutput)
{
    const auto tAnalysisDomainMesh = make_analysis_domain_mesh(mMeshFilePath, 1.0);
    {
        const auto tMeshOutput = detail::overwrite_mesh_output(tAnalysisDomainMesh, kFileToWrite, 1.0);
        EXPECT_NE(dynamic_cast<const MeshFieldWriter*>(tMeshOutput.get()), nullptr);
    }
    {
        const auto tMeshOutput = mesh_output(OutputMode::kOverwrite, tAnalysisDomainMesh, kFileToWrite, 1.0);
        EXPECT_NE(dynamic_cast<const MeshFieldWriter*>(tMeshOutput.get()), nullptr);
    }
}

TEST_F(MeshOutputTest, ExampleUsage)
{
    constexpr auto tFieldName = std::string_view{"snow"};
    for (const auto tTimeStep : {1, 2})
    {
        const auto tAnalysisDomainMesh = make_analysis_domain_mesh(mMeshFilePath, 2.0 * tTimeStep);
        const auto tMeshOutput = mesh_output(OutputMode::kAppend, tAnalysisDomainMesh, kFileToWrite, tTimeStep);
        tMeshOutput->addFieldFromAnalysisDomainMesh(tAnalysisDomainMesh, tFieldName, -1);
    }

    ASSERT_TRUE(std::filesystem::exists(kFileToWrite));
    const auto tFields = EntityRetrieval{Mesh{kFileToWrite}}.nodalFields();
    const auto tGoldFields = std::vector<std::string>{"coordinates", std::string{tFieldName}};
    EXPECT_EQ(tFields, tGoldFields);
    const auto tTimeSteps = EntityCounts{Mesh{kFileToWrite}}.timeSteps();
    const auto tGoldTimeSteps = std::vector<double>{1.0, 2.0};
    EXPECT_EQ(tGoldTimeSteps, tTimeSteps);

    const auto tFieldAtTime2 = EntityRetrieval{Mesh{kFileToWrite}}.designDomainNodalField(tFieldName, 2.0);
    const auto tGoldFieldAtTime2 = std::vector<double>(kNumberOfNodes, 4.0);
    EXPECT_EQ(tFieldAtTime2, tGoldFieldAtTime2);
}

TEST(MeshOutput, OutputMode)
{
    constexpr auto kOverwrite = true;
    EXPECT_EQ(output_mode(kOverwrite), OutputMode::kOverwrite);

    constexpr auto kDontOverwrite = false;
    EXPECT_EQ(output_mode(kDontOverwrite), OutputMode::kAppend);
}

}  // namespace plato::mesh::unittest
