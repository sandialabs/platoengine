#include <Kokkos_Core.hpp>
#include <boost/mpi/environment.hpp>

#include "plato/main/library/CommandLineInputHandler.hpp"
#include "plato/main/library/Startup.hpp"
#include "plato/utilities/ArgcArgvToVector.hpp"

int main(int argc, char** argv)
{
    auto tEnvironment = boost::mpi::environment{argc, argv};
    Kokkos::initialize(argc, argv);

    std::vector<std::string> tArguments = plato::utilities::argc_argv_to_std_vector(argc, argv);
    plato::main::library::startup();
    plato::main::library::handle_input(tArguments);

    Kokkos::finalize();
    return 0;
}
