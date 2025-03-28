#include <gtest/gtest.h>

#include "plato/input_parser/ComponentType.hpp"
#include "plato/input_parser/CrossLinker.hpp"
#include "plato/input_parser/CrossReference.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputBlocks.hpp"

namespace
{
using FilterNewCrossReference = plato::input_parser::NewCrossReference<plato::input_parser::ComponentType::kFilter>;
}

// clang-format off
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         uninspired_thing,
                         (double, a_number, "help")
                         (FilterNewCrossReference, my_filter, "help")
                         (int, another_number, "help"))
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         uninspired_filter,
                         (double, filteriness, "help"))
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         very_inspired_filter,
                         (double, filteritude, "help"))

// clang-format on

namespace plato::input_parser::unittest
{
TEST(CrossLinker, CrossLinkUnspecified)
{
    const auto tCrossLinker = make_cross_linker<uninspired_thing>();
    auto tUninspiredInputBlock =
        InputDataBlock{/*.mComponentType=*/ComponentType::kGeometry,
                       /*.mBlockName=*/"uninspired_thing", /*.mInput=*/CrossReferencedInput{uninspired_thing{}}};

    ASSERT_FALSE(tUninspiredInputBlock.mInput.get<uninspired_thing>()
                     .my_filter.has_value());  // Make sure the cross-reference is empty

    constexpr auto tFilterValue = 42.0;
    const auto tFilterInputBlock = InputDataBlock{/*.mComponentType=*/
                                                  ComponentType::kFilter,
                                                  /*.mBlockName=*/"uninspired_filter",   /*.mInput=*/
                                                  CrossReferencedInput{uninspired_filter{/*.filteriness=*/
                                                                                         tFilterValue}}};
    const auto tParsedInput = NewParsedInput{{tUninspiredInputBlock, tFilterInputBlock}};
    tCrossLinker.crossLink(tUninspiredInputBlock, tParsedInput);

    const auto& tUninspiredInput = tUninspiredInputBlock.mInput.get<uninspired_thing>();

    ASSERT_TRUE(tUninspiredInputBlock.mInput.get<uninspired_thing>().my_filter.has_value());
    ASSERT_TRUE(tUninspiredInput.my_filter.value().mInputBlock.has_value());
    ASSERT_TRUE(tUninspiredInput.my_filter.value().mInputBlock.holds_expected_type<uninspired_filter>());
    ASSERT_TRUE(tUninspiredInput.my_filter.value().mInputBlock.get<uninspired_filter>().filteriness.has_value());
    EXPECT_EQ(tUninspiredInput.my_filter.value().mInputBlock.get<uninspired_filter>().filteriness.value(),
              tFilterValue);
}

TEST(CrossLinker, CrossLinkSpecified)
{
    const auto tCrossLinker = make_cross_linker<uninspired_thing>();
    const auto tUninspiredThing = uninspired_thing{
        /*.a_number=*/42.0, /*.my_filter=*/FilterNewCrossReference{/*.mName=*/"very_inspired_filter", {}},
        /*.another_number=*/13};

    auto tUninspiredInputBlock =
        InputDataBlock{/*.mComponentType=*/ComponentType::kGeometry,
                       /*.mBlockName=*/"uninspired_thing", /*.mInput=*/CrossReferencedInput{tUninspiredThing}};

    const auto tUninspiredFilterInputBlock = InputDataBlock{/*.mComponentType=*/
                                                            ComponentType::kFilter,
                                                            /*.mBlockName=*/"uninspired_filter",   /*.mInput=*/
                                                            CrossReferencedInput{uninspired_filter{/*.filteriness=*/
                                                                                                   42.0}}};
    constexpr auto tFilterValue = 13.0;
    const auto tVeryInspiredFilterInputBlock =
        InputDataBlock{/*.mComponentType=*/
                       ComponentType::kFilter,
                       /*.mBlockName=*/"very_inspired_filter",   /*.mInput=*/
                       CrossReferencedInput{very_inspired_filter{/*.filteritude=*/
                                                                 tFilterValue}}};

    const auto tParsedInput =
        NewParsedInput{{tUninspiredInputBlock, tUninspiredFilterInputBlock, tVeryInspiredFilterInputBlock}};
    tCrossLinker.crossLink(tUninspiredInputBlock, tParsedInput);

    const auto& tUninspiredInput = tUninspiredInputBlock.mInput.get<uninspired_thing>();

    ASSERT_TRUE(tUninspiredInputBlock.mInput.get<uninspired_thing>().my_filter.has_value());
    ASSERT_TRUE(tUninspiredInput.my_filter.value().mInputBlock.has_value());
    ASSERT_TRUE(tUninspiredInput.my_filter.value().mInputBlock.holds_expected_type<very_inspired_filter>());
    ASSERT_TRUE(tUninspiredInput.my_filter.value().mInputBlock.get<very_inspired_filter>().filteritude.has_value());
    EXPECT_EQ(tUninspiredInput.my_filter.value().mInputBlock.get<very_inspired_filter>().filteritude.value(),
              tFilterValue);
}

}  // namespace plato::input_parser::unittest
