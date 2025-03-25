#include "plato/integration_tests/test_mass_criteria/ParallelMassObjectiveInterface.hpp"

#include <boost/mpi/communicator.hpp>

#include "plato/integration_tests/test_mass_criteria/MassObjectiveInterface.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/DynamicVectorSerialization.hpp"
#include "plato/test_utilities/ParallelTestWrapper.hpp"

namespace plato::integration_tests::test_mass_criteria
{
ParallelMassObjectiveInterface::ParallelMassObjectiveInterface(MPI_Comm aComm) : mComm{aComm, boost::mpi::comm_attach}
{
}

double ParallelMassObjectiveInterface::value(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    const auto tParallelizedValue =
        test_utilities::ParallelTestFunctionWrapper<double, const analysis::AnalysisDomainMesh&>{
            [](const analysis::AnalysisDomainMesh& aAnalysisDomainMeshLambdaArg)
            { return MassObjectiveInterface{}.value(aAnalysisDomainMeshLambdaArg); }};
    const auto tResult = tParallelizedValue(aAnalysisDomainMesh, mComm);
    return tResult;
}

std::vector<double> ParallelMassObjectiveInterface::gradient(
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
{
    const auto tParallelizedGradient =
        test_utilities::ParallelTestFunctionWrapper<linear_algebra::DynamicVector<double>,
                                                    const analysis::AnalysisDomainMesh&>{
            [](const analysis::AnalysisDomainMesh& aAnalysisDomainMeshLambdaArg) {
                return linear_algebra::DynamicVector<double>{
                    MassObjectiveInterface{}.gradient(aAnalysisDomainMeshLambdaArg)};
            }};
    return tParallelizedGradient(aAnalysisDomainMesh, mComm).stdVector();
}

}  // namespace plato::integration_tests::test_mass_criteria

std::unique_ptr<::plato::criteria::library::CriterionInterface> plato_create_parallel_criterion(
    const std::vector<std::string>&, const MPI_Comm aComm)
{
    return std::make_unique<::plato::integration_tests::test_mass_criteria::ParallelMassObjectiveInterface>(aComm);
}

std::unique_ptr<::plato::criteria::library::CriterionInterface> plato_create_parallel_test_mass_criterion(
    const std::vector<std::string>&, const MPI_Comm aComm)
{
    return std::make_unique<::plato::integration_tests::test_mass_criteria::ParallelMassObjectiveInterface>(aComm);
}
