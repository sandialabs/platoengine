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
class CoutCerrPrintTestFixture : public ::testing::Test
{
   public:
    CoutCerrPrintTestFixture();
    ~CoutCerrPrintTestFixture();

    void checkRankZeroCoutStringStream(const std::string& aGold, const TestContext& aTestContext) const;
    void checkRankZeroCerrStringStream(const std::string& aGold, const TestContext& aTestContext) const;

    void checkRankZeroCoutStringStreamStreamForPattern(const std::vector<std::string>& aKeyList,
                                                       const TestContext& aTestContext) const;
    void checkRankZeroCerrStringStreamStreamForPattern(const std::vector<std::string>& aKeyList,
                                                       const TestContext& aTestContext) const;

    void clearStreams();

   private:
    std::streambuf* mOriginalCoutBuffer;
    std::streambuf* mOriginalCerrBuffer;
    std::ostringstream mCoutStringStream;
    std::ostringstream mCerrStringStream;
};
}  // namespace plato::test_utilities

#endif
