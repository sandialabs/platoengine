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

TEST(ObjectiveTypes, SymbolParser)
{
    std::vector<ConstraintTypes> tParsedObjectiveTypes;

    const std::string tValidEnums = "equal_to, less_than, greater_than";
    EXPECT_TRUE(parse_symbol_table(tValidEnums.begin(), tValidEnums.end(), tParsedObjectiveTypes));
    ASSERT_EQ(tParsedObjectiveTypes.size(), 3u);
    EXPECT_EQ(tParsedObjectiveTypes.front(), ConstraintTypes::kEqualTo);
    EXPECT_EQ(tParsedObjectiveTypes.at(1U), ConstraintTypes::kLessThan);
    EXPECT_EQ(tParsedObjectiveTypes.back(), ConstraintTypes::kGreaterThan);
}

TEST(KernelFilterCenteringTypes, EnumTable)
{
    EXPECT_TRUE(kKernelFilterCenteringTypesTable.toString(KernelFilterCenteringTypes::kElementCentered));
    EXPECT_EQ(kKernelFilterCenteringTypesTable.toString(KernelFilterCenteringTypes::kElementCentered).value(),
              "element");
    EXPECT_TRUE(kKernelFilterCenteringTypesTable.toString(KernelFilterCenteringTypes::kNodeCentered));
    EXPECT_EQ(kKernelFilterCenteringTypesTable.toString(KernelFilterCenteringTypes::kNodeCentered).value(), "node");
}

TEST(ConstraintTypes, EnumTable)
{
    EXPECT_TRUE(kConstraintTypesTable.toString(ConstraintTypes::kEqualTo));
    EXPECT_EQ(kConstraintTypesTable.toString(ConstraintTypes::kEqualTo).value(), "equal_to");

    EXPECT_TRUE(kConstraintTypesTable.toString(ConstraintTypes::kLessThan));
    EXPECT_EQ(kConstraintTypesTable.toString(ConstraintTypes::kLessThan).value(), "less_than");

    EXPECT_TRUE(kConstraintTypesTable.toString(ConstraintTypes::kGreaterThan));
    EXPECT_EQ(kConstraintTypesTable.toString(ConstraintTypes::kGreaterThan).value(), "greater_than");
}

}  // namespace plato::input_parser::unittest
