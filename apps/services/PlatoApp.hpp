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

#pragma once

#include "lightmp.hpp"

#include "Plato_LocalOperation.hpp"
#include "Plato_Application.hpp"
#include "Plato_Exceptions.hpp"
#include "Plato_TimersTree.hpp"
#include <Plato_FreeFunctions.hpp>
#include "Plato_SerializationHeaders.hpp"

namespace pugi
{

class xml_document;

}

namespace Plato
{

class SharedData;
class AbstractFilter;

}

/******************************************************************************//**
 * @brief PLATO Application
**********************************************************************************/
class PlatoApp : public Plato::Application
{
public:
    PlatoApp() = default;
    /******************************************************************************//**
     * @brief Constructor
     * @param [in] aLocalComm local communicator
    **********************************************************************************/
    PlatoApp(MPI_Comm& aLocalComm);

    /******************************************************************************//**
     * @brief Constructor
     * @param [in] aArgc input arguments
     * @param [in] aArgv input arguments
     * @param [in] aLocalComm local communicator
    **********************************************************************************/
    PlatoApp(int aArgc, char **aArgv, MPI_Comm& aLocalComm);

    /******************************************************************************//**
     * @brief Constructor
     * @param [in] aPhysics_XML_File interface input file
     * @param [in] aApp_XML_File PLATO application input file
     * @param [in] aLocalComm local communicator
    **********************************************************************************/
    PlatoApp(const std::string &aPhysics_XML_File, const std::string &aApp_XML_File, MPI_Comm& aLocalComm);

    /******************************************************************************//**
     * @brief Destructor
    **********************************************************************************/
    virtual ~PlatoApp();

    /******************************************************************************//**
     * @brief Safely deallocate local memory
    **********************************************************************************/
    void finalize() override;

    /******************************************************************************//**
     * @brief Safely allocate local memory
    **********************************************************************************/
    void initialize() override;

    /******************************************************************************//**
     * @brief reinitialize
    **********************************************************************************/
    void reinitialize() override;

    /******************************************************************************//**
     * @brief Perform local operation
     * @param [in] aOperationName local operation name
    **********************************************************************************/
    void compute(const std::string & aOperationName) override;

    /******************************************************************************//**
     * @brief Import data
     * @param [in] aArgumentName argument name used to identify import data
     * @param [in] aImportData data
    **********************************************************************************/
    void importData(const std::string & aArgumentName, const Plato::SharedData & aImportData) override;

    /******************************************************************************//**
     * @brief Export local data
     * @param [in] aArgumentName argument name used to identify export data
     * @param [in] aImportData data
    **********************************************************************************/
    void exportData(const std::string & aArgumentName, Plato::SharedData & aImportData) override;

    /******************************************************************************//**
     * @brief Export parallel graph
     * @param [in] aDataLayout data layout
     * @param [in] aMyOwnedGlobalIDs local rank owned identifiers
    **********************************************************************************/
    void exportDataMap(const Plato::data::layout_t & aDataLayout, std::vector<int> & aMyOwnedGlobalIDs) override;

    /******************************************************************************//**
     * @brief Return pointer to application-specific services
     * @return pointer to application-specific services
    **********************************************************************************/
    LightMP* getLightMP();

    /******************************************************************************//**
     * @brief Return reference to local communicator
     * @return reference to local communicator
    **********************************************************************************/
    const MPI_Comm& getComm() const;

    /******************************************************************************//**
     * @brief Return pointer to local filter operator
     * @return pointer to local filter operator
    **********************************************************************************/
    Plato::AbstractFilter* getFilter();

    /******************************************************************************//**
     * @brief Return pointer to parallel graph
     * @return pointer to parallel graph
    **********************************************************************************/
    SystemContainer* getSysGraph();

    /******************************************************************************//**
     * @brief Return pointer to local mesh services
     * @return pointer to local mesh services
    **********************************************************************************/
    MeshServices* getMeshServices();

    /******************************************************************************//**
     * @brief Return pointer to timer services
     * @return pointer to timer services
    **********************************************************************************/
    Plato::TimersTree* getTimersTree();

    /******************************************************************************//**
     * @brief Return shared data name
     * @param [in] aName argument name
     * @return shared data name
    **********************************************************************************/
    std::string getSharedDataName(const std::string & aName) const;

    /******************************************************************************//**
     * @brief Return local number of elements
     * @param [in] aName field name
     * @return local number of elements
    **********************************************************************************/
    size_t getLocalNumElements() const;

