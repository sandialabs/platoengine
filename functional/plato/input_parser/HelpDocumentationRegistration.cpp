#include "plato/input_parser/HelpDocumentationRegistration.hpp"

#include <iomanip>

#include "plato/utilities/FormatText.hpp"
namespace plato::input_parser
{

namespace detail
{

auto registered_help_documentation_functions() -> std::map<std::string, HelpDocumentationFunction>&
{
    static auto tFunctions = std::map<std::string, HelpDocumentationFunction>{};
    return tFunctions;
}

}  // namespace detail

HelpDocumentationRegistration::HelpDocumentationRegistration(const std::string& aKey,
                                                             HelpDocumentationFunction aFunction)
{
    detail::registered_help_documentation_functions()[aKey] = std::move(aFunction);
}

auto generate_documentation() -> std::map<std::string, Documentation>
{
    const auto tFunctions = detail::registered_help_documentation_functions();
    std::map<std::string, Documentation> tFullDocumentation;
    for (const auto& tFunction : tFunctions)
    {
        auto tDocumentation = tFunction.second();
        tFullDocumentation[tFunction.first] = tDocumentation;
    }

    return tFullDocumentation;
}

void known_inputs(std::ostream& aOutputStream)
{
    constexpr auto tColumnWidth = ulong{80};
    constexpr auto tAdditionalIndent = ulong{5};
    const auto tDocumentation = plato::input_parser::generate_documentation();
    aOutputStream << "\nKnown inputs:" << std::endl;
    for (const auto& tEntry : tDocumentation)
    {
        const auto tFirstIndentSize = static_cast<int>(tEntry.first.length());
        aOutputStream << tEntry.first << std::endl;
        for (const auto& tInputs : tEntry.second)
        {
            aOutputStream << std::left << std::setw(tFirstIndentSize) << "" << tInputs.mName << "\t [" << tInputs.mType
                          << "] " << std::endl;
            plato::utilities::word_block_justify(tInputs.mComment, utilities::TextWidth{tColumnWidth},
                                                 utilities::TextIndent{tFirstIndentSize + tAdditionalIndent},
                                                 aOutputStream);
        }
        aOutputStream << std::endl;
    }
    aOutputStream << std::endl;
}

}  // namespace plato::input_parser
