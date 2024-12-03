#ifndef PLATO_THIRDPARTYINTEGRATION_KRINO_KRINOWRAPPER
#define PLATO_THIRDPARTYINTEGRATION_KRINO_KRINOWRAPPER

#include <Akri_LevelSet.hpp>
#include <Akri_LevelSetPolicy.hpp>
#include <Akri_MeshInterface.hpp>
#include <filesystem>
#include <functional>
#include <stk_mesh/base/MetaData.hpp>

#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"

namespace plato::third_party_integration::krino
{
/// @brief Interface with Krino
///
/// This interface with Krino can create a cut mesh from a background mesh and either a set of primitives
/// or a given level set field. It also provides member functions to compute the nodal coordinate sensitivities with
/// respect to the level set field.
class KrinoWrapper
{
   public:
    KrinoWrapper(const std::filesystem::path &aFilename,
                 const LevelSetPrimitives &aLevelSetPrimitives,
                 const VoidPhase aIncludeVoidRegion = VoidPhase::kExcludeFromMesh);
    KrinoWrapper(const std::filesystem::path &aFilename,
                 const std::vector<double> &aLevelSetValues,
                 const VoidPhase aIncludeVoidRegion = VoidPhase::kExcludeFromMesh);

    void setLevelSetValues(const std::vector<double> &aValuesIn);
    void writeMesh(const std::filesystem::path &aFilename);
    void redistance();

    [[nodiscard]] auto sensitivities() const -> std::unordered_map<stk::mesh::EntityId, LevelSetJacobianColumn>;
    [[nodiscard]] auto levelSetValues() const -> std::vector<double>;
    [[nodiscard]] auto coordinates() const -> std::unordered_map<unsigned int, stk::math::Vector3d>;
    [[nodiscard]] auto bulkData() const -> const stk::mesh::BulkData &;

   private:
    VoidPhase mVoidRegion;
    std::unique_ptr<::krino::MeshInterface> mKrinoMesh;
    std::vector<::krino::LS_Field> mLevelSetFields;
};

}  // namespace plato::third_party_integration::krino

#endif  // PLATO_THIRD_PARTY_INTEGRATION_KRINO_KRINO_WRAPPER
