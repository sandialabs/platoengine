#include <gtest/gtest.h>

#include "plato/core/ValidationUtilities.hpp"
#include "plato/filter/library/FilterFactory.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::process_manager::library::unittest
{
TEST(FilterFactory, ValidIdentityFilterFromGeometry)
{
    namespace ptu = plato::test_utilities;
    const input_parser::ParsedInput tInput =
        ptu::create_valid_density_topology_geometry() | ptu::create_valid_identity_filter() |
        ptu::create_valid_example_objective() | ptu::create_valid_example_rol_optimization();

    const auto tValidatedGeometry = make_validated_input(tInput).geometry();
    const auto& tGeometryInput = core::validated_variant_raw_input<input_parser::density_topology>(tValidatedGeometry);
    EXPECT_NO_THROW(auto tUnused = plato::filter::library::make_filter_function(
                        plato::geometry::library::get_cross_referenced_filter(tGeometryInput)));
}
}  // namespace plato::process_manager::library::unittest