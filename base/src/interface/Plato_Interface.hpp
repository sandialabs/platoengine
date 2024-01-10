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

/*
 * Plato_Interface.hpp
 *
 *  Created on: March 20, 2017
 *
 */

#ifndef SRC_INTERFACE_HPP_
#define SRC_INTERFACE_HPP_

#include <Teuchos_ParameterList.hpp>

#include <mpi.h>
#include <memory>
#include <vector>
#include <string>

#include "Plato_Parser.hpp"
#include "Plato_DataLayer.hpp"
#include "Plato_Exceptions.hpp"
#include "Plato_SharedData.hpp"
#include "Plato_Console.hpp"
#include "Plato_Stage.hpp"

#include "Plato_SerializationHeaders.hpp"
#include "Plato_SerializationLoadSave.hpp"

#include <boost/serialization/unique_ptr.hpp>

namespace Plato
{

class Performer;
class SharedData;
class Application;
struct aSharedDataInfo;
struct CommunicationData;

enum stage_index_t
{
    INVALID_STAGE = -2,
    TERMINATE_STAGE = -1
};

struct PerformerInfo
{
    std::vector<std::string> mNames;
    std::string mCodeName;
    int mId;

    template<typename Archive>
    void serialize(Archive& aArchive, const unsigned int /*aVersion*/)
    {
        aArchive & boost::serialization::make_nvp("Names", mNames);
        aArchive & boost::serialization::make_nvp("Code", mCodeName);
        aArchive & boost::serialization::make_nvp("ID", mId);
    }
};

struct SharedDataSerializedInfo
{
    std::string mName;
    std::string mLayout;
    int mSize;
    bool mIsDynamic;
    std::vector<std::string> mProviderNames;
    std::vector<std::string> mReceiverNames;

    template<typename Archive>
    void serialize(Archive& aArchive, const unsigned int /*aVersion*/)
    {
        aArchive & boost::serialization::make_nvp("Name", mName);
        aArchive & boost::serialization::make_nvp("Layout", mLayout);
        aArchive & boost::serialization::make_nvp("Size", mSize);
        aArchive & boost::serialization::make_nvp("Dynamic", mIsDynamic);
        aArchive & boost::serialization::make_nvp("OwnerName", mProviderNames);
        aArchive & boost::serialization::make_nvp("UserName", mReceiverNames);
    }
};

/**
 * This class provides an interface between PlatoEngine and the
 * hosted codes.
 */

/******************************************************************************/
//!  Interface between driver and Plato
/*!
 This should be separated into two virtual bases.
 */
/******************************************************************************/
class Interface final
{
public:
    explicit Interface(MPI_Comm aGlobalComm = MPI_COMM_WORLD);
    /// This ctor initializes Interface from the saved state in the XML file with name
    /// @a aFileName, and assumes the data with XML tag @a aNodeName.
    Interface(const XMLFileName& aFileName, const XMLNodeName& aNodeName, MPI_Comm aGlobalComm = MPI_COMM_WORLD);

    void registerApplication(Plato::Application* aApplication);

    /// Register @a aApplication and initialize its data layer MPI. The purpose of this function is
    /// similar to registerApplication, however, it should be used when using the serialization 
    /// interface. 
    /// @pre @a aApplication must have been initialized already using the serialization interface.
    void registerApplicationOnlyInitializeMPI(Application* aApplication);

    // driver interface
    void run();
    void perform();
    void compute(const std::string & stageName, Teuchos::ParameterList & aArguments);
    void compute(const std::vector<std::string> & stageNames, Teuchos::ParameterList & aArguments);
    void finalize( std::string aStageName = std::string() );

    // data motion
    int size(const std::string & aName) const;
    void exportData(const double* aFrom, Plato::SharedData& aTo);
    void importData(double* aTo, const Plato::SharedData& aFrom);

    // local communicator functionality
    void getLocalComm(MPI_Comm& aLocalComm);

