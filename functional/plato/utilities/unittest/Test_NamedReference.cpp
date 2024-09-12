#include <gtest/gtest.h>

#include "plato/utilities/NamedReference.hpp"

namespace plato::utilities::unittest
{
namespace
{

using NamedReferenceToInt = NamedReference<int, struct NamedReferenceToIntTag>;
using NamedReferenceToConstInt = NamedReference<const int, struct NamedReferenceToIntTag>;
}  // namespace

TEST(NamedReference, Constness)
{
    constexpr auto tInitialValue = int{42};
    auto tAnInt = tInitialValue;
    auto tNamedReference = NamedReferenceToConstInt{tAnInt};
    EXPECT_EQ(tNamedReference.mValue.get(), tInitialValue);

    static_assert(std::is_const_v<decltype(tNamedReference.mValue)::type>);
}

TEST(NamedReference, Mutability)
{
    auto tAnInt = 0;
    auto tNamedReference = NamedReferenceToInt{tAnInt};
    constexpr auto tNewValue = int{42};
    tNamedReference.mValue.get() = tNewValue;
    EXPECT_EQ(tNamedReference.mValue.get(), tNewValue);

    static_assert(!std::is_const_v<decltype(tNamedReference.mValue)::type>);
}

}  // namespace plato::utilities::unittest
