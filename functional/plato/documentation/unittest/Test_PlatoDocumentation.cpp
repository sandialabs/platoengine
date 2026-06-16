#include <gtest/gtest.h>

#include "plato/documentation/PlatoDocumentation.hpp"

namespace plato::documentation::unittest
{
TEST(PlatoDocumentation, DocumentationExists)
{
    const auto tDocumentation = std::string{PlatoDocumentation::mDocumentation};
    EXPECT_NE(tDocumentation.find("Plato"), std::string::npos) << "Documentation string: " << tDocumentation;
}
}  // namespace plato::documentation::unittest
