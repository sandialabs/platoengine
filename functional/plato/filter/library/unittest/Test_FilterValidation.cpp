#include <gtest/gtest.h>

#include "plato/core/ValidationRegistration.hpp"
#include "plato/filter/extension/HelmholtzFilter.hpp"
#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/filter/library/FilterValidation.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::filter::library::unittest
{
TEST(FilterValidation, TypeExists)
{
    auto tDensityTopology = plato::test_utilities::create_valid_density_topology_geometry();

    EXPECT_FALSE(validate_filter_type(tDensityTopology).has_value());
    tDensityTopology.filter_type = boost::none;
    EXPECT_TRUE(validate_filter_type(tDensityTopology).has_value());

    // Check in registered function list
    auto tErrorMessages = std::vector<std::string>{};
    tErrorMessages = core::validate(tDensityTopology, std::move(tErrorMessages));
    EXPECT_EQ(tErrorMessages.size(), 1);
}

}  // namespace plato::filter::library::unittest