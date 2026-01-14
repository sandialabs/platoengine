#ifndef PLATO_GEOMETRY_EXTENSION_OUTPUTUTILITIES
#define PLATO_GEOMETRY_EXTENSION_OUTPUTUTILITIES

#include <filesystem>
#include <functional>
#include <string_view>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/output/OutputInfo.hpp"

namespace plato::linear_algebra
{
template <typename T>
class DynamicVector;
}

namespace plato::geometry::extension
{
/// @brief Container for the info needed to write mesh field output to disk.
struct MeshFieldOutputInfo
{
    std::filesystem::path mOutputPath;
    std::reference_wrapper<const analysis::AnalysisDomainMesh> mSourceAnalysisDomainMesh;
    std::string_view mFieldName;
    double mFixedFieldValue;
    output::OutputInfo mOutputInfo;
};

/// @brief Appends or overwrites the field provided to the mesh specified.
void output_field(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                  const std::filesystem::path& aRestartFileName,
                  const std::string_view aFieldName,
                  const double aFixedValue,
                  const output::OutputInfo& aOutputInfo);

template <typename T>
concept HasBoostOptionalOutputNameMember = requires(T a) {
    { a.output_name } -> std::same_as<boost::optional<input_parser::FileName>&>;
};

template <typename T>
concept HasBoostOptionalMeshNameMember = requires(T a) {
    { a.mesh_name } -> std::same_as<boost::optional<input_parser::FileName>&>;
};

/// @brief The name of the output file containing the unfiltered control field, which may be used as a restart
/// file.
template <typename GeometryInput>
    requires HasBoostOptionalOutputNameMember<GeometryInput>
[[nodiscard]] auto restart_file_name(const GeometryInput& aInput) -> std::filesystem::path;

/// @brief Take geometry input @a aInput, the filter @a aFilterFunction and a solution @a aSolution and create a pair of
/// analysis domain meshes of the solution and the filtered solution for output.
template <typename GeometryInput>
    requires HasBoostOptionalMeshNameMember<GeometryInput>
[[nodiscard]] auto solution_and_filtered_solution_mesh(const GeometryInput& aInput,
                                                       const filter::library::FilterFunction& aFilterFunction,
                                                       const linear_algebra::DynamicVector<double>& aSolution)
    -> std::pair<analysis::AnalysisDomainMesh, analysis::AnalysisDomainMesh>;

template <typename GeometryInput>
    requires HasBoostOptionalOutputNameMember<GeometryInput>
auto restart_file_name(const GeometryInput& aInput) -> std::filesystem::path
{
    constexpr auto tRestartFileNamePrefix = std::string_view{"restart_"};
    return std::filesystem::path{std::string{tRestartFileNamePrefix} + aInput.output_name->mToken};
}

template <typename GeometryInput>
    requires HasBoostOptionalMeshNameMember<GeometryInput>
auto solution_and_filtered_solution_mesh(const GeometryInput& aInput,
                                         const filter::library::FilterFunction& aFilterFunction,
                                         const linear_algebra::DynamicVector<double>& aSolution)
    -> std::pair<analysis::AnalysisDomainMesh, analysis::AnalysisDomainMesh>
{
    const auto tAnalysisDomainMesh =
        mesh::DesignVariablesConversion{mesh_from_input(aInput)}.nodalFieldToAnalysisDomainMesh(
            mesh::NodalFieldVectorReference{aSolution.stdVector()});
    const auto tFilteredFieldAnalysisDomainMesh =
        aFilterFunction.evaluate<core::evaluation::kFunction>(tAnalysisDomainMesh);
    return std::make_pair(tAnalysisDomainMesh, tFilteredFieldAnalysisDomainMesh);
}

}  // namespace plato::geometry::extension

#endif
