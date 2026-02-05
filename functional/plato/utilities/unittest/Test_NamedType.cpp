#include <gtest/gtest.h>

#include "plato/utilities/NamedType.hpp"

namespace plato::utilities::unittest
{
namespace
{
struct Input
{
    double mValue;
    std::string mName;
};
using ValidatedInput = NamedType<Input, struct ValidatedInputTag>;

struct DefaultConstructable
{
    double mDouble = 21.0;
    int mInt = 3;
};
using NamedDefaultConstructable = NamedType<DefaultConstructable, struct DefaultConstructableTag>;

}  // namespace

TEST(NamedType, Creation)
{
    Input tInput{3.0, "invalid name"};
    ValidatedInput tValid{tInput};
    EXPECT_EQ(tValid.mValue.mValue, tInput.mValue);
    EXPECT_EQ(tValid.mValue.mName, tInput.mName);
}

TEST(NamedType, DefaultConstructor)
{
    constexpr NamedDefaultConstructable tWrapped{};
    EXPECT_EQ(tWrapped.mValue.mDouble, 21.0);
    EXPECT_EQ(tWrapped.mValue.mInt, 3);
}
}  // namespace plato::utilities::unittest
