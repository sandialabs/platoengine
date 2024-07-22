#include <gtest/gtest.h>

#include <filesystem>

#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"

namespace plato::third_party_integration::stk_io
{
struct CommandGenerator;
}

namespace plato::mesh::unittest
{
/// @brief A base class fixture that creates a mesh using CommandGenerator on construction, and deletes it on
/// destruction.
class MeshGeneratingTestFixture : public ::testing::Test
{
   protected:
    MeshGeneratingTestFixture(const std::filesystem::path& aMeshName,
                              const third_party_integration::stk_io::CommandGenerator& aCommandGenerator);
    ~MeshGeneratingTestFixture();

    std::filesystem::path mMeshFilePath;
    third_party_integration::stk_io::CommandGenerator mCommandGenerator;
};

/// @brief A mesh creating test fixture that creates a 3x1x1 hex mesh.
class OneBlock3x1x1HexMesh : public MeshGeneratingTestFixture
{
   protected:
    OneBlock3x1x1HexMesh();
};

/// @brief A mesh test fixture that provides the path to a specific two-block mesh and has members
/// giving its node and element sizes.
class TwoBlockMeshOnDisk : public ::testing::Test
{
   protected:
    TwoBlockMeshOnDisk();

    std::filesystem::path mMeshFilePath;

    constexpr static auto mMeshFileName = std::string_view{"box_2x4x10_hex_and_tet.cdf"};
    constexpr static auto mExpectedNumberOfBlocks = 2u;
    constexpr static auto mExpectedNumberOfElementsInBlock1 = 273u;
    constexpr static auto mExpectedNumberOfElementsInBlock2 = 40u;
    constexpr static auto mExpectedNumberOfNodesInBlock1 = 93u;
    constexpr static auto mExpectedNumberOfNodesInBlock2 = 90u;
};
}  // namespace plato::mesh::unittest
