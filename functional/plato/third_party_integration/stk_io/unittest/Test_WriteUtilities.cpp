#include <gtest/gtest.h>
#include <mpi.h>

#include <cmath>
#include <filesystem>
#include <numeric>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Comm.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_util/parallel/Parallel.hpp>
#include <string_view>

#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshIOHelpers.hpp"

namespace plato::third_party_integration::stk_io::unittest
{
namespace
{

constexpr auto kTopologyFieldName = std::string_view{"topology"};
constexpr auto kFixedValue = double{42.0};

struct MapField
{
    std::unordered_map<std::size_t, double> mData;

    auto operator()(const std::size_t aGlobalIndex) const
    {
        const auto tDensityIterator = mData.find(aGlobalIndex);
        return tDensityIterator != mData.end() ? tDensityIterator->second : kFixedValue;
    };
};

void check_write_scalar_field(const std::filesystem::path& aInputFileName,
                              const std::unordered_map<std::size_t, double>& aData,
                              const std::filesystem::path& aOutputFileName,
                              const std::vector<double>& aExpected,
                              const plato::test_utilities::TestContext& aTestContext)
{
    constexpr auto tFieldName = std::string_view{"Topology"};
    // Nodal
    {
        write_bulk_data(aInputFileName, generate_bulk_data(CommandGenerator{}));
        test_utilities::write_nodal_scalar_field(aInputFileName, MapField{aData}, tFieldName, aOutputFileName);
        const auto tResult = test_utilities::read_nodal_field_as_vector(aOutputFileName, kTopologyFieldName);
        EXPECT_EQ(tResult, aExpected) << aTestContext;
    }
    // Element
    {
        write_bulk_data(aInputFileName, generate_bulk_data(CommandGenerator{{2, 2, 2}}));
        test_utilities::write_element_scalar_field(aInputFileName, MapField{aData}, tFieldName, aOutputFileName);
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

TEST(WriteUtilities, WriteTwoFields)
{
    constexpr auto tInputFileName = std::string_view{"brick.exo"};
    write_bulk_data(tInputFileName, generate_bulk_data(CommandGenerator{}));

    constexpr auto tField1Name = std::string_view{"aardvark"};
    constexpr auto tNumberOfNodes = 8U;
    auto tExpectedField1 = std::vector<double>(tNumberOfNodes);
    std::iota(tExpectedField1.begin(), tExpectedField1.end(), 1.0);
    const auto tData1 = std::unordered_map<std::size_t, double>{
        {1, tExpectedField1[0]}, {2, tExpectedField1[1]}, {3, tExpectedField1[2]}, {4, tExpectedField1[3]},
        {5, tExpectedField1[4]}, {6, tExpectedField1[5]}, {7, tExpectedField1[6]}, {8, tExpectedField1[7]}};

    constexpr auto tField2Name = std::string_view{"anteater"};
    constexpr auto tElementValue = 8.0;
    const auto tData2 = std::unordered_map<std::size_t, double>{{1, tElementValue}};

    constexpr auto tOutputFileName = std::string_view{"brick-out.exo"};
    {
        const auto tIOBroker = create_io_mesh_broker(tInputFileName);
        const auto tFileHandle = create_output_mesh(tOutputFileName, *tIOBroker);
        write_nodal_scalar_field(*tIOBroker, MapField{tData1}, tField1Name, tFileHandle);
        write_element_scalar_field(*tIOBroker, MapField{tData2}, tField2Name, tFileHandle);
        finalize_mesh_data(*tIOBroker, tFileHandle);
    }

    const auto tReadField1 = test_utilities::read_nodal_field_as_vector(tOutputFileName, tField1Name);
    EXPECT_EQ(tReadField1, tExpectedField1);

    const auto tReadField2 = test_utilities::read_element_field_as_vector(tOutputFileName, tField2Name);
    ASSERT_EQ(tReadField2.size(), 1U);
    EXPECT_EQ(tReadField2.front(), tElementValue);

    std::filesystem::remove(tInputFileName);
    std::filesystem::remove(tOutputFileName);
}

}  // namespace plato::third_party_integration::stk_io::unittest
