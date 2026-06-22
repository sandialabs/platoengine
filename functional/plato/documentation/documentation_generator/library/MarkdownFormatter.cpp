#include "plato/documentation/documentation_generator/library/MarkdownFormatter.hpp"

#include <cassert>
#include <format>
#include <fstream>
#include <numeric>

#include "plato/components/ComponentType.hpp"
#include "plato/components/ComponentTypeStream.hpp"
#include "plato/components/Documentation.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::documentation::documentation_generator::library
{
namespace
{
constexpr auto kTableHeader = std::string_view{"|Keyword|Type|Description|\n|-------|----|-----------|"};
constexpr auto kIntroMarkdownFileName = std::string_view{"intro.md"};

[[nodiscard]] auto data_file_contents(const std::filesystem::path& aFilePath) -> std::string
{
    const auto tDataFile = utilities::data_file_path(aFilePath.filename().string());
    if (tDataFile)
    {
        auto tFileStream = std::ifstream{tDataFile.value()};
        assert(tFileStream);
        return std::string{std::istreambuf_iterator<char>{tFileStream}, std::istreambuf_iterator<char>{}} + "\n";
    }
    return std::string{};
}
}  // namespace

auto main_section() -> std::string
{
    auto tIntroText = data_file_contents(kIntroMarkdownFileName);
    const auto tAllComponentTypes = components::all_component_types();
    tIntroText = std::accumulate(
        tAllComponentTypes.begin(), tAllComponentTypes.end(), std::move(tIntroText),
        [](std::string aCurrentText, const auto& aComponentType)
        {
            return std::move(aCurrentText) +
                   std::format("* {}: {}\n", detail::section_cross_reference(components::to_string(aComponentType)),
                               components::component_documentation_brief(aComponentType));
        });

    const auto tToGlobString =
        tAllComponentTypes |
        std::views::transform([](const auto aComponentType)
                              { return std::string{"./"} + components::to_string(aComponentType); });
    constexpr auto tAdditionalOptions = std::string_view{":caption: Input Blocks\n:glob:"};
    constexpr auto tMaxDepth = 5;
    return std::format("{}\n{}", tIntroText, detail::toc_tree(tToGlobString, tAdditionalOptions, tMaxDepth));
}

auto component_type_subsection(const HelpDocumentationMap& aComponentDocumentation,
                               const components::ComponentType aComponentType,
                               const std::uint8_t aSubsectionDepth) -> std::string
{
    auto tComponentDocumentationText = data_file_contents(detail::component_file_name(aComponentType));

    auto tDocumentation =
        std::format("{}\n{:#>{}} {}\n\n{}\nCurrently supported:\n\n",
                    detail::section_reference_definition(components::to_string(aComponentType)), "", aSubsectionDepth,
                    components::to_string(aComponentType), tComponentDocumentationText);

    auto tComponentsOfType =
        aComponentDocumentation | std::views::filter([aComponentType](const auto& aMapEntry)
                                                     { return aMapEntry.second.mComponentType == aComponentType; });
    const auto tComponentList =
        std::accumulate(tComponentsOfType.begin(), tComponentsOfType.end(), tDocumentation,
                        [](std::string aCurrentDocumentation, const auto& aMapEntry)
                        {
                            return std::move(aCurrentDocumentation) +
                                   std::format("* {{ref}}`{}`: {}\n", detail::section_reference(aMapEntry.first),
                                               aMapEntry.second.mDescription);
                        });

    auto tToComponentNames =
        tComponentsOfType | std::views::transform([](const auto& aMapEntry) { return aMapEntry.first; });
    const auto tComponentNames = std::vector<std::string>(tToComponentNames.begin(), tToComponentNames.end());
    constexpr auto tMaxTOCDepth = 5;
    constexpr auto tTOCOptions = std::string_view{};
    return std::format("{}\n{}", tComponentList, detail::toc_tree(tComponentNames, tTOCOptions, tMaxTOCDepth));
}

auto component_subsection(const std::string_view aComponentName,
                          const ComponentDocumentation& aComponentDocumentation,
                          const std::uint8_t aSubsectionDepth) -> std::string
{
    return std::format("{}\n{:#>{}} {}\n{}\n{}", detail::section_reference_definition(aComponentName), "",
                       aSubsectionDepth, aComponentName,
                       data_file_contents(std::filesystem::path{aComponentName}.replace_extension(".md")),
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
