#ifndef PLATO_FILTER_LIBRARY_FILTERVALIDATION
#define PLATO_FILTER_LIBRARY_FILTERVALIDATION

#include <string>
#include <vector>

namespace plato::input_parser
{
struct ParsedInput;
}

namespace plato::filter::library
{
[[nodiscard]] std::vector<std::string> validate_filter(const input_parser::ParsedInput& aInput,
                                                       std::vector<std::string>&& aCurrentMessageList);
}  // namespace plato::filter::library

#endif
