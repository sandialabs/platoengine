#ifndef PLATO_GEOMETRY_EXTENSION_LEVELSETTOPOLOGY
#define PLATO_GEOMETRY_EXTENSION_LEVELSETTOPOLOGY

#include <filesystem>
#include <optional>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/library/FilterRegistration.hpp"
#include "plato/geometry/extension/KrinoWrapper.hpp"
#include "plato/geometry/extension/LevelSetTopologySphereParser.hpp"
#include "plato/geometry/extension/LevelSetTopologySpherePatternParser.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/input_parser/Bounds.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockStruct.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/utilities/NamedType.hpp"
#include "plato/utilities/StateCache.hpp"

// clang-format off
PLATO_GEOMETRY_INPUT_BLOCK_STRUCT(
    (plato)(input_parser), level_set_topology,
    (plato::input_parser::FileName, mesh_name, "Required field specifying the file name of the exodus mesh to read and generate controls from.")
    (plato::input_parser::FileName, output_name, "Required field specifying the exodus output file name to use when writing results.")
    (bool, include_void_region, "Required field specifying whether to include the elements of the void region when writing the cut mesh.")
    (double, max_edge_length_percentage_for_snapping, "Optional field specifying maximum fraction of an edge length that can be collapsed by snapping. "
        "Can range from 0 to 1. A value of 0 turns off snapping (only cutting), a value of 1 will collapse all edges near the level set interface (no cutting). Default is 0.15.")
    (plato::input_parser::SpherePattern, sphere_pattern,"Method of specifying the initial level set field with a 'swiss cheese' pattern. "
                                                        "Define a sphere pattern by specifying its radius, space between radius centers, a minimum point and a maximum point. "
                                                        "Omit if a sphere_list or initial_field_name is specified."
                                                        "This is an example pattern: sphere_pattern radius 1.1 spacing 2.5 min (0,0,0) max (10,10,10) ")
    (plato::input_parser::Bounds, level_set_bounds, "Required field specifying the lower and upper bounds of the nodal design variables that define the level set field.")
    (plato::input_parser::CrossReference<plato::components::ComponentType::kFilter>, filter, "Name of the filter block to apply to the controls. "
                                                                                               "Only required if more than one filter is specified.")
    (plato::input_parser::FixedBlockList, fixed_blocks, "Optional list of blocks in the mesh that will have level-set fields assigned to the level_set_upper_bound value.")
    (plato::input_parser::IdentifierString, initial_field_name, "Method to read the controls from the specified field name within the 'mesh_name' exodus mesh. "
                                                                "The read in field will automatically be centered and scaled using an affine transformation to match the 'level_set_bounds'. "
                                                                "Omit if a sphere_pattern or sphere_list is specified.")
    (plato::input_parser::LevelSetSphereList, sphere_list, "Method of specifying the initial level set field using a list of sphere primitives. "
                                                            "Define as many as you like, comma separated, in this manner: sphere_list radius_value (center_x,center_y,center_z), ... . "
                                                            "Omit if a sphere_pattern or initial_field_name is specified. "
                                                            "This is an example list: sphere_list radius 1.2 center (1,2,3), radius 0.5 center (-1,-2,-3)")
)
// clang-format on

namespace plato::input_parser
{
struct level_set_topology;
}

namespace plato::third_party_integration::krino
{
enum struct VoidPhase : std::uint8_t;
}  // namespace plato::third_party_integration::krino

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
/// The mesh that is specified in the input is the background mesh. The level set field is defined at all the nodes
/// of this mesh. The 0-isocontour represents the boundary of the level-set field dividing material and void. Krino
/// creates nodes on this boundary and new elements. Krino methods maintain two representations of the same mesh,
/// one that is the original background mesh and one that has these additional nodes and elements called the cut
/// mesh. When the cut-mesh is written to disk for a criteria it can include or exclude the void region.
class LevelSetTopology
{
   public:
    explicit LevelSetTopology(const input_parser::level_set_topology& aInput);

    ~LevelSetTopology();
    LevelSetTopology(const LevelSetTopology&) = delete;
    LevelSetTopology(LevelSetTopology&&) = delete;
    LevelSetTopology& operator=(const LevelSetTopology&) = delete;
    LevelSetTopology& operator=(LevelSetTopology&&) = delete;

    /// @brief Returns the bounds on the design variables.
    /// @return Design variable bounds vectors, `.first` containing the lower bounds and `.second` containing the upper
    /// bounds. The size of each vector will match the total number of design variables.
    [[nodiscard]] auto bounds() const -> std::pair<std::vector<double>, std::vector<double>>;

    /// @brief Returns the initial guess design variable vector.
    /// @return The size of the returned vector is equal to the number of design variables, which will be the number of
    /// nodes in design domain of the background mesh.
    [[nodiscard]] auto initialGuess() const -> linear_algebra::DynamicVector<double>;

    /// @brief Writes a cut mesh to disk based on the design variables @a aDesignParameter.
    /// @param aDesignParameter The vector size must be equal to the number of design variables.
    /// @return An AnalysisDomainMesh, only containing the location of the cut mesh on disk.
    [[nodiscard]] auto generateMesh(const linear_algebra::DynamicVector<double>& aDesignParameter) const
        -> analysis::AnalysisDomainMesh;

    /// @brief Writes user output corresponding to the input parameters in @a aInput.
    ///
    /// The size of the solution vector @a aSolution must match the number of design variables. This represents a scalar
    /// level-set field that is filtered with @a aFilterFunction in the output file.
    static void output(const input_parser::level_set_topology& aInput,
                       const filter::library::FilterFunction& aFilterFunction,
                       const linear_algebra::DynamicVector<double>& aSolution,
                       const library::OutputInfo& aOutputInfo);

