#include "plato/filter/library/FilterValidation.hpp"

#include "plato/core/ValidationRegistration.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/input_parser/InputBlocks.hpp"

namespace plato::filter::library
{
namespace
{
[[maybe_unused]] static auto kFilterValidationRegistration =
    core::ValidationRegistration<input_parser::density_topology>{[](const input_parser::density_topology& aInput)
                                                                 { return validate_filter_type(aInput); }};
}

std::optional<std::string> validate_filter_type(const input_parser::density_topology& aInput)
{
    if (!aInput.filter_type)
    {
        return core::error_message_for_empty_parameter(input_parser::block_name<input_parser::density_topology>(),
                                                       aInput.filter_type, "filter_type");
    }
    else
    {
        return std::nullopt;
    }
}
}  // namespace plato::filter::library