    /******************************************************************************//**
     * @brief Return element field identifier
     * @param [in] aName argument/field name
     * @return element field identifier
    **********************************************************************************/
    VarIndex getElementField(const std::string & aName);

    /******************************************************************************//**
     * @brief Return pointer to element field data
     * @param [in] aName argument/field name
     * @return pointer to element field data
    **********************************************************************************/
    double* getElementFieldData(const std::string & aName);

    /******************************************************************************//**
     * @brief Return local number of node field elements
     * @param [in] aName argument/field name
     * @return local number of node field elements
    **********************************************************************************/
    size_t getNodeFieldLength(const std::string & aName);

    /******************************************************************************//**
     * @brief Return pointer to node field data
     * @param [in] aName argument/field name
     * @return pointer to node field data
    **********************************************************************************/
    double* getNodeFieldData(const std::string & aName);

    /******************************************************************************//**
     * @brief Return distributed node field
     * @param [in] aName field name
     * @return pointer to node field
    **********************************************************************************/
    DistributedVector* getNodeField(const std::string & aName);

    /******************************************************************************//**
     * @brief Return scalar values
     * @param [in] aName quantity name
     * @return pointer to array of values
    **********************************************************************************/
    std::vector<double>* getValue(const std::string & aName);

    /******************************************************************************//**
     * @brief Combines values from all processes and distributes the result back to all processes
     * @param [in] aLocalValue local process value
     * @param [in] aGlobalValue global, i.e. reduced, value
    **********************************************************************************/
    void reduceScalarValue(const double& aLocalValue, double& aGlobalValue);

    /******************************************************************************//**
     * @brief Communicates boundary data to the owner process and update values in ghost indices
     * @param [in] aName argument name
    **********************************************************************************/
    void compressAndUpdateNodeField(const std::string & aName);

    /******************************************************************************//**
     * @brief Import data operation
     * @param [in] aArgumentName name used to identify data
     * @param [in] aImportData data
    **********************************************************************************/
    template<typename SharedDataT>
    void importDataT(const std::string& aArgumentName, const SharedDataT& aImportData)
    {
        if(aImportData.myLayout() == Plato::data::layout_t::SCALAR_FIELD)
        {
            mSharedDataNames[aArgumentName] = aImportData.myName();

            DistributedVector* tLocalData = getNodeField(aArgumentName);

            int tMyLength = tLocalData->getEpetraVector()->MyLength();
            assert(tMyLength == aImportData.size());
            std::vector<double> tImportData(tMyLength);
            aImportData.getData(tImportData);

            double* tDataView;
            tLocalData->getEpetraVector()->ExtractView(&tDataView);
            std::copy(tImportData.begin(), tImportData.end(), tDataView);

            tLocalData->Import();
            tLocalData->DisAssemble();
        }
        else if(aImportData.myLayout() == Plato::data::layout_t::ELEMENT_FIELD)
        {
            mSharedDataNames[aArgumentName] = aImportData.myName();

            auto dataContainer = mLightMp->getDataContainer();
            double* tDataView;
            dataContainer->getVariable(getElementField(aArgumentName), tDataView);
            int tMyLength = mLightMp->getMesh()->getNumElems();

            assert(tMyLength == aImportData.size());

            std::vector<double> tImportData(tMyLength);
            aImportData.getData(tImportData);

            std::copy(tImportData.begin(), tImportData.end(), tDataView);
        }
        else if(aImportData.myLayout() == Plato::data::layout_t::SCALAR)
        {
            mSharedDataNames[aArgumentName] = aImportData.myName();

            std::vector<double>* tLocalData = getValue(aArgumentName);
            tLocalData->resize(aImportData.size());
            aImportData.getData(*tLocalData);
        }
    }

