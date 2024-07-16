#include <gtest/gtest.h>

#include "plato/input_parser/CrossReference.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/process_manager/library/ValidateCrossReferences.hpp"

namespace plato::process_manager::library::unittest
{
template <typename T>
struct IsSomeOtherBlock
{
    constexpr static bool value = false;
};
}  // namespace plato::process_manager::library::unittest

// clang-format off
PLATO_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), other_block_with_cross_ref, 
    (plato::input_parser::CrossReference<plato::process_manager::library::unittest::IsSomeOtherBlock>, cross_ref)
)

PLATO_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), one_other_block, 
    (double, cross_ref_field)
)

PLATO_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), two_other_block, 
    (int, cross_ref_field)
)

BOOST_FUSION_DEFINE_STRUCT(
    (plato)(input_parser)(unittest), TestCrossRefParsedInput2,
    (boost::optional<plato::input_parser::other_block_with_cross_ref>, mBlockWithCrossRef)
    (boost::optional<plato::input_parser::one_other_block>, mOneBlock)
    (boost::optional<plato::input_parser::two_other_block>, mTwoBlock)
)
// clang-format on

namespace plato::process_manager::library::unittest
{
template <>
struct IsSomeOtherBlock<plato::input_parser::one_other_block>
{
    constexpr static bool value = true;
};

template <>
struct IsSomeOtherBlock<plato::input_parser::two_other_block>
{
    constexpr static bool value = true;
};

using CrossRefVariant = std::variant<plato::input_parser::two_other_block, plato::input_parser::one_other_block>;

TEST(CrossReferenceUtilities, IsCrossReference)
{
    static_assert(!detail::kIsCrossReference<double>);
    static_assert(detail::kIsCrossReference<
                  plato::input_parser::CrossReference<plato::process_manager::library::unittest::IsSomeOtherBlock>>);
}

TEST(ValidateCrossReferences, IsNotExpectedType)
{
    auto tBadCrossRef = plato::input_parser::CrossReference<IsSomeOtherBlock>{};
    tBadCrossRef.mInputBlock.set(88u);

    plato::input_parser::unittest::TestCrossRefParsedInput2 tInput{};
    tInput.mBlockWithCrossRef = plato::input_parser::other_block_with_cross_ref{tBadCrossRef};

    const auto tMessages = validate_cross_referenced_input(tInput, {});
    EXPECT_EQ(tMessages.size(), 1u);
}

TEST(ValidateCrossReferences, CrossReferenceToNonexistantInputBlock)
{
    auto tBadCrossRef = plato::input_parser::CrossReference<IsSomeOtherBlock>{};
    tBadCrossRef.mInputBlock.set(std::vector<CrossRefVariant>{});

    plato::input_parser::unittest::TestCrossRefParsedInput2 tInput{};
    tInput.mBlockWithCrossRef = plato::input_parser::other_block_with_cross_ref{tBadCrossRef};

    const auto tMessages = validate_cross_referenced_input(tInput, {});
    EXPECT_EQ(tMessages.size(), 1u);
}

TEST(ValidateCrossReferences, CrossReferenceNameIsNotAVariantType)
{
    plato::input_parser::unittest::TestCrossRefParsedInput2 tInput{
        plato::input_parser::other_block_with_cross_ref{
            plato::input_parser::CrossReference<IsSomeOtherBlock>{"bad_bad_name", {}}},
        plato::input_parser::one_other_block{8.8}, boost::none};

    const auto tMessages = validate_cross_referenced_input(tInput, {});
    EXPECT_EQ(tMessages.size(), 1u);
}

TEST(ValidateCrossReferences, CrossReferenceNameIsNotABlockInInput)
{
    plato::input_parser::unittest::TestCrossRefParsedInput2 tInput{
        plato::input_parser::other_block_with_cross_ref{
            plato::input_parser::CrossReference<IsSomeOtherBlock>{"two_other_block", {}}},
        plato::input_parser::one_other_block{8.8}, boost::none};

    const auto tMessages = validate_cross_referenced_input(tInput, {});
    EXPECT_EQ(tMessages.size(), 1u);
}

TEST(ValidateCrossReferences, CrossReferenceAmbiguousWithNoName)
{
    plato::input_parser::unittest::TestCrossRefParsedInput2 tInput{
        plato::input_parser::other_block_with_cross_ref{boost::none}, plato::input_parser::one_other_block{8.8},
        plato::input_parser::two_other_block{false}};

    const auto tMessages = validate_cross_referenced_input(tInput, {});
    EXPECT_EQ(tMessages.size(), 1u);
}

TEST(ValidateCrossReferences, ValidWithCrossReferenceOptionalAsNone)
{
    plato::input_parser::unittest::TestCrossRefParsedInput2 tInput{
        plato::input_parser::other_block_with_cross_ref{boost::none}, plato::input_parser::one_other_block{8.8},
        boost::none};

    const auto tMessages = validate_cross_referenced_input(tInput, {});
    EXPECT_TRUE(tMessages.empty()) << tMessages.front();
}

TEST(ValidateCrossReferences, ValidWithNamedCrossReference)
{
    plato::input_parser::unittest::TestCrossRefParsedInput2 tInput{
        plato::input_parser::other_block_with_cross_ref{
            plato::input_parser::CrossReference<IsSomeOtherBlock>{"one_other_block", {}}},
        plato::input_parser::one_other_block{8.8}, boost::none};

    const auto tMessages = validate_cross_referenced_input(tInput, {});
    EXPECT_TRUE(tMessages.empty()) << tMessages.front();
}

}  // namespace plato::process_manager::library::unittest