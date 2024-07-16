#include "plato/filter/library/FilterValidation.hpp"

#include "plato/core/ValidationRegistration.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"

namespace plato::filter::library
{
std::vector<std::string> validate_filter(const input_parser::ParsedInput& aInput,
                                         std::vector<std::string>&& aCurrentMessageList)
{
    aCurrentMessageList = core::validate_all_variants<FilterInput>(aInput, std::move(aCurrentMessageList));
    return core::validate(aInput, std::move(aCurrentMessageList));
}
}  // namespace plato::filter::library
