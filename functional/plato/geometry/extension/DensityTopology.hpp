#ifndef PLATO_GEOMETRY_EXTENSION_DENSITYTOPOLOGY
#define PLATO_GEOMETRY_EXTENSION_DENSITYTOPOLOGY

#include <filesystem>
#include <optional>
#include <unordered_map>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/core/ValidationRegistration.hpp"
#include "plato/filter/library/FilterFactory.hpp"
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
    explicit DensityTopology(const input_parser::density_topology& aInput,
                             plato::filter::library::FilterFunction aFilterFunction);

    [[nodiscard]] analysis::AnalysisDomainMesh generateMesh(
        const linear_algebra::DynamicVector<double>& aDesignParameter) const;

    [[nodiscard]] linear_algebra::JacobianMultiplier jacobian(
        const linear_algebra::DynamicVector<double>& aDesignParameter) const;

    [[nodiscard]] static linear_algebra::DynamicVector<double> initialGuess(
        const input_parser::density_topology& aInput);

    [[nodiscard]] static std::pair<std::vector<double>, std::vector<double>> bounds(
        const input_parser::density_topology& aInput);

    static void output(const linear_algebra::DynamicVector<double>& aSolution,
                       const input_parser::density_topology& aInput);

   private:
    mesh::Mesh mMesh;
    plato::filter::library::FilterFunction mFilter;
};

/// @brief Generate a geometry function, that can be composed with an objective function.
[[nodiscard]] auto make_topology_geometry(const DensityTopology& aDensityTopology) -> library::GeometryFunction;

namespace detail
{
/// @brief Validates that the `output_name` field in @a aInput has a value.
[[nodiscard]] std::optional<std::string> validate_output_name(const input_parser::density_topology& aInput);

[[nodiscard]] std::optional<std::string> validate_filter_with_mesh(const input_parser::density_topology& aInput);
/// @brief Validates that all fixed block names in the input are unique.
[[nodiscard]] std::optional<std::string> validate_unique_fixed_block_names(
    const input_parser::density_topology& aInput);

/// @brief Validates that all fixed block names in @a aInput exist in the mesh.
[[nodiscard]] std::optional<std::string> validate_fixed_block_names_exist(const input_parser::density_topology& aInput);

/// @brief Validates that at least 1 block remains as the design domain
[[nodiscard]] std::optional<std::string> validate_at_least_one_design_block(
    const input_parser::density_topology& aInput);

/// @brief Validates that the `initial_density_value` field in @a aInput has a value.
[[nodiscard]] std::optional<std::string> validate_initial_density_value(const input_parser::density_topology& aInput);

/// @brief Validates that the `initial_density_value` field in @a aInput has a value or that the mesh in `mesh_name`
/// has a Topology field to be read.
[[nodiscard]] std::optional<std::string> validate_initial_topology_source(const input_parser::density_topology& aInput);

/// @brief Validates that exactly one specifier for the intitial topology is used, the `initial_density_value` or
/// `initial_density_field_name`
[[nodiscard]] std::optional<std::string> validate_exactly_one_initial_topology_specifier(
    const input_parser::density_topology& aInput);

/// @brief Converts the vector of fixed block names in @a aInput to a set.
///
/// A set is used since the list of fixed blocks must be unique. That the raw input is a unique list of names
/// can be validated with validate_unique_fixed_block_names.
[[nodiscard]] std::set<std::string> fixed_blocks(const input_parser::density_topology& aInput);

/// @brief Creates a Mesh from an density_topology input block.
/// @pre The mesh_name field in @a aInput has a value. Checked with an assertion.
[[nodiscard]] mesh::Mesh mesh_from_input(const input_parser::density_topology& aInput);

/// @brief Retrieve the initial density field from a mesh in @a aInput
/// @pre The mesh_name in @a aInput has already been checked during validation and it does exist and
/// that the field 'aInput.initial_density_field_name' exists on the mesh
/// @post The returned vector size will match the number nodal design variables
[[nodiscard]] auto initial_density_value_from_mesh(const input_parser::density_topology& aInput) -> std::vector<double>;

}  // namespace detail

}  // namespace plato::geometry::extension

#endif
