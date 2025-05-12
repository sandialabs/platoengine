#include "plato/process_manager/extension/test_utilities/NodalResultFilterUtilities.hpp"

#include <gtest/gtest.h>

#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/process_manager/extension/NodalResultFilter.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::process_manager::extension::test_utilities
{
void run_and_check_nodal_filter_result(const input_parser::ParsedInput& aParsedInput,
                                       const std::vector<double>& aExpectedTimeSteps,
                                       const std::filesystem::path& aMeshPath,
                                       const plato::test_utilities::TestContext& aTestContext)
{
    const auto tValidatedInput = input_validation::make_validated_input(aParsedInput);

    ASSERT_TRUE(tValidatedInput.hasValue()) << tValidatedInput.error();

    const auto tElementToNodeInput =
        tValidatedInput.value().get<input_parser::ComponentType::kProcessManager>().rawInput().front();
    EXPECT_NO_THROW(NodalResultFilter{tElementToNodeInput}.run()) << aTestContext;

    // Retrieve nodal fields names from the mesh and check that the expected field name is found.
    const auto tMeshRetrieval = mesh::EntityCounts{mesh::Mesh{aMeshPath}};
    EXPECT_TRUE(tMeshRetrieval.hasNodalFieldVariable(NodalResultFilter::field_name())) << aTestContext;
    EXPECT_TRUE(tMeshRetrieval.hasNodalFieldVariable(geometry::extension::density_mesh_field_name())) << aTestContext;

    EXPECT_EQ(mesh::EntityCounts{tMeshRetrieval}.timeSteps(), aExpectedTimeSteps) << aTestContext;
}

}  // namespace plato::process_manager::extension::test_utilities
