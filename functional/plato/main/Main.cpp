#include <Kokkos_Core.hpp>
#include <boost/mpi/environment.hpp>

#include "plato/main/library/CommandLineInputHandler.hpp"
#include "plato/main/library/Startup.hpp"
#include "plato/utilities/ArgcArgvToVector.hpp"

auto main(int argc, char** argv) -> int
{
    auto tEnvironment = boost::mpi::environment{argc, argv, boost::mpi::threading::level::funneled};
    Kokkos::initialize(argc, argv);

    std::vector<std::string> tArguments = plato::utilities::argc_argv_to_std_vector(argc, argv);
    plato::main::library::startup();
    const int tExitStatus = static_cast<int>(plato::main::library::handle_input(tArguments));

    Kokkos::finalize();
    return tExitStatus;
}
