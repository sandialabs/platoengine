#include "plato/geometry/extension/DensityResultToCutMesh.hpp"

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/geometry/extension/KrinoWrapper.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/FixedBlockUtilities.hpp"
#include "plato/mesh/MeshQuantities.hpp"
#include "plato/third_party_integration/krino/SnappingParameters.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"

namespace plato::geometry::extension
{

namespace
{

void generate_cut_mesh(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                       const input_parser::density_topology& aInput,
                       const boost::mpi::communicator& aCommunicator)
{
    third_party_integration::krino::set_stk_communicator_for_krino(
        boost::mpi::communicator{aCommunicator, boost::mpi::comm_attach});
    namespace tpik = third_party_integration::krino;
    make_krino_wrapper_from_analysis_domain_mesh(aAnalysisDomainMesh, mesh::fixed_blocks(aInput),
                                                 tpik::VoidPhase::kExcludeFromMesh,
                                                 tpik::SnappingParameters{.mMaxSnappingEdgeLength = 0.5})
        .writeCutMesh(aInput.cut_mesh_name.value().mToken);
}

}  // namespace

void write_density_result_as_cut_mesh(analysis::AnalysisDomainMesh aAnalysisDomainMesh,
                                      const input_parser::density_topology& aInput,
                                      const boost::mpi::communicator& aCommunicator)
{
    constexpr auto tThreshold = 0.5;
    aAnalysisDomainMesh = detail::transform_to_level_set_field(std::move(aAnalysisDomainMesh), tThreshold);

    if (const auto tMesh = mesh::Mesh{aAnalysisDomainMesh};
        mesh::EntityCounts{tMesh}.areElementDesignVariables(aAnalysisDomainMesh))
    {
        const auto tNodalProjectionsAnalysisDomainMesh = detail::convert_to_nodal(aAnalysisDomainMesh, tMesh);
        generate_cut_mesh(tNodalProjectionsAnalysisDomainMesh, aInput, aCommunicator);
    }
    else
    {
        generate_cut_mesh(aAnalysisDomainMesh, aInput, aCommunicator);
    }
}

namespace detail
{

auto convert_to_nodal(const analysis::AnalysisDomainMesh& aElementAnalysisDomainMesh, const mesh::Mesh& aMesh)
    -> analysis::AnalysisDomainMesh
{
    const auto tElementDesignVariables = mesh::DesignVariablesConversion{aMesh}
                                             .analysisDomainMeshToElementFieldVector(aElementAnalysisDomainMesh)
                                             .mValue;
    const auto tNodalAverageElementProjectedVariables =
        mesh::MeshQuantities{aMesh}.designDomainNodalAverageElementProjection(tElementDesignVariables);
    return mesh::DesignVariablesConversion{aMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tNodalAverageElementProjectedVariables});
}

auto transform_to_level_set_field(analysis::AnalysisDomainMesh aAnalysisDomainMesh, const double aDensityThreshold)
    -> analysis::AnalysisDomainMesh
{
    const auto tMutableView = analysis::AnalysisDomainMeshMutableSequentialView{aAnalysisDomainMesh};
    std::transform(tMutableView.begin(), tMutableView.end(), tMutableView.begin(),
                   [aDensityThreshold](const auto& tEntry)
                   {
                       auto tEntryField = static_cast<analysis::ScalarFieldValue>(tEntry);
                       tEntryField.mValue -= aDensityThreshold;
                       return tEntryField;
                   });
    return aAnalysisDomainMesh;
}

}  // namespace detail

}  // namespace plato::geometry::extension
