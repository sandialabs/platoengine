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

    // CPD-OFF
    using value_type = char;

    [[nodiscard]] auto begin() const -> std::string::const_iterator;
    [[nodiscard]] auto end() const -> std::string::const_iterator;
    [[nodiscard]] auto begin() -> std::string::iterator;
    [[nodiscard]] auto end() -> std::string::iterator;

    void insert(std::string::iterator aIter, char aVal);
    // CPD-ON

    [[nodiscard]] auto operator==(const UserDefinedToken&) const -> bool = default;

    std::string mToken;
};

template <typename ValidCharFunction>
auto UserDefinedToken<ValidCharFunction>::begin() const -> std::string::const_iterator
{
    return mToken.begin();
}

template <typename ValidCharFunction>
auto UserDefinedToken<ValidCharFunction>::end() const -> std::string::const_iterator
{
    return mToken.end();
}

template <typename ValidCharFunction>
auto UserDefinedToken<ValidCharFunction>::begin() -> std::string::iterator
{
    return mToken.begin();
}

template <typename ValidCharFunction>
auto UserDefinedToken<ValidCharFunction>::end() -> std::string::iterator
{
    return mToken.end();
}

template <typename ValidCharFunction>
void UserDefinedToken<ValidCharFunction>::insert(const std::string::iterator aIter, const char aVal)
{
    mToken.insert(aIter, aVal);
}

template <typename ValidCharFunction>
auto operator<<(std::ostream& stream, const UserDefinedToken<ValidCharFunction>& aUserDefinedToken) -> std::ostream&
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

    static auto call()
    {
        return boost::proto::deep_copy((boost::spirit::qi::lexeme[+boost::spirit::qi::char_(kValidChars.data())]));
    }
    using type = decltype(call());
};

}  // namespace boost::spirit::traits

#endif
