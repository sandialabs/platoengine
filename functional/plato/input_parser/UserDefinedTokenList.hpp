#ifndef PLATO_INPUTPARSER_USERDEFINEDTOKENLIST
#define PLATO_INPUTPARSER_USERDEFINEDTOKENLIST

#include <boost/spirit/include/qi.hpp>
#include <ostream>
#include <string>
#include <vector>

#include "plato/input_parser/AutoList.hpp"
#include "plato/input_parser/UserDefinedToken.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::input_parser
{

/// @brief An auto parser for parsing lists of user-defined tokens with specific character sets.
///
/// @tparam ValidCharFunction A type that provides a call operator returning the set of valid characters that may be
/// parsed for a specific type of token.
/// @sa UserDefinedToken
template <typename ValidCharFunction>
class UserDefinedTokenList
{
   public:
    static constexpr std::string_view kValidChars = ValidCharFunction{}();
    UserDefinedTokenList() = default;
    UserDefinedTokenList(std::vector<std::string> aStringList);

    using AutoListString = AutoList<std::string>;
    using value_type = typename AutoListString::value_type;
    using iterator = typename AutoListString::iterator;
    using const_iterator = typename AutoListString::const_iterator;
    [[nodiscard]] auto begin();
    [[nodiscard]] auto end();
    [[nodiscard]] auto begin() const;
    [[nodiscard]] auto end() const;

    void insert(iterator aIter, value_type aVal);

    [[nodiscard]] auto list() const -> const AutoListString&;
    [[nodiscard]] auto size() const -> std::size_t;

   private:
    AutoListString mList;
};

template <typename ValidCharFunction>
std::ostream& operator<<(std::ostream& stream, const UserDefinedTokenList<ValidCharFunction>& aTokenList);

template <typename ValidCharFunction>
[[nodiscard]] auto operator==(const UserDefinedTokenList<ValidCharFunction>& aUserDefinedTokenList,
                              const std::vector<std::string>& aStringList) -> bool;

template <typename ValidCharFunction>
UserDefinedTokenList<ValidCharFunction>::UserDefinedTokenList(std::vector<std::string> aStringList)
    : mList(std::move(aStringList))
{
}

template <typename ValidCharFunction>
auto UserDefinedTokenList<ValidCharFunction>::begin()
{
    return mList.begin();
}

template <typename ValidCharFunction>
auto UserDefinedTokenList<ValidCharFunction>::end()
{
    return mList.end();
}

template <typename ValidCharFunction>
auto UserDefinedTokenList<ValidCharFunction>::begin() const
{
    return mList.begin();
}

template <typename ValidCharFunction>
auto UserDefinedTokenList<ValidCharFunction>::end() const
{
    return mList.end();
}

template <typename ValidCharFunction>
void UserDefinedTokenList<ValidCharFunction>::insert(iterator aIter, value_type aVal)
{
    mList.insert(aIter, std::move(aVal));
}

template <typename ValidCharFunction>
auto UserDefinedTokenList<ValidCharFunction>::list() const -> const AutoListString&
{
    return mList;
}

template <typename ValidCharFunction>
auto UserDefinedTokenList<ValidCharFunction>::size() const -> std::size_t
{
    return mList.mList.size();
}

template <typename ValidCharFunction>
std::ostream& operator<<(std::ostream& stream, const UserDefinedTokenList<ValidCharFunction>& aTokenList)
{
    constexpr auto tDelimiter = std::string_view{", "};
    stream << utilities::concatenate_container(aTokenList.list().mList, tDelimiter);
    return stream;
}

template <typename ValidCharFunction>
auto operator==(const UserDefinedTokenList<ValidCharFunction>& aUserDefinedTokenList,
                const std::vector<std::string>& aStringList) -> bool
{
    return aUserDefinedTokenList.list().mList == aStringList;
}

}  // namespace plato::input_parser

namespace boost::spirit::traits
{

template <typename ValidCharFunction>
struct create_parser<plato::input_parser::UserDefinedTokenList<ValidCharFunction>>
{
    static constexpr auto kValidChars = plato::input_parser::UserDefinedTokenList<ValidCharFunction>::kValidChars;

    static auto call() { return proto::deep_copy((qi::lexeme[+qi::char_(kValidChars.data())] % ',')); }
    using type = decltype(call());
};
}  // namespace boost::spirit::traits

#endif
