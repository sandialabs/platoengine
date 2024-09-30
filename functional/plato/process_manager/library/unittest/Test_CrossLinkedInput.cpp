#include <gtest/gtest.h>

#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/process_manager/library/CrossLinkedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::process_manager::library::unittest
{
namespace
{
using FilterVariant =
    std::variant<input_parser::kernel_filter, input_parser::helmholtz_filter, input_parser::identity_filter>;

void test_cross_linked_filter(const CrossLinkedInput& aCrossLinkedInput,
                              const test_utilities::TestContext& aTestContext)
{
    ASSERT_TRUE(aCrossLinkedInput.rawInput().mDensityTopology->filter) << aTestContext;
    auto tFilterCrossReference = aCrossLinkedInput.rawInput().mDensityTopology->filter->mInputBlock;
    EXPECT_TRUE(tFilterCrossReference.has_value()) << aTestContext;
    EXPECT_TRUE(tFilterCrossReference.holds_expected_type<FilterVariant>()) << aTestContext;
}

}  // namespace

TEST(MakeCrossLinkedInput, LinksDensityTopologyToOnlyFilter)
{
    input_parser::ParsedInput tInput =
        test_utilities::create_valid_density_topology_geometry() | test_utilities::create_valid_helmholtz_filter();
    ASSERT_FALSE(tInput.mDensityTopology->filter);

    test_cross_linked_filter(make_cross_linked_input(tInput), TEST_CONTEXT("Only one filter available"));
}

TEST(MakeCrossLinkedInput, LinksDensityTopologyToSpecifiedFilter)
{
    input_parser::ParsedInput tInput = test_utilities::create_valid_density_topology_geometry() |
                                       test_utilities::create_valid_helmholtz_filter() |
                                       test_utilities::create_valid_identity_filter();
    ASSERT_FALSE(tInput.mDensityTopology->filter);

    tInput.mDensityTopology->filter = input_parser::FilterCrossReference{"helmholtz_filter", {}};
    ASSERT_TRUE(tInput.mDensityTopology->filter);
    ASSERT_FALSE(tInput.mDensityTopology->filter->mInputBlock.has_value());

    test_cross_linked_filter(make_cross_linked_input(tInput),
                             TEST_CONTEXT("Filter specified when multiple are available"));
}
}  // namespace plato::process_manager::library::unittest