#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "plato/services/ExternalLoggerFileSink.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/SeverityLogger.hpp"

namespace plato::services::unittest
{
namespace
{
void check_external_log(const std::filesystem::path& aTestLogPath, const test_utilities::TestContext& aTestContext)
{
    namespace tpi_bl = third_party_integration::boost_log;

    constexpr auto tExternalMessage = std::string_view{"External log message"};
    constexpr auto tInternalMessage = std::string_view{"Internal log message"};

    {
        // Scoped to close log sink before checking results log file
        [[maybe_unused]] const auto tLogSink = external_logger_file_sink(aTestLogPath);

        auto tExternalLogger = tpi_bl::SeverityLogger{tpi_bl::LogSourceAttribute<tpi_bl::LogSource::kExternal>{}};
        tExternalLogger.logMessage(tExternalMessage, tpi_bl::Severity::kInfo);

        auto tInternalLogger = tpi_bl::SeverityLogger{tpi_bl::LogSourceAttribute<tpi_bl::LogSource::kInternal>{}};
        tInternalLogger.logMessage(tInternalMessage, tpi_bl::Severity::kInfo);
    }

    const auto tFileContents = test_utilities::file_to_string(aTestLogPath);

    EXPECT_NE(tFileContents.find(tExternalMessage), std::string::npos) << aTestContext << "Log: " << tFileContents;
    EXPECT_EQ(tFileContents.find(tInternalMessage), std::string::npos) << aTestContext << "Log: " << tFileContents;
}
}  // namespace

TEST(ExternalLoggerFileSink, LogsToNewFile)
{
    const auto tTestLogPath = std::filesystem::path{"test-log.txt"};
    check_external_log(tTestLogPath, TEST_CONTEXT("New file, no directory"));
    EXPECT_TRUE(std::filesystem::remove(tTestLogPath));
}

TEST(ExternalLoggerFileSink, LogsToNewFileWithinExistentDirectory)
{
    const auto tDirectory = std::filesystem::path{"logs"};
    std::filesystem::create_directory(tDirectory);
    const auto tTestLogPath = tDirectory / std::filesystem::path{"test-log.txt"};
    check_external_log(tTestLogPath, TEST_CONTEXT("New file, existing directory"));

    EXPECT_EQ(std::filesystem::remove_all(tDirectory), 2U);
}

TEST(ExternalLoggerFileSink, LogsToNewFileWithinNonexistentDirectory)
{
    const auto tDirectory = std::filesystem::path{"logs"};
    const auto tTestLogPath = tDirectory / std::filesystem::path{"test-log.txt"};
    check_external_log(tTestLogPath, TEST_CONTEXT("New file, new directory"));

    EXPECT_EQ(std::filesystem::remove_all(tDirectory), 2U);
}

TEST(ExternalLoggerFileSink, LogsToExistingFile)
{
    const auto tTestLogPath = std::filesystem::path{"test-log.txt"};
    constexpr auto tExistingMessage = std::string_view{"Previous log message\n"};
    {
        auto tFileStream = std::ofstream{tTestLogPath};
        tFileStream << tExistingMessage;
    }

    check_external_log(tTestLogPath, TEST_CONTEXT("Existing file"));

    const auto tFileContents = test_utilities::file_to_string(tTestLogPath);

    EXPECT_NE(tFileContents.find(tExistingMessage), std::string::npos) << "Log: " << tFileContents;
    EXPECT_TRUE(std::filesystem::remove(tTestLogPath));
}

TEST(ExternalLoggerFileSink, ExternalLogFileName)
{
    const auto tLogFilePath =
        external_log_file_path(components::ComponentType::kProcessManager, boost::mpi::communicator{});
    const auto tExpected = std::filesystem::path{"logs/process-manager-rank-0.txt"};
    EXPECT_EQ(tLogFilePath, tExpected);
}

}  // namespace plato::services::unittest
