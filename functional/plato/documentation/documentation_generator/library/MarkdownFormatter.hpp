#ifndef PLATO_DOCUMENTATION_DOCUMENTATION_GENERATOR_LIBRARY_MARKDOWNINPUTSTABLE
#define PLATO_DOCUMENTATION_DOCUMENTATION_GENERATOR_LIBRARY_MARKDOWNINPUTSTABLE

#include <format>
#include <numeric>
#include <ranges>
#include <string>
#include <string_view>

#include "plato/components/ComponentTypeStream.hpp"
#include "plato/components/Documentation.hpp"
#include "plato/documentation/HelpDocumentationRegistration.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::documentation::documentation_generator::library
{

using HelpDocumentationMap = std::map<std::string, ComponentDocumentation>;

/// @brief Generates the main documentation section, including a description of Plato and links to its component types.
[[nodiscard]] auto main_section() -> std::string;

/// @brief Generates a component type subsection, including links to all examples of @a aComponentType.
template <components::ComponentType kComponentType>
[[nodiscard]] auto component_type_subsection(const HelpDocumentationMap& aComponentDocumentation,
                                             std::uint8_t aSubsectionDepth) -> std::string;

/// @brief Generates a component's subsection, with a subsection depth given by @a aSubsectionDepth.
[[nodiscard]] auto component_subsection(std::string_view aComponentName,
                                        const ComponentDocumentation& aComponentDocumentation,
                                        std::uint8_t aSubsectionDepth) -> std::string;

/// @brief Generates a Markdown-style table containing all inputs in @a aComponentDocumentation.
[[nodiscard]] auto markdown_inputs_table(const ComponentDocumentation& aComponentDocumentation) -> std::string;

namespace detail
{
/// @brief Generates a string that defines a markdown section reference.
[[nodiscard]] auto section_reference_definition(std::string_view aComponentName) -> std::string;

/// @brief Generates a string that defines a markdown section cross-reference.
[[nodiscard]] auto section_cross_reference(std::string_view aComponentName) -> std::string;

/// @brief Generates a section reference that can be used for cross-referencing this section in other parts of the
/// document.
[[nodiscard]] auto section_reference(std::string_view aComponentName) -> std::string;

/// @brief Generates a Sphinx toctree entry listing @a aEntries preceeded by the @a aAdditionalOptions.
[[nodiscard]] auto toc_tree(const std::ranges::range auto& aEntries,
                            std::string_view aAdditionalOptions,
                            std::size_t aMaxDepth) -> std::string;

auto toc_tree(const std::ranges::range auto& aEntries,
              std::string_view aAdditionalOptions,
              std::size_t aMaxDepth) -> std::string
{
    return std::format("```{{toctree}}\n:hidden:\n:maxdepth: {}\n{}\n\n{}\n```", aMaxDepth, aAdditionalOptions,
                       utilities::concatenate_container(aEntries, "\n"));
}

}  // namespace detail

template <components::ComponentType kComponentType>
auto component_type_subsection(const HelpDocumentationMap& aComponentDocumentation,
                               const std::uint8_t aSubsectionDepth) -> std::string
{
    auto tDocumentation = std::format("{}\n{:#>{}} {}\n\n{}\nCurrently supported:\n\n",
                                      detail::section_reference_definition(components::to_string(kComponentType)), "",
                                      aSubsectionDepth, components::to_string(kComponentType),
                                      components::ComponentTypeDocumentation<kComponentType>::mDocumentation);

    auto tComponentsOfType =
        aComponentDocumentation |
        std::views::filter([](const auto& aMapEntry) { return aMapEntry.second.mComponentType == kComponentType; });
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

}  // namespace plato::documentation::documentation_generator::library

#endif
