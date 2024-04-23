#include <gtest/gtest.h>
#include <Slib_Startup.h>
#include <stk_util/environment/Env.hpp>
#include <stk_util/parallel/Parallel.hpp>
#include <Kokkos_Core.hpp>
#include <parser/Prsr_Sierra_XMLDB.h>

int gl_argc = 0;
char** gl_argv = 0;

int main(int argc, char **argv)
{
    stk::parallel_machine_init(&argc, &argv);
    testing::InitGoogleTest(&argc, argv);
    Kokkos::initialize(argc, argv);

    sierra::Env::set_input_file_required(false);

    sierra::Env::StartupSierra(&argc, &argv, "PlatoKrinoUnit", "today");

    testing::InitGoogleTest(&argc, argv);
    int returnVal = RUN_ALL_TESTS();

    Kokkos::finalize();
    sierra::Env::ShutDownSierra(true);

    return returnVal;
}
