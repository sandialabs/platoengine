#include <gtest/gtest.h>

#include <Kokkos_Core.hpp>
#include <stk_util/environment/Env.hpp>

#include "PlatoKrinoUtilities.hpp"

int gl_argc = 0;
char** gl_argv = 0;

int main(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    Plato::Krino::initializeSTKEnvironment(MPI_COMM_WORLD);
    Plato::Krino::initializeKrinoLogging();
    Kokkos::initialize(argc, argv);
    testing::InitGoogleTest(&argc, argv);
    int returnVal = RUN_ALL_TESTS();
    Kokkos::finalize();
    MPI_Finalize();
    return returnVal;
}
