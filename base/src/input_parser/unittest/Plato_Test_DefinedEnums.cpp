#include <gtest/gtest.h>

#include "Plato_SuppressBoostNvccWarnings.hpp"

#include "Plato_InputEnumTypes.hpp"

namespace
{
/// Assumes an auto-parser has been defined for `EnumType` via create_parser
template<typename Iterator, typename EnumType>
bool parse_symbol_table(Iterator aBegin, Iterator aEnd, std::vector<EnumType>& aEnums)
{
    namespace bsq = boost::spirit::qi;
    namespace bsa = boost::spirit::ascii;

    const bool tResult = bsq::phrase_parse(
        aBegin, aEnd, ( bsq::auto_ % ',' ), bsa::space, aEnums);

    if(aBegin != aEnd)
    {
        return false;
    }
    return tResult;
}
}

TEST(CodeOptions, EnumTable)
{
    EXPECT_TRUE(Plato::kCodeOptionsTable.toString(Plato::CodeOptions::kSierraMassApp));
    EXPECT_EQ(Plato::kCodeOptionsTable.toString(Plato::CodeOptions::kSierraMassApp).value(), "sierra_mass_app");
}

TEST(CodeOptions, SymbolParser)
{
    std::vector<Plato::CodeOptions> tParsedCodeOptions;

    const std::string tValidApps = "sierra_mass_app, sierra_mass_app";
    EXPECT_TRUE(parse_symbol_table(tValidApps.begin(), tValidApps.end(), tParsedCodeOptions));
    ASSERT_EQ(tParsedCodeOptions.size(), 2);
    EXPECT_EQ(tParsedCodeOptions.front(), Plato::CodeOptions::kSierraMassApp);
    EXPECT_EQ(tParsedCodeOptions.back(), Plato::CodeOptions::kSierraMassApp);
}

TEST(ObjectiveTypes, EnumTable)
{
    EXPECT_TRUE(Plato::kObjectiveTypesTable.toString(Plato::ObjectiveTypes::kMinimize));
    EXPECT_EQ(Plato::kObjectiveTypesTable.toString(Plato::ObjectiveTypes::kMinimize).value(), "minimize");
    EXPECT_TRUE(Plato::kObjectiveTypesTable.toString(Plato::ObjectiveTypes::kMaximize));
    EXPECT_EQ(Plato::kObjectiveTypesTable.toString(Plato::ObjectiveTypes::kMaximize).value(), "maximize");
}

TEST(ObjectiveTypes, SymbolParser)
{
    std::vector<Plato::ObjectiveTypes> tParsedObjectiveTypes;

    const std::string tValidEnums = "minimize, maximize";
    EXPECT_TRUE(parse_symbol_table(tValidEnums.begin(), tValidEnums.end(), tParsedObjectiveTypes));
    ASSERT_EQ(tParsedObjectiveTypes.size(), 2);
    EXPECT_EQ(tParsedObjectiveTypes.front(), Plato::ObjectiveTypes::kMinimize);
    EXPECT_EQ(tParsedObjectiveTypes.back(), Plato::ObjectiveTypes::kMaximize);
}

#include "Plato_RestoreBoostNvccWarnings.hpp"
