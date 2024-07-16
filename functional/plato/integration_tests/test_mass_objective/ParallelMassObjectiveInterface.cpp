#include "plato/integration_tests/test_mass_objective/ParallelMassObjectiveInterface.hpp"

#include <boost/mpi/communicator.hpp>

#include "plato/integration_tests/test_mass_objective/MassObjectiveInterface.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/DynamicVectorSerialization.hpp"
#include "plato/test_utilities/ParallelTestWrapper.hpp"

namespace plato::integration_tests::test_mass_objective
{
ParallelMassObjectiveInterface::ParallelMassObjectiveInterface(MPI_Comm aComm) : mComm{aComm, boost::mpi::comm_attach}
{
}

double ParallelMassObjectiveInterface::value(const core::MeshProxy& aMeshProxy) const
{
    const auto tParallelizedValue = test_utilities::ParallelTestFunctionWrapper<double, const core::MeshProxy&>{
        [](const core::MeshProxy& aMeshProxyLambdaArg) { return MassObjectiveInterface{}.value(aMeshProxyLambdaArg); }};
    const auto tResult = tParallelizedValue(aMeshProxy, mComm);
    return tResult;
}

std::vector<double> ParallelMassObjectiveInterface::gradient(const core::MeshProxy& aMeshProxy) const
{
    const auto tParallelizedGradient =
        test_utilities::ParallelTestFunctionWrapper<linear_algebra::DynamicVector<double>, const core::MeshProxy&>{
            [](const core::MeshProxy& aMeshProxyLambdaArg)
            { return linear_algebra::DynamicVector<double>{MassObjectiveInterface{}.gradient(aMeshProxyLambdaArg)}; }};
    return tParallelizedGradient(aMeshProxy, mComm).stdVector();
}

}  // namespace plato::integration_tests::test_mass_objective

std::unique_ptr<::plato::criteria::library::CriterionInterface> plato_create_parallel_criterion(
    const std::vector<std::string>&, const MPI_Comm aComm)
{
    return std::make_unique<::plato::integration_tests::test_mass_objective::ParallelMassObjectiveInterface>(aComm);
}

std::unique_ptr<::plato::criteria::library::CriterionInterface> plato_create_parallel_test_mass_criterion(
    const std::vector<std::string>&, const MPI_Comm aComm)
{
    return std::make_unique<::plato::integration_tests::test_mass_objective::ParallelMassObjectiveInterface>(aComm);
}
