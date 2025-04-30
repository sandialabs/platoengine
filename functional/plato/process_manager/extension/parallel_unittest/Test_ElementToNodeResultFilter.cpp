#include <gtest/gtest.h>

#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/filter/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/process_manager/extension/ElementToNodeResultFilter.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshWithFieldWriter.hpp"

namespace plato::process_manager::extension::parallel_unittest
{
namespace
{
const auto kMeshPath =
    geometry::extension::test_utilities::create_valid_density_topology_geometry_input().mesh_name.value().mToken;

class ElementToNodeResultFilterRunFixture
    : public third_party_integration::stk_io::test_utilities::MeshWithNodalDensities
{
   public:
    ElementToNodeResultFilterRunFixture()
        : MeshWithNodalDensities{kMeshPath, geometry::extension::density_mesh_field_name()}
    {
    }
};
}  // namespace

TEST_F(ElementToNodeResultFilterRunFixture, CreateAndRunNoExternalMesh)
{
    auto tGeometryInput = geometry::extension::test_utilities::create_valid_density_topology_geometry_input();
    const auto tOutputMeshPath = mMeshName;
    tGeometryInput.mesh_name = input_parser::FileName{mMeshName};
    tGeometryInput.output_name = input_parser::FileName{tOutputMeshPath};

    const auto tInput = input_parser::element_to_node_result_filter{} | tGeometryInput |
                        criteria::library::test_utilities::create_valid_example_objective_input() |
                        criteria::library::test_utilities::create_valid_example_objective_input() |
                        filter::extension::test_utilities::create_valid_kernel_filter_input();
    const auto tValidatedInput = input_validation::make_validated_input(tInput);
    ASSERT_TRUE(tValidatedInput.hasValue()) << tValidatedInput.error();

    const auto tElementToNodeInput =
        tValidatedInput.value().get<input_parser::ComponentType::kProcessManager>().rawInput().front();
    EXPECT_NO_THROW(ElementToNodeResultFilter{tElementToNodeInput}.run(
        library::make_process_manager_data(tValidatedInput.value())));

    // Retrieve nodal fields names from the mesh and check that the expected field name is found.
    const auto tMeshRetrieval = mesh::EntityRetrieval{mesh::Mesh{tOutputMeshPath}};
    EXPECT_TRUE(tMeshRetrieval.hasNodalField(ElementToNodeResultFilter::field_name())) << "New field was not written";
    EXPECT_TRUE(tMeshRetrieval.hasNodalField(geometry::extension::density_mesh_field_name()))
        << "Original field is not present";
    boost::mpi::communicator{}.barrier();
}
}  // namespace plato::process_manager::extension::parallel_unittest
