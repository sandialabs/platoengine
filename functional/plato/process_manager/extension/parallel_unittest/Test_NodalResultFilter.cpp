#include <gtest/gtest.h>

#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/filter/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/process_manager/extension/NodalResultFilter.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshWithFieldWriter.hpp"
#include "plato/utilities/IndexRange.hpp"

namespace plato::process_manager::extension::parallel_unittest
{
namespace
{
const auto kMeshPath =
    geometry::extension::test_utilities::create_valid_density_topology_geometry_input().mesh_name.value().mToken;

class NodalResultFilterRunFixture : public third_party_integration::stk_io::test_utilities::MeshWithNodalDensities
{
   public:
    NodalResultFilterRunFixture() : MeshWithNodalDensities{kMeshPath, geometry::extension::density_mesh_field_name()} {}
};

void run_and_check_output_files(const std::filesystem::path& aMeshName, const unsigned int aNumberOfProcessorsForFilter)
{
    auto tGeometryInput = geometry::extension::test_utilities::create_valid_density_topology_geometry_input();
    tGeometryInput.mesh_name = input_parser::FileName{aMeshName};
    tGeometryInput.output_name = input_parser::FileName{aMeshName};

    auto tFilterInput = filter::extension::test_utilities::create_valid_kernel_filter_input();
    tFilterInput.number_of_processors = aNumberOfProcessorsForFilter;

    auto tCriteriaInput = input_parser::ParsedInput{};
    for ([[maybe_unused]] const auto tIndex : utilities::IndexRange{boost::mpi::communicator{}.size()})
    {
        tCriteriaInput = tCriteriaInput | criteria::library::test_utilities::create_valid_example_objective_input();
    }

    const auto tInput = tCriteriaInput | input_parser::nodal_result_filter{} | tGeometryInput | tFilterInput;
    const auto tValidatedInput = input_validation::make_validated_input(tInput);

    ASSERT_TRUE(tValidatedInput.hasValue()) << tValidatedInput.error();

    const auto tElementToNodeInput =
        tValidatedInput.value().get<input_parser::ComponentType::kProcessManager>().rawInput().front();
    EXPECT_NO_THROW(NodalResultFilter{tElementToNodeInput}.run());

    // Retrieve nodal fields names from the mesh and check that the expected field name is found.
    const auto tMeshRetrieval = mesh::EntityCounts{mesh::Mesh{aMeshName}};
    EXPECT_TRUE(tMeshRetrieval.hasNodalFieldVariable(NodalResultFilter::field_name())) << "New field was not written";
    EXPECT_TRUE(tMeshRetrieval.hasNodalFieldVariable(geometry::extension::density_mesh_field_name()))
        << "Original field is not present";

    const auto tExpectedTimeSteps = std::vector{1.0};
    EXPECT_EQ(mesh::EntityCounts{tMeshRetrieval}.timeSteps(), tExpectedTimeSteps);

    boost::mpi::communicator{}.barrier();
}

}  // namespace

TEST_F(NodalResultFilterRunFixture, CreateAndRunUseAllRanksInFilter)
{
    const auto tNumberOfProcessorsForFilter = boost::mpi::communicator{}.size();
    run_and_check_output_files(mMeshName, tNumberOfProcessorsForFilter);
}

TEST_F(NodalResultFilterRunFixture, CreateAndRunUseFewerRanksInFilter)
{
    const auto tNumberOfProcessorsForFilter = boost::mpi::communicator{}.size() / 2;
    run_and_check_output_files(mMeshName, tNumberOfProcessorsForFilter);
}

}  // namespace plato::process_manager::extension::parallel_unittest
