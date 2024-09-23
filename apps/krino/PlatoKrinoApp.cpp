#include <numeric>

#include "PlatoKrinoApp.hpp"
#include "PlatoKrinoAppUtils.hpp"

#include <Kokkos_Core.hpp>
#include <Plato_Parser.hpp>
#include <Plato_TimersTree.hpp>
#include <stk_coupling/SplitComms.hpp>
#include <stk_io/StkMeshIoBroker.hpp>
#include <stk_mesh/base/MeshBuilder.hpp>
#include <stk_util/environment/Env.hpp>
#include <stk_util/environment/EnvData.hpp>
#include <stk_util/parallel/Parallel.hpp>

#include "plato/krino_integration/Parse.hpp"
#include "plato/krino_integration/Utilities.hpp"

namespace Plato
{

/******************************************************************************/
PlatoKrinoApp::PlatoKrinoApp(Plato::Interface* aInterface, const CommandLineOptions& aOptions)
    : /******************************************************************************/
      mInterface(aInterface),
      mBGMeshFilename(aOptions.mBackgroundMeshName),
      mCutMeshFilename(aOptions.mCutMeshName),
      mAppfileData(parseAppFile(aOptions.mKrinoOperationsFileName)),
      mFieldMeshName(aOptions.mFieldMeshName),
      mFieldName(aOptions.mFieldName),
      mFieldDataTimeStep(aOptions.mFieldDataTimeStep)
{
    mKrinoWrapper.setIncludeVoidRegion(aOptions.mIncludeVoidRegion);
}

Plato::InputData PlatoKrinoApp::parseAppFile(const std::string& aFile)
{
    const char* input_char = !aFile.empty() ? aFile.c_str() : getenv("PLATO_APP_FILE");
    Plato::PugiParser tParser;
    return tParser.parseFile(input_char);
}

/******************************************************************************/
void PlatoKrinoApp::initialize()
/******************************************************************************/
{
    MPI_Comm tComm;
    mInterface->getLocalComm(tComm);

    // conditionally begin timers
    if (mAppfileData.size<Plato::InputData>("Timers"))
    {
        auto tTimersNode = mAppfileData.get<Plato::InputData>("Timers");
        if (tTimersNode.size<std::string>("time") > 0)
        {
            const bool do_time = Plato::Get::Bool(tTimersNode, "time");
            if (do_time)
            {
                mTimersTree = std::make_unique<Plato::TimersTree>(tComm);
            }
        }
    }

    mLevelsetPrimitives = readLevelsetInitializationData(mAppfileData);
    mKrinoWrapper.readAndSetupMeshForDecomposition(mBGMeshFilename);

    buildParallelMaps();

    initializeLocalSharedDataVariables();
}

/******************************************************************************/
void PlatoKrinoApp::exportDataMap(const Plato::data::layout_t& aDataLayout, std::vector<int>& aMyOwnedGlobalIDs)
/******************************************************************************/
{
    if (aDataLayout == Plato::data::layout_t::SCALAR_FIELD)
    {
        aMyOwnedGlobalIDs = mLocallyOwnedKrinoNodes;
    }
    else if (aDataLayout == Plato::data::layout_t::ELEMENT_FIELD)
    {
    }
}

void PlatoKrinoApp::initializeLocalSharedDataVariables()
/******************************************************************************/
{
    std::vector<double> tDummyData(1u, 0.);
    setDoubleVector("LevelsetValues", tDummyData);
    setDoubleVector("DFDX", tDummyData);
    setDoubleVector("DFDLS", tDummyData);
    setDoubleVector("CutMeshGlobalNodeIDMap", tDummyData);
}

/******************************************************************************/
bool PlatoKrinoApp::useFieldForInitialization()
/******************************************************************************/
{
    return !mFieldMeshName.empty() && !mFieldName.empty() && mFieldDataTimeStep != 0;
}

/******************************************************************************/
void PlatoKrinoApp::executeInitialMesh()
/******************************************************************************/
{
    if (useFieldForInitialization())
    {
        executeInitialMeshFromField();
    }
    else
    {
        executeInitialMeshFromPrimitives();
    }
}

/******************************************************************************/
void PlatoKrinoApp::executeInitialMeshFromPrimitives()
/******************************************************************************/
{
    mLevelsetPrimitives = readLevelsetInitializationData(mAppfileData);
    mKrinoWrapper.readAndSetupMeshForDecomposition(mBGMeshFilename);
    mKrinoWrapper.initializeLevelsetsFromPrimitives(mLevelsetPrimitives);
    mKrinoWrapper.cutMesh();
    mKrinoWrapper.writeMesh(mCutMeshFilename);
}

/******************************************************************************/
std::vector<double> PlatoKrinoApp::getLevelsetValuesFromFieldInMesh()
/******************************************************************************/
{
    std::vector<double> tValues;

    std::shared_ptr<stk::mesh::MetaData> tMetaData = stk::mesh::MeshBuilder().create_meta_data();
    tMetaData->use_simple_fields();
    std::shared_ptr<stk::mesh::BulkData> tBulkData = stk::mesh::MeshBuilder(MPI_COMM_WORLD).create(tMetaData);
    std::shared_ptr<stk::io::StkMeshIoBroker> tIoBroker = std::make_shared<stk::io::StkMeshIoBroker>(MPI_COMM_WORLD);
    tIoBroker->set_bulk_data(*tBulkData);

    tIoBroker->set_option_to_not_collapse_sequenced_fields();
    tIoBroker->property_add(Ioss::Property("MAXIMUM_NAME_LENGTH", 256));
    tIoBroker->add_mesh_database(mFieldMeshName, "exodus", stk::io::READ_MESH);
    tIoBroker->create_input_mesh();
    tIoBroker->add_all_mesh_fields_as_input_fields();
    tIoBroker->populate_bulk_data();
    const stk::mesh::Field<double>* tField = tMetaData->get_field<double>(stk::topology::NODE_RANK, mFieldName);
    tIoBroker->read_defined_input_fields(mFieldDataTimeStep);

    // Get the number of nodes for sizing the return vector
    int tNumNodes = 0;
    stk::mesh::BucketVector const& tOwnedBuckets =
        tBulkData->get_buckets(stk::topology::NODE_RANK, tMetaData->locally_owned_part());
    for (auto&& tBucketPtr : tOwnedBuckets)
    {
        tNumNodes += tBucketPtr->size();
    }
    tValues.resize(tNumNodes, 0.0);

    // We don't currently handle node maps.  However, if the node map is non trivial
    // at least we can return the values in the order of ascending global node ids
    // so we will get the values and then load the return vector in an ordered way.
    std::map<unsigned int, double> tGlobalNodeIDToLevelsetValueMap;
    std::set<unsigned int> tSortedGlobalNodeIDs;
    for (auto&& tBucketPtr : tOwnedBuckets)
    {
        for (const auto& tNode : *tBucketPtr)
        {
            int tGlobalNodeID = tBulkData->identifier(tNode);
            double* val = stk::mesh::field_data(*tField, tNode);
            tGlobalNodeIDToLevelsetValueMap[tGlobalNodeID] = *val;
            tSortedGlobalNodeIDs.insert(tGlobalNodeID);
        }
    }

    auto tSetIterator = tSortedGlobalNodeIDs.cbegin();
    int tCntr = 0;
    while (tSetIterator != tSortedGlobalNodeIDs.cend())
    {
        tValues[tCntr++] = tGlobalNodeIDToLevelsetValueMap[*tSetIterator];
        tSetIterator++;
    }
    return tValues;
}

/******************************************************************************/
void PlatoKrinoApp::executeInitialMeshFromField()
/******************************************************************************/
{
    std::vector<double> tLevelsetValues = getLevelsetValuesFromFieldInMesh();
    mKrinoWrapper.readAndSetupMeshForDecomposition(mBGMeshFilename);
    mKrinoWrapper.setLevelsetValues(tLevelsetValues);
    mKrinoWrapper.cutMesh();
    mKrinoWrapper.writeMesh(mCutMeshFilename);
}

/******************************************************************************/
void PlatoKrinoApp::compute(const std::string& aName)
/******************************************************************************/
{
    // begin timer for doing physics computation
    if (mTimersTree)
    {
        mTimersTree->begin_partition(Plato::timer_partition_t::timer_partition_t::physics_compute);
    }

    MPI_Comm tComm;
    mInterface->getLocalComm(tComm);
    MPI_Barrier(tComm);

    if (aName == "Initialize Levelsets")
    {
        initializeLevelsets();
    }
    else if (aName == "Update Geometry")
    {
        updateGeometry();
    }
    else if (aName == "Recalculate Distance Field")
    {
        recalculateDistanceField();
    }
    else if (aName == "Apply Chain Rule 1 to N Format")
    {
        applyChainRule1ToNFormat();
    }
    else if (aName == "Apply Chain Rule Global ID Format")
    {
        applyChainRuleGlobalIDFormat();
    }

    // end timer for doing physics computation
    if (mTimersTree)
    {
        mTimersTree->end_partition();
    }
}

/******************************************************************************/
void PlatoKrinoApp::applyChainRuleGlobalIDFormat()
/******************************************************************************/
{
    std::map<unsigned int, stk::math::Vector3d> tDFDX = getDFDXFromDataLayer(DFDXFormatting::GlobalID);
    const std::map<stk::mesh::EntityId, InterfaceNode_DXDP> tGlobalIDToDXDP = mKrinoWrapper.getSensitivities();
    std::vector<unsigned int> tBackgroundNodeMap(mKrinoWrapper.getUncutBackgroundMeshSize());
    // For now we are assuming there is no node map in the background mesh.  This is not 
    // a restriction in the new architecture.
    std::iota (std::begin(tBackgroundNodeMap), std::end(tBackgroundNodeMap), 1);
    std::map<unsigned int, double> tDFDLS = calculateDFDLS(tDFDX, tGlobalIDToDXDP, tBackgroundNodeMap);
    setDFDLSInDataLayer(tDFDLS);
}

/******************************************************************************/
void PlatoKrinoApp::applyChainRule1ToNFormat()
/******************************************************************************/
{
    std::map<unsigned int, stk::math::Vector3d> tDFDX = getDFDXFromDataLayer(DFDXFormatting::OneToN);
    const std::map<stk::mesh::EntityId, InterfaceNode_DXDP> tGlobalIDToDXDP = mKrinoWrapper.getSensitivities();
    std::vector<unsigned int> tBackgroundNodeMap(mKrinoWrapper.getUncutBackgroundMeshSize());
    // For now we are assuming there is no node map in the background mesh.  This is not 
    // a restriction in the new architecture.
    std::iota (std::begin(tBackgroundNodeMap), std::end(tBackgroundNodeMap), 1);
    std::map<unsigned int, double> tDFDLS = calculateDFDLS(tDFDX, tGlobalIDToDXDP, tBackgroundNodeMap);
    setDFDLSInDataLayer(tDFDLS);
}

/******************************************************************************/
void PlatoKrinoApp::recalculateDistanceField()
/******************************************************************************/
{
    std::vector<double> tLevelsetValues = getLevelsetValuesFromDataLayer();
    mKrinoWrapper.setLevelsetValues(tLevelsetValues);
    mKrinoWrapper.resetMesh();
    mKrinoWrapper.redistance();
    std::vector<double> tCurLevelsetValues = mKrinoWrapper.getLevelsetValues();
    setLevelsetValuesInDataLayer(tCurLevelsetValues);
}

/******************************************************************************/
void PlatoKrinoApp::updateGeometry()
/******************************************************************************/
{
    std::vector<double> tLevelsetValues = getLevelsetValuesFromDataLayer();
    mKrinoWrapper.setLevelsetValues(tLevelsetValues);
    mKrinoWrapper.resetMesh();
    mKrinoWrapper.cutMesh();
    mKrinoWrapper.getSensitivities();
    mKrinoWrapper.writeMesh(mCutMeshFilename);
}

/******************************************************************************/
void PlatoKrinoApp::initializeLevelsets()
/******************************************************************************/
{
    mKrinoWrapper.initializeLevelsetsFromPrimitives(mLevelsetPrimitives);
    mKrinoWrapper.cutMesh();
    mKrinoWrapper.writeMesh(mCutMeshFilename);
    std::vector<double> tCurLevelsetValues = mKrinoWrapper.getLevelsetValues();
    setLevelsetValuesInDataLayer(tCurLevelsetValues);
}

/******************************************************************************/
void PlatoKrinoApp::setDFDLSInDataLayer(std::map<unsigned int, double>& aDFDLS)
/******************************************************************************/
{
    std::vector<double> tDFDLSForDL(aDFDLS.size());
    std::map<unsigned int, double>::iterator it = aDFDLS.begin();
    for (size_t i = 0; i < tDFDLSForDL.size(); i++)
    {
        tDFDLSForDL[i] = it->second;
        it++;
    }
    setDoubleVector("DFDLS", tDFDLSForDL);
}

/******************************************************************************/
std::map<unsigned int, stk::math::Vector3d> PlatoKrinoApp::getDFDXFromDataLayer(const DFDXFormatting aDFDXFormat)
/******************************************************************************/
{
    // get DFDX
    std::vector<double> tDFDXFromDL = getDoubleVector("DFDX");

    // get CutMeshGlobalNodeIDMap
    std::vector<double> tCutMeshGlobalNodeIDMapFromDL = getDoubleVector("CutMeshGlobalNodeIDMap");

    return assembleGlobalIDToDFDXMap(tDFDXFromDL, tCutMeshGlobalNodeIDMapFromDL, aDFDXFormat);
}

/******************************************************************************/
std::vector<double> PlatoKrinoApp::getLevelsetValuesFromDataLayer()
/******************************************************************************/
{
    return getDoubleVector("LevelsetValues");
}

/******************************************************************************/
void PlatoKrinoApp::setLevelsetValuesInDataLayer(std::vector<double> aValues)
/******************************************************************************/
{
    setDoubleVector("LevelsetValues", std::move(aValues));
}

/******************************************************************************/
void PlatoKrinoApp::finalize()
/******************************************************************************/
{
    // timers
    if (mTimersTree)
    {
        mTimersTree->print_results();
    }

    MPI_Finalize();
}

/******************************************************************************/
void PlatoKrinoApp::importData(const std::string& aName, const Plato::SharedData& aSharedData)
/******************************************************************************/
{
    if (!doesDoubleVectorExist(aName))
    {
        return;
    }

    int tSize = getSharedDataSize(aName, aSharedData);
    std::vector<double> tData(tSize, 0.0);
    aSharedData.getData(tData);
    std::vector<double> tCommunicatedData;
    communicateData(aSharedData, tData, DataTransferMode::IMPORT, tCommunicatedData);
    setDoubleVector(aName, tCommunicatedData);
}

/******************************************************************************/
void PlatoKrinoApp::exportData(const std::string& aName, Plato::SharedData& aSharedData)
/******************************************************************************/
{
    if (!doesDoubleVectorExist(aName))
    {
        return;
    }

    std::vector<double> tData = getDoubleVector(aName);
    std::vector<double> tCommunicatedData;
    communicateData(aSharedData, tData, DataTransferMode::EXPORT, tCommunicatedData);
    aSharedData.setData(tCommunicatedData);
}

/******************************************************************************/
void PlatoKrinoApp::communicateScalarFieldData(const std::vector<double>& aDataIn,
                                               const DataTransferMode& aTransferMode,
                                               std::vector<double>& aDataOut)
/******************************************************************************/
{
    if (aTransferMode == DataTransferMode::IMPORT)
    {
        parallelFieldFromLocallyOwnedToLocal(aDataIn, aDataOut);
    }
    else if (aTransferMode == DataTransferMode::EXPORT)
    {
        parallelFieldFromLocalToLocallyOwned(aDataIn, aDataOut);
    }
    else
    {
        std::stringstream tError;
        tError << std::endl
               << "ERROR: Unknown data transfer mode type in PlatoKrinoApp::communicateScalarFieldData()." << std::endl;
        Plato::ParsingException tParsingException(tError.str());
        throw tParsingException;
    }
}

/******************************************************************************/
void PlatoKrinoApp::communicateData(const Plato::SharedData& aSharedData,
                                    const std::vector<double>& aDataIn,
                                    const DataTransferMode& aTransferMode,
                                    std::vector<double>& aDataOut)
/******************************************************************************/
{
    if (aSharedData.myLayout() == Plato::data::layout_t::SCALAR_FIELD)
    {
        communicateScalarFieldData(aDataIn, aTransferMode, aDataOut);
    }
    else if (aSharedData.myLayout() == Plato::data::layout_t::SCALAR ||
             aSharedData.myLayout() == Plato::data::layout_t::ELEMENT_FIELD)
    {
        aDataOut = aDataIn;
    }
    else
    {
        std::stringstream tError;
        tError << std::endl << "ERROR: Unknown layout type in PlatoKrinoApp::communicateData()." << std::endl;
        Plato::ParsingException tParsingException(tError.str());
        throw tParsingException;
    }
}

/******************************************************************************/
int PlatoKrinoApp::getSharedDataSize(const std::string& aName, const Plato::SharedData& aSharedData)
/******************************************************************************/
{
    int tReturn = 0;
    if (aSharedData.myLayout() == Plato::data::layout_t::SCALAR_FIELD ||
        aSharedData.myLayout() == Plato::data::layout_t::ELEMENT_FIELD)
    {
        tReturn = aSharedData.size();
    }
    else if (aSharedData.myLayout() == Plato::data::layout_t::SCALAR)
    {
        tReturn = getDoubleVectorSize(aName);
    }
    else
    {
        std::stringstream tError;
        tError << std::endl << "ERROR: Unknown layout type in PlatoKrinoApp::getSharedDataSize()." << std::endl;
        Plato::ParsingException tParsingException(tError.str());
        throw tParsingException;
    }
    return tReturn;
}

/******************************************************************************/
void PlatoKrinoApp::buildParallelNodeMaps(std::vector<int>& aLocallyOwnedNodes, std::vector<int>& aAllLocalNodes)
/******************************************************************************/
{
    // Get the locally owned nodes
    stk::mesh::BucketVector const& tOwnedBuckets = mKrinoWrapper.bulkData()->get_buckets(
        stk::topology::NODE_RANK, mKrinoWrapper.bulkData()->mesh_meta_data().locally_owned_part());
    for (auto&& tBucketPtr : tOwnedBuckets)
    {
        for (auto tNode : *tBucketPtr)
        {
            auto tGID = mKrinoWrapper.bulkData()->identifier(tNode);
            aLocallyOwnedNodes.push_back(tGID);
        }
    }
    // Make sure we are sorted by global node id (may need to revisit this)
    std::sort(aLocallyOwnedNodes.begin(), aLocallyOwnedNodes.end());

    // Get all the local nodes
    stk::mesh::Selector tOwnedAndSharedSelector =
        mKrinoWrapper.bulkData()->mesh_meta_data().locally_owned_part() |
        mKrinoWrapper.bulkData()->mesh_meta_data().globally_shared_part();
    stk::mesh::BucketVector const& tOwnedAndSharedBuckets =
        mKrinoWrapper.bulkData()->get_buckets(stk::topology::NODE_RANK, tOwnedAndSharedSelector);
    for (auto&& tBucketPtr : tOwnedAndSharedBuckets)
    {
        for (auto tNode : *tBucketPtr)
        {
            auto tGID = mKrinoWrapper.bulkData()->identifier(tNode);
            aAllLocalNodes.push_back(tGID);
        }
    }
    std::sort(aAllLocalNodes.begin(), aAllLocalNodes.end());
}

/******************************************************************************/
void PlatoKrinoApp::buildParallelMaps()
/******************************************************************************/
{
    MPI_Comm tComm;
    mInterface->getLocalComm(tComm);

    std::vector<int> tAllLocalNodes;  // global ids of all nodes on this rank
    buildParallelNodeMaps(mLocallyOwnedKrinoNodes, tAllLocalNodes);

    // build communicator
    mEpetraComm = std::make_unique<Epetra_MpiComm>(tComm);

    // build maps
    const int tBaseIndex = 0;
    mOwnedAndSharedNodeMap =
        std::make_unique<Epetra_Map>(-1, tAllLocalNodes.size(), tAllLocalNodes.data(), tBaseIndex, *mEpetraComm);
    mOwnedNodeMap = std::make_unique<Epetra_Map>(-1, mLocallyOwnedKrinoNodes.size(), mLocallyOwnedKrinoNodes.data(),
                                                 tBaseIndex, *mEpetraComm);

    // build vectors
    mExporter = std::make_unique<Epetra_Export>(*mOwnedAndSharedNodeMap, *mOwnedNodeMap);
    mOwnedAndSharedVec = std::make_unique<Epetra_Vector>(*mOwnedAndSharedNodeMap);
    mOwnedVec = std::make_unique<Epetra_Vector>(*mOwnedNodeMap);
}
/******************************************************************************/
void PlatoKrinoApp::parallelFieldFromLocallyOwnedToLocal(const std::vector<double>& aLocallyOwnedField,
                                                         std::vector<double>& aLocalField)
/******************************************************************************/
{
    // copy locally owned data
    double* tOwnedData;
    mOwnedVec->ExtractView(&tOwnedData);
    std::copy(aLocallyOwnedField.begin(), aLocallyOwnedField.end(), tOwnedData);

    // project to local data
    mOwnedAndSharedVec->PutScalar(0.);
    mOwnedAndSharedVec->Import(*mOwnedVec, *mExporter, Insert);

    // copy local data
    const int tAllLength = mOwnedAndSharedVec->MyLength();
    double* tAllData;
    mOwnedAndSharedVec->ExtractView(&tAllData);
    aLocalField.assign(tAllData, tAllData + tAllLength);
}

/******************************************************************************/
void PlatoKrinoApp::parallelFieldFromLocalToLocallyOwned(const std::vector<double>& aLocalField,
                                                         std::vector<double>& aLocallyOwnedField)
/******************************************************************************/
{
    // copy local data
    double* tLocalData;
    mOwnedAndSharedVec->ExtractView(&tLocalData);
    std::copy(aLocalField.begin(), aLocalField.end(), tLocalData);

    // project to owned data
    mOwnedVec->PutScalar(0.);
    mOwnedVec->Export(*mOwnedAndSharedVec, *mExporter, Insert);

    // copy locally owned data
    const int tOwnedLength = mOwnedVec->MyLength();
    double* tOwnedData;
    mOwnedVec->ExtractView(&tOwnedData);
    aLocallyOwnedField.assign(tOwnedData, tOwnedData + tOwnedLength);
}

bool PlatoKrinoApp::doesDoubleVectorExist(const std::string& aName)
{
    std::map<std::string, std::vector<double> >::iterator map_iter = mDoubleVectorMap.find(aName);
    const bool is_found = (map_iter != mDoubleVectorMap.end());
    return is_found;
}

int PlatoKrinoApp::getDoubleVectorSize(const std::string& aName)
{
    auto map_iter = mDoubleVectorMap.find(aName);
    if (map_iter == mDoubleVectorMap.end())
    {
        std::stringstream tError;
        tError << std::endl
               << "ERROR: PlatoKrinoApp::getDoubleVectorSize: unknown vector '" << aName
               << "'. Try calling doesDoubleVectorExist(...) first. Aborting." << std::endl;
        Plato::ParsingException tParsingException(tError.str());
        throw tParsingException;
    }
    else
    {
        return map_iter->second.size();
    }
}

std::vector<double> PlatoKrinoApp::getDoubleVector(const std::string& aName)
{
    auto map_iter = mDoubleVectorMap.find(aName);
    if (map_iter == mDoubleVectorMap.end())
    {
        std::stringstream tError;
        tError << std::endl
               << "ERROR: PlatoKrinoApp::getDoubleVector: unknown vector '" << aName
               << "'. Try calling doesDoubleVectorExist(...) first. Aborting." << std::endl;
        Plato::ParsingException tParsingException(tError.str());
        throw tParsingException;
    }
    return map_iter->second;
}

void PlatoKrinoApp::setDoubleVector(const std::string& aName, std::vector<double> aVector)
{
    mDoubleVectorMap[aName] = std::move(aVector);
}

void PlatoKrinoApp::createAndWriteBoundingBoxMesh(const stk::math::Vector3d& aMinCorner,
                                                  const stk::math::Vector3d& aMmaxCorner,
                                                  const double& aMeshSize,
                                                  const std::string& aFilename)
{
    mKrinoWrapper.createBoundingBoxMesh(aMinCorner, aMmaxCorner, aMeshSize, aFilename);
}
unsigned int PlatoKrinoApp::getNumTetsInNamedBlock(const std::string& aBlockName)
{
   return mKrinoWrapper.getNumTetsInNamedBlock(aBlockName);
}
std::vector<double> PlatoKrinoApp::getLevelsetValues() { return mKrinoWrapper.getLevelsetValues(); }
void PlatoKrinoApp::writeMesh(const std::string& aFilename) { mKrinoWrapper.writeMesh(aFilename); }
void PlatoKrinoApp::resetMesh() { mKrinoWrapper.resetMesh(); }

}  // namespace Plato
