#ifndef PLATO_THIRD_PARTY_INTEGRATION_KRINO_KRINO_WRAPPER
#define PLATO_THIRD_PARTY_INTEGRATION_KRINO_KRINO_WRAPPER

#include <Akri_LevelSet.hpp>
#include <Akri_LevelSetPolicy.hpp>
#include <Akri_MeshInterface.hpp>
#include <filesystem>
#include <functional>
#include <stk_mesh/base/MetaData.hpp>

#include "plato/third_party_integration/krino/LevelsetPrimitives.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"

namespace plato::third_party_integration::krino
{

constexpr int kNumDimensions = 3;

class KrinoWrapper
{
   public:
    KrinoWrapper(const std::filesystem::path &aFilename, const bool aIncludeVoidRegion = false);
    KrinoWrapper(const stk::math::Vector3d &aMinCorner,
                 const stk::math::Vector3d &aMaxCorner,
                 const double aMeshSize,
                 const std::filesystem::path &aFilename,
                 const bool aIncludeVoidRegion = false);

    void setLevelsetValues(const std::vector<double> &aValuesIn);
    void cutMesh();
    void writeMesh(const std::filesystem::path &aFilename);
    auto getSensitivities() -> std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP> { return mSensitivities; }
    void initializeLevelsetsFromPrimitives(const LevelsetPrimitives &aLevelsetPrimitives);
    [[nodiscard]] auto getLevelsetValues() -> std::vector<double>;
    void getNodalCoordinates(const unsigned int &aNodeID, double &aX, double &aY, double &aZ);
    void initializeSphereLevelset(const std::vector<std::pair<stk::math::Vector3d, double>> &aSpheres);
    unsigned int getNumTetsInNamedBlock(const std::string &aBlockName);
    void initializePlaneLevelset(const double aNormalX,
                                 const double aNormalY,
                                 const double aNormalZ,
                                 const double aOffset);
    void resetMesh();
    void redistance();
    [[nodiscard]] auto getCoordinateValues() -> std::unordered_map<unsigned int, stk::math::Vector3d>;
    [[nodiscard]] auto predictNewCoordinatesBasedOnPerturbedLevelsetValues(
        const std::unordered_map<unsigned int, stk::math::Vector3d> &tCoordVals, const double aPerturbation)
        -> std::unordered_map<unsigned int, stk::math::Vector3d>;
    [[nodiscard]] auto bulkData() -> const stk::mesh::BulkData & { return mKrinoMesh->bulk_data(); }
    [[nodiscard]] auto getUncutBackgroundMeshSize() -> unsigned int { return mUncutBackgroundMeshSize; }

   private:
    [[nodiscard]] auto readAndSetupMeshForDecomposition(const std::filesystem::path &aFilename)
        -> std::unique_ptr<::krino::MeshInterface>;
    [[nodiscard]] auto createBoundingBoxMesh(const stk::math::Vector3d &aMinCorner,
                                             const stk::math::Vector3d &aMaxCorner,
                                             const double aMeshSize,
                                             const std::filesystem::path &aFilename)
        -> std::unique_ptr<::krino::MeshInterface>;
    [[nodiscard]] auto buildOutputSelector(const stk::mesh::MetaData &meta, const stk::mesh::Part &activePart)
        -> stk::mesh::Selector;
    void setupFieldsForConformingDecomposition(const stk::mesh::MetaData &meta);
    bool includeVoidRegionPart(const stk::mesh::Part *aPart);
    void decomposeMeshToConformToLevelsets(stk::mesh::BulkData &mesh, const std::vector<::krino::LS_Field> &lsFields);
    [[nodiscard]] auto getLevelsetShapeSensitivities(const stk::mesh::BulkData &mesh,
                                                     const ::krino::FieldRef levelSetField)
        -> std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP>;
    void fillNodeIdsForNodes(const stk::mesh::BulkData &mesh,
                             const std::vector<stk::mesh::Entity> &parentNodes,
                             std::vector<stk::mesh::EntityId> &parentNodeIds);
    void fillDCoordsDLevelsets(const ::krino::FieldRef coordsField,
                               const ::krino::FieldRef levelSetField,
                               const std::vector<stk::mesh::Entity> &parentNodes,
                               std::vector<stk::math::Vector3d> &dCoordsdParentLevelSets);
    void initializeLevelsetFieldsFromPrimitives(const stk::mesh::BulkData &mesh,
                                                ::krino::FieldRef levelSetField,
                                                const LevelsetPrimitives &aLevelsetPrimitives);
    void initializeLevelsetFieldForSpheres(const stk::mesh::BulkData &mesh,
                                           ::krino::FieldRef levelSetField,
                                           const std::vector<std::pair<stk::math::Vector3d, double>> &spheres);
    void initializeLevelsetFieldForPlane(const stk::mesh::BulkData &mesh,
                                         ::krino::FieldRef levelSetField,
                                         const stk::math::Vector3d &normal,
                                         const double offset);
    [[nodiscard]] auto getNodeEntitiesInMesh() -> stk::mesh::EntityVector;

   private:
    unsigned int mUncutBackgroundMeshSize;
    bool mIncludeVoidRegion;
    std::vector<::krino::LS_Field> mLSFields;
    std::unique_ptr<::krino::MeshInterface> mKrinoMesh;
    std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP> mSensitivities;
};

}  // namespace plato::third_party_integration::krino

#endif  // PLATO_THIRD_PARTY_INTEGRATION_KRINO_KRINO_WRAPPER
