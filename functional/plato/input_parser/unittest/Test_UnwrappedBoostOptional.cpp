#include <gtest/gtest.h>

#include <boost/optional/optional.hpp>
#include <string>
#include <type_traits>

#include "plato/input_parser/UnwrappedBoostOptional.hpp"

namespace plato::input_parser::unittest
{

TEST(UnwrappedBoostOptional, LeavesNonOptionalTypeUnchanged)
{
    static_assert(std::is_same_v<UnwrappedBoostOptional<int>::Type, int>);
    static_assert(std::is_same_v<UnwrappedBoostOptional<double>::Type, double>);
    static_assert(std::is_same_v<UnwrappedBoostOptional<std::string>::Type, std::string>);
}

TEST(UnwrappedBoostOptional, RemovesCvRefFromNonOptionalType)
{
    static_assert(std::is_same_v<UnwrappedBoostOptional<const int>::Type, int>);
    static_assert(std::is_same_v<UnwrappedBoostOptional<int&>::Type, int>);
    static_assert(std::is_same_v<UnwrappedBoostOptional<const int&>::Type, int>);
    static_assert(std::is_same_v<UnwrappedBoostOptional<int&&>::Type, int>);
    static_assert(std::is_same_v<UnwrappedBoostOptional<const int&&>::Type, int>);
}

TEST(UnwrappedBoostOptional, ExtractsValueTypeFromBoostOptional)
{
    static_assert(std::is_same_v<UnwrappedBoostOptional<boost::optional<int>>::Type, int>);
    static_assert(std::is_same_v<UnwrappedBoostOptional<boost::optional<double>>::Type, double>);
    static_assert(std::is_same_v<UnwrappedBoostOptional<boost::optional<std::string>>::Type, std::string>);
}

TEST(UnwrappedBoostOptionalAlias, LeavesNonOptionalTypeUnchanged)
{
    static_assert(std::is_same_v<UnwrappedBoostOptionalT<int>, int>);
    static_assert(std::is_same_v<UnwrappedBoostOptionalT<double>, double>);
    static_assert(std::is_same_v<UnwrappedBoostOptionalT<std::string>, std::string>);
}

TEST(UnwrappedBoostOptionalAlias, RemovesCvRefFromNonOptionalType)
{
    static_assert(std::is_same_v<UnwrappedBoostOptionalT<const int>, int>);
    static_assert(std::is_same_v<UnwrappedBoostOptionalT<int&>, int>);
    static_assert(std::is_same_v<UnwrappedBoostOptionalT<const int&>, int>);
    static_assert(std::is_same_v<UnwrappedBoostOptionalT<int&&>, int>);
    static_assert(std::is_same_v<UnwrappedBoostOptionalT<const int&&>, int>);
}

TEST(UnwrappedBoostOptionalAlias, ExtractsValueTypeFromBoostOptional)
{
    static_assert(std::is_same_v<UnwrappedBoostOptionalT<boost::optional<int>>, int>);
    static_assert(std::is_same_v<UnwrappedBoostOptionalT<boost::optional<double>>, double>);
    static_assert(std::is_same_v<UnwrappedBoostOptionalT<boost::optional<std::string>>, std::string>);
}

TEST(UnwrappedBoostOptionalAlias, ExtractsValueTypeFromCvRefQualifiedBoostOptional)
{
    using OptionalInt = boost::optional<int>;

    static_assert(std::is_same_v<UnwrappedBoostOptionalT<const OptionalInt>, int>);
    static_assert(std::is_same_v<UnwrappedBoostOptionalT<OptionalInt&>, int>);
    static_assert(std::is_same_v<UnwrappedBoostOptionalT<const OptionalInt&>, int>);
    static_assert(std::is_same_v<UnwrappedBoostOptionalT<OptionalInt&&>, int>);
    static_assert(std::is_same_v<UnwrappedBoostOptionalT<const OptionalInt&&>, int>);
}

}  // namespace plato::input_parser::unittest
