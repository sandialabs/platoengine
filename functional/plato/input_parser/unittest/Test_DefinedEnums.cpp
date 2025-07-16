#include <gtest/gtest.h>

#include "plato/input_parser/EnumTypeHelpers.hpp"

namespace plato::enum_test
{
enum struct Dragons
{
    kRed,
    kBlue,
    kGold
};
}

DECLARE_ENUM_SYMBOL_TABLE(Dragons, plato::enum_test, (kRed, "red")(kBlue, "blue")(kGold, "gold"))
DEFINE_ENUM_SYMBOL_TABLE(Dragons, plato::enum_test)

namespace plato::input_parser::unittest
{
namespace
{
/// Assumes an auto-parser has been defined for `EnumType` via create_parser
template <typename Iterator, typename EnumType>
bool parse_symbol_table(Iterator aBegin, Iterator aEnd, std::vector<EnumType>& aEnums)
{
    namespace bsq = boost::spirit::qi;
    namespace bsa = boost::spirit::ascii;

    const bool tResult = bsq::phrase_parse(aBegin, aEnd, (bsq::auto_ % ','), bsa::space, aEnums);

    if (aBegin != aEnd)
    {
        return false;
    }
    return tResult;
}
}  // namespace

TEST(DefinedEnums, SeparateEnumDeclaration)
{
    EXPECT_EQ(plato::enum_test::kDragonsTable.toString(plato::enum_test::Dragons::kBlue).value(), "blue");
    EXPECT_EQ(plato::enum_test::kDragonsTable.toString(plato::enum_test::Dragons::kRed).value(), "red");
    EXPECT_EQ(plato::enum_test::kDragonsTable.toString(plato::enum_test::Dragons::kGold).value(), "gold");

    const auto tValidEnums = std::string{"gold, red, blue"};
    auto tParsedObjectiveTypes = std::vector<plato::enum_test::Dragons>{};
    EXPECT_TRUE(parse_symbol_table(tValidEnums.begin(), tValidEnums.end(), tParsedObjectiveTypes));
    ASSERT_EQ(tParsedObjectiveTypes.size(), 3U);
    EXPECT_EQ(tParsedObjectiveTypes.front(), plato::enum_test::Dragons::kGold);
    EXPECT_EQ(tParsedObjectiveTypes.at(1U), plato::enum_test::Dragons::kRed);
    EXPECT_EQ(tParsedObjectiveTypes.back(), plato::enum_test::Dragons::kBlue);
}

}  // namespace plato::input_parser::unittest
