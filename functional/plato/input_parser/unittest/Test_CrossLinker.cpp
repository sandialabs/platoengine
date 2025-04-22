#include <gtest/gtest.h>

#include "plato/input_parser/ComponentType.hpp"
#include "plato/input_parser/CrossLinker.hpp"
#include "plato/input_parser/CrossReference.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace
{
using FilterCrossReference = plato::input_parser::CrossReference<plato::input_parser::ComponentType::kFilter>;
}

// clang-format off
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         uninspired_thing,
                         plato::input_parser::ComponentType::kGeometry,
                         (double, a_number, "help")
                         (FilterCrossReference, my_filter, "help")
                         (int, another_number, "help"))
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         uninspired_filter,
                         plato::input_parser::ComponentType::kFilter,
                         (double, filteriness, "help"))
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         very_inspired_filter,
                         plato::input_parser::ComponentType::kFilter,
                         (double, filteritude, "help"))
PLATO_INPUT_BLOCK_STRUCT((plato)(input_parser),
                         no_cross_reference_thing,
                         plato::input_parser::ComponentType::kGeometry,
                         (double, a_number, "help")
                         (int, another_number, "help"))
// clang-format on

namespace plato::input_parser::unittest
{
namespace
{
constexpr auto kUninspiredFilterValue = 42.0;
constexpr auto kVeryInspiredFilterValue = 13.0;

const auto kUninspiredFilterInputBlock = InputDataBlock{/*.mComponentType=*/
                                                        ComponentType::kFilter,
                                                        /*.mBlockName=*/"uninspired_filter", /*.mInput=*/
                                                        InputBlockWrapper{uninspired_filter{ /*.filteriness=*/
                                                                                            kUninspiredFilterValue}}};
const auto kVeryInspiredFilterInputBlock =
    InputDataBlock{/*.mComponentType=*/
                   ComponentType::kFilter,
                   /*.mBlockName=*/"very_inspired_filter", /*.mInput=*/
                   InputBlockWrapper{very_inspired_filter{ /*.filteritude=*/
                                                          kVeryInspiredFilterValue}}};

template <typename CrossReferenceType>
void check_cross_linked_field(const ParsedInput& aParsedInput,
                              const InputDataBlock& aBlockToLink,
                              const double tExpectedFilterValue,
                              const test_utilities::TestContext& aTestContext)
{
    const auto tCrossLinker = make_cross_linker<uninspired_thing>();
    const auto tCrossLinkedInputOrError = tCrossLinker.crossLink(aBlockToLink, aParsedInput);

    ASSERT_TRUE(tCrossLinkedInputOrError.hasValue()) << aTestContext;
    const auto& tUninspiredInput = tCrossLinkedInputOrError.value().mInput.template get<uninspired_thing>();

    ASSERT_TRUE(tUninspiredInput.my_filter.has_value()) << aTestContext;
    ASSERT_TRUE(tUninspiredInput.my_filter.value().mInputBlock.hasValue()) << aTestContext;
    ASSERT_TRUE(tUninspiredInput.my_filter.value().mInputBlock.template holdsExpectedType<CrossReferenceType>())
        << aTestContext;

    if constexpr (std::is_same_v<CrossReferenceType, uninspired_filter>)
    {
        EXPECT_EQ(tUninspiredInput.my_filter.value().mInputBlock.template get<CrossReferenceType>().filteriness.value(),
                  tExpectedFilterValue)
            << aTestContext;
    }
    else
    {
        EXPECT_EQ(tUninspiredInput.my_filter.value().mInputBlock.template get<CrossReferenceType>().filteritude.value(),
                  tExpectedFilterValue)
            << aTestContext;
    }
}

}  // namespace

TEST(CrossLinker, CrossLinkUnspecified)
{
    auto tUninspiredInputBlock =
        InputDataBlock{/*.mComponentType=*/ComponentType::kGeometry,
                       /*.mBlockName=*/"uninspired_thing", /*.mInput=*/InputBlockWrapper{uninspired_thing{}}};

    ASSERT_FALSE(tUninspiredInputBlock.mInput.get<uninspired_thing>()
                     .my_filter.has_value());  // Make sure the cross-reference is empty

    const auto tParsedInput = ParsedInput{{tUninspiredInputBlock, kUninspiredFilterInputBlock}};
    check_cross_linked_field<uninspired_filter>(tParsedInput, tUninspiredInputBlock, kUninspiredFilterValue,
                                                TEST_CONTEXT("Cross-reference not specified"));
}

TEST(CrossLinker, CrossLinkSpecified)
{
    const auto tUninspiredThing =
        uninspired_thing{/*.a_number=*/42.0, /*.my_filter=*/FilterCrossReference{/*.mName=*/"very_inspired_filter", {}},
                         /*.another_number=*/13};

    auto tUninspiredInputBlock =
        InputDataBlock{/*.mComponentType=*/ComponentType::kGeometry,
                       /*.mBlockName=*/"uninspired_thing", /*.mInput=*/InputBlockWrapper{tUninspiredThing}};

    const auto tParsedInput =
        ParsedInput{{tUninspiredInputBlock, kUninspiredFilterInputBlock, kVeryInspiredFilterInputBlock}};

    check_cross_linked_field<very_inspired_filter>(tParsedInput, tUninspiredInputBlock, kVeryInspiredFilterValue,
                                                   TEST_CONTEXT("Cross-reference specified"));
}

TEST(CrossLinker, NoOpForTypeWithNoCrossReferences)
{
    const auto tNoCrossReferenceThing = no_cross_reference_thing{/*.a_number=*/42.0,
                                                                 /*.another_number=*/13};
    auto tNoCrossReferenceInputBlock = InputDataBlock{/*.mComponentType=*/ComponentType::kGeometry,
                                                      /*.mBlockName=*/"no_cross_reference_thing",
                                                      /*.mInput=*/InputBlockWrapper{tNoCrossReferenceThing}};

    const auto tParsedInput = ParsedInput{{tNoCrossReferenceInputBlock, kUninspiredFilterInputBlock}};

    const auto tCrossLinkedInputOrError =
        make_cross_linker<no_cross_reference_thing>().crossLink(std::move(tNoCrossReferenceInputBlock), tParsedInput);
    EXPECT_TRUE(tCrossLinkedInputOrError.hasValue());
}

TEST(CrossLinker, ErrorMissingFieldName)
{
    auto tUninspiredInputBlock =
        InputDataBlock{/*.mComponentType=*/ComponentType::kGeometry,
                       /*.mBlockName=*/"uninspired_thing", /*.mInput=*/InputBlockWrapper{uninspired_thing{}}};

    const auto tParsedInput =
        ParsedInput{{tUninspiredInputBlock, kUninspiredFilterInputBlock, kVeryInspiredFilterInputBlock}};

    const auto tCrossLinkedInputOrError =
        make_cross_linker<uninspired_thing>().crossLink(std::move(tUninspiredInputBlock), tParsedInput);

    EXPECT_TRUE(tCrossLinkedInputOrError.hasError());
}

TEST(CrossLinker, ErrorMissingLinkableComponents)
{
    auto tUninspiredInputBlock =
        InputDataBlock{/*.mComponentType=*/ComponentType::kGeometry,
                       /*.mBlockName=*/"uninspired_thing", /*.mInput=*/InputBlockWrapper{uninspired_thing{}}};

    const auto tParsedInput = ParsedInput{{tUninspiredInputBlock}};

    const auto tCrossLinkedInputOrError =
        make_cross_linker<uninspired_thing>().crossLink(std::move(tUninspiredInputBlock), tParsedInput);

    EXPECT_TRUE(tCrossLinkedInputOrError.hasError());
}

TEST(CrossLinker, ErrorCrossLinkNameNotFound)
{
    const auto tUninspiredThing =
        uninspired_thing{/*.a_number=*/42.0, /*.my_filter=*/FilterCrossReference{/*.mName=*/"very_inspired_filter", {}},
                         /*.another_number=*/13};
    auto tUninspiredInputBlock =
        InputDataBlock{/*.mComponentType=*/ComponentType::kGeometry,
                       /*.mBlockName=*/"uninspired_thing", /*.mInput=*/InputBlockWrapper{tUninspiredThing}};

    const auto tParsedInput = ParsedInput{{tUninspiredInputBlock, kUninspiredFilterInputBlock}};

    const auto tCrossLinkedInputOrError =
        make_cross_linker<uninspired_thing>().crossLink(std::move(tUninspiredInputBlock), tParsedInput);

    EXPECT_TRUE(tCrossLinkedInputOrError.hasError());
}

}  // namespace plato::input_parser::unittest
