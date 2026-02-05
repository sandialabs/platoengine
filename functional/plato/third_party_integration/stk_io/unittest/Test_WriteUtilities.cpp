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
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshIOHelpers.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/Zip.hpp"

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
        write_bulk_data(aInputFileName, *generate_bulk_data(CommandGenerator{}));
        test_utilities::write_nodal_scalar_field(aInputFileName, MapField{aData}, tFieldName, aOutputFileName);
        const auto tResult = test_utilities::read_nodal_field_as_vector(aOutputFileName, kTopologyFieldName);
        EXPECT_EQ(tResult, aExpected) << aTestContext;
    }
    // Element
    {
        write_bulk_data(aInputFileName, *generate_bulk_data(CommandGenerator{{2, 2, 2}}));
        test_utilities::write_element_scalar_field(aInputFileName, MapField{aData}, tFieldName, aOutputFileName);
        const auto tResult = test_utilities::read_element_field_as_vector(aOutputFileName, kTopologyFieldName);
        EXPECT_EQ(tResult, aExpected) << aTestContext;
    }
    EXPECT_TRUE(std::filesystem::remove(aInputFileName)) << aTestContext;
    EXPECT_TRUE(std::filesystem::remove(aOutputFileName)) << aTestContext;
}

auto full_map_from_vector(const std::vector<double>& aVector) -> std::unordered_map<std::size_t, double>
{
    std::unordered_map<std::size_t, double> tMap;
    tMap.reserve(aVector.size());

    std::size_t tGlobalID = 1;
    std::transform(aVector.begin(), aVector.end(), std::inserter(tMap, tMap.begin()),
                   [&tGlobalID](const auto& tVal) {
                       return std::pair{tGlobalID++, tVal};
                   });
    return tMap;
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
    constexpr int tFieldSize = 8;
    auto tExpected = std::vector<double>(tFieldSize);
    std::iota(tExpected.begin(), tExpected.end(), 1.0);
    const auto tData = full_map_from_vector(tExpected);

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
    write_bulk_data(tInputFileName, *generate_bulk_data(CommandGenerator{}));

    constexpr auto tField1Name = std::string_view{"aardvark"};
    constexpr auto tNumberOfNodes = 8U;
    auto tExpectedField1 = std::vector<double>(tNumberOfNodes);
    std::iota(tExpectedField1.begin(), tExpectedField1.end(), 1.0);
    const auto tData1 = full_map_from_vector(tExpectedField1);

    constexpr auto tField2Name = std::string_view{"anteater"};
    constexpr auto tElementValue = 8.0;
    const auto tData2 = std::unordered_map<std::size_t, double>{{1, tElementValue}};

    constexpr auto tOutputFileName = std::string_view{"brick-out.exo"};
    {
        auto tIOBroker = create_io_broker_from_input_file(tInputFileName);
        const auto tFileHandle = create_output_mesh(tOutputFileName, *tIOBroker);

        initialize_nodal_scalar_field(*tIOBroker, tField1Name);
        populate_nodal_scalar_field_values(*tIOBroker, tField1Name, MapField{tData1});
        add_nodal_field_to_output_file(*tIOBroker, tFileHandle, tField1Name);

        initialize_element_scalar_field(*tIOBroker, tField2Name);
        populate_element_scalar_field_values(*tIOBroker, tField2Name, MapField{tData2});
        add_element_field_to_output_file(*tIOBroker, tFileHandle, tField2Name);

        write_fields_at_time(*tIOBroker, tFileHandle, /*aOutputTime=*/1.0);
    }

    const auto tReadField1 = test_utilities::read_nodal_field_as_vector(tOutputFileName, tField1Name);
    EXPECT_EQ(tReadField1, tExpectedField1);

    const auto tReadField2 = test_utilities::read_element_field_as_vector(tOutputFileName, tField2Name);
    ASSERT_EQ(tReadField2.size(), 1U);
    EXPECT_EQ(tReadField2.front(), tElementValue);

    std::filesystem::remove(tInputFileName);
    std::filesystem::remove(tOutputFileName);
}

