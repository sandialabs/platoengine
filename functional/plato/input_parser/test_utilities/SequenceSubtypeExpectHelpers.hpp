#ifndef PLATO_INPUTPARSER_TESTUTILITIES_SEQUENCESUBTYPEEXPECTHELPERS
#define PLATO_INPUTPARSER_TESTUTILITIES_SEQUENCESUBTYPEEXPECTHELPERS

#include <gtest/gtest.h>

#include <boost/fusion/sequence/comparison/equal_to.hpp>
#include <string_view>

#include "plato/input_parser/SequenceSubtype.hpp"
#include "plato/input_parser/test_utilities/TestHelpers.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::input_parser::test_utilities
{
template <typename PlatoInputSequenceSubtype>
    requires plato::input_parser::kIsInputSequenceSubtype<PlatoInputSequenceSubtype>
void expect_valid_input(const std::string_view aInput,
                        const PlatoInputSequenceSubtype aGold,
                        const plato::test_utilities::TestContext& aTestContext)
{
    const auto [tResult, tSuccess] = test_utilities::parse_input<PlatoInputSequenceSubtype>(aInput);
    ASSERT_TRUE(tSuccess) << aTestContext;
    EXPECT_TRUE(boost::fusion::equal_to(aGold, tResult)) << aTestContext;
}

template <typename PlatoInputSequenceSubtype>
    requires plato::input_parser::kIsInputSequenceSubtype<PlatoInputSequenceSubtype>
void expect_invalid_input(const std::string_view aInput, const plato::test_utilities::TestContext& aTestContext)
{
    const auto [tResult, tSuccess] = test_utilities::parse_input<PlatoInputSequenceSubtype>(aInput);
    EXPECT_FALSE(tSuccess) << aTestContext;
}

}  // namespace plato::input_parser::test_utilities
#endif
