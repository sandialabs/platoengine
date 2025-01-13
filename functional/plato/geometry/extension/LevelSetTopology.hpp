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
#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"

namespace plato::input_parser
{
struct level_set_topology;
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
    [[nodiscard]] auto initialGuess() const -> linear_algebra::DynamicVector<double>;
    [[nodiscard]] auto generateMesh(const linear_algebra::DynamicVector<double>& aDesignParameter) const
        -> analysis::AnalysisDomainMesh;
    static void output(const input_parser::level_set_topology& aInput,
                       const linear_algebra::DynamicVector<double>& aSolution);
    [[nodiscard]] auto jacobian(const linear_algebra::DynamicVector<double>& aDesignParameter) const
        -> linear_algebra::JacobianMultiplier;
    [[nodiscard]] auto adjointJacobian(const linear_algebra::DynamicVector<double>& aDesignParameter) const
        -> linear_algebra::AdjointJacobianMultiplier;

    [[nodiscard]] auto backgroundMesh() const -> const mesh::Mesh&;

   private:
    mesh::Mesh mBackgroundMesh;
    std::filesystem::path mCutMesh;
    std::filesystem::path mOutputMesh;
    third_party_integration::krino::VoidPhase mVoidRegion = third_party_integration::krino::VoidPhase::kExcludeFromMesh;
    double mLevelSetLowerBound = -1.0;
    double mLevelSetUpperBound = 1.0;
    third_party_integration::krino::SpherePatternData mSpherePattern;
    third_party_integration::krino::LevelSetPrimitives mLevelSetPrimitives;
};

/// @brief Create a LevelSetTopology Geometry function with a filter.
auto make_level_set_geometry(const std::shared_ptr<LevelSetTopology>& aLevelSetTopology,
                             const filter::library::FilterFunction& aFilterFunction) -> library::GeometryFunction;

namespace detail
{
[[nodiscard]] std::optional<std::string> validate_output_mesh_name(const input_parser::level_set_topology& aInput);
[[nodiscard]] std::optional<std::string> validate_background_mesh_name(const input_parser::level_set_topology& aInput);
[[nodiscard]] std::optional<std::string> validate_lower_bound(const input_parser::level_set_topology& aInput);
[[nodiscard]] std::optional<std::string> validate_upper_bound(const input_parser::level_set_topology& aInput);
[[nodiscard]] std::optional<std::string> validate_sphere_pattern_bbox(const input_parser::level_set_topology& aInput);
[[nodiscard]] std::optional<std::string> validate_sphere_pattern_radius(const input_parser::level_set_topology& aInput);
[[nodiscard]] std::optional<std::string> validate_sphere_pattern_spacing(
    const input_parser::level_set_topology& aInput);

}  // namespace detail

}  // namespace plato::geometry::extension

#endif
