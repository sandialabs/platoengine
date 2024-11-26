#include "plato/third_party_integration/krino/Utilities.hpp"

#include <Akri_BoundingBoxMesh.hpp>
#include <Akri_CDFEM_Support.hpp>
#include <Akri_DiagWriter.hpp>
#include <Akri_LevelSet.hpp>
#include <Akri_MeshHelpers.hpp>
#include <Akri_OutputUtils.hpp>
#include <numeric>
#include <stk_util/diag/WriterRegistry.hpp>
#include <stk_util/environment/Env.hpp>
#include <stk_util/environment/EnvData.hpp>
#include <stk_util/environment/OutputLog.hpp>
#include <string_view>

namespace plato::third_party_integration::krino
{

namespace
{
constexpr auto kOutputDescription = std::string_view{"out>pout dout>out pout>null"};

}  // namespace

void initialize_environment_for_krino(const std::filesystem::path &aLogFile, const MPI_Comm &aComm)
{
    // Initialize STK environment
    stk::EnvData::instance().m_parallelComm = aComm;
    MPI_Comm_size(stk::EnvData::parallel_comm(), &stk::EnvData::instance().m_parallelSize);
    MPI_Comm_rank(stk::EnvData::parallel_comm(), &stk::EnvData::instance().m_parallelRank);

    // Initialize krino logging
    sierra::Diag::registerWriter(std::string{aLogFile}, ::krinolog, ::krino::theDiagWriterParser());
    stk::bind_output_streams(std::string{kOutputDescription});
}

void create_bounding_box_mesh(const stk::math::Vector3d &aMinCorner,
                              const stk::math::Vector3d &aMaxCorner,
                              const double aMeshSize,
                              const std::filesystem::path &aFilename)
{
    auto tBoundingBoxMesh =
        std::make_unique<::krino::BoundingBoxMesh>(stk::topology::TET_4, stk::EnvData::parallel_comm());
    tBoundingBoxMesh->set_domain(::krino::BoundingBoxMesh::BoundingBoxType(aMinCorner, aMaxCorner), aMeshSize);
    tBoundingBoxMesh->set_mesh_structure_type(::krino::FLAT_WALLED_BCC_BOUNDING_BOX_MESH);
    tBoundingBoxMesh->populate_mesh();
    ::krino::activate_all_entities(tBoundingBoxMesh->bulk_data(),
                                   ::krino::AuxMetaData::get(tBoundingBoxMesh->meta_data()).active_part());
    ::krino::output_composed_mesh_with_fields(tBoundingBoxMesh->bulk_data(),
                                              ::krino::AuxMetaData::get(tBoundingBoxMesh->meta_data()).active_part(),
                                              aFilename.string(), 1, 0.0);
}

}  // namespace plato::third_party_integration::krino
