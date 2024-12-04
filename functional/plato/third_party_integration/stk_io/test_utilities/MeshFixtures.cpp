#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

#include <filesystem>

#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::third_party_integration::stk_io::test_utilities
{
namespace
{
std::filesystem::path affirm_test_file_path(const std::string_view tMeshFileName)
{
    const auto tMeshFilePath = plato::utilities::data_file_path(tMeshFileName);
    assert(tMeshFilePath);
    return tMeshFilePath.value();  // NOLINT
}
}  // namespace

MeshGeneratingTestFixture::MeshGeneratingTestFixture(
    const std::filesystem::path& aMeshName, const third_party_integration::stk_io::CommandGenerator& aCommandGenerator)
    : mMeshFilePath{aMeshName}, mCommandGenerator{aCommandGenerator}
{
    third_party_integration::stk_io::write_mesh(aMeshName, aCommandGenerator);
}

MeshGeneratingTestFixture::~MeshGeneratingTestFixture() { std::filesystem::remove(mMeshFilePath); }

OneBlock3x1x1HexMesh::OneBlock3x1x1HexMesh()
    : MeshGeneratingTestFixture("one_block_3x1x1_hex.exo",
                                third_party_integration::stk_io::CommandGenerator{{3, 1, 1}, {0, 0, 0}, {3, 1, 1}})
{
}

OneBlock3x1x1HexMeshWithNodeSets::OneBlock3x1x1HexMeshWithNodeSets()
    : MeshGeneratingTestFixture(
          "one_block_3x1x1_hex_nodesets.exo",
          third_party_integration::stk_io::CommandGenerator{
              {3, 1, 1}, {0, 0, 0}, {3, 1, 1}, CommandElementType::Hex, {UseLowerX{true}, UseUpperX{true}}, {}})
{
}

TwoBlockMeshOnDisk::TwoBlockMeshOnDisk() : mMeshFilePath{affirm_test_file_path(mMeshFileName)} {}

TwoDNonUniformHexMesh::TwoDNonUniformHexMesh() : mMeshFilePath{affirm_test_file_path(mMeshFileName)} {}

Tet4MeshOnDisk::Tet4MeshOnDisk() : mMeshFilePath{affirm_test_file_path(mMeshFileName)} {}

TwoDThreeBlockMesh::TwoDThreeBlockMesh()
{
    third_party_integration::stk_io::write_mesh(mMeshFilePath, mMeshDescription);
}

TwoDThreeBlockMesh::~TwoDThreeBlockMesh() { std::filesystem::remove(mMeshFilePath); }

TwoDTwoBlockMesh::TwoDTwoBlockMesh() { third_party_integration::stk_io::write_mesh(mMeshFilePath, mMeshDescription); }

TwoDTwoBlockMesh::~TwoDTwoBlockMesh() { std::filesystem::remove(mMeshFilePath); }

}  // namespace plato::third_party_integration::stk_io::test_utilities
