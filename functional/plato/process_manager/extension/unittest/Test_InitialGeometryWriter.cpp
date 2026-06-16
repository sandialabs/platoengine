#include <gtest/gtest.h>

#include <boost/none.hpp>
#include <filesystem>
#include <vector>

#include "plato/components/ComponentType.hpp"
#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/filter/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/MeshFieldWriter.hpp"
#include "plato/process_manager/extension/InitialGeometryWriter.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::process_manager::extension::unittest
{
namespace
{
const auto kBackgroundMeshFilePath = utilities::data_file_path("four_tri_two_block.cdf");
const auto kDensityMeshName =
    geometry::extension::test_utilities::create_valid_density_topology_geometry_input().mesh_name.value().mToken;
const auto kLevelSetOutputName =
    geometry::extension::test_utilities::create_valid_level_set_topology_geometry_input().output_name.value().mToken;
const auto kDensityOutputName =
    geometry::extension::test_utilities::create_valid_density_topology_geometry_input().output_name.value().mToken;
const auto kUnfilteredDensityFieldName = geometry::extension::density_mesh_field_name();

void write_mesh_with_density_field()
{
    const auto tMesh = mesh::Mesh{kBackgroundMeshFilePath.value()};
    const auto tNodalField = std::vector<double>{1.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    const auto tNodalAnalysisMesh = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tNodalField});

    constexpr auto tFixedValue = geometry::extension::density_fixed_value();
    mesh::MeshFieldWriter{tMesh, std::filesystem::path{kDensityMeshName}}.addFieldFromAnalysisDomainMesh(
        tNodalAnalysisMesh, kUnfilteredDensityFieldName, tFixedValue);
}

void validate_and_run(const plato::input_parser::ParsedInput& aInput)
{
    write_mesh_with_density_field();

    const auto tValidatedInput = input_validation::make_validated_input(aInput);
    ASSERT_TRUE(tValidatedInput.hasValue()) << tValidatedInput.error();

    const auto tProblem = library::make_process_manager_data(tValidatedInput.value());

    const auto tAllProcessManagerInputs = tValidatedInput.value().get<components::ComponentType::kProcessManager>();
    ASSERT_EQ(tAllProcessManagerInputs.rawInput().size(), 1U);

    plato::process_manager::extension::InitialGeometryWriter::run(tProblem);

    std::filesystem::remove(kDensityMeshName);
}

}  // namespace

TEST(InitialGeometryWriter, GenerateOutputMeshLevelSetTO)
{
    auto tGeometryInput = geometry::extension::test_utilities::create_valid_level_set_topology_geometry_input();
    tGeometryInput.mesh_name = input_parser::FileName{kDensityMeshName};
    tGeometryInput.sphere_pattern = boost::none;
    tGeometryInput.initial_field_name =
        input_parser::IdentifierString{.mToken = std::string{kUnfilteredDensityFieldName}};

    const auto tInput = input_parser::initial_geometry_writer{} | tGeometryInput |
                        criteria::library::test_utilities::create_valid_example_objective_input() |
                        filter::extension::test_utilities::create_valid_kernel_filter_input();

    validate_and_run(tInput);

    plato::test_utilities::test_for_existence_and_remove({kLevelSetOutputName},
                                                         TEST_CONTEXT("Level set output: cut mesh"));
    plato::test_utilities::test_for_existence_and_remove(
        {"restart_" + kLevelSetOutputName},
        TEST_CONTEXT("Level set output: unfiltered level set field on background mesh"));
    plato::test_utilities::test_for_existence_and_remove(
        {"filtered_level_set_" + kLevelSetOutputName},
        TEST_CONTEXT("Level set output: filtered level set field on background mesh"));
}

TEST(InitialGeometryWriter, GenerateOutputMeshDensityTO)
{
    const auto tInput = input_parser::initial_geometry_writer{} |
                        geometry::extension::test_utilities::create_valid_density_topology_geometry_input() |
                        criteria::library::test_utilities::create_valid_example_objective_input() |
                        filter::extension::test_utilities::create_valid_kernel_filter_input();

    validate_and_run(tInput);

    plato::test_utilities::test_for_existence_and_remove({kDensityOutputName},
                                                         TEST_CONTEXT("Density output: filtered density field"));

    plato::test_utilities::test_for_existence_and_remove({"restart_" + kDensityOutputName},
                                                         TEST_CONTEXT("Density output: unfiltered density field"));
}

TEST(InitialGeometryWriter, Registration)
{
    EXPECT_TRUE(library::is_process_manager_function_registered("initial_geometry_writer"));
}
}  // namespace plato::process_manager::extension::unittest
