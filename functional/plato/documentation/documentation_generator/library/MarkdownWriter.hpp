#ifndef PLATO_DOCUMENTATION_DOCUMENTATION_GENERATOR_LIBRARY_MARKDOWNWRITER
#define PLATO_DOCUMENTATION_DOCUMENTATION_GENERATOR_LIBRARY_MARKDOWNWRITER

#include <filesystem>

#include "plato/components/ComponentTypeStream.hpp"
#include "plato/documentation/documentation_generator/library/MarkdownFormatter.hpp"

namespace plato::documentation::documentation_generator::library
{
/// @brief Writes all documentation defined in to a set of markdown files.
void write_markdown_documentation(const HelpDocumentationMap& aDocumentation);

namespace detail
{
template <std::size_t kComponentTypeIndex>
[[nodiscard]] auto component_file_name() -> std::filesystem::path
{
    return {components::to_string(components::component_type_from_index<kComponentTypeIndex>()) + ".md"};
}

}  // namespace detail
}  // namespace plato::documentation::documentation_generator::library

#endif
