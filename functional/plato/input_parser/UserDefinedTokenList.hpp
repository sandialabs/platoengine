#ifndef PLATO_INPUT_PARSER_USERDEFINEDTOKENLIST
#define PLATO_INPUT_PARSER_USERDEFINEDTOKENLIST

#include <boost/spirit/include/qi.hpp>
#include <ostream>
#include <string>
#include <vector>

#include "plato/utilities/StringUtilities.hpp"

namespace plato::input_parser
{

/// @brief An auto parser for parsing lists of user-defined tokens with specific character sets.
///
/// @tparam ValidCharFunction A type that provides a call operator returning the set of valid characters that may be
/// parsed for a specific type of token.
/// @sa UserDefinedToken
template <typename ValidCharFunction>
struct UserDefinedTokenList
{
    using value_type = std::string;
    using iterator = std::vector<std::string>::iterator;
    using const_iterator = std::vector<std::string>::const_iterator;

    static constexpr std::string_view kValidChars = ValidCharFunction{}();

    [[nodiscard]] iterator begin();
    [[nodiscard]] iterator end();
    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;

    void insert(std::vector<std::string>::iterator aIter, std::string aVal);

    std::vector<std::string> mList;
};

template <typename ValidCharFunction>
std::ostream& operator<<(std::ostream& stream, const UserDefinedTokenList<ValidCharFunction>& aTokenList);

template <typename ValidCharFunction>
auto UserDefinedTokenList<ValidCharFunction>::begin() -> UserDefinedTokenList<ValidCharFunction>::iterator
{
    return mList.begin();
}

template <typename ValidCharFunction>
auto UserDefinedTokenList<ValidCharFunction>::end() -> UserDefinedTokenList<ValidCharFunction>::iterator
{
    return mList.end();
}

template <typename ValidCharFunction>
auto UserDefinedTokenList<ValidCharFunction>::begin() const -> UserDefinedTokenList<ValidCharFunction>::const_iterator
{
    return mList.cbegin();
}

template <typename ValidCharFunction>
auto UserDefinedTokenList<ValidCharFunction>::end() const -> UserDefinedTokenList<ValidCharFunction>::const_iterator
{
    return mList.cend();
}

template <typename ValidCharFunction>
void UserDefinedTokenList<ValidCharFunction>::insert(std::vector<std::string>::iterator aIter, std::string aVal)
{
    mList.insert(aIter, std::move(aVal));
}

template <typename ValidCharFunction>
std::ostream& operator<<(std::ostream& stream, const UserDefinedTokenList<ValidCharFunction>& aTokenList)
{
    constexpr auto tDelimiter = std::string_view{", "};
    stream << utilities::concatenate_container(aTokenList.mList, tDelimiter);
    return stream;
}
}  // namespace plato::input_parser

namespace boost::spirit::traits
{
template <typename ValidCharFunction>
struct create_parser<plato::input_parser::UserDefinedTokenList<ValidCharFunction>>
{
    static constexpr auto kValidChars = plato::input_parser::UserDefinedTokenList<ValidCharFunction>::kValidChars;

    using type =
        typename proto::result_of::deep_copy<BOOST_TYPEOF((qi::lexeme[+qi::char_(kValidChars.data())] % ','))>::type;

    static type call() { return proto::deep_copy((qi::lexeme[+qi::char_(kValidChars.data())] % ',')); }
};
}  // namespace boost::spirit::traits

#endif
