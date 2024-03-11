#ifndef PLATO_INPUT_PARSER_FILELIST
#define PLATO_INPUT_PARSER_FILELIST

#include <boost/spirit/include/qi.hpp>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace plato::input_parser
{
/// @brief Helper for parsing a single file with a path
/// Use this type in the input structs for a file name
struct FileName
{
    /// Valid characters for a file name, based on
    /// POSIX "Fully Portable Filenames" from https://en.wikipedia.org/wiki/Filename
    static constexpr std::string_view kValidChars = "-a-zA-Z0-9._/";

    using value_type = char;

    [[nodiscard]] std::string::const_iterator begin() const;
    [[nodiscard]] std::string::const_iterator end() const;
    [[nodiscard]] std::string::iterator begin();
    [[nodiscard]] std::string::iterator end();

    void insert(std::string::iterator aIter, char aVal);

    std::string mName;
};

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
std::ostream& operator<<(std::ostream& stream, const FileName& aFileName);
}  // namespace plato::input_parser

namespace boost::spirit::traits
{
template <>
struct create_parser<plato::input_parser::FileList>
{
    typedef proto::result_of::deep_copy<BOOST_TYPEOF(
        (qi::lexeme[+qi::char_(plato::input_parser::FileName::kValidChars.data())] % ','))>::type type;

    static type call()
    {
        return proto::deep_copy(
            (qi::lexeme[+qi::char_(plato::input_parser::FileName::kValidChars.data())] % ','));
    }
};

template <>
struct create_parser<plato::input_parser::FileName>
{
    typedef proto::result_of::deep_copy<BOOST_TYPEOF(
        (qi::lexeme[+qi::char_(plato::input_parser::FileName::kValidChars.data())]))>::type type;

    static type call()
    {
        return proto::deep_copy(
            (qi::lexeme[+qi::char_(plato::input_parser::FileName::kValidChars.data())]));
    }
};

}  // namespace boost::spirit::traits

#endif
