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
    ASSERT_EQ(tParsedObjectiveTypes.size(), 2u);
    EXPECT_EQ(tParsedObjectiveTypes.front(), ObjectiveTypes::kMinimize);
    EXPECT_EQ(tParsedObjectiveTypes.back(), ObjectiveTypes::kMaximize);
}

TEST(KernelFilterCenteringTypes, EnumTable)
{
    EXPECT_TRUE(kKernelFilterCenteringTypesTable.toString(KernelFilterCenteringTypes::kElementCentered));
    EXPECT_EQ(kKernelFilterCenteringTypesTable.toString(KernelFilterCenteringTypes::kElementCentered).value(),
              "element");
    EXPECT_TRUE(kKernelFilterCenteringTypesTable.toString(KernelFilterCenteringTypes::kNodeCentered));
    EXPECT_EQ(kKernelFilterCenteringTypesTable.toString(KernelFilterCenteringTypes::kNodeCentered).value(), "node");
}
}  // namespace plato::input_parser::unittest
