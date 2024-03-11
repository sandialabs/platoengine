#include <Kokkos_Core.hpp>
#include <boost/mpi/environment.hpp>
#include <iostream>

#include "plato/main/library/OptimizationProblem.hpp"
#include "plato/utilities/Exception.hpp"

namespace
{
void printMessage(const std::string_view aMessage)
{
    if (boost::mpi::communicator{}.rank() == 0)
    {
        std::cout << aMessage << std::endl;
    }
}
}  // namespace

int main(int argc, char** argv)
{
    auto tEnvironment = boost::mpi::environment{argc, argv};
    Kokkos::initialize(argc, argv);

    if (argc == 2)
    {
        try
        {
            auto tOptimizationProblem = plato::main::library::OptimizationProblem{argv[1]};
            tOptimizationProblem.optimize();
        }
        catch (const plato::utilities::Exception& tError)
        {
            printMessage(tError.what());
        }
    }
    else
    {
        printMessage("Executable expects an input file name as an argument. Aborting.");
    }

    Kokkos::finalize();

    return 0;
}
