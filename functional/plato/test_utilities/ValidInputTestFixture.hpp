#ifndef PLATO_TEST_UTILITIES_VALIDINPUTTESTFIXTURE
#define PLATO_TEST_UTILITIES_VALIDINPUTTESTFIXTURE

#include <gtest/gtest.h>

#include "plato/input_parser/InputBlocks.hpp"
#include "plato/test_utilities/FileCreatingTestFixture.hpp"

namespace plato::test_utilities
{
/// @brief A test fixture that provides an input deck and creates an empty file so that the input is valid.
class ValidInputTestFixture : public FileCreatingTestFixture
{
   public:
    ValidInputTestFixture();

    const input_parser::ParsedInput& parsedInput() const;

   private:
    input_parser::ParsedInput mInput;
    std::filesystem::path mFilePath{};
};
}  // namespace plato::test_utilities

#endif
