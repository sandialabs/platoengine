#include "plato/documentation/HelpDocumentationRegistration.hpp"

#include <sys/types.h>

#include <iomanip>
#include <ios>
#include <map>
#include <ostream>
#include <string>
#include <utility>

#include "plato/components/ComponentTypeColors.hpp"
#include "plato/components/ComponentTypeStream.hpp"
#include "plato/utilities/Colorize.hpp"
#include "plato/utilities/FormatText.hpp"

namespace plato::documentation
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

auto generate_documentation() -> std::map<std::string, ComponentDocumentation>
{
    const auto tFunctions = detail::registered_help_documentation_functions();
    std::map<std::string, ComponentDocumentation> tFullDocumentation;
    for (const auto& tFunction : tFunctions)
    {
        tFullDocumentation[tFunction.first] = tFunction.second();
    }

    return tFullDocumentation;
}

void known_inputs(std::ostream& aOutputStream)
{
    constexpr auto tColumnWidth = ulong{80};
    constexpr auto tAdditionalIndent = ulong{5};
    const auto tDocumentation = plato::documentation::generate_documentation();
    aOutputStream << "\nKnown inputs:" << std::endl;
    for (const auto& tEntry : tDocumentation)
    {
        const auto tFirstIndentSize = static_cast<int>(tEntry.first.length());
        aOutputStream << utilities::colorize(tEntry.first, utilities::TextColor::kCyan) << " ("
                      << utilities::colorize(components::to_string(tEntry.second.mComponentType),
                                             components::component_text_color(tEntry.second.mComponentType))
                      << ")\n";
        utilities::word_block_justify(tEntry.second.mDescription,
                                      utilities::TextWidth{tColumnWidth + tAdditionalIndent},
                                      utilities::TextIndent{tAdditionalIndent}, aOutputStream);
        for (const auto& tInputs : tEntry.second.mInputs)
        {
            aOutputStream << std::left << std::setw(tFirstIndentSize) << ""
                          << utilities::colorize(tInputs.mName, utilities::TextColor::kBlue) << "\t ["
                          << utilities::colorize(tInputs.mType, utilities::TextColor::kLightMagenta) << "] " << "\n";
            plato::utilities::word_block_justify(tInputs.mComment, utilities::TextWidth{tColumnWidth},
                                                 utilities::TextIndent{tFirstIndentSize + tAdditionalIndent},
                                                 aOutputStream);
        }
        aOutputStream << "\n";
    }
    aOutputStream << std::endl;
}

}  // namespace plato::documentation
