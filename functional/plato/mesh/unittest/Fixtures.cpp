#include "plato/mesh/unittest/Fixtures.hpp"

#include <filesystem>

#include "plato/test_utilities/TestDataFilePath.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::mesh::unittest
{
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

TwoBlockMeshOnDisk::TwoBlockMeshOnDisk() : mMeshFilePath{test_utilities::test_data_file_path(mMeshFileName).value()} {}
}  // namespace plato::mesh::unittest
