#ifndef PLATO_DOCUMENTATION_DOCUMENTATION_GENERATOR_LIBRARY_MARKDOWNWRITER
#define PLATO_DOCUMENTATION_DOCUMENTATION_GENERATOR_LIBRARY_MARKDOWNWRITER

#include "plato/main/documentation_generator/library/MarkdownFormatter.hpp"

namespace plato::documentation::documentation_generator::library
{
/// @brief Writes all documentation defined in to a set of markdown files.
void write_markdown_documentation(const HelpDocumentationMap& aDocumentation);
}  // namespace plato::documentation::documentation_generator::library

#endif
