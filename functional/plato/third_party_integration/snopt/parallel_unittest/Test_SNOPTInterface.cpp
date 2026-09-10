#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <regex>
#include <string>

#include "plato/third_party_integration/snopt/SNOPTInterface.hpp"
#include "plato/third_party_integration/snopt/test_utilities/TestUtilities.hpp"
#include "plato/utilities/MPIUtilities.hpp"

namespace plato::third_party_integration::snopt::parallel_unittest
{

namespace
{
const auto kLogFilePath = std::filesystem::path{"parallel_snopt.log"};

[[nodiscard]] auto number_of_occurrences_of_snopt_header(const std::filesystem::path& aPath)
{
    std::ifstream tFile{aPath};
    const std::string tContents{std::istreambuf_iterator<char>{tFile}, std::istreambuf_iterator<char>{}};
    const std::regex tPattern{R"(S N O P T  [0-9]\.[0-9]\.[0-9])"};
    return std::distance(std::sregex_iterator{tContents.begin(), tContents.end(), tPattern}, std::sregex_iterator{});
}
}  // namespace

TEST(SNOPTInterface, OutputFunctionGetsCalledByRankZero)
{
    const auto tCommunicator = boost::mpi::communicator{};
    test_utilities::run_snopt_write_output_counter_test(kLogFilePath, tCommunicator);
    tCommunicator.barrier();
    constexpr int tExpectedCount = 1;
    const auto tCount = number_of_occurrences_of_snopt_header(kLogFilePath);
    EXPECT_EQ(tCount, tExpectedCount);

    utilities::execute_on_root(tCommunicator, []() { std::filesystem::remove(kLogFilePath); });
}

TEST(SNOPTInterface, DetailMakeSNOPTOutputConfig)
{
    const auto tCommunicator = boost::mpi::communicator{};
    const auto tOutputConfig = detail::make_snopt_output_config(tCommunicator, kLogFilePath);

    if (constexpr auto tRootRank = 0; tCommunicator.rank() == tRootRank)
    {
        EXPECT_EQ(tOutputConfig.mPrintFile, kLogFilePath.string());
        EXPECT_NE(tOutputConfig.mPrintUnit, 0);
    }
    else
    {
        EXPECT_TRUE(tOutputConfig.mPrintFile.empty());
        EXPECT_EQ(tOutputConfig.mPrintUnit, 0);
    }
    EXPECT_EQ(tOutputConfig.mSummaryUnit, 0);
}

}  // namespace plato::third_party_integration::snopt::parallel_unittest
