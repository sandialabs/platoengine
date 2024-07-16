
#include <gtest/gtest.h>

#include <filesystem>
#include <iomanip>

#include "plato/test_utilities/TestDataFilePath.hpp"
#include "plato/third_party_integration/common/unittest/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"

namespace plato::third_party_integration::stk_io::unittest
{

namespace
{

void create_mesh_test_volume(const CommandGenerator& aCommandGenerator)
{
    const double tResult = mesh_volume(*generate_mesh(aCommandGenerator));

    std::cout << std::setprecision(16) << tResult << std::endl;
    EXPECT_DOUBLE_EQ(tResult, aCommandGenerator.volume());
}

void create_single_element_mesh_test_centroid(const CommandGenerator& aCommandGenerator, const common::Coordinate aGold)
{
    const auto tMesh = generate_mesh(aCommandGenerator);
    ASSERT_TRUE(tMesh);

    const auto tElements = element_vector(*tMesh);
    ASSERT_EQ(tElements.size(), 1u);

    const common::Coordinate tElemCentroid = element_centroid(tElements[0], *tMesh);
    common::unittest::test_double_equality_of_components(tElemCentroid, aGold, TEST_CONTEXT("Single element centroid"));
}

void read_mesh_and_test_volume(const std::string_view tMeshFileName, const double aGold)
{
    const auto tFilePath = test_utilities::test_data_file_path(tMeshFileName);
    ASSERT_TRUE(tFilePath);
    const double tResult = mesh_volume(*read_mesh_bulk_data(std::string{tFilePath.value()}));
    std::cout << std::setprecision(16) << tResult << std::endl;
    EXPECT_DOUBLE_EQ(tResult, aGold);
}

void test_first_element_volume_and_coordinates(const CommandGenerator& aCommandGenerator,
                                               const double aGoldVolume,
                                               const std::vector<common::Coordinate>& aGoldCoordinates)
{
    const auto tBulk = generate_mesh(aCommandGenerator);
    const stk::mesh::EntityVector tElements = element_vector(*tBulk);
    ASSERT_FALSE(tElements.empty());
    const auto tOnlyElement = tElements[0];
    const double tVolume = element_volume(tOnlyElement, *tBulk);
    EXPECT_DOUBLE_EQ(tVolume, aGoldVolume);
    std::vector<common::Coordinate> tCoordinates = element_coordinates(tOnlyElement, *tBulk);
    ASSERT_EQ(tCoordinates.size(), aGoldCoordinates.size());
    for (unsigned int iIndex = 0; iIndex < tCoordinates.size(); ++iIndex)
    {
        common::unittest::test_double_equality_of_components(tCoordinates[iIndex], aGoldCoordinates[iIndex],
                                                             TEST_CONTEXT("Element coordinates"));
    }
}

}  // namespace

TEST(STKVolumeUtilities, HexVolumeAndCoordinates)
{
    test_first_element_volume_and_coordinates(
        CommandGenerator{}, 1.0,
        {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}});
}

TEST(STKVolumeUtilities, TetVolumeAndCoordinates)
{
    test_first_element_volume_and_coordinates(
        CommandGenerator{{1, 1, 1}, {0, 0, 0}, {1, 1, 1}, CommandElementType::Tet}, 1.0 / 6.0,
        {{0, 0, 0}, {1, 1, 0}, {0, 1, 0}, {1, 1, 1}});
}

TEST(STKVolumeUtilities, MeshVolume)
{
    {
        const CommandGenerator tCommandGenerator{{2, 3, 4}};
        create_mesh_test_volume(tCommandGenerator);
    }
    {
        const CommandGenerator tCommandGenerator{{3, 4, 5}, {}, {3, 4, 5}, CommandElementType::Tet};
        create_mesh_test_volume(tCommandGenerator);
    }
    {
        const CommandGenerator tCommandGenerator{{1, 1, 1}, {}, {}, CommandElementType::Tet};
        create_mesh_test_volume(tCommandGenerator);
    }

    {
        const CommandGenerator tCommandGenerator{{20, 2, 3}, {}, {20, 2, 3}, CommandElementType::Tet};
        create_mesh_test_volume(tCommandGenerator);
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

TEST(STKVolumeUtilities, ElementCentroid_Hex)
{
    // unit hex
    create_single_element_mesh_test_centroid(CommandGenerator{{1, 1, 1}, {0, 0, 0}, {1, 1, 1}},
                                             common::Coordinate{0.5, 0.5, 0.5});

    // scaled hex
    constexpr common::Coordinate tLower{-1.0, -2.0, -4.0};
    constexpr common::Coordinate tUpper{6.0, 8.0, 2.0};
    constexpr common::Coordinate tCentroid = (tLower + tUpper) / 2.0;
    create_single_element_mesh_test_centroid(CommandGenerator{{1, 1, 1}, tLower, tUpper}, tCentroid);
}

TEST(STKVolumeUtilities, ElementCentroids)
{
    CommandGenerator tCommandGenerator{{4, 1, 1}, {0, 0, 0}, {4, 1, 1}};

    const auto tBulk = generate_mesh(tCommandGenerator);
    const auto tCentroids = element_centroids(*tBulk);
    ASSERT_EQ(tCentroids.size(), 4u);

    common::unittest::test_double_equality_of_components(tCentroids[0], {0.5, 0.5, 0.5},
                                                         TEST_CONTEXT("Element centroids 0"));
    common::unittest::test_double_equality_of_components(tCentroids[1], {1.5, 0.5, 0.5},
                                                         TEST_CONTEXT("Element centroids 1"));
    common::unittest::test_double_equality_of_components(tCentroids[2], {2.5, 0.5, 0.5},
                                                         TEST_CONTEXT("Element centroids 2"));
    common::unittest::test_double_equality_of_components(tCentroids[3], {3.5, 0.5, 0.5},
                                                         TEST_CONTEXT("Element centroids 3"));
}

TEST(STKVolumeUtilities, AverageNodalDensity)
{
    const CommandGenerator tCommandGenerator{{4, 4, 4}, {0, 0, 0}, {4, 4, 4}};

    const auto tBulk = generate_mesh(tCommandGenerator);
    const int tTotalNumberOfNodes = tCommandGenerator.numberOfNodes();
    const double tTotalVolume = tCommandGenerator.volume();
    const double tGold = static_cast<double>(tTotalNumberOfNodes) / tTotalVolume;
    const double tResult = average_nodal_density(*tBulk);

    EXPECT_DOUBLE_EQ(tGold, tResult);
}

}  // namespace plato::third_party_integration::stk_io::unittest
