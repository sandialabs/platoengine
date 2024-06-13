#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>
#include <numeric>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Comm.hpp>
#include <stk_util/parallel/Parallel.hpp>
#include <string_view>

#include "plato/utilities/STKCommandGenerator.hpp"
#include "plato/utilities/STKUtilities.hpp"

namespace plato::utilities::unittest
{
TEST(STKGenerateMesh, Box)
{
    ASSERT_EQ(stk::parallel_machine_size(MPI_COMM_WORLD), 1);

    constexpr std::string_view fileName = "test.exo";
    const STKCommandGenerator tSTKCommandGenerator{{3, 3, 4}, {-1, -2, -1}, {2, 1, 2}, STKCommandElementType::Hex};
    auto bulk = create_mesh(tSTKCommandGenerator.toString());
    write_mesh(fileName, bulk);

    EXPECT_EQ(tSTKCommandGenerator.numberOfNodes(), read_mesh_node_size(fileName));
    EXPECT_EQ(tSTKCommandGenerator.numberOfElements(), element_size(fileName));

    EXPECT_TRUE(std::filesystem::exists(fileName));
    EXPECT_TRUE(std::filesystem::remove(fileName));
}

TEST(STKUtilities, NumberOfNodesAndElementsFromBulk)
{
    const STKCommandGenerator tSTKCommandGenerator{{2, 2, 2}};
    const auto tMesh = create_mesh(tSTKCommandGenerator.toString());
    ASSERT_TRUE(tMesh);
    EXPECT_EQ(node_size(*tMesh), tSTKCommandGenerator.numberOfNodes());
    EXPECT_EQ(element_size(*tMesh), tSTKCommandGenerator.numberOfElements());
}

TEST(STKUtilities, SpatialDimensions)
{
    const STKCommandGenerator tSTKCommandGenerator{{2, 2, 2}};
    const auto tMesh = create_mesh(tSTKCommandGenerator.toString());
    ASSERT_TRUE(tMesh);
    EXPECT_EQ(spatial_dimensions(*tMesh), 3u);
}

TEST(STKUtilities, ReadCoordinates)
{
    const STKCommandGenerator tSTKCommandGenerator;
    const std::vector<double> gold = {0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1};
    const auto tMesh = create_mesh(tSTKCommandGenerator.toString());
    ASSERT_TRUE(tMesh);
    const auto res = nodal_coordinates(*tMesh);
    EXPECT_EQ(gold, res);
    EXPECT_EQ(node_size(*tMesh), tSTKCommandGenerator.numberOfNodes());
}

TEST(STKUtilities, WriteDensityField)
{
    constexpr std::string_view tInputFileName = "brick.exo";
    write_mesh(tInputFileName, create_mesh(STKCommandGenerator{}.toString()));
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
    const STKCommandGenerator tSTKCommandGenerator{{2, 2, 2}};
    write_mesh(tInputFileName, create_mesh(tSTKCommandGenerator.toString()));
    const std::vector<double> data = {1, 2, 3, 4, 5, 6, 7, 8};
    constexpr std::string_view tOutputFileName = "brick-out.exo";
    write_element_density(tInputFileName, data, tOutputFileName);
    auto res = read_element_density(tOutputFileName);

    EXPECT_EQ(data, res);

    EXPECT_TRUE(std::filesystem::remove(tInputFileName));
    EXPECT_TRUE(std::filesystem::remove(tOutputFileName));
}

}  // namespace plato::utilities::unittest
