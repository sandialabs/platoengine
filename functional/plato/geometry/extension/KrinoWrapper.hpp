#ifndef PLATO_GEOMETRY_EXTENSION_KRINOWRAPPER
#define PLATO_GEOMETRY_EXTENSION_KRINOWRAPPER

#include <Akri_LevelSet.hpp>
#include <Akri_LevelSetPolicy.hpp>
#include <filesystem>
#include <utility>
#include <vector>

#include "plato/third_party_integration/krino/BackgroundMesh.hpp"
#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"
#include "plato/third_party_integration/krino/SensitivityMapUtilities.hpp"
#include "plato/third_party_integration/krino/SnappingParameters.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/utilities/MultiVectorView.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::geometry::extension
{

/// @brief Interface with all the Krino utilities ensuring that everything gets called in the correct order.
///
/// Three make functions are provided to create the required krino objects and auxiliary objects for the KrinoWrapper
/// constructor. Once constructed, the KrinoWrapper can evaluate the row vector Jacobian product or the row vector
/// Adjoint Jacobian product
/// @pre the environment for krino was initialized by calling 'set_stk_communicator_for_krino' in
/// third_party_integration\krino\Utilities.hpp
class KrinoWrapper
{
   public:
    /// @brief Construct a KrinoWrapper from its data type members. Use free make functions below instead. The parameter
    /// @a aVoidPhase specifies at construction whether to include or exclude the void region in sensitivities and in
    /// writing the mesh to disk.
    /// @note See helper functions for the most common methods to create a KrinoWrapper
    KrinoWrapper(third_party_integration::krino::BackgroundMesh aBackgroundMesh,
                 third_party_integration::krino::VoidPhase aVoidPhase,
                 third_party_integration::krino::SnappingParameters aSnappingParameters);

    /// @brief Write a cut mesh to the file @a aFileName.
    void writeCutMesh(const std::filesystem::path& aFileName) const;

    /// @brief Return the sensitivity map determined after the cut procedure. Used for unit testing.
    [[nodiscard]] auto sensitivities() const -> const third_party_integration::krino::SensitivityMap&;

    /// @brief Compute the row vector jacobian product for the cut mesh row vector @a aCutMeshRowVector.
    [[nodiscard]] auto rowVectorJacobianProduct(const std::vector<double>& aCutMeshRowVector) const
        -> std::vector<double>;

    /// @brief Compute the row vector jacobian product for the background mesh row vector @a aBackgroundMeshRowVector.
    [[nodiscard]] auto rowVectorAdjointJacobianProduct(const std::vector<double>& aBackgroundMeshRowVector) const
        -> std::vector<double>;

   private:
    third_party_integration::krino::BackgroundMesh mBackgroundMesh;
    std::size_t mNumberOfDesignDomainBackgroundNodes;
    third_party_integration::krino::SensitivityMap mSensitivityMap;
    stk::mesh::Selector mSelector;
    third_party_integration::krino::VoidPhase mVoidPhase;
};

using LevelSetBounds = utilities::NamedType<std::pair<double, double>, struct LevelSetBoundsTag>;

/// @brief Free function to facilitate making an initial guess.
///
/// Read a mesh from @a aFileName, and use the level set primitives @a aLevelSetPrimitives, along with
/// specification of any fixed blocks @a aFixedBlocks (blocks that will not be cut). This is primarily used to specify a
/// starting point for a level set optimization.
[[nodiscard]] auto make_initial_guess_from_level_set_primitives(
    const std::filesystem::path& aFileName,
    const third_party_integration::krino::LevelSetPrimitives& aLevelSetPrimitives,
    const std::set<std::string>& aFixedBlocks,
    const LevelSetBounds& aBounds) -> std::vector<double>;

/// @brief Helper function to facilitate making a KrinoWrapper.
///
/// Use a mesh along with level set values and fixed blocks specified in an analysis domain mesh @a
/// aAnalysisDomainMesh to create a KrinoWrapper. Design domain IDs are determined from the AnalysisDomainMesh.
[[nodiscard]] auto make_krino_wrapper_from_analysis_domain_mesh(
    const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
    const std::set<std::string>& aFixedBlocks,
    const third_party_integration::krino::VoidPhase& aVoidPhase,
    third_party_integration::krino::SnappingParameters aSnappingParameters) -> KrinoWrapper;

namespace detail
{
/// @brief Take a krino mesh's bulk data and level set fields stored in @a aBackgroundMesh, along with the design
/// domain ids @a aDesignDomainBackgroundNodes and compute the SensitivityMap. The map takes cut mesh surface node ids
/// and maps them to LevelSetJacobianColumn which is a list of Parent Ids along with their sensitivities. If a parent is
/// a fixed node and not part of the @a aDesignDomainBackgroundNodes it is not included in the sensitivty map.
/// @pre The environment for krino must be initialized and set up as follows:
/// @code{.cpp}
/// set_stk_communicator_for_krino(...);
/// cut_mesh(...);
/// @endcode
[[nodiscard]] auto compute_sensitivities(
    const third_party_integration::krino::BackgroundMesh& aBackgroundMesh,
    const std::vector<third_party_integration::krino::BackgroundMeshNodeId>& aDesignDomainBackgroundNodes)
    -> third_party_integration::krino::SensitivityMap;

/// @brief Take a flattened row vector @a aRowVector and extract the Vector3 stored at the unflattened index @a
/// aVectorIndex.
/// @post a Vector3 is returned with z entry 0 if @a aDimensions is 2.
/// For example, if a row vector input is: {x0, y0, z0, x1, y1, z1, x2, y2, z2}, and index is 1
/// the return will be {x1, y1, z1}.
[[nodiscard]] auto row_vector_to_vector3(const std::vector<double>& aRowVector,
                                         utilities::VectorIndex aVectorIndex,
                                         std::size_t aDimensions) -> third_party_integration::common::Vector3;

/// @brief Takes a vector of doubles @a aValues and clamps the values to bounds defined by @a aBounds.
[[nodiscard]] auto clamp_to_bounds(const std::vector<double>& aValues, const LevelSetBounds& aBounds)
    -> std::vector<double>;
}  // namespace detail

}  // namespace plato::geometry::extension

#endif
