#ifndef PLATO_GEOMETRY_EXTENSION_LEVELSETTOPOLOGY
#define PLATO_GEOMETRY_EXTENSION_LEVELSETTOPOLOGY

#include <filesystem>
#include <optional>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::input_parser
{
struct level_set_topology;
}

namespace plato::third_party_integration::krino
{
enum struct VoidPhase;
}

namespace plato::geometry::extension
{

/// @brief Level set-based topology representation of a geometry.
///
/// Implementation for level set-based topology optimization. The design
/// variable used by this class is a nodal level set field in `[-inf, inf]` defining
/// the interface between solid material and void or air (or the interface between
/// two different materials). The interface is defined to be at a level set value of
/// 0 with solid on the positive side and void on the negative side (or material 1
/// and material 2). This class may use a Filter to smooth the level set field
/// and reduce mesh dependency in the solution.
/// The mesh that is specified in the input is the background mesh. The level set field is defined at all the nodes of
/// this mesh. The 0-isocontour represents the boundary of the level-set field dividing material and void. Krino creates
/// nodes on this boundary and new elements. Krino methods maintain two representations of the same mesh, one that is
/// the original background mesh and one that has these additional nodes and elements called the cut mesh. When the
/// cut-mesh is written to disk for a criteria it can include or exclude the void region.
class LevelSetTopology
{
   public:
    explicit LevelSetTopology(const input_parser::level_set_topology& aInput);

    ~LevelSetTopology();
    LevelSetTopology(const LevelSetTopology&) = default;
    LevelSetTopology(LevelSetTopology&&) = default;
    LevelSetTopology& operator=(const LevelSetTopology&) = default;
    LevelSetTopology& operator=(LevelSetTopology&&) = default;

    [[nodiscard]] auto bounds() const -> std::pair<std::vector<double>, std::vector<double>>;
    [[nodiscard]] auto initialGuess(const input_parser::level_set_topology& aInput) const
        -> linear_algebra::DynamicVector<double>;
    [[nodiscard]] auto generateMesh(const linear_algebra::DynamicVector<double>& aDesignParameter) const
        -> analysis::AnalysisDomainMesh;
    static void output(const input_parser::level_set_topology& aInput,
                       const filter::library::FilterFunction& aFilterFunction,
                       const linear_algebra::DynamicVector<double>& aSolution,
                       const library::OutputInfo& aOutputInfo);
    [[nodiscard]] auto jacobian(const linear_algebra::DynamicVector<double>& aDesignParameter) const
        -> linear_algebra::JacobianMultiplier;
    [[nodiscard]] auto adjointJacobian(const linear_algebra::DynamicVector<double>& aDesignParameter) const
        -> linear_algebra::AdjointJacobianMultiplier;

    [[nodiscard]] auto backgroundMesh() const -> const mesh::Mesh&;

   private:
    mesh::Mesh mBackgroundMesh;
    std::filesystem::path mCutMesh;
    std::filesystem::path mOutputMesh;
    third_party_integration::krino::VoidPhase mVoidRegion;
    std::pair<double, double> mLevelSetBounds = std::make_pair(-1.0, 1.0);
};

/// @brief Create a LevelSetTopology Geometry function with a filter.
[[nodiscard]] auto make_level_set_geometry(const std::shared_ptr<LevelSetTopology>& aLevelSetTopology,
                                           const filter::library::FilterFunction& aFilterFunction)
    -> library::GeometryFunction;

/// @brief The name of the output file containing the unfiltered level-set field, which may be used as a restart file.
[[nodiscard]] auto restart_file_name(const input_parser::level_set_topology& aInput) -> std::filesystem::path;

/// @brief The label of the unfiltered level-set field used in the output mesh.
[[nodiscard]] constexpr auto level_set_mesh_field_name() -> std::string_view;

/// @brief The label of the filtered level-set field used in the output mesh.
[[nodiscard]] constexpr auto filtered_level_set_mesh_field_name() -> std::string_view;

namespace detail
{

[[nodiscard]] std::optional<std::string> validate_lower_bound(const input_parser::level_set_topology& aInput);
[[nodiscard]] std::optional<std::string> validate_upper_bound(const input_parser::level_set_topology& aInput);
[[nodiscard]] std::optional<std::string> validate_sphere_pattern_bbox(const input_parser::level_set_topology& aInput);
[[nodiscard]] std::optional<std::string> validate_sphere_pattern_radius(const input_parser::level_set_topology& aInput);
[[nodiscard]] std::optional<std::string> validate_sphere_pattern_spacing(
    const input_parser::level_set_topology& aInput);

/// @brief Validates that exactly one specifier for the intitial level set is used, either the sphere pattern commands
/// or `initial_field_name`
[[nodiscard]] std::optional<std::string> validate_exactly_one_initial_level_set_specifier(
    const input_parser::level_set_topology& aInput);

using StartingLimits = utilities::NamedType<std::pair<double, double>, struct StartingLimitsTag>;
using EndingLimits = utilities::NamedType<std::pair<double, double>, struct EndingLimitsTag>;

/// @brief Takes a vector of doubles @a aVector and linearly rescales them based on the starting limits @a
/// aStartingLimits and the ending limits @a aEndingLimits.
///
/// For example, if the starting limits are 0 -> 1, and the ending limits are -1 -> 1, a value of 0.5 gets mapped to 0,
/// 1 gets mapped to 1, and 0 gets mapped to -1.
/// @pre Both limits are ordered from lower to upper, eg, aStartingLimits.mValue.first < aStartingLimits.mValue.second
[[nodiscard]] auto affine_transformation(std::vector<double> aVector,
                                         const StartingLimits& aStartingLimits,
                                         const EndingLimits& aEndingLimits) -> std::vector<double>;

}  // namespace detail

constexpr auto level_set_mesh_field_name() -> std::string_view { return std::string_view{"level_set"}; }

constexpr auto filtered_level_set_mesh_field_name() -> std::string_view
{
    return std::string_view{"filtered_level_set"};
}

}  // namespace plato::geometry::extension

#endif
