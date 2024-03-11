#include <gtest/gtest.h>

#include "plato/filter/library/FilterFactory.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::filter::extension::unittest
{
TEST(FilterFactory, HelmholtzFilterThrows)
{
    // For the Helmholtz filter, the filter may be loaded depending on whether
    // or not the PA shared library is available. This checks if we can load it,
    // and if not, checks that we get the right exception type.
    namespace pff = plato::filter;

    auto tDensityTopology = input_parser::density_topology{};
    tDensityTopology.filter_type = input_parser::FilterTypes::kHelmholtz;
    bool tCreationSuccessful = false;
    bool tCorrectException = false;
    try
    {
        const pff::library::FilterFunction tFunction = pff::library::make_filter_function(tDensityTopology);
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