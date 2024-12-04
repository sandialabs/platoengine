#include <Epetra_Export.h>
#include <Epetra_Map.h>
#include <Epetra_MpiComm.h>
#include <Epetra_Vector.h>

#include <Plato_Application.hpp>
#include <Plato_InputData.hpp>
#include <Plato_Interface.hpp>
#include <Plato_SharedData.hpp>
#include <Plato_TimersTree.hpp>
#include <vector>

#include "PlatoKrinoEnums.hpp"
#include "PlatoKrinoWrapper.hpp"

#include "PlatoKrinoAppUtils.hpp"

#pragma once

namespace Plato
{

enum struct DFDXFormatting
{
    GlobalID,
    OneToN
};

/**********************************************************************/
class PlatoKrinoApp : public Plato::Application
/**********************************************************************/
{
   public:
    explicit PlatoKrinoApp(Plato::Interface *aInterface, const apps::krino_app::CommandLineOptions &aOptions);
    void finalize() override;
    void initialize() override;
    void compute(const std::string &aName) override;
    void importData(const std::string &aName, const Plato::SharedData &tSharedData) override;
    void exportData(const std::string &aName, Plato::SharedData &tSharedData) override;
    void exportDataMap(const Plato::data::layout_t &aDataLayout, std::vector<int> &aMyOwnedGlobalIDs) override;
    void executeInitialMeshFromPrimitives();
    void executeInitialMeshFromField();
    void executeInitialMesh();

   private:
    Plato::Interface *mInterface;
    std::string mBGMeshFilename;
    std::string mCutMeshFilename;
    Plato::InputData mAppfileData;
    std::string mFieldMeshName;
    std::string mFieldName;
    int mFieldDataTimeStep;
    std::unique_ptr<KrinoWrapper> mKrinoWrapper;
    std::map<std::string, std::vector<double> > mDoubleVectorMap;
    std::unique_ptr<Plato::TimersTree> mTimersTree;
    std::unique_ptr<Epetra_MpiComm> mEpetraComm;
    std::unique_ptr<Epetra_Map> mOwnedAndSharedNodeMap;
    std::unique_ptr<Epetra_Map> mOwnedNodeMap;
    std::unique_ptr<Epetra_Export> mExporter;
    std::unique_ptr<Epetra_Vector> mOwnedAndSharedVec;
    std::unique_ptr<Epetra_Vector> mOwnedVec;
    std::vector<int> mLocallyOwnedKrinoNodes;
    LevelsetPrimitives mLevelsetPrimitives;

   private:
    bool useFieldForInitialization();
    std::vector<double> getLevelsetValuesFromFieldInMesh();
    void setDFDLSInDataLayer(const std::unordered_map<unsigned int, double>& aDFDLS,
                             const std::vector<unsigned int>& aBackgroundMeshNodeMap);
    std::unordered_map<unsigned int, stk::math::Vector3d> getDFDXFromDataLayer(const DFDXFormatting aDFDXFormat);
    std::vector<double> getLevelsetValuesFromDataLayer();
    void setLevelsetValuesInDataLayer(std::vector<double> aValues);
    void initializeLocalSharedDataVariables();
    void setDoubleVector(const std::string &aName, std::vector<double> aVector);
    int getDoubleVectorSize(const std::string &aName);
    bool doesDoubleVectorExist(const std::string &aName);
    std::vector<double> getDoubleVector(const std::string &aName);
    void buildParallelNodeMaps(std::vector<int> &aLocallyOwnedNodes, std::vector<int> &aAllLocalNodes);
    void buildParallelMaps();
    void parallelFieldFromLocallyOwnedToLocal(const std::vector<double> &aLocallyOwnedField,
                                              std::vector<double> &aLocalField);
    void parallelFieldFromLocalToLocallyOwned(const std::vector<double> &aLocalField,
                                              std::vector<double> &aLocallyOwnedField);
    void initializeLevelsets();
    void updateGeometry();
    void recalculateDistanceField();
    void applyChainRuleGlobalIDFormat();
    void applyChainRule1ToNFormat();
    void communicateData(const Plato::SharedData &aSharedData,
                         const std::vector<double> &aDataIn,
                         const DataTransferMode &aTransferMode,
                         std::vector<double> &aDataOut);
    void communicateScalarFieldData(const std::vector<double> &aDataIn,
                                    const DataTransferMode &aTransferMode,
                                    std::vector<double> &aDataOut);
    int getSharedDataSize(const std::string &aName, const Plato::SharedData &aSharedData);
    Plato::InputData parseAppFile(const std::string &aFile);
};

}  // namespace Plato
