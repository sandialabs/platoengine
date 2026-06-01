#include <gtest/gtest.h>

#include <boost/none.hpp>

#include "plato/components/ComponentType.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/filter/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/OutputUtilities.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/process_manager/extension/InitialGeometryWriter.hpp"
#include "plato/process_manager/extension/NodalResultFilter.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::integration_tests::unittest
{
namespace
{
using NodalResultFilterMesh = third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;
}

TEST_F(NodalResultFilterMesh, DensityTopology)
{
    auto tGeometryInput = geometry::extension::test_utilities::create_valid_density_topology_geometry_input();
    tGeometryInput.mesh_name = input_parser::FileName{mMeshFilePath};
    const auto tFilteredMeshName = std::filesystem::path{"nodal_filter_result.exo"};
    const auto tVolumeObjective = input_parser::objective{/*.name=*/std::string{"volume_criterion"},
                                                          /*.active=*/true,
                                                          /*.app=*/input_parser::AppName{"platoengine"},
                                                          /*.criterion=*/input_parser::CriterionName{"volume"},
                                                          /*.number_of_processors=*/1U,
                                                          /*.input_files=*/boost::none,
                                                          /*.aggregation_weight=*/1.0,
                                                          /*.normalize_by_initial_value=*/false,
                                                          /*.objective_goal=*/boost::none};

    const auto tKernelFilterInput = filter::extension::test_utilities::create_valid_kernel_filter_input();
    const auto tInitialGeometryWriterInput = input_parser::initial_geometry_writer{};
    const auto tNodalResultFilterInput =
        input_parser::nodal_result_filter{boost::none, boost::none, input_parser::FileName{tFilteredMeshName}};

    const auto tInput =
        tGeometryInput | tKernelFilterInput | tVolumeObjective | tInitialGeometryWriterInput | tNodalResultFilterInput;
    const auto tValidatedInput = input_validation::make_validated_input(tInput);

    const auto tProblem = process_manager::library::make_process_manager_data(tValidatedInput.value());
    const auto tAllProcessManagerInputs = tValidatedInput.value().get<components::ComponentType::kProcessManager>();
    ASSERT_EQ(tAllProcessManagerInputs.rawInput().size(), 2U);

    plato::process_manager::extension::InitialGeometryWriter::run(tProblem);
    const auto tNodalResultFilter =
        process_manager::extension::NodalResultFilter{tAllProcessManagerInputs.rawInput().back()};
    tNodalResultFilter.run();

    test_utilities::test_for_existence_and_remove(
        {tFilteredMeshName, tGeometryInput.output_name->mToken, geometry::extension::restart_file_name(tGeometryInput)},
        TEST_CONTEXT("Check for nodal result file."));
}
}  // namespace plato::integration_tests::unittest
