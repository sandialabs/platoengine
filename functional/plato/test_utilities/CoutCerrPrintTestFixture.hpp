#ifndef PLATO_TEST_UTILITIES_COUTPRINTESTFIXTURE
#define PLATO_TEST_UTILITIES_COUTPRINTESTFIXTURE

#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "plato/test_utilities/TestContext.hpp"

namespace plato::test_utilities
{
/// @brief A test fixture that redirects std::cout to a stream that can be tested. Upon destruction it returns std::cout
/// to print as it usually would.
class CoutPrintTestFixture : public ::testing::Test
{
   public:
    CoutPrintTestFixture();
    ~CoutPrintTestFixture();

    void checkRankZeroStringStream(const std::string& aGold, const TestContext& aTestContext);
    void checkRankZeroStringStreamForPattern(const std::vector<std::string>& aKeyList, const TestContext& aTestContext);
    void clearStream();

   private:
    std::streambuf* mOriginalCoutBuffer;
    std::ostringstream mOutputStringStream;
};
}  // namespace plato::test_utilities

#endif
