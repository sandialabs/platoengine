#ifndef PLATO_INTEGRATION_TESTS_UTILITIES_VALIDINPUTTESTFIXTURE
#define PLATO_INTEGRATION_TESTS_UTILITIES_VALIDINPUTTESTFIXTURE

#include <gtest/gtest.h>

#include "plato/input_parser/ParsedInput.hpp"
#include "plato/test_utilities/FileCreatingTestFixture.hpp"

namespace plato::integration_tests::utilities
{
/// @brief A test fixture that provides an input deck and creates an empty file so that the input is valid.
class ValidInputTestFixture : public test_utilities::FileCreatingTestFixture
{
   public:
    ValidInputTestFixture();

    [[nodiscard]] auto parsedInput() const -> const input_parser::ParsedInput&;

   private:
    input_parser::ParsedInput mInput;
    std::filesystem::path mFilePath{};
};
}  // namespace plato::integration_tests::utilities

#endif
