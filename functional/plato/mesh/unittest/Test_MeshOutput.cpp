#include <gtest/gtest.h>

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

class MeshOutputTest : public third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh
{
    void TearDown() override
    {
        third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh::TearDown();
        std::filesystem::remove(kFileToWrite);
    }
};
}  // namespace

TEST_F(MeshOutputTest, FactoryAppend)
{
    const auto tMeshOutput = make_mesh_output(OutputMode::kAppend, InputFilePath{mMeshFilePath},
                                              OutputFilePath{mMeshFilePath}, kFixedBlocks);
    EXPECT_NE(dynamic_cast<const MeshFieldAppender*>(tMeshOutput.get()), nullptr);
}

TEST_F(MeshOutputTest, FactoryWriter)
{
    const auto tMeshOutput = make_mesh_output(OutputMode::kOverwrite, InputFilePath{mMeshFilePath},
                                              OutputFilePath{kFileToWrite}, kFixedBlocks);
    EXPECT_NE(dynamic_cast<const MeshFieldWriter*>(tMeshOutput.get()), nullptr);
}

TEST(MeshOutput, OutputMode)
{
    constexpr auto kOverwrite = true;
    EXPECT_EQ(output_mode(kOverwrite), OutputMode::kOverwrite);

    constexpr auto kDontOverwrite = false;
    EXPECT_EQ(output_mode(kDontOverwrite), OutputMode::kAppend);
}

}  // namespace plato::mesh::unittest
