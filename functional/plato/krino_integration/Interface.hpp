#ifndef PLATO_KRINO_INTEGRATION_INTERFACE
#define PLATO_KRINO_INTEGRATION_INTERFACE

#include <Akri_BoundingBoxMesh.hpp>
#include <Akri_LevelSet.hpp>
#include <Akri_LevelSetPolicy.hpp>
#include <Akri_MeshFromFile.hpp>
#include <Akri_Phase_Support.hpp>
#include <memory>
#include <stk_mesh/base/MetaData.hpp>

#include "Utilities.hpp"
#include "stk_mesh/base/Types.hpp"

namespace plato::krino_integration
{

class PlatoKrinoInterface
{
#if 0

   public:
 
    std::map<stk::mesh::EntityId, InterfaceNode_DXDP> generateComputationalMesh(const std::string &aBackgroundMeshName,
                  const std::string &aCutMesh,
                  const std::vector<double> &aLevelsetValues,
                  const bool aIncludeVoidRegion);
    //std::map<stk::mesh::EntityId, InterfaceNode_DXDP> getSensitivities();


private:
    // API functions used in PlatoKrinoApp in an optimization run

    // The guts of this need to be behind a krino API
    void setLevelsetValues_parallel(const std::vector<double> &aValuesIn);

public:
    // Functions used for unit testing
    std::map<unsigned int, double> calculateDFDLS(std::map<unsigned int, stk::math::Vector3d> &aDFDX);
    void setSensitivities(
        const std::vector<std::pair<unsigned int, std::vector<std::pair<unsigned int, stk::math::Vector3d>>>>
            &aSensitivities);
    void createBoundingBoxMesh(const double &aXMin,
                               const double &aYMin,
                               const double &aZMin,
                               const double &aXMax,
                               const double &aYMax,
                               const double &aZMax,
                               const double &aMeshSize);
        

   private:
    //krino-specific data

    // Krino functionality that needs to be in a higher level API and provided/supported by krino
    void create_bounding_box_mesh(const double &aXMin,
                                  const double &aYMin,
                                  const double &aZMin,
                                  const double &aXMax,
                                  const double &aYMax,
                                  const double &aZMax,
                                  const double &aMeshSize);
    std::map<stk::mesh::EntityId, InterfaceNode_DXDP> convertSensitivitiesToMap();
#endif
};

}  // namespace plato::krino_integration

#endif  // PLATO_KRINO_INTEGRATION_INTERFACE
