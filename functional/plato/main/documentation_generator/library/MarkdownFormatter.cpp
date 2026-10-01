#include "plato/main/documentation_generator/library/MarkdownFormatter.hpp"

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <numeric>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include "plato/components/ComponentType.hpp"
#include "plato/components/ComponentTypeStream.hpp"
#include "plato/components/Documentation.hpp"
#include "plato/criteria/library/CriterionDocumentation.hpp"
#include "plato/documentation/HelpDocumentationRegistration.hpp"
#include "plato/main/documentation_generator/library/Utilities.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::main::documentation_generator::library
{
namespace
{
constexpr auto kIntroMarkdownFileName = std::string_view{"intro.md"};
constexpr auto kPostBlocksMarkdownFileName = std::string_view{"outro.md"};
constexpr auto kCriteriaMarkdownFileName = std::string_view{"criteria.md"};

[[nodiscard]] auto documentation_path(std::filesystem::path aComponentName) -> std::filesystem::path
{
    const auto tComponentsDocumentationDirectory = std::filesystem::path{"documentation"};
    return tComponentsDocumentationDirectory / aComponentName.replace_extension(".md");
}

[[nodiscard]] auto documentation_file_contents(const std::filesystem::path& aFileName) -> std::string
{
    const auto tDataFile = utilities::data_file_path(documentation_path(aFileName));
    if (tDataFile)
    {
        auto tFileStream = std::ifstream{tDataFile.value()};
        assert(tFileStream);
        return std::string{std::istreambuf_iterator<char>{tFileStream}, std::istreambuf_iterator<char>{}} + "\n";
    }
    return std::string{};
}

}  // namespace

auto main_section(const std::vector<criteria::library::CriterionDocumentation>& aCriteriaDocumentation) -> std::string
{
    auto tIntroText = documentation_file_contents(kIntroMarkdownFileName);
    const auto tAllComponentTypes = components::all_component_types();
    tIntroText = std::accumulate(
        tAllComponentTypes.begin(), tAllComponentTypes.end(), std::move(tIntroText),
        [](std::string&& aCurrentText, const auto& aComponentType)
        {
            return std::move(aCurrentText) +
                   std::format("* {}: {}\n",
                               detail::section_cross_reference(detail::component_type_name(aComponentType)),
                               components::component_documentation_brief(aComponentType));
        });

    const auto tToGlobString =
        tAllComponentTypes |
        std::views::transform([](const auto aComponentType)
                              { return std::string{"./"} + detail::component_type_name(aComponentType); });
    constexpr auto tAdditionalOptions = std::string_view{":caption: Input Blocks\n:glob:"};
    constexpr auto tMaxDepth = 5;
    const auto tTableOfContentsTree =
        std::format("{}\n{}", tIntroText, detail::toc_tree(tToGlobString, tAdditionalOptions, tMaxDepth));

    auto tCriteriaText =
        documentation_file_contents(kCriteriaMarkdownFileName) + criteria_table(aCriteriaDocumentation);

    return tTableOfContentsTree + documentation_file_contents(kPostBlocksMarkdownFileName) + std::move(tCriteriaText);
}

auto criteria_table(const std::vector<criteria::library::CriterionDocumentation>& aCriteriaDocumentation) -> std::string
{
    const auto tAppLabel = std::string{"App"};
    const auto tCriterionLabel = std::string{"Criterion"};
    const auto tConcurrencyLabel = std::string{"Parallelization"};
    const auto tDimensionalityLabel = std::string{"Dimensionality"};

    const auto tColumnWidths = {30, 30, 20, 20};
    const auto tColumnLabels = {tAppLabel, tCriterionLabel, tConcurrencyLabel, tDimensionalityLabel};
    const auto tHeader = myst_table_header(tColumnWidths, tColumnLabels);
    const auto tConsolidatedDocumentation = consolidated_criteria_documentation(aCriteriaDocumentation);
    return std::accumulate(tConsolidatedDocumentation.begin(), tConsolidatedDocumentation.end(), tHeader,
                           [&](std::string&& aCurrent, const auto& aCriterionDocumentation)
                           {
                               const auto tTableRow = {aCriterionDocumentation.mAppName,
                                                       aCriterionDocumentation.mCriterionName,
                                                       aCriterionDocumentation.mTraits.at(tConcurrencyLabel),
                                                       aCriterionDocumentation.mTraits.at(tDimensionalityLabel)};
                               return std::move(aCurrent) + myst_table_row(tTableRow);
                           }) +
           myst_table_footer();
}

auto component_type_subsection(const HelpDocumentationMap& aComponentDocumentation,
                               const components::ComponentType aComponentType,
                               const std::uint8_t aSubsectionDepth) -> std::string
{
    auto tComponentDocumentationText = documentation_file_contents(detail::component_file_name(aComponentType));

    auto tDocumentation =
        std::format("{}\n{:#>{}} {}\n\n{}\nCurrently supported:\n\n",
                    detail::section_reference_definition(detail::component_type_name(aComponentType)), "",
                    aSubsectionDepth, components::to_string(aComponentType), tComponentDocumentationText);

    auto tComponentsOfType =
        aComponentDocumentation | std::views::filter([aComponentType](const auto& aMapEntry)
                                                     { return aMapEntry.second.mComponentType == aComponentType; });
    const auto tComponentList =
        std::accumulate(tComponentsOfType.begin(), tComponentsOfType.end(), tDocumentation,
                        [](std::string&& aCurrentDocumentation, const auto& aMapEntry)
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
                          const documentation::ComponentDocumentation& aComponentDocumentation,
                          const std::uint8_t aSubsectionDepth) -> std::string
{
    return std::format("{}\n{:#>{}} {}\n{}\n{}", detail::section_reference_definition(aComponentName), "",
                       aSubsectionDepth, aComponentName, documentation_file_contents(aComponentName),
                       markdown_inputs_table(aComponentDocumentation));
}

auto markdown_inputs_table(const documentation::ComponentDocumentation& aComponentDocumentation) -> std::string
{
    if (aComponentDocumentation.mInputs.empty())
    {
        return {};
    }
    const auto tColumnWidths = {28, 27, 45};
    const auto tColumnLabels = {"Keyword", "Type", "Description"};
    auto tTableHeader = myst_table_header(tColumnWidths, tColumnLabels);

    return std::accumulate(aComponentDocumentation.mInputs.begin(), aComponentDocumentation.mInputs.end(),
                           std::move(tTableHeader),
                           [](std::string aCurrentTable, const auto& aInput)
                           {
                               const auto tTableRow = {aInput.mName, aInput.mType, aInput.mComment};
                               return std::move(aCurrentTable) + myst_table_row(tTableRow);
                           }) +
           myst_table_footer();
}

namespace detail
{
auto component_type_name(const components::ComponentType aComponentType) -> std::string
{
    return components::to_string(aComponentType) + std::string{"-component-type"};
}

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

auto component_file_name(const components::ComponentType aComponentType) -> std::filesystem::path
{
    return {detail::component_type_name(aComponentType) + ".md"};
}
}  // namespace detail
}  // namespace plato::main::documentation_generator::library
