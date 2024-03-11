#include <gtest/gtest.h>

#include "plato/filter/library/FilterFactory.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_parser/InputEnumTypes.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::filter::extension::unittest
{
TEST(FilterFactory, KernelFilterThrows)
{
    auto tDensityTopology = input_parser::density_topology{};
    tDensityTopology.filter_type = input_parser::FilterTypes::kKernel;
    EXPECT_THROW(auto tFunction = plato::filter::library::make_filter_function(tDensityTopology),
                 plato::utilities::Exception);
}
}  // namespace plato::filter::extension::unittest