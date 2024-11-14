#include <gtest/gtest.h>

#include "plato/utilities/EnumTable.hpp"

namespace plato::utilities::unittest
{
namespace
{
enum class TestEnum
{
    kInky,
    kPinky,
    kBlinky,
    kClyde,
    kArbitraryEnumWithNoGhostlyRelation
};

const auto kTestTable = EnumTable<TestEnum>{
    {TestEnum::kInky, "Inky"}, {TestEnum::kPinky, "Pinky"}, {TestEnum::kBlinky, "Blinky"}, {TestEnum::kClyde, "Clyde"}};

}  // namespace

TEST(EnumTable, Conversion)
{
    // String to enum
    ASSERT_TRUE(kTestTable.toEnum("Inky"));
    ASSERT_TRUE(kTestTable.toEnum("Blinky"));
    ASSERT_TRUE(kTestTable.toEnum("Pinky"));
    ASSERT_TRUE(kTestTable.toEnum("Clyde"));

    EXPECT_EQ(*kTestTable.toEnum("Inky"), TestEnum::kInky);
    EXPECT_EQ(*kTestTable.toEnum("Blinky"), TestEnum::kBlinky);
    EXPECT_EQ(*kTestTable.toEnum("Pinky"), TestEnum::kPinky);
    EXPECT_EQ(*kTestTable.toEnum("Clyde"), TestEnum::kClyde);

    EXPECT_FALSE(kTestTable.toEnum("clyde"));
    EXPECT_FALSE(kTestTable.toEnum("Clyde "));
    EXPECT_FALSE(kTestTable.toEnum("arbitrary string that has no ghostly relation"));

    // Enum to string
    ASSERT_TRUE(kTestTable.toString(TestEnum::kInky));
    ASSERT_TRUE(kTestTable.toString(TestEnum::kBlinky));
    ASSERT_TRUE(kTestTable.toString(TestEnum::kPinky));
    ASSERT_TRUE(kTestTable.toString(TestEnum::kClyde));

    EXPECT_EQ(*kTestTable.toString(TestEnum::kInky), "Inky");
    EXPECT_EQ(*kTestTable.toString(TestEnum::kBlinky), "Blinky");
    EXPECT_EQ(*kTestTable.toString(TestEnum::kPinky), "Pinky");
    EXPECT_EQ(*kTestTable.toString(TestEnum::kClyde), "Clyde");

    EXPECT_FALSE(kTestTable.toString(TestEnum::kArbitraryEnumWithNoGhostlyRelation));
}

TEST(EnumTable, AllStringsFromTable)
{
    const auto tAllEntries = all_strings_from_table(kTestTable);
    constexpr auto tExpectedNumberOfEntries = 4U;
    ASSERT_EQ(tAllEntries.size(), tExpectedNumberOfEntries);
    EXPECT_EQ(tAllEntries.at(0), "Inky");
    EXPECT_EQ(tAllEntries.at(1), "Pinky");
    EXPECT_EQ(tAllEntries.at(2), "Blinky");
    EXPECT_EQ(tAllEntries.at(3), "Clyde");
}

}  // namespace plato::utilities::unittest