    /// @brief Returns a JacobianMultiplier function object that computes the row-vector-Jacobian-product of the
    /// level-set operation evaluated at @a aDesignParameter.
    /// @param aDesignParameter The vector size must be equal to the number of design variables.
    [[nodiscard]] auto jacobian(const linear_algebra::DynamicVector<double>& aDesignParameter) const
        -> linear_algebra::JacobianMultiplier;

    /// @brief Returns an AdjointJacobianMultiplier function object that computes the
    /// row-vector-adjoint-Jacobian-product of the level-set operation evaluated at @a aDesignParameter.
    /// @param aDesignParameter The vector size must be equal to the number of design variables.
    [[nodiscard]] auto adjointJacobian(const linear_algebra::DynamicVector<double>& aDesignParameter) const
        -> linear_algebra::AdjointJacobianMultiplier;

    /// @brief Returns the background mesh on which the level-set field is defined.
    [[nodiscard]] auto backgroundMesh() const -> const mesh::Mesh&;

   private:
    input_parser::level_set_topology mInput;
    mesh::Mesh mBackgroundMesh;
    std::filesystem::path mCutMesh;
    std::filesystem::path mOutputMesh;
    third_party_integration::krino::VoidPhase mVoidRegion;
    std::pair<double, double> mLevelSetBounds = std::make_pair(-1.0, 1.0);

    using KrinoWrapperCache = utilities::StateCache<KrinoWrapper, const analysis::AnalysisDomainMesh&>;
    mutable KrinoWrapperCache mKrinoWrapperCache;
};

/// @brief Create a LevelSetTopology Geometry function with a filter.
[[nodiscard]] auto make_level_set_geometry(const std::shared_ptr<LevelSetTopology>& aLevelSetTopology,
                                           const filter::library::FilterFunction& aFilterFunction)
    -> library::GeometryFunction;

/// @brief The name of the output file containing the unfiltered level-set field, which may be used as a restart
/// file.
[[nodiscard]] auto restart_file_name(const input_parser::level_set_topology& aInput) -> std::filesystem::path;

/// @brief The label of the unfiltered level-set field used in the output mesh.
[[nodiscard]] constexpr auto level_set_mesh_field_name() -> std::string_view;

/// @brief The label of the filtered level-set field used in the output mesh.
[[nodiscard]] constexpr auto filtered_level_set_mesh_field_name() -> std::string_view;

namespace detail
{

/// @brief Validation function that takes input @a aInput and makes sure the lower bound is specified and less than 0
[[nodiscard]] auto validate_lower_bound(const input_parser::level_set_topology& aInput) -> std::optional<std::string>;

/// @brief Validation function that takes input @a aInput and makes sure the upper bound is specified and greater than 0
[[nodiscard]] auto validate_upper_bound(const input_parser::level_set_topology& aInput) -> std::optional<std::string>;

/// @brief Validation function that takes input @a aInput and makes sure the snapping edge length is unit bounded if
/// specified
[[nodiscard]] auto validate_max_snapping_edge_length(const input_parser::level_set_topology& aInput)
    -> std::optional<std::string>;

/// @brief Validation function that takes input @a aInput and makes sure the sphere pattern min bounds < max bounds if
/// specified
[[nodiscard]] auto validate_sphere_pattern_bbox(const input_parser::level_set_topology& aInput)
    -> std::optional<std::string>;

/// @brief Validation function that takes input @a aInput and makes sure the sphere pattern radius > a small positive
/// number if specified
[[nodiscard]] auto validate_sphere_pattern_radius(const input_parser::level_set_topology& aInput)
    -> std::optional<std::string>;

/// @brief Validation function that takes input @a aInput and makes sure the sphere pattern radius > a small positive
/// number if specified
[[nodiscard]] auto validate_sphere_pattern_spacing(const input_parser::level_set_topology& aInput)
    -> std::optional<std::string>;

/// @brief Validation function that takes input @a aInput and makes sure the sphere pattern spacing is more than twice
/// the radius if specified
[[nodiscard]] auto validate_sphere_pattern_spacing_greater_than_twice_radius(
    const input_parser::level_set_topology& aInput) -> std::optional<std::string>;

/// @brief Validation function that takes input @a aInput and makes sure the sphere list radii are all positive if
/// specified
[[nodiscard]] auto validate_sphere_list_radii(const input_parser::level_set_topology& aInput)
    -> std::optional<std::string>;

/// @brief Take the validated input @a aInput and convert the sphere_list into a vector of krino spheres
/// @pre @a aInput has a sphere_list
[[nodiscard]] auto generate_spheres_from_list(const input_parser::level_set_topology& aInput)
    -> std::vector<third_party_integration::krino::Sphere>;

/// @brief Validates that exactly one specifier for the intitial level set is used, either the sphere pattern
/// commands or `initial_field_name`
[[nodiscard]] std::optional<std::string> validate_exactly_one_initial_level_set_specifier(
    const input_parser::level_set_topology& aInput);

using StartingLimits = utilities::NamedType<std::pair<double, double>, struct StartingLimitsTag>;
using EndingLimits = utilities::NamedType<std::pair<double, double>, struct EndingLimitsTag>;

/// @brief Takes a vector of doubles @a aVector and linearly rescales them based on the starting limits @a
/// aStartingLimits and the ending limits @a aEndingLimits.
///
/// For example, if the starting limits are 0 -> 1, and the ending limits are -1 -> 1, a value of 0.5 gets mapped to
/// 0, 1 gets mapped to 1, and 0 gets mapped to -1.
/// @pre Both limits are ordered from lower to upper, eg, aStartingLimits.mValue.first <
/// aStartingLimits.mValue.second
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
