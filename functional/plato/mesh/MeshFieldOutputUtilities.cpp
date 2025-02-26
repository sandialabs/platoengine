#include "plato/mesh/MeshFieldOutputUtilities.hpp"

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshRandomAccessView.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"

namespace plato::mesh
{
void update_output_field(stk::io::StkMeshIoBroker& aIOBroker,
                         const std::size_t aOutputFileHandle,
                         const MeshFieldType aFieldType,
                         const std::string_view aFieldName,
                         const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                         const double aFixedValue)
{
    namespace tpi = plato::third_party_integration;

    const auto tScalarField =
        [tDesignVariablesView = analysis::AnalysisDomainMeshRandomAccessView{aAnalysisDomainMesh},
         tFixedScalarField = analysis::ScalarFieldValue{0, 0, aFixedValue}](const std::size_t aGlobalIndex)
    { return tDesignVariablesView[aGlobalIndex].value_or(tFixedScalarField).mValue; };

    if (aFieldType == MeshFieldType::kElement)
    {
        tpi::stk_io::populate_element_scalar_field_values(aIOBroker, aFieldName, tScalarField);
        tpi::stk_io::add_element_field_to_output_file(aIOBroker, aOutputFileHandle, aFieldName);
    }
    else
    {
        tpi::stk_io::populate_nodal_scalar_field_values(aIOBroker, aFieldName, tScalarField);
        tpi::stk_io::add_nodal_field_to_output_file(aIOBroker, aOutputFileHandle, aFieldName);
    }
}
}  // namespace plato::mesh
