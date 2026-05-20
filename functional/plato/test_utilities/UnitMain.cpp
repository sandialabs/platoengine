#include "plato/test_utilities/UnitMain.hpp"

#include <gtest/gtest.h>
#include <mpi.h>

#include <boost/core/null_deleter.hpp>
#include <boost/log/attributes/attribute_value_set.hpp>
#include <boost/log/core.hpp>  // NOLINT(misc-include-cleaner)
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/mpi/environment.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <cstddef>
#include <cstdlib>
#include <impl/Kokkos_InitializeFinalize.hpp>
#include <iostream>
#include <memory>
#include <ostream>
#include <ranges>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

#include "plato/utilities/NamedType.hpp"

// openmpi casts MPI communicators through void via a macro, so this check has to be disabled in this file.
// NOLINTBEGIN(bugprone-casting-through-void)

namespace plato::test_utilities
{
namespace
{
using RankExitCode = utilities::NamedType<int, struct RankExitCodeTag>;
using GroupExitCode = utilities::NamedType<int, struct GroupExitCodeTag>;

using TextOstreamSink = boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend>;

/// @brief Sets the default log sink filter to filter out all messages. This only applies when no other log sink has
/// been set up.
[[nodiscard]] auto null_log_sink() -> boost::shared_ptr<TextOstreamSink>
{
    auto tSink = boost::make_shared<TextOstreamSink>();
    auto tSinkStream = boost::shared_ptr<std::ostream>{&std::cout, boost::null_deleter()};
    tSink->locked_backend()->add_stream(tSinkStream);
    tSink->set_filter(boost::log::filter{[](const boost::log::attribute_value_set&) { return false; }});
    boost::log::core::get()->add_sink(tSink);
    return tSink;
}

void output_xml(const RankExitCode aMyExitCode, const GroupExitCode aGroupExitCode)
{
    int tRank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &tRank);
    const bool tShouldOutputForSuccess = aGroupExitCode.mValue == 0 && tRank == 0;
    const bool tShouldOutputForFailure = aMyExitCode.mValue != 0;
    const bool tShouldSilenceOutput = !(tShouldOutputForSuccess || tShouldOutputForFailure);
    if (tShouldSilenceOutput)
    {
        ::testing::TestEventListeners& tListeners = ::testing::UnitTest::GetInstance()->listeners();
        [[maybe_unused]] const auto tListenersXMLGenerator =
            std::unique_ptr<testing::TestEventListener>{tListeners.Release(tListeners.default_xml_generator())};
    }
}

auto program_name_and_arguments_for_mpi(const int aArgc,
                                        char* const* const aArgv) -> std::pair<std::string_view, std::vector<char*>>
{
    const auto tArgvSpan = std::span(aArgv, static_cast<std::size_t>(aArgc));
    const auto tProgramName = std::string_view{tArgvSpan[0]};
    const auto tProgramArguments = tArgvSpan | std::views::drop(1U);

    auto tArgumentsAsVector = std::vector<char*>(tProgramArguments.begin(), tProgramArguments.end());
    tArgumentsAsVector.push_back(nullptr);  // MPI lists are null-terminated
    return {tProgramName, tArgumentsAsVector};
}

/// @brief Spawns child processes if necessary.
///
/// This will spawn child processes if test suite was launched with only one rank.
/// @return The intercomm created when calling `MPI_Comm_spawn` for parents, `MPI_COMM_NULL` otherwise.
auto setup_children(int aArgc, char** aArgv, unsigned int aNumRanks) -> MPI_Comm
{
    MPI_Comm tInterComm = MPI_COMM_NULL;
    int tNumRanks = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &tNumRanks);
    if (tNumRanks == 1)
    {
        std::vector<int> tErrorCodes(aNumRanks);
        MPI_Comm tParentCommunicator = MPI_COMM_NULL;
        MPI_Comm_get_parent(&tParentCommunicator);
        if (tParentCommunicator == MPI_COMM_NULL)
        {
            std::cout << "Spawning " << aNumRanks << " new ranks\n";
            auto [tProgramName, tArguments] = program_name_and_arguments_for_mpi(aArgc, aArgv);
            MPI_Comm_spawn(tProgramName.data(), tArguments.data(), static_cast<int>(aNumRanks), MPI_INFO_NULL, 0,
                           MPI_COMM_WORLD, &tInterComm, tErrorCodes.data());
        }
    }
    return tInterComm;
}

auto rank_should_run_tests(const MPI_Comm& aInterCommunicator) -> bool { return aInterCommunicator == MPI_COMM_NULL; }

auto communicate_exit_code(const MPI_Comm& aInterCommunicator, const int aTExitStatus) -> int
{
    int tMaxExitStatus = aTExitStatus;
    if (aInterCommunicator != MPI_COMM_NULL)
    {
        MPI_Allreduce(&aTExitStatus, &tMaxExitStatus, 1, MPI_INT, MPI_MAX, aInterCommunicator);
    }
    else
    {
        MPI_Comm tParentCommunicator = MPI_COMM_NULL;
        MPI_Comm_get_parent(&tParentCommunicator);
        if (tParentCommunicator != MPI_COMM_NULL)
        {
            MPI_Allreduce(&aTExitStatus, &tMaxExitStatus, 1, MPI_INT, MPI_MAX, tParentCommunicator);
        }
    }
    return tMaxExitStatus;
}

}  // namespace

auto unit_main(int argc, char** argv) -> int
{
    auto tEnvironment = boost::mpi::environment{argc, argv, boost::mpi::threading::level::funneled};
    Kokkos::initialize(argc, argv);
    [[maybe_unused]] const auto tLogSink = null_log_sink();

    testing::InitGoogleTest(&argc, argv);
    const int tReturnVal = RUN_ALL_TESTS();

    Kokkos::finalize();

    return tReturnVal;
}

auto parallel_unit_main(int aArgc, char** aArgv, unsigned int aNumRanks) -> int
{
    auto tEnvironment = boost::mpi::environment{aArgc, aArgv, boost::mpi::threading::level::funneled};
    Kokkos::initialize(aArgc, aArgv);
    [[maybe_unused]] const auto tLogSink = null_log_sink();

    int tExitStatus = EXIT_SUCCESS;
    MPI_Comm tInterComm = setup_children(aArgc, aArgv, aNumRanks);
    if (rank_should_run_tests(tInterComm))
    {
        testing::InitGoogleTest(&aArgc, aArgv);
        const int tMyExitStatus = RUN_ALL_TESTS();
        const int tGroupExitStatus = communicate_exit_code(tInterComm, tMyExitStatus);
        output_xml(RankExitCode{tMyExitStatus}, GroupExitCode{tGroupExitStatus});
        tExitStatus = tGroupExitStatus;
    }
    else
    {
        // Child processes run tests, so get their exit code
        tExitStatus = communicate_exit_code(tInterComm, tExitStatus);
    }

    Kokkos::finalize();

    return tExitStatus;
}

}  // namespace plato::test_utilities

// NOLINTEND(bugprone-casting-through-void)
