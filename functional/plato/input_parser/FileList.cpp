#include "plato/input_parser/FileList.hpp"

namespace plato::input_parser
{
auto data_type_documentation(const FileName& /*unused*/) -> std::string { return "File name"; }

using FileList = UserDefinedTokenList<ValidFilenameCharacters>;

auto data_type_documentation(const FileList& /*unused*/) -> std::string { return "Comma separated list of file names"; }

}  // namespace plato::input_parser
