#include <fstream>
#include <iostream>

#include "plato/documentation/HelpDocumentationRegistration.hpp"
#include "plato/documentation/documentation_generator/library/LatexFormatter.hpp"

namespace
{
constexpr auto kLatexFileExtension = std::string_view{".tex"};

void print_commands(const plato::documentation::documentation_generator::library::BlockCommands& aBlockCommands)
{
    for (const auto& tEntry : aBlockCommands)
    {
        std::ofstream tOutputFile(tEntry.first + std::string{kLatexFileExtension});
        for (const auto& tLine : tEntry.second)
        {
            tOutputFile << tLine << std::endl;
        }
    }
}

}  // namespace

auto main() -> int
{
    const auto tDocumentation = plato::documentation::generate_documentation();
    const auto tResult = plato::documentation::documentation_generator::library::format_command_latex(tDocumentation);
    print_commands(tResult);

    return 0;
}
