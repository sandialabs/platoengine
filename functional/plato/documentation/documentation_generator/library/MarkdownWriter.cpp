#include "plato/documentation/documentation_generator/library/MarkdownWriter.hpp"

#include <filesystem>
#include <fstream>

#include "plato/components/ComponentType.hpp"

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

    [&aDocumentation]<std::size_t... kIndices>(std::index_sequence<kIndices...>)
    {
        constexpr auto tComponentTypeSubsectionDepth = std::uint8_t{2};
        (write_to_file(detail::component_file_name<kIndices>(),
                       component_type_subsection<components::component_type_from_index<kIndices>()>(
                           aDocumentation, tComponentTypeSubsectionDepth)),
         ...);
    }(std::make_index_sequence<utilities::number_of_enumerates<components::ComponentType>()>());

    for (const auto& [tComponentName, tComponentDocumentation] : aDocumentation)
    {
        constexpr auto tComponentSubsectionDepth = std::uint8_t{3};
        const auto tComponentFileName = std::filesystem::path{tComponentName + ".md"};
        write_to_file(tComponentFileName,
                      component_subsection(tComponentName, tComponentDocumentation, tComponentSubsectionDepth));
    }
}
}  // namespace plato::documentation::documentation_generator::library
