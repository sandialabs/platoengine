#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>
#include <numeric>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Comm.hpp>
#include <stk_util/parallel/Parallel.hpp>
#include <string_view>

#include "plato/third_party_integration/common/unittest/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"
#include "plato/third_party_integration/stk_io/VolumeUtilities.hpp"

namespace plato::third_party_integration::stk_io::unittest
{
TEST(STKGenerateMesh, Box)
{
    ASSERT_EQ(stk::parallel_machine_size(MPI_COMM_WORLD), 1);

    constexpr std::string_view fileName = "test.exo";
    const CommandGenerator tCommandGenerator{{3, 3, 4}, {-1, -2, -1}, {2, 1, 2}, CommandElementType::Hex};
    auto bulk = generate_mesh(tCommandGenerator);
    write_mesh(fileName, bulk);

    EXPECT_EQ(tCommandGenerator.numberOfNodes(), read_mesh_node_size(fileName));
    EXPECT_EQ(tCommandGenerator.numberOfElements(), element_size(fileName));

    EXPECT_TRUE(std::filesystem::exists(fileName));
    EXPECT_TRUE(std::filesystem::remove(fileName));
}

TEST(STKUtilities, NumberOfNodesAndElementsFromBulk)
{
    const CommandGenerator tCommandGenerator{{2, 2, 2}};
    const auto tMesh = generate_mesh(tCommandGenerator);
    ASSERT_TRUE(tMesh);
    EXPECT_EQ(node_size(*tMesh), tCommandGenerator.numberOfNodes());
    EXPECT_EQ(element_size(*tMesh), tCommandGenerator.numberOfElements());
}

TEST(STKUtilities, SpatialDimensions)
{
    const CommandGenerator tCommandGenerator{{2, 2, 2}};
    const auto tMesh = generate_mesh(tCommandGenerator);
    ASSERT_TRUE(tMesh);
    EXPECT_EQ(spatial_dimensions(*tMesh), 3u);
}

TEST(STKUtilities, ReadCoordinates)
{
    const CommandGenerator tCommandGenerator;
    const std::vector<double> gold = {0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1};
    const auto tMesh = generate_mesh(tCommandGenerator);
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
    const auto tMesh = generate_mesh(tCommandGenerator);
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
    write_mesh(tInputFileName, generate_mesh(CommandGenerator{}));
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
    write_mesh(tInputFileName, generate_mesh(tCommandGenerator));
    const std::vector<double> data = {1, 2, 3, 4, 5, 6, 7, 8};
    constexpr std::string_view tOutputFileName = "brick-out.exo";
    write_element_density(tInputFileName, data, tOutputFileName);
    auto res = read_element_density(tOutputFileName);

    EXPECT_EQ(data, res);

    EXPECT_TRUE(std::filesystem::remove(tInputFileName));
    EXPECT_TRUE(std::filesystem::remove(tOutputFileName));
}

}  // namespace plato::third_party_integration::stk_io::unittest
