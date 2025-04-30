#include <gtest/gtest.h>

#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/filter/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/MeshFieldAppender.hpp"
#include "plato/mesh/MeshFieldWriter.hpp"
#include "plato/process_manager/extension/NodalResultFilter.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/test_utilities/FileCreatingTestFixture.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshWithFieldWriter.hpp"

namespace plato::process_manager::extension::unittest
{
namespace
{
const auto kTimeSteps = std::vector{1.0, 10.0};

const auto kMeshPath =
    geometry::extension::test_utilities::create_valid_density_topology_geometry_input().mesh_name.value().mToken;

class NodalResultFilterValidationFixture : public test_utilities::FileCreatingTestFixture
{
   public:
    NodalResultFilterValidationFixture() : FileCreatingTestFixture{kMeshPath} {}
};

class NodalResultFilterRunFixture : public third_party_integration::stk_io::test_utilities::MeshWithNodalDensities
{
   public:
    NodalResultFilterRunFixture() : MeshWithNodalDensities{kMeshPath, geometry::extension::density_mesh_field_name()}
    {
        // Add another field time step
        const auto tMesh = mesh::Mesh{mMeshName};
        const auto tNodalField =
            mesh::EntityRetrieval{tMesh}.designDomainNodalField(geometry::extension::density_mesh_field_name());
        const auto tNodalAnalysisMesh = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
            mesh::NodalFieldVectorReference{tNodalField});

        constexpr auto tFixedValue = 1.0;
        mesh::MeshFieldAppender{tMesh, kTimeSteps.back()}.addFieldOnAnalysisDomainMesh(
            tNodalAnalysisMesh, geometry::extension::density_mesh_field_name(), tFixedValue);
    }
};

template <typename ComponentInput>
[[nodiscard]] auto cross_reference()
{
    return input_parser::CrossReference<input_parser::ComponentTypeOfInputBlock<ComponentInput>::value>{
        input_parser::block_name<ComponentInput>(), input_parser::InputBlockWrapper{ComponentInput{}}};
}
}  // namespace

TEST_F(NodalResultFilterRunFixture, CreateAndRunSeparateOutputFile)
{
    auto tGeometryInput = geometry::extension::test_utilities::create_valid_density_topology_geometry_input();
    tGeometryInput.mesh_name = input_parser::FileName{mMeshName};
    tGeometryInput.output_name = input_parser::FileName{mMeshName};

    auto tNodalResultFilter = input_parser::nodal_result_filter{};
    const auto tOutputMeshPath = mDirectory.directory() / std::filesystem::path{"output.exo"};
    tNodalResultFilter.output_file_name = input_parser::FileName{tOutputMeshPath};

    const auto tInput = tNodalResultFilter | tGeometryInput |
                        criteria::library::test_utilities::create_valid_example_objective_input() |
                        filter::extension::test_utilities::create_valid_kernel_filter_input();
    const auto tValidatedInput = input_validation::make_validated_input(tInput);
    ASSERT_TRUE(tValidatedInput.hasValue()) << tValidatedInput.error();

    const auto tElementToNodeInput =
        tValidatedInput.value().get<input_parser::ComponentType::kProcessManager>().rawInput().front();
    EXPECT_NO_THROW(NodalResultFilter{tElementToNodeInput}.run());

    // Retrieve nodal fields names from the mesh and check that the expected field name is found.
    const auto tMeshRetrieval = mesh::EntityCounts{mesh::Mesh{tOutputMeshPath}};
    EXPECT_TRUE(tMeshRetrieval.hasNodalFieldVariable(NodalResultFilter::field_name())) << "New field was not written";
    EXPECT_TRUE(tMeshRetrieval.hasNodalFieldVariable(geometry::extension::density_mesh_field_name()))
        << "Original field is not present";

    EXPECT_EQ(mesh::EntityCounts{tMeshRetrieval}.timeSteps(), kTimeSteps);
}

TEST(NodalResultFilter, ValidateFilterIsKernelFilter)
{
    {
        // Wrong filter
        auto tInput = input_parser::nodal_result_filter{};
        tInput.filter = cross_reference<input_parser::identity_filter>();
        EXPECT_TRUE(detail::validate_filter_is_kernel_filter(tInput));
    }
    {
        // Correct filter
        auto tInput = input_parser::nodal_result_filter{};
        tInput.filter = cross_reference<input_parser::kernel_filter>();
        const auto tErrorMessage = detail::validate_filter_is_kernel_filter(tInput);
        EXPECT_FALSE(tErrorMessage) << tErrorMessage.value();
    }
}

TEST_F(NodalResultFilterValidationFixture, ValidateFilterIsKernelFilter)
{
    // Via registered validation
    const auto tInput = input_parser::nodal_result_filter{} |
                        geometry::extension::test_utilities::create_valid_density_topology_geometry_input() |
                        criteria::library::test_utilities::create_valid_example_objective_input() |
                        filter::extension::test_utilities::create_valid_identity_filter_input();

    // Wrong filter
    const auto tValidatedInput = input_validation::make_validated_input(tInput);
    EXPECT_TRUE(tValidatedInput.hasError());
}

TEST(NodalResultFilter, ValidateGeometryIsDensityTopology)
{
    {
        // Wrong geometry
        auto tInput = input_parser::nodal_result_filter{};
        tInput.geometry = cross_reference<input_parser::brick_shape_geometry>();
        EXPECT_TRUE(detail::validate_geometry_is_density_topology(tInput));
    }
    {
        // Correct geometry
        auto tInput = input_parser::nodal_result_filter{};
        tInput.geometry = cross_reference<input_parser::density_topology>();
        const auto tErrorMessage = detail::validate_geometry_is_density_topology(tInput);
        EXPECT_FALSE(tErrorMessage) << tErrorMessage.value();
    }
}

TEST_F(NodalResultFilterValidationFixture, ValidateGeometryIsDensityTopology)
{
    // Via registered validation
    const auto tInput = input_parser::nodal_result_filter{} |
                        geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                        criteria::library::test_utilities::create_valid_example_objective_input() |
                        filter::extension::test_utilities::create_valid_kernel_filter_input();

    const auto tValidatedInput = input_validation::make_validated_input(tInput);
    EXPECT_TRUE(tValidatedInput.hasError());
}

TEST(NodalResultFilter, Registration)
{
    EXPECT_TRUE(library::is_process_manager_function_registered("nodal_result_filter"));
}
}  // namespace plato::process_manager::extension::unittest
