#include "plato/test_utilities/CoutPrintTestFixture.hpp"

#include <boost/mpi/communicator.hpp>
#include <boost/regex.hpp>

#include "plato/test_utilities/TestContext.hpp"

namespace plato::test_utilities
{

CoutPrintTestFixture::CoutPrintTestFixture() : mOriginalCoutBuffer(std::cout.rdbuf())
{
    std::cout.rdbuf(mOutputStringStream.rdbuf());
}

CoutPrintTestFixture::~CoutPrintTestFixture() { std::cout.rdbuf(mOriginalCoutBuffer); }

void CoutPrintTestFixture::checkRankZeroStringStream(const std::string& aGold, const TestContext& aTestContext)
{
    if (boost::mpi::communicator{}.rank() == 0)
    {
        EXPECT_EQ(mOutputStringStream.str(), aGold) << aTestContext;
    }
}

void CoutPrintTestFixture::checkRankZeroStringStreamForPattern(const std::vector<std::string>& aKeyList,
                                                               const TestContext& aTestContext)
{
    const auto tString = mOutputStringStream.str();
    if (boost::mpi::communicator{}.rank() == 0)
    {
        for (const auto& tKey : aKeyList)
        {
            boost::regex tPattern(tKey);
            EXPECT_TRUE(boost::regex_search(tString, tPattern)) << aTestContext << ": " << tKey;
        }
    }
}

void CoutPrintTestFixture::clearStream() { mOutputStringStream.clear(); }

}  // namespace plato::test_utilities
