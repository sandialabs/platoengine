#include "plato/process_manager/extension/test_utilities/NodalResultFilterUtilities.hpp"

#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <optional>
#include <vector>

#include "plato/components/ComponentType.hpp"
#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/filter/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/extension/OutputUtilities.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshFieldAppender.hpp"
#include "plato/process_manager/extension/NodalResultFilter.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshWithFieldWriter.hpp"
#include "plato/utilities/IndexRange.hpp"
#include "plato/utilities/MPIUtilities.hpp"

namespace plato::process_manager::extension::test_utilities
{
namespace
{
const auto kTimeSteps = std::vector{1.0, 10.0};
const auto kDensityTopologyInput = geometry::extension::test_utilities::create_valid_density_topology_geometry_input();
const auto kMeshPath = kDensityTopologyInput.output_name->mToken;
const auto kRestartMeshPath = geometry::extension::restart_file_name(kDensityTopologyInput);

}  // namespace

NodalResultFilterRunFixture::NodalResultFilterRunFixture()
    : MeshWithNodalDensities{kMeshPath, geometry::extension::density_mesh_field_name()}
{
    utilities::execute_on_root(
        boost::mpi::communicator{},
        [this]()
        {
            // Add another field time step
            const auto tMesh = mesh::Mesh{mMeshName};
            const auto tNodalField =
                mesh::EntityRetrieval{tMesh}.designDomainNodalField(geometry::extension::density_mesh_field_name());
            const auto tNodalAnalysisMesh = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
                mesh::NodalFieldVectorReference{tNodalField});

            constexpr auto tFixedValue = geometry::extension::density_fixed_value();
            mesh::MeshFieldAppender{tMesh, kTimeSteps.back()}.addFieldFromAnalysisDomainMesh(
                tNodalAnalysisMesh, geometry::extension::density_mesh_field_name(), tFixedValue);
            copy_file(mMeshName, mDirectory.directory() / kRestartMeshPath, std::filesystem::copy_options::none);
        });
}

NodalResultFilterRunFixture::~NodalResultFilterRunFixture()
{
    utilities::execute_on_root(boost::mpi::communicator{},
                               [this]() { std::filesystem::remove(mDirectory.directory() / kRestartMeshPath); });
}

void NodalResultFilterRunFixture::runAndCheckNodalFilterResult(
    const std::optional<std::filesystem::path>& aNodalResultOutMeshPath,
    const plato::test_utilities::TestContext& aTestContext,
    const unsigned int aNumberOfProcessorsForFilter)
{
    auto tGeometryInput = kDensityTopologyInput;
    tGeometryInput.output_name = input_parser::FileName{mDirectory.directory() / kMeshPath};
    tGeometryInput.mesh_name = input_parser::FileName{mDirectory.directory() / kMeshPath};

    auto tKernelFilter = filter::extension::test_utilities::create_valid_kernel_filter_input();
    tKernelFilter.number_of_processors = aNumberOfProcessorsForFilter;

    auto tNodalResultFilter = input_parser::nodal_result_filter{};
    if (aNodalResultOutMeshPath)
    {
        tNodalResultFilter.output_file_name = input_parser::FileName{aNodalResultOutMeshPath.value()};
    }

    auto tCriteriaInput = input_parser::ParsedInput{};
    for ([[maybe_unused]] const auto tIndex : utilities::IndexRange{boost::mpi::communicator{}.size()})
    {
        tCriteriaInput = tCriteriaInput | criteria::library::test_utilities::create_valid_example_objective_input();
    }

    const auto tParsedInput = tCriteriaInput | tNodalResultFilter | tGeometryInput | tKernelFilter;
    const auto tValidatedInput = input_validation::make_validated_input(tParsedInput);
    ASSERT_TRUE(tValidatedInput.hasValue()) << tValidatedInput.error();

    const auto tElementToNodeInput =
        tValidatedInput.value().get<components::ComponentType::kProcessManager>().rawInput().front();
    EXPECT_NO_THROW(NodalResultFilter{tElementToNodeInput}.run()) << aTestContext;
    // Retrieve nodal fields names from the mesh and check that the expected field name is found.
    const auto tMeshRetrieval =
        mesh::EntityCounts{mesh::Mesh{aNodalResultOutMeshPath.value_or(mDirectory.directory() / kRestartMeshPath)}};
    EXPECT_TRUE(tMeshRetrieval.hasNodalFieldVariable(NodalResultFilter::fieldName())) << aTestContext;
    EXPECT_TRUE(tMeshRetrieval.hasNodalFieldVariable(geometry::extension::density_mesh_field_name())) << aTestContext;
    EXPECT_EQ(mesh::EntityCounts{tMeshRetrieval}.timeSteps(), kTimeSteps) << aTestContext;
}

}  // namespace plato::process_manager::extension::test_utilities
