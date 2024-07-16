#ifndef PLATO_INPUT_PARSER_FILELIST
#define PLATO_INPUT_PARSER_FILELIST

#include <boost/spirit/include/qi.hpp>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "plato/input_parser/UserDefinedToken.hpp"

namespace plato::input_parser
{
/// @brief Valid characters for a file name, based on POSIX "Fully Portable Filenames"
///  from https://en.wikipedia.org/wiki/Filename
struct ValidFilenameCharacters
{
    constexpr const char* operator()() { return "-a-zA-Z0-9._/"; }
};

using FileName = UserDefinedToken<ValidFilenameCharacters>;

/// @brief Helper for parsing lists of files
/// Use this type in the input structs for a list of files
struct FileList
{
    using value_type = std::string;

    using iterator = std::vector<std::string>::iterator;
    using const_iterator = std::vector<std::string>::const_iterator;

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;
    [[nodiscard]] iterator begin();
    [[nodiscard]] iterator end();

    void insert(std::vector<std::string>::iterator, std::string val);

    std::vector<std::string> mList;
};

std::ostream& operator<<(std::ostream& stream, const FileList& aFileList);
}  // namespace plato::input_parser

namespace boost::spirit::traits
{
template <>
struct create_parser<plato::input_parser::FileList>
{
    static constexpr std::string_view kValidChars = plato::input_parser::ValidFilenameCharacters{}();

    using type =
        typename proto::result_of::deep_copy<BOOST_TYPEOF((qi::lexeme[+qi::char_(kValidChars.data())] % ','))>::type;

    static type call() { return proto::deep_copy((qi::lexeme[+qi::char_(kValidChars.data())] % ',')); }
};

}  // namespace boost::spirit::traits

#endif
