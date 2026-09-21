#include <gtest/gtest.h>

#include <boost/smart_ptr/make_shared_object.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "plato/main/library/CommandLineInputHandler.hpp"
#include "plato/main/library/ExitStatus.hpp"
#include "plato/services/ExternalLoggerFileSink.hpp"
#include "plato/services/InternalLoggerConsoleSink.hpp"
#include "plato/test_utilities/CoutCerrPrintTestFixture.hpp"
#include "plato/test_utilities/Strings.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::main::library::unittest
{

namespace
{

const auto kPlatoArgumentsEmpty = std::vector<std::string>{};
const auto kPlatoArgumentsHelp = std::vector<std::string>{"--help"};
const auto kPlatoArgumentsVersion = std::vector<std::string>{"--version"};
const auto kPlatoArgumentsInputFile = std::vector<std::string>{"input.i"};
const auto kPlatoArgumentsJunk = std::vector<std::string>{"junk"};

const auto kAbortingKeys = std::vector<std::string>{"Executable", "--help", "Aborting"};
const auto kKnownInputKeys = std::vector<std::string>{"identity_filter", "filter_radius", "density_topology",
                                                      "initial_density_value", "[double]"};
const auto kKnownVersionKeys = std::vector<std::string>{"Version", "commit"};

const auto kCouldNotValidateKeys = std::vector<std::string>{"Parsing ", "error", "near:"};

}  // namespace

class CommandLineInputHandlerCoutFixture : public plato::test_utilities::CoutCerrPrintTestFixture
{
};

TEST_F(CommandLineInputHandlerCoutFixture, HandleInputEmpty)
{
    const auto tExitStatus = handle_input(kPlatoArgumentsEmpty);
    EXPECT_EQ(tExitStatus, ExitStatus::kCommandLineInputError);
    checkRankZeroCoutStringStreamStreamForPattern(kAbortingKeys, TEST_CONTEXT("Error message nothing sent to plato."));
}

TEST_F(CommandLineInputHandlerCoutFixture, HandleInputJunk)
{
    const auto tExitStatus = handle_input(kPlatoArgumentsJunk);
    EXPECT_EQ(tExitStatus, ExitStatus::kCommandLineInputError);
    checkRankZeroCoutStringStreamStreamForPattern(kAbortingKeys, TEST_CONTEXT("Error message junk was sent to plato."));
}

TEST_F(CommandLineInputHandlerCoutFixture, HandleInputHelp)
{
    const auto tExitStatus = handle_input(kPlatoArgumentsHelp);
    EXPECT_EQ(tExitStatus, ExitStatus::kExecutionSuccessful);
    checkRankZeroCoutStringStreamStreamForPattern(kKnownInputKeys, TEST_CONTEXT("Help message requested from plato."));
}

TEST_F(CommandLineInputHandlerCoutFixture, HandleInputVersion)
{
    const auto tExitStatus = handle_input(kPlatoArgumentsVersion);
    EXPECT_EQ(tExitStatus, ExitStatus::kExecutionSuccessful);
    checkRankZeroCoutStringStreamStreamForPattern(kKnownVersionKeys,
                                                  TEST_CONTEXT("Version message requested from plato."));
}

TEST(CommandLineInputHandler, RunPlatoEmptyFile)
{
    const auto tInput = std::filesystem::path{"input.i"};
    auto tOutfile = std::ofstream{tInput};
    const auto tLogSinkStream = boost::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLoggerSink = services::internal_logger_console_sink(tLogSinkStream);

    const auto tExitStatus = detail::run_plato(tInput);
    EXPECT_EQ(tExitStatus, ExitStatus::kInvalidInput);

    for (const auto& tCheckKey : kCouldNotValidateKeys)
    {
        test_utilities::expect_string_contains_substring(tLogSinkStream->str(), tCheckKey,
                                                         TEST_CONTEXT("Run plato empty file"));
    }

    std::filesystem::remove(tInput);
}

TEST_F(CommandLineInputHandlerCoutFixture, HandleInputEmptyFile)
{
    const auto tInput = std::filesystem::path{"input.i"};
    auto tOutfile = std::ofstream{tInput};
    const auto tArguments = std::vector<std::string>{tInput};

    const auto tExitStatus = handle_input(tArguments);

    EXPECT_EQ(tExitStatus, ExitStatus::kInvalidInput);
    checkRankZeroCoutStringStreamStreamForPattern(kCouldNotValidateKeys,
                                                  TEST_CONTEXT("Plato couldn't validate empty deck via handle_input."));

    std::filesystem::remove(tInput);
}

TEST(CommandLineInputHandler, DetailPrintErrorMessage)
{
    const auto tLogSinkStream = boost::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLoggerSink = services::internal_logger_console_sink(tLogSinkStream);

    detail::print_command_line_error_message();

    for (const auto& tCheckKey : kAbortingKeys)
    {
        test_utilities::expect_string_contains_substring(tLogSinkStream->str(), tCheckKey,
                                                         TEST_CONTEXT("Detail print error message"));
    }
}

TEST_F(CommandLineInputHandlerCoutFixture, DetailPrintKnownInputs)
{
    detail::print_known_inputs();
    checkRankZeroCoutStringStreamStreamForPattern(kKnownInputKeys, TEST_CONTEXT("Print known inputs."));
}

TEST_F(CommandLineInputHandlerCoutFixture, HelpDoesNotGenerateLogDirectory)
{
    const auto tExitStatus = handle_input(kPlatoArgumentsHelp);
    EXPECT_EQ(tExitStatus, ExitStatus::kExecutionSuccessful);
    EXPECT_FALSE(std::filesystem::exists(services::logging_directory_path()));
}

}  // namespace plato::main::library::unittest
