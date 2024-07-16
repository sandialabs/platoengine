#ifndef PLATO_INPUT_PARSER_USERDEFINEDTOKEN
#define PLATO_INPUT_PARSER_USERDEFINEDTOKEN

#include <boost/proto/deep_copy.hpp>
#include <boost/spirit/include/qi.hpp>
#include <string>
#include <string_view>

namespace plato::input_parser
{
/// @brief A type for parsing user-defined tokens such as app names or file names.
///
/// This is templated on a function object that provides the set of valid characters that may
/// appear in the token.
template <typename ValidCharFunction>
struct UserDefinedToken
{
    static constexpr std::string_view kValidChars = ValidCharFunction{}();

    using value_type = char;

    [[nodiscard]] std::string::const_iterator begin() const;
    [[nodiscard]] std::string::const_iterator end() const;
    [[nodiscard]] std::string::iterator begin();
    [[nodiscard]] std::string::iterator end();

    void insert(std::string::iterator aIter, char aVal);

    std::string mToken;
};

template <typename ValidCharFunction>
std::string::const_iterator UserDefinedToken<ValidCharFunction>::begin() const
{
    return mToken.begin();
}

template <typename ValidCharFunction>
std::string::const_iterator UserDefinedToken<ValidCharFunction>::end() const
{
    return mToken.end();
}

template <typename ValidCharFunction>
std::string::iterator UserDefinedToken<ValidCharFunction>::begin()
{
    return mToken.begin();
}

template <typename ValidCharFunction>
std::string::iterator UserDefinedToken<ValidCharFunction>::end()
{
    return mToken.end();
}

template <typename ValidCharFunction>
void UserDefinedToken<ValidCharFunction>::insert(const std::string::iterator aIter, const char aVal)
{
    mToken.insert(aIter, aVal);
}

template <typename ValidCharFunction>
std::ostream& operator<<(std::ostream& stream, const UserDefinedToken<ValidCharFunction>& aUserDefinedToken)
{
    stream << aUserDefinedToken.mToken;
    return stream;
}
}  // namespace plato::input_parser

namespace boost::spirit::traits
{
template <typename ValidCharFunction>
struct create_parser<plato::input_parser::UserDefinedToken<ValidCharFunction>>
{
    static constexpr std::string_view kValidChars =
        plato::input_parser::UserDefinedToken<ValidCharFunction>::kValidChars;

    using type = typename boost::proto::result_of::deep_copy<BOOST_TYPEOF(
        (boost::spirit::qi::lexeme[+boost::spirit::qi::char_(kValidChars.data())]))>::type;

    static type call()
    {
        return boost::proto::deep_copy((boost::spirit::qi::lexeme[+boost::spirit::qi::char_(kValidChars.data())]));
    }
};

}  // namespace boost::spirit::traits

#endif
