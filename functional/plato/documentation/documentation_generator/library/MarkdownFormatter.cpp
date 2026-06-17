#include "plato/documentation/documentation_generator/library/MarkdownFormatter.hpp"

#include <cassert>
#include <format>
#include <fstream>
#include <numeric>

#include "plato/components/ComponentType.hpp"
#include "plato/components/ComponentTypeStream.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::documentation::documentation_generator::library
{
namespace
{
constexpr auto kTableHeader = std::string_view{"|Keyword|Type|Description|\n|-------|----|-----------|"};
constexpr auto kIntroMarkdownFileName = std::string_view{"intro.md"};
}  // namespace

auto main_section() -> std::string
{
    const auto tIntroFile = utilities::data_file_path(kIntroMarkdownFileName);
    assert(tIntroFile);

    auto tIntroFileStream = std::ifstream{tIntroFile.value()};
    assert(tIntroFileStream);

    auto tIntroText =
        std::string{std::istreambuf_iterator<char>{tIntroFileStream}, std::istreambuf_iterator<char>{}} + "\n";
    [&tIntroText]<std::size_t... kIndices>(std::index_sequence<kIndices...>)
    {
        (tIntroText += ... += std::format("* {}\n", detail::section_cross_reference(components::to_string(
                                                        components::component_type_from_index<kIndices>()))));
    }(std::make_index_sequence<utilities::number_of_enumerates<components::ComponentType>()>());

    return tIntroText;
}

auto component_subsection(const std::string_view aComponentName,
                          const ComponentDocumentation& aComponentDocumentation,
                          const std::uint8_t aSubsectionDepth) -> std::string
{
    return std::format("{}\n{:#>{}} {}\n{}\n{}", detail::section_reference_definition(aComponentName), "",
                       aSubsectionDepth, aComponentName, aComponentDocumentation.mDescription,
                       markdown_inputs_table(aComponentDocumentation));
}

auto markdown_inputs_table(const ComponentDocumentation& aComponentDocumentation) -> std::string
{
    return std::accumulate(aComponentDocumentation.mInputs.begin(), aComponentDocumentation.mInputs.end(),
                           std::string{kTableHeader},
                           [](std::string aCurrentTable, const auto& aInput) {
                               return std::move(aCurrentTable) +
                                      std::format("\n|`{}`|{}|{}|", aInput.mName, aInput.mType, aInput.mComment);
                           });
}

namespace detail
{
auto section_reference_definition(const std::string_view aComponentName) -> std::string
{
    return std::format("({})=", section_reference(aComponentName));
}

auto section_cross_reference(const std::string_view aComponentName) -> std::string
{
    return std::format("{{ref}}`{}`", section_reference(aComponentName));
}

auto section_reference(const std::string_view aComponentName) -> std::string
{
    return std::string{aComponentName} + "-section-ref";
}
}  // namespace detail
}  // namespace plato::documentation::documentation_generator::library
