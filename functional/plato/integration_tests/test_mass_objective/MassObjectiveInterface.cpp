#include "plato/integration_tests/test_mass_objective/MassObjectiveInterface.hpp"

#include <mpi.h>

#include <iostream>

#include "plato/integration_tests/test_mass_objective/MassObjective.hpp"

namespace plato::integration_tests::test_mass_objective
{
MassObjectiveInterface::MassObjectiveInterface()
{
    int tMPIInitialized = 0;
    MPI_Initialized(&tMPIInitialized);
    if (tMPIInitialized == 0)
    {
        std::cout << "Initializing MPI in MassObjective" << std::endl;
        int tArgc = 0;
        char** tArgv = nullptr;
        MPI_Init(&tArgc, &tArgv);
    }
    else
    {
        std::cout << "MPI already initialized in MassObjective" << std::endl;
    }
}

double MassObjectiveInterface::value(const core::MeshProxy& aMeshProxy) const
{
    constexpr double tDensity = 1.0;
    constexpr double tTarget = 0.0;
    const auto tMassObjective = MassObjective{tDensity, tTarget};
    return tMassObjective.mass(aMeshProxy.mFileName.string());
}

std::vector<double> MassObjectiveInterface::gradient(const core::MeshProxy& aMeshProxy) const
{
    ///@todo Populate the gradient with actual values
    constexpr double tDensity = 1.0;
    constexpr double tTarget = 0.0;
    const auto tMassObjective = MassObjective{tDensity, tTarget};
    constexpr unsigned int tNumDimensions = 3;
    const unsigned int tGradientSize = tMassObjective.numMeshNodes(aMeshProxy.mFileName.string()) * tNumDimensions;
    return std::vector<double>(tGradientSize, 0.0);
}
}  // namespace plato::integration_tests::test_mass_objective

namespace plato
{
std::unique_ptr<criteria::library::CriterionInterface> plato_create_criterion(const std::vector<std::string>&)
{
    return std::make_unique<integration_tests::test_mass_objective::MassObjectiveInterface>();
}
}  // namespace plato
