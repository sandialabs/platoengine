#include "PlatoKrinoUtilities.hpp"

#include <string>

#include <Akri_DiagWriter.hpp>
#include <stk_util/diag/WriterRegistry.hpp>
#include <stk_util/environment/Env.hpp>
#include <stk_util/environment/EnvData.hpp>
#include <stk_util/environment/OutputLog.hpp>

namespace Plato
{
namespace
{
constexpr std::string_view kKrinoLogName = "krinolog";
}

void initialize_environment_for_krino(const MPI_Comm &aComm)
{
    // Initialize STK environment
    stk::EnvData::instance().m_parallelComm = aComm;
    MPI_Comm_size(stk::EnvData::parallel_comm(), &stk::EnvData::instance().m_parallelSize);
    MPI_Comm_rank(stk::EnvData::parallel_comm(), &stk::EnvData::instance().m_parallelRank);

    // Initialize krino logging
    sierra::Diag::registerWriter(std::string{kKrinoLogName}, ::krinolog, ::krino::theDiagWriterParser());
    const std::string tOutputDescription = "out>pout dout>out";
    const std::string tParallelOutputDescription = " pout>null";
    stk::bind_output_streams(tOutputDescription + tParallelOutputDescription);
}

}  // namespace Plato
