#include "plato/integration_tests/test_mass_criteria/ParallelMassObjectiveInterface.hpp"

#include <mpi.h>

#include <boost/mpi/communicator.hpp>
#include <memory>
#include <string>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/library/CriterionInterface.hpp"
#include "plato/integration_tests/test_mass_criteria/MassObjectiveInterface.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/DynamicVectorSerialization.hpp"  // NOLINT(misc-include-cleaner)
#include "plato/test_utilities/ParallelTestWrapper.hpp"

namespace plato::integration_tests::test_mass_criteria
{
ParallelMassObjectiveInterface::ParallelMassObjectiveInterface(MPI_Comm aCommunicator)
    : mComm{aCommunicator, boost::mpi::comm_attach}
{
}

auto ParallelMassObjectiveInterface::value(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const -> double
{
    const auto tParallelizedValue =
        test_utilities::ParallelTestFunctionWrapper<double, const analysis::AnalysisDomainMesh&>{
            [](const analysis::AnalysisDomainMesh& aAnalysisDomainMeshLambdaArg)
            { return MassObjectiveInterface{}.value(aAnalysisDomainMeshLambdaArg); }};
    const auto tResult = tParallelizedValue(aAnalysisDomainMesh, mComm);
    return tResult;
}

auto ParallelMassObjectiveInterface::gradient(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
    -> std::vector<double>
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

auto plato_create_parallel_criterion(const std::vector<std::string>&, MPI_Comm aCommunicator)
    -> std::unique_ptr<::plato::criteria::library::CriterionInterface>
{
    return std::make_unique<::plato::integration_tests::test_mass_criteria::ParallelMassObjectiveInterface>(
        aCommunicator);
}

auto plato_create_parallel_test_mass_criterion(const std::vector<std::string>&, MPI_Comm aCommunicator)
    -> std::unique_ptr<::plato::criteria::library::CriterionInterface>
{
    return std::make_unique<::plato::integration_tests::test_mass_criteria::ParallelMassObjectiveInterface>(
        aCommunicator);
}
