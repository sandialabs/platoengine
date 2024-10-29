#include <Kokkos_Core.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/mpi/environment.hpp>
#include <iostream>

#include "plato/main/library/Executor.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerFactory.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
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
    namespace ppml = plato::process_manager::library;

    auto tEnvironment = boost::mpi::environment{argc, argv};
    Kokkos::initialize(argc, argv);

    if (argc == 2)
    {
        try
        {
            const auto tValidatedInput = ppml::parse_and_validate_from_file(argv[1]);
            const auto tExecutor =
                plato::main::library::Executor{ppml::make_process_managers(tValidatedInput.processManagers())};
            const auto tProcessManagerData = ppml::make_process_manager_data(tValidatedInput);
            tExecutor.execute(tProcessManagerData);
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
