#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <fstream>

#include "plato/main/library/CommandLineInputHandler.hpp"
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
const auto kPlatoArgumentsInputFile = std::vector<std::string>{"input.i"};
const auto kPlatoArgumentsJunk = std::vector<std::string>{"junk"};

const auto kAbortingKeys = std::vector<std::string>{"Executable", "--help", "Aborting"};
const auto kKnownInputKeys = std::vector<std::string>{"identity_filter", "filter_radius", "density_topology",
                                                      "initial_density_value", "[double]"};

const auto kCouldNotValidateKeys = std::vector<std::string>{"Parsing ", "error", "near:"};

}  // namespace

class CommandLineInputHandlerCoutFixture : public plato::test_utilities::CoutCerrPrintTestFixture
{
};

TEST_F(CommandLineInputHandlerCoutFixture, HandleInputEmpty)
{
    handle_input(kPlatoArgumentsEmpty);
    checkRankZeroCoutStringStreamStreamForPattern(kAbortingKeys, TEST_CONTEXT("Error message nothing sent to plato."));
}

TEST_F(CommandLineInputHandlerCoutFixture, HandleInputJunk)
{
    handle_input(kPlatoArgumentsJunk);
    checkRankZeroCoutStringStreamStreamForPattern(kAbortingKeys, TEST_CONTEXT("Error message junk was sent to plato."));
}

TEST_F(CommandLineInputHandlerCoutFixture, HandleInputHelp)
{
    handle_input(kPlatoArgumentsHelp);
    checkRankZeroCoutStringStreamStreamForPattern(kKnownInputKeys, TEST_CONTEXT("Help message requested from plato."));
}

TEST(CommandLineInputHandler, RunPlatoEmptyFile)
{
    const auto tInput = std::filesystem::path{"input.i"};
    auto tOutfile = std::ofstream{tInput};
    const auto tLogSinkStream = boost::make_shared<std::stringstream>();
    [[maybe_unused]] const auto tLoggerSink = services::internal_logger_console_sink(tLogSinkStream);

    detail::run_plato(tInput);

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

    handle_input(tArguments);

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

}  // namespace plato::main::library::unittest
