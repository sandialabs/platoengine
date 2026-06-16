#ifndef PLATO_GEOMETRY_EXTENSION_UNITTEST_LEVELSETTOPOLOGYTESTFIXTURES
#define PLATO_GEOMETRY_EXTENSION_UNITTEST_LEVELSETTOPOLOGYTESTFIXTURES

#include <gtest/gtest.h>

#include <filesystem>

#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/geometry/extension/LevelSetTopology.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::geometry::extension::unittest
{

[[nodiscard]] inline auto level_set_input_fixture()
{
    return geometry::extension::test_utilities::create_valid_level_set_topology_geometry_input();
}

class LevelSetTopologyBaseFixture : virtual public ::testing::Test
{
   protected:
    void SetUp() override
    {
        third_party_integration::krino::set_stk_communicator_for_krino(
            boost::mpi::communicator{MPI_COMM_WORLD, boost::mpi::comm_attach});
    }
    void TearDown() override {}
};

class LevelSetTopologyFixture : public LevelSetTopologyBaseFixture
{
   protected:
    void SetUp() override { LevelSetTopologyBaseFixture::SetUp(); }
    void TearDown() override
    {
        std::filesystem::remove(level_set_input_fixture().mesh_name->mToken);
        LevelSetTopologyBaseFixture::TearDown();
    }

   public:
    constexpr static unsigned int mExpectedBackgroundLevelSetSize = 27U;
    constexpr static std::size_t mNumDimensions = std::size_t{3};
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
        auto tInput = level_set_input_fixture();
        tInput.mesh_name = input_parser::FileName{Tet4MeshOnDisk::mMeshFilePath};
        tInput.sphere_pattern.value().max = input_parser::Point{0, 0, 0};
        tInput.sphere_pattern.value().radius = 1.0;
        return tInput;
    }

   public:
    constexpr static double mExpectedJacobianSum = -43.3185837663019484;
    constexpr static double mExpectedFilteredJacobianSum = -72.780945645414576;
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

    [[nodiscard]] auto levelSetTopologyInputWithFixedBlocks(std::vector<std::string> aFixedBlocks) const
        -> input_parser::level_set_topology
    {
        auto tInput = level_set_input_fixture();
        tInput.mesh_name = input_parser::FileName{mMeshFilePath};
        tInput.fixed_blocks = input_parser::BlockList{std::move(aFixedBlocks)};
        return tInput;
    }

    [[nodiscard]] auto levelSetTopologyWithFixedBlocks(std::vector<std::string> aFixedBlocks) const -> LevelSetTopology
    {
        return LevelSetTopology{levelSetTopologyInputWithFixedBlocks(std::move(aFixedBlocks))};
    }

    [[nodiscard]] auto levelSetTopologyWithRadiusAndFixedBlocks(const double aRadius,
                                                                std::vector<std::string> aFixedBlocks) const
        -> LevelSetTopology
    {
        auto tInput = levelSetTopologyInputWithFixedBlocks(std::move(aFixedBlocks));
        tInput.sphere_pattern.value().radius = aRadius;
        return LevelSetTopology{tInput};
    }
};

inline void create_large_sphere_input(input_parser::level_set_topology& aInput, const double aSize, const double aShift)
{
    aInput.sphere_pattern = boost::none;
    aInput.sphere_list = input_parser::LevelSetSphereList{
        {input_parser::LevelSetSphere{aSize, input_parser::Point{0, -aSize + aShift, 0}}}};
    aInput.include_void_region = true;
}

class LevelSetTopology2DFixture : public LevelSetTopologyBaseFixture
{
   protected:
    void SetUp() override { LevelSetTopologyBaseFixture::SetUp(); }
    void TearDown() override { LevelSetTopologyBaseFixture::TearDown(); }
    [[nodiscard]] static auto levelSetTopologyInputFor2D() -> input_parser::level_set_topology
    {
        const auto tFilePath = utilities::data_file_path("rectangle_3x4_tri3.cdf");
        assert(tFilePath.has_value());
        auto tInput = level_set_input_fixture();
        constexpr double tSize = 100;
        constexpr double tShift = 0.95;
        create_large_sphere_input(tInput, tSize, tShift);
        tInput.mesh_name = input_parser::FileName{tFilePath.value()};
        return tInput;
    }

   public:
    constexpr static std::size_t mNumDimensions = std::size_t{2};
    std::filesystem::path mFileName = utilities::data_file_path("rectangle_3x4_tri3.cdf").value();
};

class LevelSetTopologyLargeSphereFixture : public LevelSetTopologyBaseFixture
{
   protected:
    void SetUp() override { LevelSetTopologyBaseFixture::SetUp(); }
    void TearDown() override { LevelSetTopologyBaseFixture::TearDown(); }
    [[nodiscard]] static auto levelSetTopologyInputForLargeSphere() -> input_parser::level_set_topology
    {
        const auto tFilePath = utilities::data_file_path("box_3x4x7_tet4.cdf");
        assert(tFilePath.has_value());
        auto tInput = level_set_input_fixture();
        constexpr double tSize = 500;
        constexpr double tShift = 0.95;
        create_large_sphere_input(tInput, tSize, tShift);
        tInput.mesh_name = input_parser::FileName{tFilePath.value()};
        return tInput;
    }

   public:
    constexpr static std::size_t mNumDimensions = std::size_t{3};
    std::filesystem::path mFileName = utilities::data_file_path("box_3x4x7_tet4.cdf").value();
};

class LevelSetTopology2DOneTriFixture : public LevelSetTopologyBaseFixture
{
   protected:
    void SetUp() override { LevelSetTopologyBaseFixture::SetUp(); }
    void TearDown() override { LevelSetTopologyBaseFixture::TearDown(); }
    [[nodiscard]] static auto levelSetTopologyInputFor2D() -> input_parser::level_set_topology
    {
        const auto tFilePath = utilities::data_file_path("one_tri.cdf");
        assert(tFilePath.has_value());
        auto tInput = level_set_input_fixture();
        tInput.include_void_region = true;
        constexpr double tSize = 100000;
        constexpr double tShift = 0.75;
        create_large_sphere_input(tInput, tSize, tShift);
        tInput.mesh_name = input_parser::FileName{tFilePath.value()};
        return tInput;
    }

   public:
    constexpr static std::size_t mNumDimensions = std::size_t{2};
    std::filesystem::path mFileName = utilities::data_file_path("one_tri.cdf").value();
};

}  // namespace plato::geometry::extension::unittest

#endif
