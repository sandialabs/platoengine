#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <fstream>

#include "plato/main/library/CommandLineInputHandler.hpp"
#include "plato/test_utilities/CoutCerrPrintTestFixture.hpp"
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

using plato::test_utilities::CoutCerrPrintTestFixture;

TEST_F(CoutCerrPrintTestFixture, HandleInputEmpty)
{
    handle_input(kPlatoArgumentsEmpty);
    checkRankZeroCoutStringStreamStreamForPattern(kAbortingKeys, TEST_CONTEXT("Error message nothing sent to plato."));
}

TEST_F(CoutCerrPrintTestFixture, HandleInputJunk)
{
    handle_input(kPlatoArgumentsJunk);
    checkRankZeroCoutStringStreamStreamForPattern(kAbortingKeys, TEST_CONTEXT("Error message junk was sent to plato."));
}

TEST_F(CoutCerrPrintTestFixture, HandleInputHelp)
{
    handle_input(kPlatoArgumentsHelp);
    checkRankZeroCoutStringStreamStreamForPattern(kKnownInputKeys, TEST_CONTEXT("Help message requested from plato."));
}

TEST_F(CoutCerrPrintTestFixture, RunPlato)
{
    const auto tInput = std::filesystem::path{"input.i"};
    const auto tArguments = std::vector<std::string>{tInput};
    std::ofstream tOutfile(tInput);
    {
        detail::run_plato(tInput);
        checkRankZeroCoutStringStreamStreamForPattern(
            kCouldNotValidateKeys, TEST_CONTEXT("Plato couldn't validate empty deck via run_plato."));
        clearStreams();
    }
    {
        handle_input(tArguments);
        checkRankZeroCoutStringStreamStreamForPattern(
            kCouldNotValidateKeys, TEST_CONTEXT("Plato couldn't validate empty deck via handle_input."));
    }
    std::filesystem::remove(tInput);
}

TEST_F(CoutCerrPrintTestFixture, DetailPrintErrorMessage)
{
    detail::print_command_line_error_message();
    checkRankZeroCoutStringStreamStreamForPattern(kAbortingKeys, TEST_CONTEXT("Error message with abort statement."));
}

TEST_F(CoutCerrPrintTestFixture, DetailPrintKnownInputs)
{
    detail::print_known_inputs();
    checkRankZeroCoutStringStreamStreamForPattern(kKnownInputKeys, TEST_CONTEXT("Print known inputs."));
}

}  // namespace plato::main::library::unittest
