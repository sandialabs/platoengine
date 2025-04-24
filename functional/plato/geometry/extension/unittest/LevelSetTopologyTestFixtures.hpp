#ifndef PLATO_GEOMETRY_EXTENSION_UNITTEST_LEVELSETTOPOLOGYTESTFIXTURES
#define PLATO_GEOMETRY_EXTENSION_UNITTEST_LEVELSETTOPOLOGYTESTFIXTURES

#include <gtest/gtest.h>

#include <filesystem>

#include "plato/geometry/extension/LevelSetTopology.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::geometry::extension::unittest
{

namespace
{
const auto kLevelSetInputFixture =
    geometry::extension::test_utilities::create_valid_level_set_topology_geometry_input();
const auto kKrinoLogFileName = std::filesystem::path{"Krino_Output.txt"};

}  // namespace

class LevelSetTopologyBaseFixture : virtual public ::testing::Test
{
   protected:
    void SetUp() override
    {
        static bool tFirstTime{true};
        if (tFirstTime)
        {
            third_party_integration::krino::initialize_environment_for_krino(kKrinoLogFileName, MPI_COMM_SELF);
            tFirstTime = false;
        }
    }
    void TearDown() override { std::filesystem::remove(kKrinoLogFileName); }
};

class LevelSetTopologyFixture : public LevelSetTopologyBaseFixture
{
   protected:
    void SetUp() override { LevelSetTopologyBaseFixture::SetUp(); }
    void TearDown() override
    {
        std::filesystem::remove(kLevelSetInputFixture.mesh_name->mToken);
        LevelSetTopologyBaseFixture::TearDown();
    }
    unsigned int mExpectedBackgroundLevelSetSize = 27U;
    std::size_t mNumDimensions = std::size_t{3};
};

/// @brief The purpose of this fixture is to provide a mesh with a non-trivial node map.
class LevelSetTopologyMeshFixture : public LevelSetTopologyFixture,
                                    public third_party_integration::stk_io::test_utilities::Tet4MeshOnDisk
{
   protected:
    void SetUp() override
    {
        LevelSetTopologyFixture::SetUp();
        Tet4MeshOnDisk::SetUp();
    }
    void TearDown() override
    {
        LevelSetTopologyFixture::TearDown();
        Tet4MeshOnDisk::TearDown();
    }

    auto singleSphereInput() -> input_parser::level_set_topology
    {
        auto tInput = kLevelSetInputFixture;
        tInput.mesh_name = input_parser::FileName{Tet4MeshOnDisk::mMeshFilePath};
        tInput.sphere_pattern_bbox_max_x = 0.0;
        tInput.sphere_pattern_bbox_max_y = 0.0;
        tInput.sphere_pattern_bbox_max_z = 0.0;
        tInput.sphere_pattern_radius = 1.0;
        return tInput;
    }

    double mExpectedJacobianSum = -43.3185837663019484;
    double mExpectedFilteredJacobianSum = -72.780945645414576;
};

class LevelSetTopologyTwoBlockFixture : public LevelSetTopologyFixture,
                                        public third_party_integration::stk_io::test_utilities::ThreeDTwoBlockTetMesh
{
   protected:
    void SetUp() override
    {
        LevelSetTopologyFixture::SetUp();
        ThreeDTwoBlockTetMesh::SetUp();
    }
    void TearDown() override
    {
        LevelSetTopologyFixture::TearDown();
        ThreeDTwoBlockTetMesh::TearDown();
    }

    auto levelSetTopologyInputWithFixedBlocks(std::vector<std::string> aFixedBlocks) const
        -> input_parser::level_set_topology
    {
        auto tInput = kLevelSetInputFixture;
        tInput.mesh_name = input_parser::FileName{mMeshFilePath};
        tInput.fixed_blocks = input_parser::FixedBlockList{std::move(aFixedBlocks)};
        return tInput;
    }

    auto levelSetTopologyWithFixedBlocks(std::vector<std::string> aFixedBlocks) const -> LevelSetTopology
    {
        return LevelSetTopology{levelSetTopologyInputWithFixedBlocks(std::move(aFixedBlocks))};
    }

    auto levelSetTopologyWithRadiusAndFixedBlocks(const double aRadius, std::vector<std::string> aFixedBlocks) const
        -> LevelSetTopology
    {
        auto tInput = levelSetTopologyInputWithFixedBlocks(std::move(aFixedBlocks));
        tInput.sphere_pattern_radius = aRadius;
        return LevelSetTopology{tInput};
    }
};

namespace
{
void create_large_sphere_input(input_parser::level_set_topology& aInput, const double aSize, const double aShift)
{
    aInput.sphere_pattern_bbox_max_x = 0.0;
    aInput.sphere_pattern_bbox_min_y = -aSize + aShift;
    aInput.sphere_pattern_bbox_max_y = -aSize + aShift;
    aInput.sphere_pattern_bbox_max_z = 0.0;
    aInput.sphere_pattern_radius = aSize;
    aInput.sphere_pattern_spacing = aSize * 10;
    aInput.include_void_region = true;
}
}  // namespace

class LevelSetTopology2DFixture : public LevelSetTopologyBaseFixture
{
   protected:
    void SetUp() override { LevelSetTopologyBaseFixture::SetUp(); }
    void TearDown() override { LevelSetTopologyBaseFixture::TearDown(); }
    auto levelSetTopologyInputFor2D() const -> input_parser::level_set_topology
    {
        const auto tFilePath = utilities::data_file_path("rectangle_3x4_tri3.cdf");
        assert(tFilePath.has_value());
        auto tInput = kLevelSetInputFixture;
        constexpr double tSize = 100;
        constexpr double tShift = 0.95;
        create_large_sphere_input(tInput, tSize, tShift);
        tInput.mesh_name = input_parser::FileName{tFilePath.value()};
        return tInput;
    }
    std::size_t mNumDimensions = std::size_t{2};
    std::filesystem::path mFileName = utilities::data_file_path("rectangle_3x4_tri3.cdf").value();
};

class LevelSetTopology2DOneTriFixture : public LevelSetTopologyBaseFixture
{
   protected:
    void SetUp() override { LevelSetTopologyBaseFixture::SetUp(); }
    void TearDown() override { LevelSetTopologyBaseFixture::TearDown(); }
    auto levelSetTopologyInputFor2D() const -> input_parser::level_set_topology
    {
        const auto tFilePath = utilities::data_file_path("one_tri.cdf");
        assert(tFilePath.has_value());
        auto tInput = kLevelSetInputFixture;
        tInput.include_void_region = true;
        constexpr double tSize = 100000;
        constexpr double tShift = 0.75;
        create_large_sphere_input(tInput, tSize, tShift);
        tInput.mesh_name = input_parser::FileName{tFilePath.value()};
        return tInput;
    }
    std::size_t mNumDimensions = std::size_t{2};
    std::filesystem::path mFileName = utilities::data_file_path("one_tri.cdf").value();
};

}  // namespace plato::geometry::extension::unittest

#endif
