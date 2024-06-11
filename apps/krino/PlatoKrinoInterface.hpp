#include <memory>
#include <Akri_BoundingBoxMesh.hpp>
#include <Akri_Phase_Support.hpp>
#include <Akri_LevelSetPolicy.hpp>
#include <Akri_LevelSet.hpp>
#include <Akri_MeshFromFile.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include "PlatoKrinoUtilities.hpp"
#include "stk_mesh/base/Types.hpp"

#pragma once 

namespace Plato::Krino
{

struct LevelSetShapeSensitivity
{
  LevelSetShapeSensitivity(const stk::mesh::EntityId inInterfaceNodeId, const std::vector<stk::mesh::EntityId> & inParentNodeIds, const std::vector<stk::math::Vector3d> & inDCoordsdParentLevelSets)
  : interfaceNodeId(inInterfaceNodeId), parentNodeIds(inParentNodeIds), dCoordsdParentLevelSets(inDCoordsdParentLevelSets) {}
  stk::mesh::EntityId interfaceNodeId;
  std::vector<stk::mesh::EntityId> parentNodeIds;
  std::vector<stk::math::Vector3d> dCoordsdParentLevelSets;
};

struct DFDX
{
    unsigned int NodeID;
    stk::math::Vector3d Sensitivity;
};

struct InterfaceNode_DXDP
{
  std::vector<stk::mesh::EntityId> parentNodeIds;
  std::vector<stk::math::Vector3d> parentDXDP;
};

class PlatoKrinoInterface
{

public:
    std::map<stk::mesh::EntityId, InterfaceNode_DXDP> cut_mesh_and_return_sensitivities(const std::string &aBackgroundMeshName,
                         const std::string &aCutMesh, const std::vector<double> &aLevelsetValues);
    std::vector<double> initialize_mesh_with_levelset_primitives_and_return_levelset_values(
                  const std::string &aBackgroundMeshName,
                  const std::string &aCutMesh, 
                  const LevelsetPrimitives &aLevelsetPrimitives);

    // API functions used in PlatoKrinoApp in an optimization run

    // The guts of this need to be behind a krino API
    void readAndSetupMeshForDecomposition(const std::string &aFilename);
    void initializePlaneLevelset(const double &aNormalX, const double &aNormalY, const double &aNormalZ, const double &aOffset);
    void initializeSphereLevelset(const std::vector<std::pair<stk::math::Vector3d,double>> &aSpheres);
    void initializeLevelsetsFromPrimitives(const LevelsetPrimitives &aLevelsetPrimitives);
    void cutMesh();
    void redistance();
    void writeMesh(const std::string& aFilename);
    void resetMesh();
    void getSensitivities();
    std::map<unsigned int, double> calculateDFDLS(std::map<unsigned int, stk::math::Vector3d> &aDFDX);
    unsigned int getUncutBackgroundMeshSize() { return mUncutBackgroundMeshSize; }
    std::vector<double> getLevelsetValues();
    void setLevelsetValues(const std::vector<double> &aValuesIn);
    void setLevelsetValues_parallel(const std::vector<double> &aValuesIn);
    stk::mesh::BulkData* bulkData(){return mBulkData;} 
    void includeVoidRegion(const bool aValue) { mIncludeVoidRegion = aValue; }
    bool includeVoidRegion() { return mIncludeVoidRegion; }

