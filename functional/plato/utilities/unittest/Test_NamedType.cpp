#include <gtest/gtest.h>

#include <string>

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
    static constexpr auto mDefaultDouble = 21.0;
    static constexpr auto mDefaultInt = 3;

    double mDouble = mDefaultDouble;
    int mInt = mDefaultInt;
};
using NamedDefaultConstructable = NamedType<DefaultConstructable, struct DefaultConstructableTag>;

}  // namespace

TEST(NamedType, Creation)
{
    const Input tInput{3.0, "invalid name"};
    const ValidatedInput tValid{tInput};
    EXPECT_EQ(tValid.mValue.mValue, tInput.mValue);
    EXPECT_EQ(tValid.mValue.mName, tInput.mName);
}

TEST(NamedType, DefaultConstructor)
{
    constexpr NamedDefaultConstructable tWrapped{};
    EXPECT_EQ(tWrapped.mValue.mDouble, DefaultConstructable::mDefaultDouble);
    EXPECT_EQ(tWrapped.mValue.mInt, DefaultConstructable::mDefaultInt);
}
}  // namespace plato::utilities::unittest
