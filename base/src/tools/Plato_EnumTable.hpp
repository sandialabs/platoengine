#ifndef SRC_ENUMTABLE_HPP_
#define SRC_ENUMTABLE_HPP_

#include <boost/optional.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>

#include <type_traits>
#include <string>
#include <initializer_list>
#include <utility>

namespace Plato{

/// A convenience type for mapping enums to strings.
///
/// The purpose of this class is to facilitate serialization of enumerations. Directly 
/// serializing enumerations as integers is difficult to read in an input file, so this 
/// class provides a mapping to a string. Usage is typically as static data.
/// @todo Add serialization.
template<typename Enum>
class EnumTable
{
    static_assert(std::is_enum<Enum>::value, "EnumTable must be instantiated with an enum type.");

    using MapType = boost::bimap<boost::bimaps::unordered_set_of<Enum>, 
        boost::bimaps::unordered_set_of<std::string>>;
    using ValueType = typename MapType::value_type;

public:

    EnumTable() = default;

    /// All entries in @a aEnumPairs must be unique, for both left and right sides. I.e.
    /// no enum or string can be equal. Checked with an assertion.
    EnumTable(std::initializer_list<std::pair<Enum, std::string>> aEnumPairs);

    /// @todo: Change return to std::optional in c++17
    boost::optional<Enum> toEnum(const std::string& aString) const;

    /// @todo: Change return to std::optional in c++17
    boost::optional<std::string> toString(const Enum aEnum) const;

    typename MapType::left_map::const_iterator begin() const;
    typename MapType::left_map::const_iterator end() const;
private:
    MapType mMap;
};

template<typename Enum>
EnumTable<Enum>::EnumTable(std::initializer_list<std::pair<Enum, std::string>> aEnumPairs)
{
    for(const auto& tItem : aEnumPairs)
    {
        // Must not contain duplicates
        assert(mMap.left.count(tItem.first) == 0 && mMap.right.count(tItem.second) == 0);
        mMap.insert(ValueType{tItem.first, tItem.second});
    }
}

template<typename Enum>
boost::optional<Enum> EnumTable<Enum>::toEnum(const std::string& aString) const
{
    const auto tIter = mMap.right.find(aString);
    if(tIter != mMap.right.end())
    {
        return tIter->second;
    }
    else
    {
        return boost::none;
    }
}

template<typename Enum>
boost::optional<std::string> EnumTable<Enum>::toString(const Enum aEnum) const
{
    const auto tIter = mMap.left.find(aEnum);
    if(tIter != mMap.left.end())
    {
       return tIter->second;
    }
    else
    {
       return boost::none;
    }
}

template<typename Enum>
auto EnumTable<Enum>::begin() const -> typename EnumTable<Enum>::MapType::left_map::const_iterator
{
    return mMap.left.begin();
}

template<typename Enum>
auto EnumTable<Enum>::end() const -> typename EnumTable<Enum>::MapType::left_map::const_iterator
{
    return mMap.left.end();
}

}

#endif