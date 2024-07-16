#include "plato/process_manager/library/ProcessManagerValidation.hpp"

#include <optional>

#include "plato/core/InputVariantUtilities.hpp"
#include "plato/core/ValidationRegistration.hpp"
#include "plato/core/ValidationUtilities.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::process_manager::library
{
[[maybe_unused]] static auto kProcessManagerValidationRegistration =
    core::ValidationRegistration<input_parser::ParsedInput>{
        [](const input_parser::ParsedInput& aInput) { return detail::validate_at_least_one_process_manager(aInput); }};

namespace detail
{
std::optional<std::string> validate_at_least_one_process_manager(const input_parser::ParsedInput& aInput)
{
    const std::vector<ProcessManagerInput> tProcessManagers =
        core::all_input_blocks_in_variant<ProcessManagerInput>(aInput);
    if (tProcessManagers.empty())
    {
        const std::vector<std::string> tAllProcessManagerNames = core::all_variant_block_names<ProcessManagerInput>();
        return "At least one of the following blocks must be defined: " +
               utilities::concatenate_container(tAllProcessManagerNames, "\n");
    }
    return std::nullopt;
}
}  // namespace detail

std::vector<std::string> validate_process_managers(const input_parser::ParsedInput& aInput,
                                                   std::vector<std::string>&& aCurrentMessageList)
{
    aCurrentMessageList = core::validate_all_variants<ProcessManagerInput>(aInput, std::move(aCurrentMessageList));
    return core::validate(aInput, std::move(aCurrentMessageList));
}

}  // namespace plato::process_manager::library
