#include <gtest/gtest.h>
#include <mpi.h>

#include <cmath>
#include <filesystem>
#include <numeric>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Comm.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_util/parallel/Parallel.hpp>
#include <string_view>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshIOHelpers.hpp"  //read_nodal_field_as_vector

namespace plato::third_party_integration::stk_io::unittest
{
namespace
{

constexpr auto kTopologyFieldName = std::string_view{"topology"};
constexpr auto kFixedValue = double{42.0};

void check_write_scalar_field(const std::filesystem::path& aInputFileName,
                              const std::unordered_map<std::size_t, double>& aData,
                              const std::filesystem::path& aOutputFileName,
                              const std::vector<double>& aExpected,
                              const plato::test_utilities::TestContext& aTestContext)
{
    constexpr auto tFieldName = std::string_view{"Topology"};
    const auto tScalarFieldFunction = [&aData](const std::size_t aGlobalIndex)
    {
        const auto tDensityIterator = aData.find(aGlobalIndex);
        return tDensityIterator != aData.end() ? tDensityIterator->second : kFixedValue;
    };
    // Nodal
    {
        write_bulk_data(aInputFileName, generate_bulk_data(CommandGenerator{}));
        write_nodal_scalar_field(aInputFileName, tScalarFieldFunction, tFieldName, aOutputFileName);
        const auto tResult = test_utilities::read_nodal_field_as_vector(aOutputFileName, kTopologyFieldName);
        EXPECT_EQ(tResult, aExpected) << aTestContext;
    }
    // Element
    {
        write_bulk_data(aInputFileName, generate_bulk_data(CommandGenerator{{2, 2, 2}}));
        write_element_scalar_field(aInputFileName, tScalarFieldFunction, tFieldName, aOutputFileName);
        const auto tResult = test_utilities::read_element_field_as_vector(aOutputFileName, kTopologyFieldName);
        EXPECT_EQ(tResult, aExpected) << aTestContext;
    }
    EXPECT_TRUE(std::filesystem::remove(aInputFileName)) << aTestContext;
    EXPECT_TRUE(std::filesystem::remove(aOutputFileName)) << aTestContext;
}

}  // namespace

TEST(WriteUtilities, CommandGeneratorWriteMeshToDisk)
{
    const CommandGenerator tCommandGenerator{{2, 2, 2}};
    const std::string_view tMeshFileName{"mesh.exo"};
    write_mesh(tMeshFileName, tCommandGenerator);
    EXPECT_TRUE(std::filesystem::exists(tMeshFileName));
    EXPECT_TRUE(std::filesystem::remove(tMeshFileName));
}

TEST(WriteUtilities, NumberOfNodesAndElementsFromBulk)
{
    const CommandGenerator tCommandGenerator{{2, 2, 2}};
    const auto tMesh = generate_bulk_data(tCommandGenerator);
    ASSERT_TRUE(tMesh);
    EXPECT_EQ(node_size(*tMesh), tCommandGenerator.numberOfNodes());
    EXPECT_EQ(element_size(*tMesh), tCommandGenerator.numberOfElements());
}

TEST(WriteUtilities, WriteDensityFieldAllValuesExist)
{
    constexpr std::string_view tInputFileName = "brick.exo";
    const auto tData = std::unordered_map<std::size_t, double>{{1, 1.0}, {2, 2.0}, {3, 3.0}, {4, 4.0},
                                                               {5, 5.0}, {6, 6.0}, {7, 7.0}, {8, 8.0}};

    auto tExpected = std::vector<double>(tData.size());
    std::iota(tExpected.begin(), tExpected.end(), 1.0);
    constexpr std::string_view tOutputFileName = "brick-out.exo";

    check_write_scalar_field(tInputFileName, tData, tOutputFileName, tExpected,
                             TEST_CONTEXT("All density values exist"));
}

TEST(WriteUtilities, WriteDensityFieldSomeMissing)
{
    constexpr std::string_view tInputFileName = "brick.exo";
    const auto tData =
        std::unordered_map<std::size_t, double>{{1, 1.0}, {3, 3.0}, {5, 5.0}, {6, 6.0}, {7, 7.0}, {8, 8.0}};
    const auto tMissingGlobalIDs = std::vector<std::size_t>{2U, 4U};

    auto tExpected = std::vector<double>(tData.size() + tMissingGlobalIDs.size());
    std::iota(tExpected.begin(), tExpected.end(), 1.0);
    for (const auto tMissingID : tMissingGlobalIDs)
    {
        tExpected.at(tMissingID - 1) = kFixedValue;
    }
    constexpr std::string_view tOutputFileName = "brick-out.exo";

    check_write_scalar_field(tInputFileName, tData, tOutputFileName, tExpected,
                             TEST_CONTEXT("Some missing density field values"));
}

}  // namespace plato::third_party_integration::stk_io::unittest
