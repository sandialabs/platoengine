#include <gtest/gtest.h>

#include "plato/core/ValidationUtilities.hpp"
#include "plato/filter/library/FilterFactory.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::filter::extension::unittest
{
TEST(FilterFactory, HelmholtzFilterThrows)
{
    // For the Helmholtz filter, the filter may be loaded depending on whether
    // or not the PA shared library is available. This checks if we can load it,
    // and if not, checks that we get the right exception type.
    const input_parser::ParsedInput tInput = plato::test_utilities::create_valid_density_topology_geometry() |
                                             plato::test_utilities::create_valid_helmholtz_filter() |
                                             plato::test_utilities::create_valid_example_objective() |
                                             plato::test_utilities::create_valid_example_rol_optimization();
    const auto tValidatedGeometry = process_manager::library::make_validated_input(tInput).geometry();

    namespace pff = plato::filter;

    bool tCreationSuccessful = false;
    bool tCorrectException = false;
    try
    {
        const auto& tGeometryInput =
            core::validated_variant_raw_input<input_parser::density_topology>(tValidatedGeometry);
        const pff::library::FilterFunction tFunction =
            pff::library::make_filter_function(plato::geometry::library::get_cross_referenced_filter(tGeometryInput));
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
}
}  // namespace plato::filter::extension::unittest