TEST(WriteUtilities, WriteMultipleTimeSteps)
{
    // write mesh
    constexpr auto tInputFileName = std::string_view{"brick.exo"};
    constexpr auto tOutputFileName = std::string_view{"brick-out.exo"};
    write_bulk_data(tInputFileName, *generate_bulk_data(CommandGenerator{}));
    constexpr auto tNumberOfNodes = 8U;

    // define field for first time step
    constexpr auto tField1Name = std::string_view{"ether"};
    auto tExpectedField1 = std::vector<double>(tNumberOfNodes);
    std::iota(tExpectedField1.begin(), tExpectedField1.end(), 10.0);
    const auto tData1 = full_map_from_vector(tExpectedField1);

    // define field for second time step
    auto tExpectedField2 = std::vector<double>{54, 21, 86, 77, 88, 38, 71, 20};
    const auto tData2 = full_map_from_vector(tExpectedField2);

    // write inital time step
    constexpr double tTime1 = 21.0;
    {
        auto tIOBroker = create_io_broker_from_input_file(std::filesystem::path{tInputFileName});
        const size_t tOutputFileIndex =
            tIOBroker->create_output_mesh(std::filesystem::path{tOutputFileName}, stk::io::WRITE_RESULTS);

        initialize_nodal_scalar_field(*tIOBroker, tField1Name);
        populate_nodal_scalar_field_values(*tIOBroker, tField1Name, MapField{tData1});
        add_nodal_field_to_output_file(*tIOBroker, tOutputFileIndex, tField1Name);
        write_fields_at_time(*tIOBroker, tOutputFileIndex, tTime1);
    }

    // write next time step
    constexpr double tTime2 = 25.0;
    {
        const auto tBulkData =
            read_mesh_bulk_data(std::filesystem::path{tOutputFileName});  // read in bulk data from output mesh

        auto tIOBroker = create_io_broker_from_bulk(*tBulkData);
        const size_t tOutputFileIndex =
            tIOBroker->create_output_mesh(std::filesystem::path{tOutputFileName}, stk::io::APPEND_RESULTS);

        populate_nodal_scalar_field_values(*tIOBroker, tField1Name, MapField{tData2});
        add_nodal_field_to_output_file(*tIOBroker, tOutputFileIndex, tField1Name);
        write_fields_at_time(*tIOBroker, tOutputFileIndex, tTime2);
    }

    // check that all time steps are stored
    {
        auto tIOBroker = std::make_unique<stk::io::StkMeshIoBroker>(MPI_COMM_SELF);
        tIOBroker->add_mesh_database(std::string{tOutputFileName}, stk::io::READ_MESH);

        EXPECT_EQ(2, tIOBroker->get_num_time_steps());
    }

    // read and test fields
    const auto tReadField1 = test_utilities::read_nodal_field_as_vector(tOutputFileName, tField1Name, tTime1);
    EXPECT_EQ(tReadField1, tExpectedField1);

    const auto tReadField2 = test_utilities::read_nodal_field_as_vector(tOutputFileName, tField1Name, tTime2);
    EXPECT_EQ(tReadField2, tExpectedField2);
}

TEST(WriteUtilities, ReplaceNodalCoordinateValues)
{
    const CommandGenerator tCommandGenerator{{1, 1, 1}};
    auto tBulkData = generate_bulk_data(tCommandGenerator);
    const auto tOriginalCoordinates = nodal_coordinates(*tBulkData);

    const std::vector<common::Coordinate> tPerturbation{{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, {1.0, 1.0, 1.0},
                                                        {1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, {1.0, 1.0, 1.0},
                                                        {1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}};

    auto tNewCoordinates = utilities::reserved_container<std::vector<common::Coordinate>>(tOriginalCoordinates.size());
    std::transform(tOriginalCoordinates.begin(), tOriginalCoordinates.end(), tPerturbation.begin(),
                   std::back_inserter(tNewCoordinates),
                   [](const auto& aCoord1, const auto& aCoord2) { return aCoord1 + aCoord2; });

    replace_nodal_coordinate_values(*tBulkData, tNewCoordinates);
    const auto tUpdatedCoordinates = nodal_coordinates(*tBulkData);

    ASSERT_EQ(tUpdatedCoordinates.size(), tNewCoordinates.size());
    for (const auto& [tResult, tExpected] : utilities::Zip{tUpdatedCoordinates, tNewCoordinates})
    {
        common::test_utilities::test_double_equality_of_components(tResult, tExpected,
                                                                   TEST_CONTEXT("Replacing nodal coordinates"));
    }
}

}  // namespace plato::third_party_integration::stk_io::unittest
