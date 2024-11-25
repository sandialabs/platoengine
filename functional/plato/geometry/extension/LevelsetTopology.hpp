#ifndef PLATO_GEOMETRY_EXTENSION_LEVELSETTOPOLOGY
#define PLATO_GEOMETRY_EXTENSION_LEVELSETTOPOLOGY

#include <filesystem>
#include <optional>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/geometry/library/GeometryRegistration.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/third_party_integration/krino/LevelsetPrimitives.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"

namespace plato::input_parser
{
struct levelset_topology;
}

namespace plato::geometry::extension
{

/// @brief Levelset-based topology representation of a geometry.
///
/// Implementation for levelset-based topology optimization. The design
/// variable used by this class is a nodal levelset field in `[-inf, inf]` defining
/// the interface between solid material and void or air (or the interface between
/// two different materials). The interface is defined to be at a levelset value of
/// 0 with solid on the positive side and void on the negative side (or material 1
/// and material 2). This class may use a Filter to smooth the levelset field
/// and reduce mesh dependency in the solution.
class LevelsetTopology
{
   public:
    explicit LevelsetTopology(const input_parser::levelset_topology& aInput);

    ~LevelsetTopology();
    LevelsetTopology(const LevelsetTopology&) = default;
    LevelsetTopology(LevelsetTopology&&) = default;
    LevelsetTopology& operator=(const LevelsetTopology&) = default;
    LevelsetTopology& operator=(LevelsetTopology&&) = default;

    [[nodiscard]] auto bounds(const std::filesystem::path& aMeshFileName) const
        -> std::pair<std::vector<double>, std::vector<double>>;
    [[nodiscard]] auto initialGuess(const std::filesystem::path& aMeshFileName) const
        -> linear_algebra::DynamicVector<double>;
    [[nodiscard]] auto generateMesh(const linear_algebra::DynamicVector<double>& aDesignParameter) const
        -> analysis::AnalysisDomainMesh;
    static void output(const std::filesystem::path& aInputMeshName,
                       const linear_algebra::DynamicVector<double>& aSolution,
                       const std::filesystem::path& aOutputMeshName);
    [[nodiscard]] auto jacobian(const linear_algebra::DynamicVector<double>& aDesignParameter) const
        -> linear_algebra::JacobianMultiplier;
    [[nodiscard]] auto adjointJacobian(const linear_algebra::DynamicVector<double>& aDesignParameter) const
        -> linear_algebra::AdjointJacobianMultiplier;

   private:
    mesh::Mesh mBackgroundMesh;
    std::filesystem::path mCutMesh;
    std::filesystem::path mOutputMesh;
    third_party_integration::krino::VoidPhase mVoidRegion = third_party_integration::krino::VoidPhase::kExcludeFromMesh;
    double mLevelsetLowerBound = -1.0;
    double mLevelsetUpperBound = 1.0;
    unsigned int mNumDesignParameters = 0;
    third_party_integration::krino::SpherePatternData mSpherePattern;
    third_party_integration::krino::LevelsetPrimitives mLevelsetPrimitives;
};

/// @brief Generate a geometry function, that can be composed with an objective function.
[[nodiscard]] auto make_topology_geometry(const LevelsetTopology& aLevelsetTopology) -> library::GeometryFunction;

namespace detail
{
[[nodiscard]] std::optional<std::string> validate_output_mesh_name(const input_parser::levelset_topology& aInput);
[[nodiscard]] std::optional<std::string> validate_background_mesh_name(const input_parser::levelset_topology& aInput);
[[nodiscard]] std::optional<std::string> validate_cut_mesh_name(const input_parser::levelset_topology& aInput);
[[nodiscard]] std::optional<std::string> validate_lower_bound(const input_parser::levelset_topology& aInput);
[[nodiscard]] std::optional<std::string> validate_upper_bound(const input_parser::levelset_topology& aInput);
[[nodiscard]] std::optional<std::string> validate_sphere_pattern_bbox(const input_parser::levelset_topology& aInput);
[[nodiscard]] std::optional<std::string> validate_sphere_pattern_radius(const input_parser::levelset_topology& aInput);
[[nodiscard]] std::optional<std::string> validate_sphere_pattern_spacing(const input_parser::levelset_topology& aInput);

}  // namespace detail

}  // namespace plato::geometry::extension

#endif
