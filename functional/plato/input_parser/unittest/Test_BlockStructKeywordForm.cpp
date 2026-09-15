#include <gtest/gtest.h>

#include <boost/optional/optional.hpp>
#include <string>
#include <vector>

#include "plato/components/ComponentType.hpp"
#include "plato/input_parser/BlockStructKeywordForms.hpp"
#include "plato/input_parser/Bounds.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/Point.hpp"
#include "plato/input_parser/Range.hpp"
#include "plato/test_utilities/Strings.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/NamedType.hpp"
#include "plato/utilities/StringUtilities.hpp"

// clang-format off
PLATO_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), OrcGrunt, plato::components::ComponentType::kGeometry,
    "Ready to work.",
    (plato::input_parser::FileList, catch_phrases, "A list of things this grunt can say.")
    (plato::input_parser::Bounds, power, "The inclusive bounds on this grunt's power.")
)

PLATO_NAMED_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), OrcDeathKnight, plato::components::ComponentType::kFilter,
    "The darkness comes.",
    (plato::input_parser::Range, spell_range, "The range of possible spell values.")
    (plato::input_parser::Point, rally_point, "The rally point for the death knight.")
)
// clang-format on

namespace plato::input_parser::unittest
{
namespace
{
[[nodiscard]] auto file_list_documentation() -> std::string { return data_type_documentation(FileList{}); }

[[nodiscard]] auto bounds_documentation() -> std::string { return data_type_documentation(Bounds{}); }

[[nodiscard]] auto range_documentation() -> std::string { return data_type_documentation(Range{}); }

[[nodiscard]] auto point_documentation() -> std::string { return data_type_documentation(Point{}); }

using Message = utilities::NamedType<std::string, struct MessageTag>;
using Keyword = utilities::NamedType<std::string, struct KeywordTag>;
using DataType = utilities::NamedType<std::string, struct DataTypeTag>;

void expect_contains_keyword_form(const Message& aMessage,
                                  const Keyword& aKeyword,
                                  const DataType& aDataType,
                                  const plato::test_utilities::TestContext& aTestContext)
{
    plato::test_utilities::expect_string_contains_substring(aMessage.mValue, aKeyword.mValue,
                                                            EXTEND_CONTEXT("Keyword", aTestContext));
    plato::test_utilities::expect_string_contains_substring(aMessage.mValue, aDataType.mValue,
                                                            EXTEND_CONTEXT("Data type", aTestContext));
}
}  // namespace

TEST(BlockStructKeywordForms, ExpectedKeywordFormsMessage)
{
    const auto tMessage = expected_keyword_forms_message<OrcDeathKnight>("raly_point");
    plato::test_utilities::expect_string_contains_substring(
        tMessage, detail::closest_keyword_form_syntax(),
        TEST_CONTEXT("Sorted expected keyword forms message contains closest keyword header"));

    expect_contains_keyword_form(Message{tMessage}, Keyword{"rally_point"}, DataType{point_documentation()},
                                 TEST_CONTEXT("Sorted expected keyword forms message contains closest keyword form"));
    expect_contains_keyword_form(Message{tMessage}, Keyword{"spell_range"}, DataType{range_documentation()},
                                 TEST_CONTEXT("Sorted expected keyword forms message contains remaining keyword form"));
}

TEST(BlockStructKeywordFormsDetail, BlockStructKeywordFormsForUnnamedBlock)
{
    const auto tKeywordForms = detail::block_struct_keyword_forms<OrcGrunt>();
    ASSERT_EQ(tKeywordForms.size(), 2U);
    EXPECT_EQ(tKeywordForms.front().mName, "catch_phrases");
    EXPECT_EQ(tKeywordForms.front().mDataType, file_list_documentation());
    EXPECT_EQ(tKeywordForms.back().mName, "power");
    EXPECT_EQ(tKeywordForms.back().mDataType, bounds_documentation());
}

TEST(BlockStructKeywordFormsDetail, BlockStructKeywordFormsForNamedBlockOmitsNameField)
{
    const auto tKeywordForms = detail::block_struct_keyword_forms<OrcDeathKnight>();
    ASSERT_EQ(tKeywordForms.size(), 2U);
    EXPECT_EQ(tKeywordForms.front().mName, "spell_range");
    EXPECT_EQ(tKeywordForms.front().mDataType, range_documentation());
    EXPECT_EQ(tKeywordForms.back().mName, "rally_point");
    EXPECT_EQ(tKeywordForms.back().mDataType, point_documentation());
}

TEST(BlockStructKeywordFormsDetail, BlockStructKeywordFormsSortedByDistance)
{
    const auto tKeywordForms = detail::block_struct_keyword_forms_sorted_by_distance<OrcGrunt>("powr");
    ASSERT_EQ(tKeywordForms.size(), 2U);
    EXPECT_EQ(tKeywordForms.front().mName, "power");
    EXPECT_EQ(tKeywordForms.front().mDataType, bounds_documentation());
    EXPECT_EQ(tKeywordForms.back().mName, "catch_phrases");
    EXPECT_EQ(tKeywordForms.back().mDataType, file_list_documentation());
}

TEST(BlockStructKeywordFormsDetail, FormattedBlockStructKeywordFormsSortedByDistance)
{
    const auto tFormattedKeywordForms = utilities::concatenate_container(
        detail::formatted_block_struct_keyword_forms_sorted_by_distance<OrcGrunt>("powr"), "\n");

    expect_contains_keyword_form(Message{tFormattedKeywordForms}, Keyword{"power"}, DataType{bounds_documentation()},
                                 TEST_CONTEXT("Sorted formatted keyword forms contain power and its type descriptor"));
    expect_contains_keyword_form(Message{tFormattedKeywordForms}, Keyword{"catch_phrases"},
                                 DataType{file_list_documentation()},
                                 TEST_CONTEXT("Sorted formatted keyword forms contain catch_phrases and its type "
                                              "descriptor"));
}

TEST(BlockStructKeywordFormsDetail, IsNamedBlockNameField)
{
    EXPECT_FALSE((detail::is_named_block_name_field<OrcGrunt, 0>()));
    EXPECT_FALSE((detail::is_named_block_name_field<OrcGrunt, 1>()));

    EXPECT_TRUE((detail::is_named_block_name_field<OrcDeathKnight, 0>()));
    EXPECT_FALSE((detail::is_named_block_name_field<OrcDeathKnight, 1>()));
    EXPECT_FALSE((detail::is_named_block_name_field<OrcDeathKnight, 2>()));
}

TEST(BlockStructKeywordFormsDetail, DataTypeDocumentationForType)
{
    EXPECT_EQ(detail::data_type_documentation_for_type<FileList>(), file_list_documentation());
    EXPECT_EQ(detail::data_type_documentation_for_type<Bounds>(), bounds_documentation());
    EXPECT_EQ(detail::data_type_documentation_for_type<Range>(), range_documentation());
    EXPECT_EQ(detail::data_type_documentation_for_type<Point>(), point_documentation());
}

TEST(BlockStructKeywordFormsDetail, BlockStructKeywordFormAtIndex)
{
    const auto tCatchPhraseForm = detail::block_struct_keyword_form_at_index<OrcGrunt, 0>();
    EXPECT_EQ(tCatchPhraseForm.mName, "catch_phrases");
    EXPECT_EQ(tCatchPhraseForm.mDataType, file_list_documentation());

    const auto tPowerForm = detail::block_struct_keyword_form_at_index<OrcGrunt, 1>();
    EXPECT_EQ(tPowerForm.mName, "power");
    EXPECT_EQ(tPowerForm.mDataType, bounds_documentation());

    const auto tSpellRangeForm = detail::block_struct_keyword_form_at_index<OrcDeathKnight, 1>();
    EXPECT_EQ(tSpellRangeForm.mName, "spell_range");
    EXPECT_EQ(tSpellRangeForm.mDataType, range_documentation());

    const auto tRallyPointForm = detail::block_struct_keyword_form_at_index<OrcDeathKnight, 2>();
    EXPECT_EQ(tRallyPointForm.mName, "rally_point");
    EXPECT_EQ(tRallyPointForm.mDataType, point_documentation());
}

TEST(BlockStructKeywordFormsDetail, FormatKeywordForms)
{
    const auto tKeywordForms =
        std::vector<detail::BlockStructKeywordForm>{{.mName = "axe", .mDataType = "int"},
                                                    {.mName = "grunt", .mDataType = "bool"},
                                                    {.mName = "catapult", .mDataType = "double"}};
    const auto tFormattedKeywordForms =
        utilities::concatenate_container(detail::format_keyword_forms(tKeywordForms), "\n");
    expect_contains_keyword_form(Message{tFormattedKeywordForms}, Keyword{"axe"}, DataType{"int"},
                                 TEST_CONTEXT("Formatted keyword forms contain axe and its data type"));
    expect_contains_keyword_form(Message{tFormattedKeywordForms}, Keyword{"grunt"}, DataType{"bool"},
                                 TEST_CONTEXT("Formatted keyword forms contain grunt and its data type"));
    expect_contains_keyword_form(Message{tFormattedKeywordForms}, Keyword{"catapult"}, DataType{"double"},
                                 TEST_CONTEXT("Formatted keyword forms contain catapult and its data type"));
}

TEST(BlockStructKeywordFormsDetail, SortKeywordFormsByDistance)
{
    const auto tKeywordForms =
        std::vector<detail::BlockStructKeywordForm>{{.mName = "axe", .mDataType = "int"},
                                                    {.mName = "grunt", .mDataType = "bool"},
                                                    {.mName = "catapult", .mDataType = "double"}};
    const auto tSortedKeywordForms = detail::sort_keyword_forms_by_distance(tKeywordForms, "grnt");
    ASSERT_EQ(tSortedKeywordForms.size(), 3U);
    EXPECT_EQ(tSortedKeywordForms.front().mName, "grunt");
    EXPECT_EQ(tSortedKeywordForms.front().mDataType, "bool");
}

}  // namespace plato::input_parser::unittest
