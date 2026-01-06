#ifndef PLATO_GEOMETRY_EXTENSION_OUTPUTUTILITIES
#define PLATO_GEOMETRY_EXTENSION_OUTPUTUTILITIES

#include <filesystem>
#include <string_view>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/mesh/Mesh.hpp"

namespace plato::linear_algebra
{
template <typename T>
class DynamicVector;
}

namespace plato::output
{
struct OutputInfo;
}

namespace plato::geometry::extension
{
/// @brief Container for the info needed to write mesh field output to disk.
struct MeshFieldOutputInfo
{
    mesh::Mesh mInputMesh;
    std::filesystem::path mOutputPath;
    std::set<std::string> mFixedBlocks;
    std::string_view mControlFieldName;
    std::string_view mFilteredFieldName;
    double mFixedFieldValue;
};

/// @brief Writes the nodal field @a aSolution to the output file contained in @a aMeshOutputInfo.
///
/// Both filtered and unfiltered nodal control fields are written to the field names in @a aMeshOutputInfo. The mesh
/// output file will either be overwritten or appended based on @a aOutputInfo.
/// @return The filtered field.
auto output_nodal_field(const MeshFieldOutputInfo& aMeshOutputInfo,
                        const filter::library::FilterFunction& aFilterFunction,
                        const linear_algebra::DynamicVector<double>& aSolution,
                        const output::OutputInfo& aOutputInfo) -> analysis::AnalysisDomainMesh;

}  // namespace plato::geometry::extension

#endif
