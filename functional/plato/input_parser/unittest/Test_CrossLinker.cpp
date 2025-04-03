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
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         no_cross_reference_thing,
                         (double, a_number, "help")
                         (int, another_number, "help"))
// clang-format on

namespace plato::input_parser::unittest
{
namespace
{
constexpr auto kUninspiredFilterValue = 42.0;
constexpr auto kVeryInspiredFilterValue = 13.0;

const auto kUninspiredFilterInputBlock =
    InputDataBlock{/*.mComponentType=*/
                   ComponentType::kFilter,
                   /*.mBlockName=*/"uninspired_filter",   /*.mInput=*/
                   CrossReferencedInput{uninspired_filter{/*.filteriness=*/
                                                          kUninspiredFilterValue}}};
const auto kVeryInspiredFilterInputBlock =
    InputDataBlock{/*.mComponentType=*/
                   ComponentType::kFilter,
                   /*.mBlockName=*/"very_inspired_filter",   /*.mInput=*/
                   CrossReferencedInput{very_inspired_filter{/*.filteritude=*/
                                                             kVeryInspiredFilterValue}}};

}  // namespace

TEST(CrossLinker, CrossLinkUnspecified)
{
    auto tUninspiredInputBlock =
        InputDataBlock{/*.mComponentType=*/ComponentType::kGeometry,
                       /*.mBlockName=*/"uninspired_thing", /*.mInput=*/CrossReferencedInput{uninspired_thing{}}};

    ASSERT_FALSE(tUninspiredInputBlock.mInput.get<uninspired_thing>()
                     .my_filter.has_value());  // Make sure the cross-reference is empty

    const auto tParsedInput = NewParsedInput{{tUninspiredInputBlock, kUninspiredFilterInputBlock}};

    const auto tCrossLinker = make_cross_linker<uninspired_thing>();
    const auto tCrossLinkedInputOrError = tCrossLinker.crossLink(std::move(tUninspiredInputBlock), tParsedInput);

    ASSERT_TRUE(tCrossLinkedInputOrError.hasValue());
    const auto& tUninspiredInput = tCrossLinkedInputOrError.value().mInput.get<uninspired_thing>();

    ASSERT_TRUE(tUninspiredInput.my_filter.has_value());
    ASSERT_TRUE(tUninspiredInput.my_filter.value().mInputBlock.has_value());
    ASSERT_TRUE(tUninspiredInput.my_filter.value().mInputBlock.holds_expected_type<uninspired_filter>());
    ASSERT_TRUE(tUninspiredInput.my_filter.value().mInputBlock.get<uninspired_filter>().filteriness.has_value());
    EXPECT_EQ(tUninspiredInput.my_filter.value().mInputBlock.get<uninspired_filter>().filteriness.value(),
              kUninspiredFilterValue);
}

TEST(CrossLinker, CrossLinkSpecified)
{
    const auto tUninspiredThing = uninspired_thing{
        /*.a_number=*/42.0, /*.my_filter=*/FilterNewCrossReference{/*.mName=*/"very_inspired_filter", {}},
        /*.another_number=*/13};

    auto tUninspiredInputBlock =
        InputDataBlock{/*.mComponentType=*/ComponentType::kGeometry,
                       /*.mBlockName=*/"uninspired_thing", /*.mInput=*/CrossReferencedInput{tUninspiredThing}};

    const auto tParsedInput =
        NewParsedInput{{tUninspiredInputBlock, kUninspiredFilterInputBlock, kVeryInspiredFilterInputBlock}};

    const auto tCrossLinker = make_cross_linker<uninspired_thing>();
    const auto tCrossLinkedInputOrError = tCrossLinker.crossLink(std::move(tUninspiredInputBlock), tParsedInput);

    ASSERT_TRUE(tCrossLinkedInputOrError.hasValue());
    const auto& tUninspiredInput = tCrossLinkedInputOrError.value().mInput.get<uninspired_thing>();

    ASSERT_TRUE(tUninspiredInput.my_filter.has_value());
    ASSERT_TRUE(tUninspiredInput.my_filter.value().mInputBlock.has_value());
    ASSERT_TRUE(tUninspiredInput.my_filter.value().mInputBlock.holds_expected_type<very_inspired_filter>());
    ASSERT_TRUE(tUninspiredInput.my_filter.value().mInputBlock.get<very_inspired_filter>().filteritude.has_value());
    EXPECT_EQ(tUninspiredInput.my_filter.value().mInputBlock.get<very_inspired_filter>().filteritude.value(),
              kVeryInspiredFilterValue);
}

TEST(CrossLinker, NoOpForTypeWithNoCrossReferences)
{
    const auto tNoCrossReferenceThing = no_cross_reference_thing{/*.a_number=*/42.0,
                                                                 /*.another_number=*/13};
    auto tNoCrossReferenceInputBlock = InputDataBlock{/*.mComponentType=*/ComponentType::kGeometry,
                                                      /*.mBlockName=*/"no_cross_reference_thing",
                                                      /*.mInput=*/CrossReferencedInput{tNoCrossReferenceThing}};

    const auto tParsedInput = NewParsedInput{{tNoCrossReferenceInputBlock, kUninspiredFilterInputBlock}};

    const auto tCrossLinkedInputOrError =
        make_cross_linker<no_cross_reference_thing>().crossLink(std::move(tNoCrossReferenceInputBlock), tParsedInput);
    EXPECT_TRUE(tCrossLinkedInputOrError.hasValue());
}

TEST(CrossLinker, ErrorMissingFieldName)
{
    auto tUninspiredInputBlock =
        InputDataBlock{/*.mComponentType=*/ComponentType::kGeometry,
                       /*.mBlockName=*/"uninspired_thing", /*.mInput=*/CrossReferencedInput{uninspired_thing{}}};

    const auto tParsedInput =
        NewParsedInput{{tUninspiredInputBlock, kUninspiredFilterInputBlock, kVeryInspiredFilterInputBlock}};

    const auto tCrossLinkedInputOrError =
        make_cross_linker<uninspired_thing>().crossLink(std::move(tUninspiredInputBlock), tParsedInput);

    EXPECT_TRUE(tCrossLinkedInputOrError.hasError());
}

TEST(CrossLinker, ErrorMissingLinkableComponents)
{
    auto tUninspiredInputBlock =
        InputDataBlock{/*.mComponentType=*/ComponentType::kGeometry,
                       /*.mBlockName=*/"uninspired_thing", /*.mInput=*/CrossReferencedInput{uninspired_thing{}}};

    const auto tParsedInput = NewParsedInput{{tUninspiredInputBlock}};

    const auto tCrossLinkedInputOrError =
        make_cross_linker<uninspired_thing>().crossLink(std::move(tUninspiredInputBlock), tParsedInput);

    EXPECT_TRUE(tCrossLinkedInputOrError.hasError());
}

TEST(CrossLinker, ErrorCrossLinkNameNotFound)
{
    const auto tUninspiredThing = uninspired_thing{
        /*.a_number=*/42.0, /*.my_filter=*/FilterNewCrossReference{/*.mName=*/"very_inspired_filter", {}},
        /*.another_number=*/13};
    auto tUninspiredInputBlock =
        InputDataBlock{/*.mComponentType=*/ComponentType::kGeometry,
                       /*.mBlockName=*/"uninspired_thing", /*.mInput=*/CrossReferencedInput{tUninspiredThing}};

    const auto tParsedInput = NewParsedInput{{tUninspiredInputBlock, kUninspiredFilterInputBlock}};

    const auto tCrossLinkedInputOrError =
        make_cross_linker<uninspired_thing>().crossLink(std::move(tUninspiredInputBlock), tParsedInput);

    EXPECT_TRUE(tCrossLinkedInputOrError.hasError());
}

}  // namespace plato::input_parser::unittest
