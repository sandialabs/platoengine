#include <gtest/gtest.h>

#include "plato/input_parser/CrossLinkedInput.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/ParsedInput.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/input_validation/ValidatedInputFieldAccessor.hpp"

// clang-format off
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         sea_creatures, plato::input_parser::ComponentType::kFilter,
                         (double, octopus, "")
                         (unsigned int, squid, ""))
// clang-format on

namespace plato::input_validation::unittest
{
TEST(ValidatedInputFieldAccessor, GetField)
{
    const auto tInputBlock = plato::input_parser::sea_creatures{/*.octopus=*/42.0, /*.squid=*/100};
    const auto tParsedInput = std::vector<input_parser::InputDataBlock>{input_parser::InputDataBlock{
        input_parser::ComponentType::kFilter, "sea_creatures", input_parser::CrossReferencedInput{tInputBlock}}};
    const auto tCrossLinkedInput = input_parser::make_cross_linked_input(input_parser::NewParsedInput{tParsedInput});
    ASSERT_TRUE(tCrossLinkedInput.hasValue());
    const auto tValidatedInput = make_validated_input(tCrossLinkedInput.value());
    ASSERT_TRUE(tValidatedInput.hasValue()) << tValidatedInput.error();

    const auto tSquidAccessor = ValidatedInputFieldAccessor<unsigned int, input_parser::ComponentType::kFilter>{
        [](const input_parser::sea_creatures& aInputBlock) -> const unsigned int&
        { return aInputBlock.squid.value(); }};

    EXPECT_EQ(tSquidAccessor.getField(tValidatedInput.value().get<input_parser::ComponentType::kFilter>()), 100);
}
}  // namespace plato::input_validation::unittest
