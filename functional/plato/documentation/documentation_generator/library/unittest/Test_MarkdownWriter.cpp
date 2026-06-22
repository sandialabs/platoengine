#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>

#include "plato/documentation/HelpDocumentationRegistration.hpp"
#include "plato/documentation/documentation_generator/library/MarkdownFormatter.hpp"
#include "plato/documentation/documentation_generator/library/MarkdownWriter.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::documentation::documentation_generator::library::unittest
{

TEST(MarkdownWriter, WriteMarkdownDocumentation)
{
    const auto tGeometryTestComponentDocumentation = ComponentDocumentation{
        .mComponentType = components::ComponentType::kGeometry,
        .mDescription = "Slay the spire is a fun game!",
        .mInputs = {InputKeywordDocumentation{.mName = "Strike", .mType = "attack", .mComment = "Basic attack"},
                    InputKeywordDocumentation{.mName = "Defend", .mType = "block", .mComment = "Basic defend"}}};
    const auto tFilterTestComponentDocumentation = ComponentDocumentation{
        .mComponentType = components::ComponentType::kFilter,
        .mDescription = "Deep rock galactic is a fun game!",
        .mInputs = {InputKeywordDocumentation{
                        .mName = "Rock", .mType = "essential", .mComment = "First part of rock and stone"},
                    InputKeywordDocumentation{
                        .mName = "Stone", .mType = "important", .mComment = "Second part of rock and stone"}}};
    const auto tDocumentation = HelpDocumentationMap{{"slay-the-spire", tGeometryTestComponentDocumentation},
                                                     {"deep-rock-galactic", tFilterTestComponentDocumentation}};

    write_markdown_documentation(tDocumentation);

    auto tExpectedFiles = std::vector<std::filesystem::path>{};
    tExpectedFiles.emplace_back("plato.md");

    std::ranges::transform(components::all_component_types(), std::back_inserter(tExpectedFiles),
                           [](const auto aComponentType) { return detail::component_file_name(aComponentType); });

    tExpectedFiles.emplace_back("slay-the-spire.md");
    tExpectedFiles.emplace_back("deep-rock-galactic.md");

    test_utilities::test_for_existence_and_remove(tExpectedFiles, TEST_CONTEXT("Write markdown"));
}  // namespace TEST(MarkdownWriter,WriteMarkdownDocumentation)
}  // namespace plato::documentation::documentation_generator::library::unittest
