#include "Plato_EnumTable.hpp"
#include "Plato_OperationTypes.hpp"

#include <gtest/gtest.h>

namespace PlatoTestEnumTable
{

TEST(EnumTable, Conversion)
{
    enum class TestEnum{
        kInky,
        kPinky,
        kBlinky,
        kClyde,
        kArbitraryEnumWithNoGhostlyRelation
    };
    const Plato::EnumTable<TestEnum> kTestTable = {
        {TestEnum::kInky, "Inky"}, 
        {TestEnum::kPinky, "Pinky"},
        {TestEnum::kBlinky, "Blinky"},
        {TestEnum::kClyde, "Clyde"}};

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

TEST(EnumTable, OperationType)
{
    EXPECT_EQ(Plato::operationTypeName(Plato::OperationType::kCriterionValue), "CriterionValue");
    EXPECT_EQ(Plato::operationTypeName(Plato::OperationType::kCriterionGradient), "CriterionGradient");
    EXPECT_EQ(Plato::operationTypeName(Plato::OperationType::kCriterionHessian), "CriterionHessian");

    ASSERT_TRUE(Plato::operationType("CriterionValue"));
    ASSERT_TRUE(Plato::operationType("CriterionGradient"));
    ASSERT_TRUE(Plato::operationType("CriterionHessian"));

    ASSERT_TRUE(Plato::operationTypeIgnoreSpaces(" Criterion  Value "));
    ASSERT_TRUE(Plato::operationTypeIgnoreSpaces(" C  r iterionGradient "));
    ASSERT_TRUE(Plato::operationTypeIgnoreSpaces("Criterion  Hessian"));

    EXPECT_EQ(*Plato::operationType("CriterionValue"), Plato::OperationType::kCriterionValue);
    EXPECT_EQ(*Plato::operationType("CriterionGradient"), Plato::OperationType::kCriterionGradient);
    EXPECT_EQ(*Plato::operationType("CriterionHessian"), Plato::OperationType::kCriterionHessian);

    EXPECT_EQ(*Plato::operationTypeIgnoreSpaces("  CriterionValue"), Plato::OperationType::kCriterionValue);
    EXPECT_EQ(*Plato::operationTypeIgnoreSpaces("Criterio n G r a  dient"), Plato::OperationType::kCriterionGradient);
    EXPECT_EQ(*Plato::operationTypeIgnoreSpaces("CriterionHessian"), Plato::OperationType::kCriterionHessian);
}

}