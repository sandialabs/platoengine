#include <gtest/gtest.h>

#include "plato/input_parser/InputEnumTypes.hpp"
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

TEST(CodeOptions, EnumTable)
{
    EXPECT_TRUE(kCodeOptionsTable.toString(CodeOptions::kSierraMassApp));
    EXPECT_EQ(kCodeOptionsTable.toString(CodeOptions::kSierraMassApp).value(), "sierra_mass_app");
}

TEST(CodeOptions, SymbolParser)
{
    std::vector<CodeOptions> tParsedCodeOptions;

    const std::string tValidApps = "sierra_mass_app, sierra_mass_app";
    EXPECT_TRUE(parse_symbol_table(tValidApps.begin(), tValidApps.end(), tParsedCodeOptions));
    ASSERT_EQ(tParsedCodeOptions.size(), 2);
    EXPECT_EQ(tParsedCodeOptions.front(), CodeOptions::kSierraMassApp);
    EXPECT_EQ(tParsedCodeOptions.back(), CodeOptions::kSierraMassApp);
}

TEST(ObjectiveTypes, EnumTable)
{
    EXPECT_TRUE(kObjectiveTypesTable.toString(ObjectiveTypes::kMinimize));
    EXPECT_EQ(kObjectiveTypesTable.toString(ObjectiveTypes::kMinimize).value(), "minimize");
    EXPECT_TRUE(kObjectiveTypesTable.toString(ObjectiveTypes::kMaximize));
    EXPECT_EQ(kObjectiveTypesTable.toString(ObjectiveTypes::kMaximize).value(), "maximize");
}

TEST(ObjectiveTypes, SymbolParser)
{
    std::vector<ObjectiveTypes> tParsedObjectiveTypes;

    const std::string tValidEnums = "minimize, maximize";
    EXPECT_TRUE(parse_symbol_table(tValidEnums.begin(), tValidEnums.end(), tParsedObjectiveTypes));
    ASSERT_EQ(tParsedObjectiveTypes.size(), 2);
    EXPECT_EQ(tParsedObjectiveTypes.front(), ObjectiveTypes::kMinimize);
    EXPECT_EQ(tParsedObjectiveTypes.back(), ObjectiveTypes::kMaximize);
}
}  // namespace plato::input_parser::unittest