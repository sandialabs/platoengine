#ifndef PLATO_GEOMETRY_EXTENSION_LEVELSETTOPOLOGY
#define PLATO_GEOMETRY_EXTENSION_LEVELSETTOPOLOGY

#include <PlatoKrinoInterface.hpp>
#include <PlatoKrinoUtilities.hpp>
#include <filesystem>
#include <optional>

#include "plato/core/Function.hpp"
#include "plato/core/MeshProxy.hpp"
#include "plato/core/ValidationRegistration.hpp"
#include "plato/filter/library/FilterFactory.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

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

    [[nodiscard]] std::pair<std::vector<double>, std::vector<double>> bounds(
        const std::filesystem::path& aMeshFileName);
    [[nodiscard]] linear_algebra::DynamicVector<double> initialGuess(const std::filesystem::path& aMeshFileName);
    [[nodiscard]] core::MeshProxy generateMesh(const linear_algebra::DynamicVector<double>& aDesignParameter) const;
    static void output(const std::filesystem::path& aInputMeshName,
                       const linear_algebra::DynamicVector<double>& aSolution,
                       const std::filesystem::path& aOutputMeshName);
    [[nodiscard]] linear_algebra::JacobianMultiplier jacobian(
        const linear_algebra::DynamicVector<double>& aDesignParameter) const;

   private:
    void generateLevelsetInitializationPrimitives();

   private:
    std::filesystem::path mBackgroundMesh;
    std::filesystem::path mCutMesh;
    std::filesystem::path mOutputMesh;
    unsigned int mNumDesignParameters = 0;
    Plato::Krino::LevelsetPrimitives mLevelsetPrimitives;
    Plato::Krino::SpherePatternData mSpherePattern;
};

/// @brief Generate a geometry function, that can be composed with an objective function.
[[nodiscard]] auto make_topology_geometry(const LevelsetTopology& aLevelsetTopology) -> core::
    Function<core::MeshProxy, linear_algebra::JacobianMultiplier, const linear_algebra::DynamicVector<double>&>;

namespace detail
{
[[nodiscard]] std::optional<std::string> validate_output_name(const input_parser::levelset_topology& aInput);
template <typename Geometry>
[[nodiscard]] std::optional<std::string> validate_mesh_name(const Geometry& aInput);
}  // namespace detail

}  // namespace plato::geometry::extension

#endif