    /******************************************************************************//**
     * @brief Export local data operation
     * @param [in] aArgumentName name used to identify data
     * @param [in/out] aExportData data
    **********************************************************************************/
    template<typename SharedDataT>
    void exportDataT(const std::string& aArgumentName, SharedDataT& aExportData)
    {
        if(aExportData.myLayout() == Plato::data::layout_t::SCALAR_FIELD)
        {
            mSharedDataNames[aArgumentName] = aExportData.myName();

            DistributedVector* tLocalData = getNodeField(aArgumentName);

            tLocalData->LocalExport();
            double* tDataView;
            tLocalData->getEpetraVector()->ExtractView(&tDataView);

            int tMyLength = tLocalData->getEpetraVector()->MyLength();
            assert(tMyLength == aExportData.size());
            std::vector<double> tExportData(tMyLength);
            std::copy(tDataView, tDataView + tMyLength, tExportData.begin());

            aExportData.setData(tExportData);
        }
        else if(aExportData.myLayout() == Plato::data::layout_t::ELEMENT_FIELD)
        {
            mSharedDataNames[aArgumentName] = aExportData.myName();

            auto dataContainer = mLightMp->getDataContainer();
            double* tDataView;
            dataContainer->getVariable(getElementField(aArgumentName), tDataView);
            int tMyLength = mLightMp->getMesh()->getNumElems();

            assert(tMyLength == aExportData.size());
            std::vector<double> tExportData(tMyLength);
            std::copy(tDataView, tDataView + tMyLength, tExportData.begin());

            aExportData.setData(tExportData);
        }
        else if(aExportData.myLayout() == Plato::data::layout_t::SCALAR)
        {
            mSharedDataNames[aArgumentName] = aExportData.myName();

            std::vector<double>* tLocalData = getValue(aArgumentName);
            if(aExportData.isDynamic())
            {
                aExportData.setData(*tLocalData);
            }
            else if(int(tLocalData->size()) == aExportData.size())
            {
                aExportData.setData(*tLocalData);
            }
            else if(tLocalData->size() == 1u)
            {
                std::vector<double> retVec(aExportData.size(), (*tLocalData)[0]);
                aExportData.setData(retVec);
            }
            else
            {
                throw Plato::ParsingException("SharedValued length mismatch.");
            }
        }
    }

    template<class Archive>
    void serialize(Archive & aArchive, const unsigned int /*version*/)
    {
        aArchive & boost::serialization::make_nvp("Application", boost::serialization::base_object<Application>(*this));
        aArchive & boost::serialization::make_nvp("OperationMap", mOperationMap);
    }
    
private:
    /******************************************************************************//**
     * @brief Parsing exception handler
     * @param [in] aName input data name
     * @param [in] aValueMap name-data map
    **********************************************************************************/
    template<typename ValueType>
    void throwParsingException(const std::string & aName, const std::map<std::string, ValueType> & aValueMap) const
    {
        std::stringstream tMessage;
        tMessage << "Cannot find specified Argument: " << aName.c_str() << std::endl;
        tMessage << "Available Arguments: " << std::endl;
        for(auto tIterator : aValueMap)
        {
            tMessage << tIterator.first << std::endl;
        }
        Plato::ParsingException tParsingException(tMessage.str());
        throw tParsingException;
    }


    /******************************************************************************//**
     * @brief Create local operation
     * @param [in] aOperation local operation
    **********************************************************************************/
    void createLocalData(Plato::LocalOp* aOperation);

    /******************************************************************************//**
     * @brief Create local arguments
     * @param [in] aArguments arguments associated with an operation
    **********************************************************************************/
    void createLocalData(Plato::LocalArg aArguments);

private:

    void deleteData( bool deleteTimers );
    void initialize( bool initializeTimers );

    MPI_Comm mLocalComm; /*!< local communicator */
    LightMP* mLightMp = nullptr; /*!< application-specific services */
    SystemContainer* mSysGraph = nullptr; /*!< parallel graph services */
    MeshServices* mMeshServices = nullptr; /*!< mesh services */
    Plato::AbstractFilter* mFilter = nullptr; /*!< filter services */
    Plato::InputData mAppfileData{"Appfile Data"}; /*!< PLATO application input data */
    Plato::InputData mInputfileData{"Inputfile Data"}; /*!< Shared input data */
    std::shared_ptr<pugi::xml_document> mInputTree = nullptr; /*!< Original input tree */

    std::map<std::string, std::string> mSharedDataNames; /*!< Argument name -> SharedData name */
    std::map<std::string, VarIndex> mElementFieldMap; /*!< Name - Element Field map */
    std::map<std::string, DistributedVector*> mNodeFieldMap; /*!< Name - Node Field map */
    std::map<std::string, std::vector<double>*> mValueMap; /*!< Name - Scalar values map */
    std::map<std::string, Plato::LocalOp*> mOperationMap; /*!< Name - Operation map */

    Plato::TimersTree* mTimersTree = nullptr; /*!< timer tools/services */

};
// class PlatoApp
