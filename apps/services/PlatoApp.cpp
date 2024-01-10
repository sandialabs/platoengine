/*
 //@HEADER
 // *************************************************************************
 //   Plato Engine v.1.0: Copyright 2018, National Technology & Engineering
 //                    Solutions of Sandia, LLC (NTESS).
 //
 // Under the terms of Contract DE-NA0003525 with NTESS,
 // the U.S. Government retains certain rights in this software.
 //
 // Redistribution and use in source and binary forms, with or without
 // modification, are permitted provided that the following conditions are
 // met:
 //
 // 1. Redistributions of source code must retain the above copyright
 // notice, this list of conditions and the following disclaimer.
 //
 // 2. Redistributions in binary form must reproduce the above copyright
 // notice, this list of conditions and the following disclaimer in the
 // documentation and/or other materials provided with the distribution.
 //
 // 3. Neither the name of the Sandia Corporation nor the names of the
 // contributors may be used to endorse or promote products derived from
 // this software without specific prior written permission.
 //
 // THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
 // EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 // IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 // PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
 // CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 // EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 // PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 // PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 // LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 // NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 // SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 //
 // Questions? Contact the Plato team (plato3D-help@sandia.gov)
 //
 // *************************************************************************
 //@HEADER
 */

#include <sstream>
#include <fstream>
#include <iomanip>

#include "PlatoApp.hpp"
#include "Plato_Parser.hpp"
#include "Plato_Exceptions.hpp"
#include "Plato_SharedData.hpp"
#include "Plato_PenaltyModel.hpp"
#include "PlatoEngine_FilterFactory.hpp"
#include "PlatoEngine_AbstractFilter.hpp"
#include "Plato_TimersTree.hpp"
#include "data_container.hpp"
#include "lightmp.hpp"
#include "types.hpp"
#include "matrix_container.hpp"

#include "Plato_Operations_incl.hpp"

PlatoApp::PlatoApp(MPI_Comm& aLocalComm) :
        mLocalComm(aLocalComm)
{
    WorldComm.init(aLocalComm);
}

PlatoApp::PlatoApp(int aArgc, char **aArgv, MPI_Comm& aLocalComm) :
        mLocalComm(aLocalComm),
        mAppfileData(Plato::inputDataFromPugiParsedFile(getenv("PLATO_APP_FILE"))),
        mInputfileData(Plato::inputDataFromPugiParsedFile(aArgc == 2 ? aArgv[1] : "platomain.xml"))
{
    WorldComm.init(aLocalComm);

    // create the FEM utility object
    auto tMeshSpec = mInputfileData.getByName<Plato::InputData>("mesh");
    if (tMeshSpec.size() != 0)
    {
        const std::string tInputFile{aArgc == 2 ? aArgv[1] : "platomain.xml"};
        mLightMp = new LightMP(tInputFile);
        mInputTree = mLightMp->getInput();
    }
}

PlatoApp::PlatoApp(const std::string &aPhysics_XML_File, const std::string &/*aApp_XML_File*/, MPI_Comm& aLocalComm) :
        mLocalComm(aLocalComm),
        mAppfileData(Plato::inputDataFromPugiParsedFile(getenv("PLATO_APP_FILE"))),
        mInputfileData(Plato::inputDataFromPugiParsedFile(aPhysics_XML_File))
{
    WorldComm.init(aLocalComm);

    std::shared_ptr<pugi::xml_document> tTempDoc = std::make_shared<pugi::xml_document>();
    tTempDoc->load_string(aPhysics_XML_File.c_str());

    mLightMp = new LightMP(tTempDoc);
}

PlatoApp::~PlatoApp()
{
    deleteData( true );
}

void PlatoApp::deleteData( bool deleteTimers )
{
    if(mLightMp)
    {
        delete mLightMp;
        mLightMp = nullptr;
    }

    if(mSysGraph)
    {
        delete mSysGraph;
        mSysGraph = nullptr;
    }

    if(mMeshServices)
    {
        delete mMeshServices;
        mMeshServices = nullptr;
    }

    for(auto& tMyValue : mValueMap)
    {
        delete tMyValue.second;
    }
    mValueMap.clear();

    for(auto& tMyField : mNodeFieldMap)
    {
        delete tMyField.second;
    }
    mNodeFieldMap.clear();

    for(auto& tMyOperation : mOperationMap)
    {
        delete tMyOperation.second;
    }
    mOperationMap.clear();

    if(mFilter)
    {
        delete mFilter;
        mFilter = nullptr;
    }

    if(deleteTimers && mTimersTree)
    {
        mTimersTree->print_results();
        delete mTimersTree;
        mTimersTree = nullptr;
    }
}