    // accessors
    Plato::InputData getInputData() const;
    std::string getLocalPerformerName(){return mLocalPerformerName;}

    // settors
    void setDataLayer
    (const Plato::SharedDataInfo & aSharedDataInfo, 
     const Plato::CommunicationData & aCommData);

    // error handling
    void Catch();
    void handleExceptions();
    void registerException();
    void registerException(Plato::ParsingException aParsingException);
    void registerException(Plato::LogicException aLogicException);
    void registerException(Plato::TerminateSignal aTerminateSignal);

    // control
    bool isDone();

    template<class Archive>
    void serialize(Archive & aArchive, const unsigned int /*version*/)
    {
        aArchive & boost::serialization::make_nvp("AllPerformers", mAllPerformersInfo);
        aArchive & boost::serialization::make_nvp("AllSharedData", mAllSharedDataInfo);
        aArchive & boost::serialization::make_nvp("DataLayer", mDataLayer);
        aArchive & boost::serialization::make_nvp("Stages",mStages);
    }

    void createStages();
    void createSharedData(Application* aApplication);
    void initializeConsole();
    void initializeSharedDataMPI(Application* aApplication);
    void initializePerformerMPI();
    void setPerformerOnStages();

    /// This function calls callable @a aF and uses the exception handling in Interface.
    /// Because exception handling is complicated by MPMD, this function helps 
    /// consolidate necessary MPI calls. This is most easily called with a lambda, as in:
    /// @code tryFCatchInterfaceExceptions([this](){createPerformers();});
    template<typename F>
    void tryFCatchInterfaceExceptions(const F& aF);

    bool dataLayerHasParameter(const std::string& aParameterName) const;

    void setParameterInDataLayer(
        const std::string& aParameterName,
        double aValue);

private:
    void perform(Plato::Stage& aStage);
    void broadcastStageIndex(int& aStageIndex);

    Plato::Stage* getStage();
    Plato::Stage* getStage(const std::string& aStageName);
    int getStageIndex(const std::string& aStageName) const;

    void updateStages();
    void createPerformers();
    void reinitializePerformer();

    void exportGraph(const Plato::SharedDataInfo & aSharedDataInfo,
                     Plato::Application* aApplication,
                     Plato::CommunicationData & aCommunicationData) const;
    void exportOwnedGlobalIDs(const Plato::data::layout_t & aLayout,
                              Plato::Application* aApplication,
                              Plato::CommunicationData & aCommunicationData) const;

    void getSharedDataAndCommunicationInfo(Application* aApplication, 
        SharedDataInfo& aSharedDataInfo, 
        CommunicationData& aCommunicationData) const;

    void checkAndSetApplication(Application* aApplication);

    bool parameterExists(const std::string& aParameterName) const;
    void validate();

private:
    // Serializable state
    std::vector<PerformerInfo> mAllPerformersInfo;
    std::vector<SharedDataSerializedInfo> mAllSharedDataInfo;

    // Internal state
    std::unique_ptr<Plato::DataLayer> mDataLayer;

    std::shared_ptr<Plato::Performer> mPerformer;
    std::vector<std::unique_ptr<Plato::Stage>> mStages;

    std::unique_ptr<Plato::ExceptionHandler> mExceptionHandler;

    std::unique_ptr<Plato::Console> mConsole;

    int mLocalCommID = -1;
    int mPerformerID = -1;
    std::string mLocalPerformerName;
    Plato::InputData mInputData{"Input Data"};

    MPI_Comm mLocalComm;
    MPI_Comm mGlobalComm;
    bool mIsDone = false;
};

template<typename F>
void Interface::tryFCatchInterfaceExceptions(const F& aF)
{
    try{
        aF();
    } catch(...){
        Catch();
    }
    handleExceptions();
}


} /* namespace Plato */

#endif /* SRC_INTERFACE_HPP_ */
