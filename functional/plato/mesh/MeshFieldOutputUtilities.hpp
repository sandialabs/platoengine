#ifndef PLATO_MESH_MESHFIELDOUTPUTUTILITIES
#define PLATO_MESH_MESHFIELDOUTPUTUTILITIES

#include <cstdint>
#include <string_view>

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace stk::io
{
class StkMeshIoBroker;
}

namespace plato::mesh
{
/// @brief Enum describing whether a field is on nodes or elements
enum struct MeshFieldType : std::uint8_t
{
    kElement,
    kNode
};

/// @brief Uses @a aIOBroker to fill in values of the field @a aFieldName with the design variables in @a
/// aAnalysisDomainMesh and add the fields to output file defined by handle @a aOutputFileHandle.
/// The parameter @a aFieldOnElements is used to determine whether the field is an element or nodal field.
void update_output_field(stk::io::StkMeshIoBroker& aIOBroker,
                         const std::size_t aOutputFileHandle,
                         const MeshFieldType aFieldType,
                         const std::string_view aFieldName,
                         const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                         const double aFixedValue);
}  // namespace plato::mesh
#endif
