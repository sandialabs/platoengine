#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

#include <cassert>
#include <filesystem>
#include <string_view>

#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/utilities/DataFilePath.hpp"
#include "plato/utilities/FileUtilities.hpp"

namespace plato::third_party_integration::stk_io::test_utilities
{
namespace
{
auto affirm_test_file_path(const std::string_view aMeshFileName) -> std::filesystem::path
{
    const auto tMeshFilePath = plato::utilities::data_file_path(aMeshFileName);
    assert(tMeshFilePath);
    return tMeshFilePath.value();  // NOLINT
}
}  // namespace

MeshGeneratingTestFixture::MeshGeneratingTestFixture(
    const std::filesystem::path& aMeshName, const third_party_integration::stk_io::CommandGenerator& aCommandGenerator)
    : mMeshFilePath{utilities::make_filename_unique(aMeshName)}, mCommandGenerator{aCommandGenerator}
{
    third_party_integration::stk_io::write_mesh(mMeshFilePath, aCommandGenerator);
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
              {3, 1, 1}, {0, 0, 0}, {3, 1, 1}, CommandElementType::kHex, {UseLowerX{true}, UseUpperX{true}}, {}})
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

TwoDManyBlockMesh::TwoDManyBlockMesh() { third_party_integration::stk_io::write_mesh(mMeshFilePath, mMeshDescription); }

TwoDManyBlockMesh::~TwoDManyBlockMesh() { std::filesystem::remove(mMeshFilePath); }

ThreeDTwoBlockTetMesh::ThreeDTwoBlockTetMesh()
{
    third_party_integration::stk_io::write_mesh(mMeshFilePath, mMeshDescription);
}

ThreeDTwoBlockTetMesh::~ThreeDTwoBlockTetMesh() { std::filesystem::remove(mMeshFilePath); }

TwoDTwoBlockMesh::TwoDTwoBlockMesh() { third_party_integration::stk_io::write_mesh(mMeshFilePath, mMeshDescription); }

TwoDTwoBlockMesh::~TwoDTwoBlockMesh() { std::filesystem::remove(mMeshFilePath); }

XZPlaneMesh::XZPlaneMesh() { third_party_integration::stk_io::write_mesh(mMeshFilePath, mMeshDescription); }

XZPlaneMesh::~XZPlaneMesh() { std::filesystem::remove(mMeshFilePath); }

ThirtyDegreeWedgeMesh::ThirtyDegreeWedgeMesh()
{
    third_party_integration::stk_io::write_mesh(mMeshFilePath, mMeshDescription);
}

ThirtyDegreeWedgeMesh::~ThirtyDegreeWedgeMesh() { std::filesystem::remove(mMeshFilePath); }

}  // namespace plato::third_party_integration::stk_io::test_utilities
