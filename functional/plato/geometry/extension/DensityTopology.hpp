#ifndef PLATO_GEOMETRY_EXTENSION_DENSITYTOPOLOGY
#define PLATO_GEOMETRY_EXTENSION_DENSITYTOPOLOGY

#include <filesystem>
#include <optional>
#include <unordered_map>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/core/ValidationRegistration.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/mesh/Mesh.hpp"

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

/// @brief The label of the unfiltered density field used in the output mesh.
[[nodiscard]] constexpr auto density_mesh_field_name() -> std::string_view;

/// @brief The label of the filtered density field used in the output mesh.
[[nodiscard]] constexpr auto filtered_density_mesh_field_name() -> std::string_view;

namespace detail
{
/// @brief Validates that the `output_name` field in @a aInput has a value.
[[nodiscard]] std::optional<std::string> validate_output_name(const input_parser::density_topology& aInput);

/// @brief Validates that the `initial_density_value` field in @a aInput has a value.
[[nodiscard]] std::optional<std::string> validate_initial_density_value(const input_parser::density_topology& aInput);

/// @brief Validates that the `initial_density_value` field in @a aInput has a value or that the mesh in `mesh_name`
/// has a Topology field to be read.
[[nodiscard]] std::optional<std::string> validate_initial_topology_source(const input_parser::density_topology& aInput);

/// @brief Validates that exactly one specifier for the intitial topology is used, the `initial_density_value` or
/// `initial_density_field_name`
[[nodiscard]] std::optional<std::string> validate_exactly_one_initial_topology_specifier(
    const input_parser::density_topology& aInput);

/// @brief Creates a Mesh from an density_topology input block.
/// @pre The mesh_name field in @a aInput has a value. Checked with an assertion.
[[nodiscard]] mesh::Mesh mesh_from_input(const input_parser::density_topology& aInput);

/// @brief Retrieve the initial density field from a mesh in @a aInput
/// @pre The mesh_name in @a aInput has already been checked during validation and it does exist and
/// that the field 'aInput.initial_density_field_name' exists on the mesh
/// @post The returned vector size will match the number nodal design variables
[[nodiscard]] auto initial_density_value_from_mesh(const input_parser::density_topology& aInput) -> std::vector<double>;

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
