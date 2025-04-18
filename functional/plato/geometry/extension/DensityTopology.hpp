#ifndef PLATO_GEOMETRY_EXTENSION_DENSITYTOPOLOGY
#define PLATO_GEOMETRY_EXTENSION_DENSITYTOPOLOGY

#include <filesystem>
#include <optional>
#include <unordered_map>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/IdentifierString.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/mesh/Mesh.hpp"

// clang-format off
PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), density_topology,
    (plato::input_parser::FileName, mesh_name, "Required field specifying the file name of the exodus mesh to read and generate controls from.")
    (plato::input_parser::FileName, output_name, "Required field specifying the exodus output file name to use when writing results.")
    (plato::input_parser::FixedBlockList, fixed_blocks, "Optional comma separated list of block names that should be fixed in the mesh and not be part of the optimization.")
    (plato::input_parser::NewCrossReference<plato::input_parser::ComponentType::kFilter>, filter, "Required name of the filter block to apply to the controls.")
    (double, initial_density_value, "Method to specify a uniform initial density value to give to the controls. Omit if 'initial_density_field_name' is specified.")
    (plato::input_parser::IdentifierString, initial_density_field_name, "Method to read the controls from the specified field name within the 'mesh_name' exodus mesh. Omit if 'initial_density_value' is specified.")
)
// clang-format on

namespace plato::input_parser
{
struct density_topology;
}

namespace plato::geometry::extension
{
/// @brief Density-based topology representation of a geometry.
///
/// Implementation for density-based topology optimization. The design
/// variable used by this class is a nodal density field in `[0, 1]` representing
/// the presence or absence of material. This class may use a Filter to smooth
/// the density field and reduce mesh dependency in the solution.
class DensityTopology
{
   public:
    DensityTopology(const input_parser::density_topology& aInput, filter::library::FilterFunction aFilterFunction);

    [[nodiscard]] auto generateMesh(const linear_algebra::DynamicVector<double>& aDesignParameter) const
        -> analysis::AnalysisDomainMesh;

    [[nodiscard]] auto jacobian(const linear_algebra::DynamicVector<double>& aDesignParameter) const
        -> linear_algebra::JacobianMultiplier;

    [[nodiscard]] auto adjointJacobian(const linear_algebra::DynamicVector<double>& aDesignParameter) const
        -> linear_algebra::AdjointJacobianMultiplier;

    [[nodiscard]] static auto initialGuess(const input_parser::density_topology& aInput)
        -> linear_algebra::DynamicVector<double>;

    [[nodiscard]] static auto bounds(const input_parser::density_topology& aInput)
        -> std::pair<std::vector<double>, std::vector<double>>;

    static void output(const linear_algebra::DynamicVector<double>& aSolution,
                       const filter::library::FilterFunction& aFilterFunction,
                       const input_parser::density_topology& aInput,
                       const library::OutputInfo& aOutputInfo);

   private:
    mesh::Mesh mMesh;
    filter::library::FilterFunction mFilter;
};

/// @brief Generates a valid example of a density_topology input, useful for testing.
[[nodiscard]] auto create_valid_density_topology_geometry_input() -> input_parser::density_topology;

/// @brief The label of the unfiltered density field used in the output mesh.
[[nodiscard]] constexpr auto density_mesh_field_name() -> std::string_view;

/// @brief The label of the filtered density field used in the output mesh.
[[nodiscard]] constexpr auto filtered_density_mesh_field_name() -> std::string_view;

namespace detail
{

/// @brief Validates that the `initial_density_value` field in @a aInput has a value.
[[nodiscard]] auto validate_initial_density_value(const input_parser::density_topology& aInput)
    -> std::optional<std::string>;

/// @brief Validates that exactly one specifier for the intitial topology is used, the `initial_density_value` or
/// `initial_field_name`
[[nodiscard]] auto validate_exactly_one_initial_topology_specifier(const input_parser::density_topology& aInput);

}  // namespace detail

[[nodiscard]] constexpr auto density_mesh_field_name() -> std::string_view
{
    return std::string_view{"unfiltered_density"};
}

[[nodiscard]] constexpr auto filtered_density_mesh_field_name() -> std::string_view
{
    return std::string_view{"density"};
}

}  // namespace plato::geometry::extension

#endif
