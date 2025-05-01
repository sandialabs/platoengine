#ifndef PLATO_GEOMETRY_EXTENSION_KRINOWRAPPER
#define PLATO_GEOMETRY_EXTENSION_KRINOWRAPPER

#include <Akri_LevelSet.hpp>
#include <Akri_LevelSetPolicy.hpp>
#include <filesystem>
#include <memory>
#include <unordered_map>

#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"
#include "plato/third_party_integration/krino/SensitivityMapUtilities.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/utilities/MultiVectorView.hpp"

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
/// @pre the environment for krino was initialized by calling 'initialize_environment_for_krino' in
/// third_party_integration\krino\Utilities.hpp
class KrinoWrapper
{
   public:
    /// @brief Construct a KrinoWrapper from its data type members. Use free make functions below instead.
    KrinoWrapper(std::unique_ptr<::krino::MeshInterface> aKrinoMeshInterface,
                 std::vector<::krino::LS_Field> aLevelSetField,
                 std::optional<std::vector<third_party_integration::krino::BackgroundMeshNodeId>> aBackgroundDesignIDs);

    /// @brief Write a cut mesh to the file @a aFileName with the void region specifier @a aVoidPhase.
    void writeCutMesh(const std::filesystem::path& aFileName,
                      const third_party_integration::krino::VoidPhase aVoidPhase) const;

    /// @brief Return the sensitivity map determined after the cut procedure. Used for unit testing.
    [[nodiscard]] auto sensitivities() const -> const third_party_integration::krino::SensitivityMap&;

    /// @brief Compute the row vector jacobian product for the cut mesh row vector @a aCutMeshRowVector.
    ///
    /// Background nodes that would be part of the void are not in the row vector. Whether they are in the row vector or
    /// not is specified by @a aVoidPhase.
    [[nodiscard]] auto rowVectorJacobianProduct(const std::vector<double>& aCutMeshRowVector,
                                                const third_party_integration::krino::VoidPhase aVoidPhase) const
        -> std::vector<double>;

    /// @brief Compute the row vector jacobian product for the background mesh row vector @a aBackgroundMeshRowVector.
    ///
    /// Background nodes that would be part of the void are not in the row vector. Whether they are in the row vector or
    /// not is specified by @a aVoidPhase.
    [[nodiscard]] auto rowVectorAdjointJacobianProduct(const std::vector<double>& aBackgroundMeshRowVector,
                                                       const third_party_integration::krino::VoidPhase aVoidPhase) const
        -> std::vector<double>;

   private:
    std::unique_ptr<::krino::MeshInterface> mKrinoMesh;
    std::vector<::krino::LS_Field> mLevelSetFields;
    std::size_t mNumberOfDesignDomainBackgroundNodes;
    third_party_integration::krino::SensitivityMap mSensitivityMap;
};

/// @brief Free function to facilitate making an initial guess.
///
/// Read a mesh from @a aFileName, and use the level set primitives @a aLevelSetPrimitives, along with an optional
/// specification of the background node ids @a aBackgroundDesignIDs. This is primarily used to specify a starting point
/// for a level set optimization.
[[nodiscard]] auto make_initial_guess_from_level_set_primitives(
    const std::filesystem::path& aFileName,
    const third_party_integration::krino::LevelSetPrimitives& aLevelSetPrimitives,
    const std::optional<std::vector<third_party_integration::krino::BackgroundMeshNodeId>>& aBackgroundDesignIDs)
    -> std::vector<double>;

/// @brief Helper function to facilitate making a KrinoWrapper.
///
/// Use a mesh along with level set values and fixed blocks specified in an analysis domain mesh @a
/// aAnalysisDomainMesh to create a KrinoWrapper. Design domain IDs are determined from the Analysis domain
/// mesh. Fixed regions are set to a fixed block level set value @a aFixedBlockLevelSetValue.
[[nodiscard]] auto make_krino_wrapper_from_analysis_domain_mesh(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                                const double aFixedBlockLevelSetValue) -> KrinoWrapper;

namespace detail
{
/// @brief Take a krino mesh's bulk data @a aBulkData, its level set fields in @a aLevelSetFields, along with the design
/// domain ids @a aDesignDomainBackgroundNodes and compute the SensitivityMap. The map takes cut mesh surface node ids
/// and maps them to LevelSetJacobianColumn which is a list of Parent Ids along with their sensitivities. If a parent is
/// a fixed node and not part of the @a aDesignDomainBackgroundNodes it is not included in the sensitivty map.
/// @pre The environment for krino must be initialized and set up as follows:
/// @code{.cpp}
/// initialize_environment_for_krino(...);
/// read_and_setup_for_decomposition(...);
/// setup_level_set_field_values(...);
/// cut_mesh(...);
/// @endcode
[[nodiscard]] auto compute_sensitivities(
    const stk::mesh::BulkData& aBulkData,
    const std::vector<::krino::LS_Field>& aLevelSetFields,
    const std::vector<third_party_integration::krino::BackgroundMeshNodeId>& aDesignDomainBackgroundNodes)
    -> third_party_integration::krino::SensitivityMap;

[[nodiscard]] auto row_vector_to_vector3(const std::vector<double>& aRowVector,
                                         const utilities::VectorIndex aVectorIndex,
                                         const std::size_t aDimensions) -> third_party_integration::common::Vector3;

}  // namespace detail

}  // namespace plato::geometry::extension

#endif