void PlatoApp::finalize()
{
}

void PlatoApp::initialize()
{
    this->initialize( true );
}

void PlatoApp::reinitialize()
{
    this->deleteData( false );
    this->initialize( false );
}

void PlatoApp::initialize( bool initializeTimers )
{
    // Conditionally begin the timers
    if(initializeTimers && mAppfileData.size<Plato::InputData>("Timers"))
    {
        auto tTimersNode = mAppfileData.get<Plato::InputData>("Timers");
        if(tTimersNode.size<std::string>("time") > 0)
        {
            const bool do_time = Plato::Get::Bool(tTimersNode, "time");
            if(do_time)
            {
                mTimersTree = new Plato::TimersTree(mLocalComm);
            }
        }
    }

    if( mInputTree != nullptr ) {
        if (mLightMp != nullptr) {
            delete mLightMp;
        }
        mLightMp = new LightMP(mInputTree);
    }

    // Define system graph and mesh services (e.g. output) for
    // problems with shared data fields (mesh-based fields)
    const int tDofsPerNode = 1;
    if(mLightMp != nullptr)
    {
        if(mSysGraph)
        {
            delete mSysGraph;
        }

        mSysGraph = new SystemContainer(mLightMp->getMesh(), tDofsPerNode);

        if(mMeshServices)
        {
            delete mMeshServices;
        }

        mMeshServices = new MeshServices(mLightMp->getMesh());
    }

    // If PlatoApp operations file is defined, parse/create requested operations
    if(!mAppfileData.empty())
    {
        for(auto tNode : mAppfileData.getByName<Plato::InputData>("Operation"))
        {
            std::string tStrName = Plato::Get::String(tNode, "Name");
            std::string tStrFunction = Plato::Get::String(tNode, "Function");

            std::vector<std::string> tFunctions;

            tFunctions.push_back("NormalizeObjectiveValue");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::NormalizeObjectiveValue(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("NormalizeObjectiveGradient");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::NormalizeObjectiveGradient(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("WriteGlobalValue");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::WriteGlobalValue(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("HarvestDataFromFile");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::HarvestDataFromFile(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("ReciprocateObjectiveValue");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::ReciprocateObjectiveValue(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("ReciprocateObjectiveGradient");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::ReciprocateObjectiveGradient(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("Aggregator");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::Aggregator(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("CopyField");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::CopyField(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("ChainRule");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::ChainRule(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("CopyValue");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::CopyValue(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("SystemCall");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::SystemCallOperation(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("SystemCallMPI");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::SystemCallMPIOperation(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("EnforceBounds");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::EnforceBounds(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("PlatoMainOutput");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::PlatoMainOutput(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("CSMMeshOutput");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::CSMMeshOutput(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("OutputNodalFieldSharedData");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::OutputNodalFieldSharedData(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("CSMParameterOutput");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::CSMParameterOutput(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("SetLowerBounds");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::SetLowerBounds(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("SetUpperBounds");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::SetUpperBounds(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("Filter");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::Filter(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("ComputeRoughness");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::Roughness(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("InitializeValues");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::InitializeValues(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("ComputeVolume");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::ComputeVolume(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("DesignVolume");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::DesignVolume(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("InitializeField");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::InitializeField(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("Update Problem");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::UpdateProblem(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("MeanPlusStdDev");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::MeanPlusVarianceMeasure(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("MeanPlusStdDevGradient");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::MeanPlusVarianceGradient(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            tFunctions.push_back("Reinitialize");
            if(tStrFunction == tFunctions.back())
            {
                mOperationMap[tStrName] = new Plato::Reinitialize(this, tNode);
                this->createLocalData(mOperationMap[tStrName]);
                continue;
            }

            std::stringstream tMessage;
            tMessage << "Cannot find specified Function: " << tStrFunction << std::endl;
            tMessage << "Available Functions: " << std::endl;
            for(auto tMyFunction : tFunctions)
            {
                tMessage << tMyFunction << std::endl;
            }
            Plato::ParsingException tParsingException(tMessage.str());
            throw tParsingException;
        } // Loop over XML nodes
    } // Check f operation file is defined

    // If mesh services were requested by the user, finalize mesh services setup.
    if(mLightMp != nullptr)
    {
        mLightMp->setWriteTimeStepDuringSetup(false);
        mLightMp->finalizeSetup();
    }
}

void PlatoApp::compute(const std::string & aOperationName)
{
    auto it = mOperationMap.find(aOperationName);
    if(it == mOperationMap.end())
    {
        std::stringstream ss;
        ss << "Request for operation ('" << aOperationName << "') that doesn't exist.";
        throw Plato::LogicException(ss.str());
    }

    (*mOperationMap[aOperationName])();
}

void PlatoApp::importData(const std::string & aArgumentName, const Plato::SharedData & aImportData)
{
    this->importDataT(aArgumentName, aImportData);
}

void PlatoApp::exportData(const std::string & aArgumentName, Plato::SharedData & aExportData)
{
    this->exportDataT(aArgumentName, aExportData);
}

void PlatoApp::exportDataMap(const Plato::data::layout_t & aDataLayout, std::vector<int> & aMyOwnedGlobalIDs)
{
    aMyOwnedGlobalIDs.clear();

    if(aDataLayout == Plato::data::layout_t::VECTOR_FIELD)
    {
        // Plato::data::layout_t = VECTOR_FIELD
        int tMyNumElements = mSysGraph->getRowMap()->NumMyElements();
        aMyOwnedGlobalIDs.resize(tMyNumElements);
        mSysGraph->getRowMap()->MyGlobalElements(aMyOwnedGlobalIDs.data());
    }
    else if(aDataLayout == Plato::data::layout_t::SCALAR_FIELD)
    {
        // Plato::data::layout_t = SCALAR_FIELD
        int tMyNumElements = mSysGraph->getNodeRowMap()->NumMyElements();
        aMyOwnedGlobalIDs.resize(tMyNumElements);
        mSysGraph->getNodeRowMap()->MyGlobalElements(aMyOwnedGlobalIDs.data());
    }
    else if(aDataLayout == Plato::data::layout_t::ELEMENT_FIELD)
    {
        if(mLightMp == nullptr)
        {
            aMyOwnedGlobalIDs.resize(0);
        }
        else
        {
            int tMyNumElements = mLightMp->getMesh()->getNumElems();
            int* tElemGlobalIds = mLightMp->getMesh()->elemGlobalIds;
            aMyOwnedGlobalIDs.resize(tMyNumElements);
            std::copy(tElemGlobalIds, tElemGlobalIds + tMyNumElements, aMyOwnedGlobalIDs.begin());
        }
    }
    else
    {
        throw ParsingException("PlatoApp::exportDataMap: Unknown Data Layout requested.");
    }
}

void PlatoApp::createLocalData(Plato::LocalOp* aLocalOperation)
{
    std::vector<Plato::LocalArg> tLocalArgs;
    aLocalOperation->getArguments(tLocalArgs);
    for(auto tArgument : tLocalArgs)
    {
        createLocalData(tArgument);
    }
}

void PlatoApp::createLocalData(Plato::LocalArg aLocalArguments)
{
    if(aLocalArguments.mLayout == Plato::data::layout_t::SCALAR_FIELD)
    {
        auto tIterator = mNodeFieldMap.find(aLocalArguments.mName);
        if(tIterator != mNodeFieldMap.end())
        {
            return;
        }
        if(mLightMp == nullptr)
        {
            throw ParsingException("PlatoApp::createLocalData: SCALAR_FIELD requested but no mesh defined.");
        }
        std::vector<VarIndex> tNewData(1);
        tNewData[0] =
                mLightMp->getDataContainer()->registerVariable(RealType, aLocalArguments.mName, NODE, aLocalArguments.mWrite);
        mNodeFieldMap[aLocalArguments.mName] = new DistributedVector(mSysGraph, tNewData);
    }
    else if(aLocalArguments.mLayout == Plato::data::layout_t::ELEMENT_FIELD)
    {
        auto tIterator = mElementFieldMap.find(aLocalArguments.mName);
        if(tIterator != mElementFieldMap.end())
        {
            return;
        }
        if(mLightMp == nullptr)
        {
            throw ParsingException("PlatoApp::createLocalData: ELEMENT_FIELD requested but no mesh defined.");
        }
        VarIndex tNewDataIndex;
        tNewDataIndex =
                mLightMp->getDataContainer()->registerVariable(RealType, aLocalArguments.mName, ELEM, aLocalArguments.mWrite);
        mElementFieldMap[aLocalArguments.mName] = tNewDataIndex;
    }
    else if(aLocalArguments.mLayout == Plato::data::layout_t::SCALAR)
    {
        auto tIterator = mValueMap.find(aLocalArguments.mName);
        if(tIterator != mValueMap.end())
        {
            return;
        }
        std::vector<double>* tNewData = new std::vector<double>(aLocalArguments.mLength);
        mValueMap[aLocalArguments.mName] = tNewData;
    }
}

void PlatoApp::reduceScalarValue(const double& aLocalValue, double& aGlobalValue)
{
    MPI_Allreduce(&aLocalValue, &aGlobalValue, 1, MPI_DOUBLE, MPI_MAX, mLocalComm);
}

void PlatoApp::compressAndUpdateNodeField(const std::string & aName)
{
    DistributedVector* tLocalData = this->getNodeField(aName);
    tLocalData->Import();
    tLocalData->DisAssemble();
}

std::vector<double>* PlatoApp::getValue(const std::string & aName)
{
    auto tIterator = mValueMap.find(aName);
    if(tIterator == mValueMap.end())
    {
        throwParsingException(aName, mValueMap);
    }
    return tIterator->second;
}

DistributedVector* PlatoApp::getNodeField(const std::string & aName)
{
    auto tIterator = mNodeFieldMap.find(aName);
    if(tIterator == mNodeFieldMap.end())
    {
        throwParsingException(aName, mNodeFieldMap);
    }
    return tIterator->second;
}

std::string PlatoApp::getSharedDataName(const std::string & aName) const
{
    std::string tRetVal;
    auto tIterator = mSharedDataNames.find(aName);
    if(tIterator != mSharedDataNames.end())
    {
        tRetVal = tIterator->second;
    }
    return tRetVal;
}

size_t PlatoApp::getLocalNumElements() const
{
    return mLightMp != nullptr ? mLightMp->getMesh()->getNumElems() : 0;
}

VarIndex PlatoApp::getElementField(const std::string & aName)
{
    auto tIterator = mElementFieldMap.find(aName);
    if(tIterator == mElementFieldMap.end())
    {
        throwParsingException(aName, mElementFieldMap);
    }
    return (tIterator->second);
}

double* PlatoApp::getElementFieldData(const std::string & aName)
{
    if(mLightMp == nullptr)
    {
        return nullptr;
    }
    double* tOutputDataView;
    auto tFieldIndex = this->getElementField(aName);
    auto tDataContainer = mLightMp->getDataContainer();
    tDataContainer->getVariable(tFieldIndex, tOutputDataView);
    return (tOutputDataView);
}

size_t PlatoApp::getNodeFieldLength(const std::string & aName)
{
    DistributedVector* tOutputField = this->getNodeField(aName);
    size_t tLength = tOutputField->MyLength();
    return (tLength);
}

double* PlatoApp::getNodeFieldData(const std::string & aName)
{
    double* tOutputData;
    DistributedVector* tLocalData = this->getNodeField(aName);
    tLocalData->ExtractView(&tOutputData);
    return (tOutputData);
}

LightMP* PlatoApp::getLightMP()
{
    return mLightMp;
}

const MPI_Comm& PlatoApp::getComm() const
{
    return (mLocalComm);
}

Plato::AbstractFilter* PlatoApp::getFilter()
{
    if(!mFilter)
    {
        if(mLightMp == nullptr)
        {
            return nullptr;
        }

        if(mTimersTree)
        {
            mTimersTree->begin_partition(Plato::timer_partition_t::timer_partition_t::filter);
        }

        mFilter = Plato::build_filter(mAppfileData, mLocalComm, mLightMp->getMesh());
        if(mTimersTree)
        {
            mTimersTree->end_partition();
        }
    }

    return mFilter;
}

SystemContainer* PlatoApp::getSysGraph()
{
    return mSysGraph;
}

MeshServices* PlatoApp::getMeshServices()
{
    return mMeshServices;
}

Plato::TimersTree* PlatoApp::getTimersTree()
{
    return mTimersTree;
}
