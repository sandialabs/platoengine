#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>
#include <numeric>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Comm.hpp>
#include <stk_util/parallel/Parallel.hpp>
#include <string_view>

#include "plato/utilities/STKUtilities.hpp"
namespace plato::utilities::unittest
{
TEST(STKGenerateMesh, Box)
{
    ASSERT_EQ(stk::parallel_machine_size(MPI_COMM_WORLD), 1);

    constexpr std::string_view fileName = "test.exo";
    auto bulk = create_mesh("generated:3x3x4|bbox:-1,-2,-1,2,1,2");
    write_mesh(fileName, bulk);

    constexpr unsigned expectedNumNodes = 80;
    EXPECT_EQ(expectedNumNodes, read_mesh_node_size(fileName));

    constexpr unsigned expectedNumElements = 3 * 3 * 4;
    EXPECT_EQ(expectedNumElements, element_size(fileName));

    EXPECT_TRUE(std::filesystem::exists(fileName));
    EXPECT_TRUE(std::filesystem::remove(fileName));
}

TEST(STKUtilities, NumberOfNodesAndElementsFromBulk)
{
    EXPECT_EQ(node_size(create_mesh("generated:2x2x2|bbox:0,0,0,1,1,1")), 27u);
    EXPECT_EQ(element_size(create_mesh("generated:2x2x2|bbox:0,0,0,1,1,1")), 8u);
}

TEST(STKUtilities, SpatialDimensions)
{
    EXPECT_EQ(spatial_dimensions(create_mesh("generated:2x2x2|bbox:0,0,0,1,1,1")), 3u);
}

TEST(STKUtilities, ReadCoordinates)
{
    const std::vector<double> gold = {0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1};
    const auto res = nodal_coordinates(create_mesh("generated:1x1x1|bbox:0,0,0,1,1,1"));

    EXPECT_EQ(gold, res);
    EXPECT_EQ(node_size(create_mesh("generated:1x1x1|bbox:0,0,0,1,1,1")), 8u);
}

TEST(STKUtilities, WriteDensityField)
{
    constexpr std::string_view tInputFileName = "brick.exo";
    write_mesh(tInputFileName, create_mesh("generated:1x1x1"));
    const std::vector<double> data = {1, 2, 3, 4, 5, 6, 7, 8};
    constexpr std::string_view tOutputFileName = "brick-out.exo";
    write_mesh_density(tInputFileName, data, tOutputFileName);
    auto res = read_mesh_density(tOutputFileName);

    EXPECT_EQ(data, res);

    EXPECT_TRUE(std::filesystem::remove(tInputFileName));
    EXPECT_TRUE(std::filesystem::remove(tOutputFileName));
}
}  // namespace plato::utilities::unittest