
#include <gtest/gtest.h>

#include <filesystem>
#include <iomanip>

#include "plato/test_utilities/TestDataFilePath.hpp"
#include "plato/utilities/STKCommandGenerator.hpp"
#include "plato/utilities/STKVolumeUtilities.hpp"

namespace plato::utilities::unittest
{

namespace
{

void create_mesh_test_volume(const std::string& aGenerationCommand, const double aGold)
{
    const std::string tMeshFileName = "test_mesh.exo";
    write_mesh(tMeshFileName, create_mesh(aGenerationCommand));
    const double tResult = mesh_volume(tMeshFileName);

    std::cout << std::setprecision(16) << tResult << std::endl;
    EXPECT_DOUBLE_EQ(tResult, aGold);
    EXPECT_TRUE(std::filesystem::remove(tMeshFileName));
}

void read_mesh_and_test_volume(const std::string_view tMeshFileName, const double aGold)
{
    const auto tFilePath = test_utilities::test_data_file_path(tMeshFileName);
    ASSERT_TRUE(tFilePath);
    const double tResult = mesh_volume(tFilePath.value());
    std::cout << std::setprecision(16) << tResult << std::endl;
    EXPECT_DOUBLE_EQ(tResult, aGold);
}

void test_first_element_volume_and_coordinates(const std::string& aGenerationCommand,
                                               const double aGoldVolume,
                                               const std::vector<Coordinate>& aGoldCoordinates)
{
    const auto tBulk = create_mesh(aGenerationCommand);
    const stk::mesh::EntityVector tElements = element_vector(*tBulk);
    ASSERT_FALSE(tElements.empty());
    const auto tOnlyElement = tElements[0];
    const double tVolume = element_volume(tOnlyElement, *tBulk);
    EXPECT_DOUBLE_EQ(tVolume, aGoldVolume);
    std::vector<Coordinate> tCoordinates = element_coordinates(tOnlyElement, *tBulk);
    ASSERT_EQ(tCoordinates.size(), aGoldCoordinates.size());
    for (unsigned int iIndex = 0; iIndex < tCoordinates.size(); ++iIndex)
    {
        EXPECT_DOUBLE_EQ(tCoordinates[iIndex].x, aGoldCoordinates[iIndex].x);
        EXPECT_DOUBLE_EQ(tCoordinates[iIndex].y, aGoldCoordinates[iIndex].y);
        EXPECT_DOUBLE_EQ(tCoordinates[iIndex].z, aGoldCoordinates[iIndex].z);
    }
}

}  // namespace

TEST(STKVolumeUtilities, HexVolumeAndCoordinates)
{
    test_first_element_volume_and_coordinates(
        STKCommandGenerator{}.toString(), 1.0,
        {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}});
}

TEST(STKVolumeUtilities, TetVolumeAndCoordinates)
{
    test_first_element_volume_and_coordinates(
        STKCommandGenerator{{1, 1, 1}, {0, 0, 0}, {1, 1, 1}, STKCommandElementType::Tet}.toString(), 1.0 / 6.0,
        {{0, 0, 0}, {1, 1, 0}, {0, 1, 0}, {1, 1, 1}});
}

TEST(STKVolumeUtilities, MeshVolume)
{
    {
        const STKCommandGenerator tSTKCommandGenerator{{2, 3, 4}};
        create_mesh_test_volume(tSTKCommandGenerator.toString(), tSTKCommandGenerator.volume());
    }
    {
        const STKCommandGenerator tSTKCommandGenerator{{3, 4, 5}, {}, {3, 4, 5}, STKCommandElementType::Tet};
        create_mesh_test_volume(tSTKCommandGenerator.toString(), tSTKCommandGenerator.volume());
    }
    {
        const STKCommandGenerator tSTKCommandGenerator{{1, 1, 1}, {}, {}, STKCommandElementType::Tet};
        create_mesh_test_volume(tSTKCommandGenerator.toString(), tSTKCommandGenerator.volume());
    }

    {
        const STKCommandGenerator tSTKCommandGenerator{{20, 2, 3}, {}, {20, 2, 3}, STKCommandElementType::Tet};
        create_mesh_test_volume(tSTKCommandGenerator.toString(), tSTKCommandGenerator.volume());
    }
}

TEST(STKVolumeUtilities, ExternalMeshVolume_Box_2x4x8_hex)
{
    constexpr double tGold = 2 * 4 * 8;
    read_mesh_and_test_volume("box_2x4x8_hex.cdf", tGold);
    read_mesh_and_test_volume("box_2x4x8_hex20.cdf", tGold);
}

TEST(STKVolumeUtilities, ExternalMeshVolume_Box_3x4x7_tet)
{
    constexpr double tGold = 3 * 4 * 7;
    read_mesh_and_test_volume("box_3x4x7_tet4.cdf", tGold);
    read_mesh_and_test_volume("box_3x4x7_tet10.cdf", tGold);
}

TEST(STKVolumeUtilities, ExternalMeshVolume_Box_2x4x10_hex_and_tet)
{
    read_mesh_and_test_volume("box_2x4x10_hex_and_tet.cdf", 2 * 4 * 10);
}

TEST(STKVolumeUtilities, ExternalMeshVolume_Rectangle_3x4_quad)
{
    read_mesh_and_test_volume("rectangle_3x4_quad4.cdf", 3 * 4);
}

TEST(STKVolumeUtilities, ExternalMeshVolume_Rectangle_3x4_shell)
{
    read_mesh_and_test_volume("rectangle_3x4_shell4.cdf", 3 * 4);
}

TEST(STKVolumeUtilities, ExternalMeshVolume_Rectangle_3x4_tri)
{
    read_mesh_and_test_volume("rectangle_3x4_tri3.cdf", 3 * 4);
}

TEST(STKVolumeUtilities, ExternalMeshVolume_Rectangle_4x10_tri_and_shell)
{
    read_mesh_and_test_volume("rectangle_4x10_tri_and_shell.cdf", 4 * 10);
}

}  // namespace plato::utilities::unittest
