#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>
#include <numeric>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Comm.hpp>
#include <stk_util/parallel/Parallel.hpp>
#include <string_view>

#include "plato/test_utilities/TestDataFilePath.hpp"
#include "plato/third_party_integration/common/unittest/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"
#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"

namespace plato::third_party_integration::stk_io::unittest
{

TEST(STKUtilities, CommandGeneratorWriteMeshToDisk)
{
    const CommandGenerator tCommandGenerator{{2, 2, 2}};
    const std::string_view tMeshFileName{"mesh.exo"};
    write_mesh(tMeshFileName, tCommandGenerator);
    EXPECT_TRUE(std::filesystem::exists(tMeshFileName));
    EXPECT_TRUE(std::filesystem::remove(tMeshFileName));
}

TEST(STKUtilities, NumberOfNodesAndElementsFromBulk)
{
    const CommandGenerator tCommandGenerator{{2, 2, 2}};
    const auto tMesh = generate_bulk_data(tCommandGenerator);
    ASSERT_TRUE(tMesh);
    EXPECT_EQ(node_size(*tMesh), tCommandGenerator.numberOfNodes());
    EXPECT_EQ(element_size(*tMesh), tCommandGenerator.numberOfElements());
}

TEST(STKUtilities, SpatialDimensions3)
{
    const CommandGenerator tCommandGenerator{{2, 2, 2}};
    const auto tMesh = generate_bulk_data(tCommandGenerator);
    ASSERT_TRUE(tMesh);
    EXPECT_EQ(spatial_dimensions(*tMesh), 3u);
}

TEST(STKUtilities, SpatialDimensions2)
{
    const auto tMesh = read_mesh_bulk_data(test_utilities::test_data_file_path("rectangle_3x4_tri3.cdf").value());
    ASSERT_TRUE(tMesh);
    EXPECT_EQ(spatial_dimensions(*tMesh), 2u);
}

TEST(STKUtilities, ReadCoordinates)
{
    const CommandGenerator tCommandGenerator;
    const std::vector<double> gold = {0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1};
    const auto tMesh = generate_bulk_data(tCommandGenerator);
    ASSERT_TRUE(tMesh);
    const std::vector<double> res = flattened_nodal_coordinates(*tMesh);
    EXPECT_EQ(gold, res);
    EXPECT_EQ(node_size(*tMesh), tCommandGenerator.numberOfNodes());
}

TEST(STKUtilities, ReadCoordinatesCoordinate)
{
    const CommandGenerator tCommandGenerator;
    const std::vector<common::Coordinate> tGold = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0},
                                                   {0, 0, 1}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1}};
    const auto tMesh = generate_bulk_data(tCommandGenerator);
    ASSERT_TRUE(tMesh);
    const std::vector<common::Coordinate> tResult = nodal_coordinates(*tMesh);
    ASSERT_EQ(tGold.size(), tResult.size());
    for (unsigned int tIndex = 0; tIndex < tGold.size(); ++tIndex)
    {
        common::unittest::test_double_equality_of_components(tResult[tIndex], tGold[tIndex],
                                                             TEST_CONTEXT("Read nodal coordinates"));
    }

    EXPECT_EQ(node_size(*tMesh), tCommandGenerator.numberOfNodes());
}

TEST(STKUtilities, WriteDensityField)
{
    constexpr std::string_view tInputFileName = "brick.exo";
    write_bulk_data(tInputFileName, generate_bulk_data(CommandGenerator{}));
    const std::vector<double> data = {1, 2, 3, 4, 5, 6, 7, 8};
    constexpr std::string_view tOutputFileName = "brick-out.exo";
    write_mesh_density(tInputFileName, data, tOutputFileName);
    auto res = read_mesh_density(tOutputFileName);

    EXPECT_EQ(data, res);

    EXPECT_TRUE(std::filesystem::remove(tInputFileName));
    EXPECT_TRUE(std::filesystem::remove(tOutputFileName));
}

TEST(STKUtilities, WriteElementDensityField)
{
    constexpr std::string_view tInputFileName = "brick.exo";
    const CommandGenerator tCommandGenerator{{2, 2, 2}};
    write_bulk_data(tInputFileName, generate_bulk_data(tCommandGenerator));
    const std::vector<double> data = {1, 2, 3, 4, 5, 6, 7, 8};
    constexpr std::string_view tOutputFileName = "brick-out.exo";
    write_element_density(tInputFileName, data, tOutputFileName);
    auto res = read_element_density(tOutputFileName);

    EXPECT_EQ(data, res);

    EXPECT_TRUE(std::filesystem::remove(tInputFileName));
    EXPECT_TRUE(std::filesystem::remove(tOutputFileName));
}

}  // namespace plato::third_party_integration::stk_io::unittest
