#include <gtest/gtest.h>

#include <cstdint>
#include <format>
#include <string>
#include <vector>

#include "plato/components/ComponentType.hpp"
#include "plato/components/ComponentTypeStream.hpp"
#include "plato/components/Documentation.hpp"
#include "plato/documentation/HelpDocumentationRegistration.hpp"
#include "plato/documentation/documentation_generator/library/MarkdownFormatter.hpp"
#include "plato/test_utilities/Strings.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::documentation::documentation_generator::library
{
namespace
{
constexpr auto kComponentName = std::string_view{"Magic"};
constexpr auto kTestComponentType = components::ComponentType::kGeometry;
const auto kTestComponentDocumentation = ComponentDocumentation{
    .mComponentType = kTestComponentType,
    .mDescription = "Magic cards!",
    .mInputs = {
        InputKeywordDocumentation{.mName = "land", .mType = "permanent", .mComment = "Provides mana"},
        InputKeywordDocumentation{.mName = "creature", .mType = "permanent", .mComment = "Attacks and blocks"},
        InputKeywordDocumentation{.mName = "instant", .mType = "spell", .mComment = "Spell played at instant speed"}}};

void check_table(const std::string& aTable, const test_utilities::TestContext& aTestContext)
{
    for (const auto& tInput : kTestComponentDocumentation.mInputs)
    {
        test_utilities::expect_string_contains_substring(aTable, tInput.mName, aTestContext);
        test_utilities::expect_string_contains_substring(aTable, tInput.mType, aTestContext);
        test_utilities::expect_string_contains_substring(aTable, tInput.mComment, aTestContext);
    }
}

}  // namespace

TEST(MarkdownFormatter, MainSection)
{
    const auto tMainSectionText = main_section();
    test_utilities::expect_string_contains_substring(tMainSectionText, "# Plato", TEST_CONTEXT("Main section"));
    for (const auto tComponentType : components::all_component_types())
    {
        test_utilities::expect_string_contains_substring(tMainSectionText, components::to_string(tComponentType),
                                                         TEST_CONTEXT("Main section"));
        test_utilities::expect_string_contains_substring(
            tMainSectionText, components::component_documentation_brief(tComponentType), TEST_CONTEXT("Main section"));
    }

    test_utilities::expect_string_contains_substring(tMainSectionText, "toctree", TEST_CONTEXT("Main section"));
}

TEST(MarkdownFormatter, ComponentTypeSubsection)
{
    constexpr auto tSubsectionDepth = std::uint8_t{1};
    constexpr auto tPokemonType = components::ComponentType::kConstraint;
    const auto tMoreTestComponentDocumentation = ComponentDocumentation{
        .mComponentType = tPokemonType,
        .mDescription = "Pokemon cards!",
        .mInputs = {InputKeywordDocumentation{.mName = "pikachu", .mType = "yellow", .mComment = "It's pikachu"},
                    InputKeywordDocumentation{
                        .mName = "squirtle", .mType = "blue", .mComment = "Another pokemon that I recall"}}};

    const auto tComponentDocumentation =
        HelpDocumentationMap{{"pokemon", tMoreTestComponentDocumentation}, {"magic", kTestComponentDocumentation}};

    const auto tCheckSubsectionText = [tSubsectionDepth]<components::ComponentType kComponentType>(
                                          const HelpDocumentationMap& aHelpDocumentation, const auto& aHelpEntry,
                                          const test_utilities::TestContext& aTestContext)
    {
        const auto tSubsection = component_type_subsection(aHelpDocumentation, kComponentType, tSubsectionDepth);
        test_utilities::expect_string_contains_substring(
            tSubsection, detail::section_reference(detail::component_type_name(kComponentType)), aTestContext);
        test_utilities::expect_string_contains_substring(tSubsection, "# " + components::to_string(kComponentType),
                                                         aTestContext);
        const auto tExpectedDetailText = std::format("A `{}` component", components::to_string(kComponentType));
        test_utilities::expect_string_contains_substring(tSubsection, tExpectedDetailText, aTestContext);
        test_utilities::expect_string_contains_substring(tSubsection, detail::section_reference(aHelpEntry.first),
                                                         aTestContext);
        test_utilities::expect_string_contains_substring(tSubsection, aHelpEntry.second.mDescription, aTestContext);
        test_utilities::expect_string_contains_substring(tSubsection, "toctree", aTestContext);
    };

    tCheckSubsectionText.operator()<tPokemonType>(tComponentDocumentation, *tComponentDocumentation.find("pokemon"),
                                                  TEST_CONTEXT("Pokemon subsection"));
    tCheckSubsectionText.operator()<kTestComponentType>(tComponentDocumentation, *tComponentDocumentation.find("magic"),
                                                        TEST_CONTEXT("Magic subsection"));
}

TEST(MarkdownFormatter, ComponentsSubsection)
{
    constexpr auto tSubsectionDepth = std::uint8_t{2};
    const auto tSubsection = component_subsection(kComponentName, kTestComponentDocumentation, tSubsectionDepth);
    test_utilities::expect_string_contains_substring(tSubsection, detail::section_reference(kComponentName),
                                                     TEST_CONTEXT("Component subsection"));
    test_utilities::expect_string_contains_substring(tSubsection, "# " + std::string{kComponentName},
                                                     TEST_CONTEXT("Component subsection"));
    constexpr auto tExpectedDetailDescription = std::string_view{"Magic is a fun game."};
    test_utilities::expect_string_contains_substring(tSubsection, tExpectedDetailDescription,
                                                     TEST_CONTEXT("Component subsection"));
    check_table(tSubsection, TEST_CONTEXT("Component subsection"));
}

TEST(MarkdownFormatter, ComponentsSubsectionDepth)
{
    for (const auto tDepth : {std::uint8_t{0}, std::uint8_t{1}, std::uint8_t{8}})
    {
        const auto tSubsection = component_subsection(kComponentName, kTestComponentDocumentation, tDepth);
        const auto tSubsectionMarkdown = std::string(tDepth, '#');
        test_utilities::expect_string_contains_substring(tSubsection,
                                                         tSubsectionMarkdown + " " + std::string{kComponentName},
                                                         TEST_CONTEXT("Components subsection depth"));
    }
}

TEST(MarkdownFormatter, MarkdownInputsTable)
{
    const auto tTable = markdown_inputs_table(kTestComponentDocumentation);
    check_table(tTable, TEST_CONTEXT("Markdown inputs table"));
}

TEST(MarkdownFormatter, SectionReference)
{
    const auto tSectionReference = detail::section_reference(kComponentName);
    test_utilities::expect_string_contains_substring(tSectionReference, kComponentName,
                                                     TEST_CONTEXT("Section reference"));
}

TEST(MarkdownFormatter, SectionCrossReference)
{
    constexpr auto tTestSection = std::string_view{"snoop"};
    const auto tSectionReference = detail::section_cross_reference(tTestSection);
    EXPECT_EQ(tSectionReference, "{ref}`snoop-section-ref`");
}

TEST(MarkdownFormatter, SectionReferenceDefinition)
{
    constexpr auto tTestSection = std::string_view{"shakira"};
    const auto tSectionReference = detail::section_reference_definition(tTestSection);
    EXPECT_EQ(tSectionReference, "(shakira-section-ref)=");
}

TEST(MarkdownFormatter, TOCTree)
{
    const auto tEntries = std::vector<std::string>{"hey", "there", "blimpy", "boy"};
    constexpr auto tAdditionalOptions = std::string_view{":glob:"};
    constexpr auto tMaxDepth = 5;
    constexpr auto tExpected = std::string_view{
        "```{toctree}\n"
        ":hidden:\n"
        ":maxdepth: 5\n"
        ":glob:\n"
        ""
        "\n"
        "hey\n"
        "there\n"
        "blimpy\n"
        "boy\n"
        "```"};

    EXPECT_EQ(detail::toc_tree(tEntries, tAdditionalOptions, tMaxDepth), tExpected);
}

TEST(MarkdownFormatter, ComponentTypeName)
{
    for (const auto tComponentType : components::all_component_types())
    {
        EXPECT_NE(detail::component_type_name(tComponentType), components::to_string(tComponentType));
    }
}

}  // namespace plato::documentation::documentation_generator::library
