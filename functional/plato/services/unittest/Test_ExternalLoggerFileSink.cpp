#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "plato/services/ExternalLoggerFileSink.hpp"
#include "plato/third_party_integration/boost_log/LogSource.hpp"
#include "plato/third_party_integration/boost_log/SeverityLogger.hpp"

namespace plato::services::unittest
{
TEST(ExternalLoggerFileSink, LogsToNewFile)
{
    namespace tpi_bl = third_party_integration::boost_log;

    const auto tTestLogPath = std::filesystem::path{"test-log.txt"};
    constexpr auto tExternalMessage = std::string_view{"External log message"};
    constexpr auto tInternalMessage = std::string_view{"Internal log message"};

    {
        // Scoped to close log sink before checking results log file
        [[maybe_unused]] const auto tLogSink = external_logger_file_sink(tTestLogPath);

        auto tExternalLogger = tpi_bl::SeverityLogger{tpi_bl::LogSourceAttribute<tpi_bl::LogSource::kExternal>{}};
        tExternalLogger.logMessage(tExternalMessage, tpi_bl::Severity::kInfo);

        auto tInternalLogger = tpi_bl::SeverityLogger{tpi_bl::LogSourceAttribute<tpi_bl::LogSource::kInternal>{}};
        tInternalLogger.logMessage(tInternalMessage, tpi_bl::Severity::kInfo);
    }

    auto tFile = std::ifstream{tTestLogPath};
    auto tFileContents = std::stringstream{};
    tFile >> tFileContents.rdbuf();

    EXPECT_NE(tFileContents.str().find(tExternalMessage), std::string::npos) << "Log: " << tFileContents.str();
    EXPECT_EQ(tFileContents.str().find(tInternalMessage), std::string::npos) << "Log: " << tFileContents.str();

    EXPECT_TRUE(std::filesystem::remove(tTestLogPath));
}

}  // namespace plato::services::unittest
