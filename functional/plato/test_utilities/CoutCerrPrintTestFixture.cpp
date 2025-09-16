#include "plato/test_utilities/CoutCerrPrintTestFixture.hpp"

#include <boost/mpi/communicator.hpp>
#include <regex>

#include "plato/test_utilities/TestContext.hpp"

namespace plato::test_utilities
{
namespace
{
void checkRankZeroStringStream(const std::ostringstream& aStreamWithOutput,
                               const std::string& aGold,
                               const TestContext& aTestContext)
{
    if (boost::mpi::communicator{}.rank() == 0)
    {
        EXPECT_EQ(aStreamWithOutput.str(), aGold) << aTestContext;
    }
}

void checkRankZeroStringStreamStreamForPattern(const std::ostringstream& aStreamWithOutput,
                                               const std::vector<std::string>& aKeyList,
                                               const TestContext& aTestContext)
{
    const auto tString = aStreamWithOutput.str();
    if (boost::mpi::communicator{}.rank() == 0)
    {
        for (const auto& tKey : aKeyList)
        {
            auto tPattern = std::regex{tKey};
            EXPECT_TRUE(std::regex_search(tString, tPattern)) << aTestContext << ": " << tKey;
        }
    }
}

}  // namespace

CoutCerrPrintTestFixture::CoutCerrPrintTestFixture()
    : mOriginalCoutBuffer(std::cout.rdbuf()), mOriginalCerrBuffer(std::cerr.rdbuf())
{
    std::cout.rdbuf(mCoutStringStream.rdbuf());
    std::cerr.rdbuf(mCerrStringStream.rdbuf());
}

CoutCerrPrintTestFixture::~CoutCerrPrintTestFixture()
{
    std::cout.rdbuf(mOriginalCoutBuffer);
    std::cerr.rdbuf(mOriginalCerrBuffer);
}

void CoutCerrPrintTestFixture::checkRankZeroCoutStringStream(const std::string& aGold,
                                                             const TestContext& aTestContext) const
{
    checkRankZeroStringStream(mCoutStringStream, aGold, aTestContext);
}

void CoutCerrPrintTestFixture::checkRankZeroCerrStringStream(const std::string& aGold,
                                                             const TestContext& aTestContext) const
{
    checkRankZeroStringStream(mCerrStringStream, aGold, aTestContext);
}

void CoutCerrPrintTestFixture::checkRankZeroCoutStringStreamStreamForPattern(const std::vector<std::string>& aKeyList,
                                                                             const TestContext& aTestContext) const
{
    checkRankZeroStringStreamStreamForPattern(mCoutStringStream, aKeyList, aTestContext);
}

void CoutCerrPrintTestFixture::checkRankZeroCerrStringStreamStreamForPattern(const std::vector<std::string>& aKeyList,
                                                                             const TestContext& aTestContext) const
{
    checkRankZeroStringStreamStreamForPattern(mCerrStringStream, aKeyList, aTestContext);
}

void CoutCerrPrintTestFixture::clearStreams()
{
    mCoutStringStream.clear();
    mCerrStringStream.clear();
}

}  // namespace plato::test_utilities