    // Functions used for unit testing
    void setUncutBackgroundMeshSize(const unsigned int &tValue) { mUncutBackgroundMeshSize = tValue; }
    void setSensitivities(const std::vector<std::pair<unsigned int, std::vector<std::pair<unsigned int, stk::math::Vector3d>>>> &aSensitivities);
    void createBoundingBoxMesh(const double &aXMin, const double &aYMin, const double &aZMin,
                                  const double &aXMax, const double &aYMax, const double &aZMax, 
                                  const double &aMeshSize);
    void getNodalCoordinates(const unsigned int &aNodeID, double &aX, double &aY, double &aZ);
    void createAndWriteBoundingBoxMesh(const stk::math::Vector3d & aMinCorner,
                                       const stk::math::Vector3d & aMmaxCorner,
                                       const double &aMeshSize, const std::string &aFilename);
    unsigned int getNumTetsInNamedBlock(const std::string &aBlockName);
    std::unordered_map<unsigned int, stk::math::Vector3d> getCoordinateValues();
    std::unordered_map<unsigned int, stk::math::Vector3d> predictNewCoordinatesBasedOnPerturbedLevelsetValues(std::unordered_map<unsigned int, stk::math::Vector3d> &tCoordVals,
                                                                const double &aPerturbation);

private:
    std::unique_ptr<krino::BoundingBoxMesh> mBoundingBoxMesh;
    std::unique_ptr<krino::MeshFromFile> mMeshFromFile;
    std::vector<krino::LS_Field> mLSFields;
    std::map<stk::mesh::EntityId, InterfaceNode_DXDP> mSensitivities;
    stk::mesh::BulkData *mBulkData;
    unsigned int mUncutBackgroundMeshSize;
    krino::LevelSet *mLevelSet=nullptr;
    bool mIncludeVoidRegion = false;

    // Krino functionality that needs to be in a higher level API and provided/supported by krino
    void write_mesh(const std::string& aFilename);
    void create_bounding_box_mesh(const double &aXMin, const double &aYMin, const double &aZMin,
                                                   const double &aXMax, const double &aYMax, const double &aZMax,
                                                   const double &aMeshSize);
    void read_and_setup_mesh_for_decomposition(const std::string &aFilename);
    stk::mesh::Selector build_output_selector(const stk::mesh::MetaData & meta, const stk::mesh::Part & activePart);
    void generate_bounding_box_mesh(krino::BoundingBoxMesh & bboxMesh, const stk::math::Vector3d & minCorner, const stk::math::Vector3d & maxCorner, const double meshSize);
    void setup_fields_for_conforming_decomposition(const stk::mesh::MetaData & meta);
    void initialize_levelset_field_for_plane(const stk::mesh::BulkData & mesh, krino::FieldRef levelSetField, const stk::math::Vector3d & normal, const double offset);
    void initialize_levelset_field_for_spheres(const stk::mesh::BulkData & mesh, krino::FieldRef levelSetField, const std::vector<std::pair<stk::math::Vector3d,double>> & spheres);
    void initialize_levelset_fields_from_primitives(const stk::mesh::BulkData & mesh, krino::FieldRef levelSetField,
                                                    const LevelsetPrimitives &aLevelsetPrimitives);
    void decompose_mesh_to_conform_to_levelsets(stk::mesh::BulkData & mesh, const std::vector<krino::LS_Field> & lsFields);
    std::map<stk::mesh::EntityId, InterfaceNode_DXDP> get_levelset_shape_sensitivities(const stk::mesh::BulkData & mesh, const krino::FieldRef levelSetField);
    void fill_node_ids_for_nodes(const stk::mesh::BulkData & mesh, const std::vector<stk::mesh::Entity> & parentNodes, std::vector<stk::mesh::EntityId> & parentNodeIds);
    void fill_d_coords_d_levelsets(const krino::FieldRef coordsField, const krino::FieldRef levelSetField, const std::vector<stk::mesh::Entity> & parentNodes, 
                                   /*const std::vector<double> & parentWeights,*/ std::vector<stk::math::Vector3d> & dCoordsdParentLevelSets);
    void generate_and_write_bounding_box_mesh(const stk::topology elemTopology,
                                              const stk::math::Vector3d & minCorner,
                                              const stk::math::Vector3d & maxCorner,
                                              const double meshSize, const std::string & filename);

    // Private functions
    bool includeVoidRegionPart(const stk::mesh::Part* aPart);
    std::map<stk::mesh::EntityId, InterfaceNode_DXDP> convertSensitivitiesToMap();

};

} // namespace Plato::Krino

