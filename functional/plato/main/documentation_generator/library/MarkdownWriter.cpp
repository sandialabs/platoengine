#include "plato/main/documentation_generator/library/MarkdownWriter.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string_view>

#include "plato/components/ComponentType.hpp"
#include "plato/main/documentation_generator/library/MarkdownFormatter.hpp"

namespace plato::documentation::documentation_generator::library
{
namespace
{
void write_to_file(const std::filesystem::path& aFilePath, const std::string_view aTextToWrite)
{
    auto tFileStream = std::ofstream{aFilePath};
    tFileStream << aTextToWrite;
}
}  // namespace

void write_markdown_documentation(const HelpDocumentationMap& aDocumentation)
{
    const auto tMainFileName = std::filesystem::path{"plato.md"};
    write_to_file(tMainFileName, main_section());

    for (const auto tComponentType : components::all_component_types())
    {
        constexpr auto tComponentTypeSubsectionDepth = std::uint8_t{1};
        write_to_file(detail::component_file_name(tComponentType),
                      component_type_subsection(aDocumentation, tComponentType, tComponentTypeSubsectionDepth));
    }

    for (const auto& [tComponentName, tComponentDocumentation] : aDocumentation)
    {
        constexpr auto tComponentSubsectionDepth = std::uint8_t{1};
        const auto tComponentFileName = std::filesystem::path{tComponentName + ".md"};
        write_to_file(tComponentFileName,
                      component_subsection(tComponentName, tComponentDocumentation, tComponentSubsectionDepth));
    }
}

}  // namespace plato::documentation::documentation_generator::library
