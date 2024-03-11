#include "plato/test_utilities/UnitMain.hpp"

#include <gtest/gtest.h>
#include <mpi.h>

#include <Kokkos_Core.hpp>
#include <string_view>

namespace plato::test_utilities
{
namespace
{
void output_xml(const int aMyExitCode, const int aGroupExitCode)
{
    int tRank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &tRank);
    const bool tShouldOutputForSuccess = aGroupExitCode == 0 && tRank == 0;
    const bool tShouldOutputForFailure = aMyExitCode != 0;
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
            MPI_Comm_spawn(tProgramName.data(), tArguments.data(), aNumRanks, MPI_INFO_NULL, 0, MPI_COMM_WORLD,
                           &tInterComm, tErrorCodes.data());
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

    int tExitStatus = EXIT_SUCCESS;
    const MPI_Comm tInterComm = setup_children(argc, argv, aNumRanks);
    if (rank_should_run_tests(tInterComm))
    {
        testing::InitGoogleTest(&argc, argv);
        const int tMyExitStatus = RUN_ALL_TESTS();
        const int tGroupExitStatus = communicate_exit_code(tInterComm, tMyExitStatus);
        output_xml(tMyExitStatus, tGroupExitStatus);
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
