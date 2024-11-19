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

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshRandomAccessView.hpp"
#include "plato/utilities/IndexRange.hpp"
#include "plato/utilities/MultiVectorView.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::third_party_integration::krino
{

namespace
{
constexpr std::string_view kKrinoLogName = "krinolog";
const std::string kLevelsetName = "LS";

}  // namespace

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

void setup_fields_for_conforming_decomposition(const stk::mesh::MetaData &aMeta)
{
    ::krino::CDFEM_Support &tCdfemSupport = ::krino::CDFEM_Support::get(aMeta);
    const ::krino::FieldRef tCoordsField = aMeta.coordinate_field();

    tCdfemSupport.set_coords_field(tCoordsField);
    tCdfemSupport.add_edge_interpolation_field(tCoordsField);
    tCdfemSupport.register_parent_node_ids_field();
}

void create_bounding_box_mesh(const stk::math::Vector3d &aMinCorner,
                              const stk::math::Vector3d &aMaxCorner,
                              const double aMeshSize,
                              const std::filesystem::path &aFilename)
{
    auto tBoundingBoxMesh =
        std::make_unique<::krino::BoundingBoxMesh>(stk::topology::TET_4, stk::EnvData::parallel_comm());
    ::krino::LevelSet &tLevelSet =
        ::krino::LevelSet::build(tBoundingBoxMesh->meta_data(), kLevelsetName, sierra::Diag::sierraTimer());
    tLevelSet.set_distance_name(kLevelsetName);
    tLevelSet.setup();
    setup_fields_for_conforming_decomposition(tBoundingBoxMesh->meta_data());
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
