#ifndef PLATO_KRINO_INTEGRATION_KRINO_WRAPPER
#define PLATO_KRINO_INTEGRATION_KRINO_WRAPPER

#include <Akri_BoundingBoxMesh.hpp>
#include <Akri_LevelSet.hpp>
#include <Akri_LevelSetPolicy.hpp>
#include <Akri_MeshFromFile.hpp>
// #include <Akri_Phase_Support.hpp>
// #include <memory>
#include <stk_mesh/base/MetaData.hpp>

#include "Utilities.hpp"
// #include "stk_mesh/base/Types.hpp"

namespace plato::krino_integration
{

class KrinoWrapper
{
   public:
    void setIncludeVoidRegion(const bool aValue) { mIncludeVoidRegion = aValue; }
    void readAndSetupMeshForDecomposition(const std::string &aFilename);
    void setLevelsetValues(const std::vector<double> &aValuesIn);
    void cutMesh();
    void writeMesh(const std::string &aFilename);
    std::map<stk::mesh::EntityId, InterfaceNode_DXDP> getSensitivities() { return mSensitivities; }
    void initializeLevelsetsFromPrimitives(const LevelsetPrimitives &aLevelsetPrimitives);
    std::vector<double> getLevelsetValues();
    void createBoundingBoxMesh(const stk::math::Vector3d &aMinCorner,
                               const stk::math::Vector3d &aMmaxCorner,
                               const double &aMeshSize,
                               const std::string &aFilename);
    void getNodalCoordinates(const unsigned int &aNodeID, double &aX, double &aY, double &aZ);
    void initializeSphereLevelset(const std::vector<std::pair<stk::math::Vector3d, double>> &aSpheres);
    unsigned int getNumTetsInNamedBlock(const std::string &aBlockName);
    void initializePlaneLevelset(const double &aNormalX,
                                 const double &aNormalY,
                                 const double &aNormalZ,
                                 const double &aOffset);
    void resetMesh();
    void redistance();
    std::unordered_map<unsigned int, stk::math::Vector3d> getCoordinateValues();
    std::unordered_map<unsigned int, stk::math::Vector3d> predictNewCoordinatesBasedOnPerturbedLevelsetValues(
        std::unordered_map<unsigned int, stk::math::Vector3d> &tCoordVals, const double &aPerturbation);
    stk::mesh::BulkData *bulkData() { return mBulkData; }
    unsigned int getUncutBackgroundMeshSize() { return mUncutBackgroundMeshSize; }

   private:
    //        bool getIncludeVoidRegion() { return mIncludeVoidRegion; }
    //        void setUncutBackgroundMeshSize(const unsigned int &tValue) { mUncutBackgroundMeshSize = tValue; }
    void writeMeshPrivate(const std::string &aFilename);
    stk::mesh::Selector buildOutputSelector(const stk::mesh::MetaData &meta, const stk::mesh::Part &activePart);
    void readAndSetupMeshForDecompositionPrivate(const std::string &aFilename);
    void setupFieldsForConformingDecomposition(const stk::mesh::MetaData &meta);
    bool includeVoidRegionPart(const stk::mesh::Part *aPart);
    void decomposeMeshToConformToLevelsets(stk::mesh::BulkData &mesh, const std::vector<krino::LS_Field> &lsFields);
    std::map<stk::mesh::EntityId, InterfaceNode_DXDP> getLevelsetShapeSensitivities(
        const stk::mesh::BulkData &mesh, const krino::FieldRef levelSetField);
    void fillNodeIdsForNodes(const stk::mesh::BulkData &mesh,
                             const std::vector<stk::mesh::Entity> &parentNodes,
                             std::vector<stk::mesh::EntityId> &parentNodeIds);
    void fillDCoordsDLevelsets(const krino::FieldRef coordsField,
                               const krino::FieldRef levelSetField,
                               const std::vector<stk::mesh::Entity> &parentNodes,
                               std::vector<stk::math::Vector3d> &dCoordsdParentLevelSets);
    void initializeLevelsetFieldsFromPrimitives(const stk::mesh::BulkData &mesh,
                                                krino::FieldRef levelSetField,
                                                const LevelsetPrimitives &aLevelsetPrimitives);
    void createBoundingBoxMeshPrivate(const stk::math::Vector3d &aMinCorner,
                                      const stk::math::Vector3d &aMaxCorner,
                                      const double &aMeshSize,
                                      const std::string &aFilename);
    void initializeLevelsetFieldForSpheres(const stk::mesh::BulkData &mesh,
                                           krino::FieldRef levelSetField,
                                           const std::vector<std::pair<stk::math::Vector3d, double>> &spheres);
    void initializeLevelsetFieldForPlane(const stk::mesh::BulkData &mesh,
                                         krino::FieldRef levelSetField,
                                         const stk::math::Vector3d &normal,
                                         const double offset);

   private:
    stk::mesh::BulkData *mBulkData;
    krino::LevelSet *mLevelSet = nullptr;
    std::unique_ptr<krino::BoundingBoxMesh> mBoundingBoxMesh;
    std::unique_ptr<krino::MeshFromFile> mMeshFromFile;
    std::vector<krino::LS_Field> mLSFields;
    std::map<stk::mesh::EntityId, InterfaceNode_DXDP> mSensitivities;
    unsigned int mUncutBackgroundMeshSize;
    bool mIncludeVoidRegion = false;
};

}  // namespace plato::krino_integration

#endif  // PLATO_KRINO_INTEGRATION_KRINO_WRAPPER
