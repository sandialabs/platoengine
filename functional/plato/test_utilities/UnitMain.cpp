#include "plato/test_utilities/UnitMain.hpp"

#include <gtest/gtest.h>
#include <mpi.h>

#include <Kokkos_Core.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/attributes/attribute_value_set.hpp>
#include <boost/log/core.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <string_view>

#include "plato/utilities/NamedType.hpp"

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
        delete tListeners.Release(tListeners.default_xml_generator());
    }
}

auto program_name_and_arguments_for_mpi(const int argc, char** argv) -> std::pair<std::string_view, std::vector<char*>>
{
    const auto tProgramName = std::string_view{argv[0]};
    auto tArguments = std::vector<char*>{};
    std::copy(argv + 1, argv + argc, std::back_inserter(tArguments));
    tArguments.push_back(nullptr);  // MPI lists are null-terminated
    return {tProgramName, tArguments};
}

/// @brief Spawns child processes if necessary.
///
/// This will spawn child processes if test suite was launched with only one rank.
/// @return The intercomm created when calling `MPI_Comm_spawn` for parents, `MPI_COMM_NULL` otherwise.
MPI_Comm setup_children(int argc, char** argv, unsigned int aNumRanks)
{
    MPI_Comm tInterComm = MPI_COMM_NULL;
    int tNumRanks = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &tNumRanks);
    if (tNumRanks == 1)
    {
        std::vector<int> tErrorCodes(aNumRanks);
        MPI_Comm tParentComm;
        MPI_Comm_get_parent(&tParentComm);
        if (tParentComm == MPI_COMM_NULL)
        {
            std::cout << "Spawning " << aNumRanks << " new ranks" << std::endl;
            auto [tProgramName, tArguments] = program_name_and_arguments_for_mpi(argc, argv);
            MPI_Comm_spawn(tProgramName.data(), tArguments.data(), static_cast<int>(aNumRanks), MPI_INFO_NULL, 0,
                           MPI_COMM_WORLD, &tInterComm, tErrorCodes.data());
        }
    }
    return tInterComm;
}

bool rank_should_run_tests(const MPI_Comm& tInterComm) { return tInterComm == MPI_COMM_NULL; }

int communicate_exit_code(const MPI_Comm& tInterComm, const int tExitStatus)
{
    int tMaxExitStatus = tExitStatus;
    if (tInterComm != MPI_COMM_NULL)
    {
        MPI_Allreduce(&tExitStatus, &tMaxExitStatus, 1, MPI_INT, MPI_MAX, tInterComm);
    }
    else
    {
        MPI_Comm tParentComm;
        MPI_Comm_get_parent(&tParentComm);
        if (tParentComm != MPI_COMM_NULL)
        {
            MPI_Allreduce(&tExitStatus, &tMaxExitStatus, 1, MPI_INT, MPI_MAX, tParentComm);
        }
    }
    return tMaxExitStatus;
}

}  // namespace

int unit_main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    Kokkos::initialize(argc, argv);
    [[maybe_unused]] const auto tLogSink = null_log_sink();

    testing::InitGoogleTest(&argc, argv);
    const int returnVal = RUN_ALL_TESTS();

    Kokkos::finalize();
    MPI_Finalize();

    return returnVal;
}

int parallel_unit_main(int argc, char** argv, unsigned int aNumRanks)
{
    MPI_Init(&argc, &argv);
    Kokkos::initialize(argc, argv);
    [[maybe_unused]] const auto tLogSink = null_log_sink();

    int tExitStatus = EXIT_SUCCESS;
    const MPI_Comm tInterComm = setup_children(argc, argv, aNumRanks);
    if (rank_should_run_tests(tInterComm))
    {
        testing::InitGoogleTest(&argc, argv);
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
    MPI_Finalize();

    return tExitStatus;
}

}  // namespace plato::test_utilities
