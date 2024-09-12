#ifndef PLATO_INPUT_PARSER_FILELIST
#define PLATO_INPUT_PARSER_FILELIST

#include "plato/input_parser/UserDefinedToken.hpp"
#include "plato/input_parser/UserDefinedTokenList.hpp"

namespace plato::input_parser
{
/// @brief Valid characters for a file name, based on POSIX "Fully Portable Filenames"
///  from https://en.wikipedia.org/wiki/Filename
struct ValidFilenameCharacters
{
    constexpr const char* operator()() { return "-a-zA-Z0-9._/"; }
};

using FileName = UserDefinedToken<ValidFilenameCharacters>;
using FileList = UserDefinedTokenList<ValidFilenameCharacters>;
}  // namespace plato::input_parser

#endif
