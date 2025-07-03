#include <gtest/gtest.h>

#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/filter/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/filter/library/FilterFactory.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/library/GeometryFilterUtilities.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/input_validation/ValidationUtilities.hpp"
#include "plato/integration_tests/utilities/ValidInputTestFixture.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::integration_tests::serial
{
namespace
{
struct FilterFactoryTestFixture : public integration_tests::utilities::ValidInputTestFixture
{
};
}  // namespace

TEST_F(FilterFactoryTestFixture, HelmholtzFilterThrows)
{
    // For the Helmholtz filter, the filter may be loaded depending on whether
    // or not the PA shared library is available. This checks if we can load it,
    // and if not, checks that we get the right exception type.
    const auto tInput = geometry::extension::test_utilities::create_valid_density_topology_geometry_input() |
                        filter::extension::test_utilities::create_valid_helmholtz_filter_input() |
                        criteria::library::test_utilities::create_valid_example_objective_input() |
                        process_manager::extension::test_utilities::create_valid_example_rol_optimization_input();

    // make mesh for validation of density_topology
    const auto tMeshFileName =
        std::filesystem::path{tInput.get<input_parser::density_topology>().front().mesh_name.value().mToken};
    const auto tCommandGenerator = third_party_integration::stk_io::CommandGenerator{
        {2, 2, 2}, {-1, -1, -1}, {1, 1, 1}, third_party_integration::stk_io::CommandElementType::Hex};
    third_party_integration::stk_io::write_mesh(tMeshFileName, tCommandGenerator);

    const auto tValidatedGeometry =
        input_validation::make_validated_input(tInput).value().get<components::ComponentType::kGeometry>();

    namespace pff = plato::filter;

    bool tCreationSuccessful = false;
    bool tCorrectException = false;
    try
    {
        [[maybe_unused]] const auto tFunction =
            geometry::library::make_filter_from_geometry_input<input_parser::density_topology>(tValidatedGeometry);
        tCreationSuccessful = true;
    }
    catch (const plato::utilities::Exception&)
    {
        tCorrectException = true;
    }

    if (tCreationSuccessful)
    {
        EXPECT_FALSE(tCorrectException);
    }
    else
    {
        EXPECT_TRUE(tCorrectException);
    }
    std::filesystem::remove(tMeshFileName);
}
}  // namespace plato::integration_tests::serial
