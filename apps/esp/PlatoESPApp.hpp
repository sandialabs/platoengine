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

#include <mpi.h>

#include "Plato_ESP.hpp"
#include "Plato_LocalOperation.hpp"
#include "Plato_Application.hpp"
#include "Plato_TimersTree.hpp"
#include "Plato_InputData.hpp"

typedef double ScalarType;
typedef std::vector<ScalarType> ScalarVector;
typedef Plato::Geometry::ESP<ScalarType, ScalarVector> ESPType;

namespace Plato
{
class SharedData;
}

/******************************************************************************//**
 * @brief PLATO Application
**********************************************************************************/
class PlatoESPApp : public Plato::Application
{
public:
    /******************************************************************************//**
     * @brief Constructor
     * @param [in] aArgc input arguments
     * @param [in] aArgv input arguments
     * @param [in] aLocalComm local communicator
    **********************************************************************************/
    PlatoESPApp(int aArgc, char **aArgv, MPI_Comm& aLocalComm);

    /******************************************************************************//**
     * @brief Destructor
    **********************************************************************************/
    ~PlatoESPApp() override;

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
     * @brief Return pointer to timer services
     * @return pointer to timer services
    **********************************************************************************/
    Plato::TimersTree* getTimersTree();

    /******************************************************************************//**
     * @brief Return reference to local communicator
     * @return reference to local communicator
    **********************************************************************************/
    const MPI_Comm& getComm() const;

    /******************************************************************************//**
     * @brief Return scalar values
     * @param [in] aName quantity name
     * @return pointer to array of values
    **********************************************************************************/
    const std::vector<ScalarType>& getValue() { return mLocalData; }

    /******************************************************************************//**
     * @brief Import data operation
     * @param [in] aArgumentName name used to identify data
     * @param [in] aImportData data
    **********************************************************************************/
    template<typename SharedDataT>
    void importDataT(const std::string& aArgumentName, const SharedDataT& aImportData)
    {
        if(aImportData.myLayout() == Plato::data::layout_t::SCALAR)
        {
            mParameters.resize(aImportData.size());
            aImportData.getData(mParameters);
        }
        else
        if(aImportData.myLayout() == Plato::data::layout_t::SCALAR_PARAMETER)
        {
            if(aArgumentName == "Parameter Index")
            {
                std::vector<ScalarType> value(aImportData.size());
                aImportData.getData(value);
                mParameterIndex = (int)value[0];
            }
        }
    }

    /******************************************************************************//**
     * @brief Export local data operation
     * @param [in] aArgumentName name used to identify data
     * @param [in/out] aExportData data
    **********************************************************************************/
    template<typename SharedDataT>
    void exportDataT(const std::string& /*aArgumentName*/, SharedDataT& aExportData)
    {
        if(aExportData.myLayout() == Plato::data::layout_t::SCALAR)
        {
            aExportData.setData(mLocalData);
        }
    }

private:
    MPI_Comm mLocalComm;
    Plato::InputData mAppfileData;
    Plato::InputData mInputfileData;

    std::string mModelFileName;
    std::string mTessFileName;

    int mParameterIndex;

    void *mESPInterface;
    void loadESPInterface();
    typedef ESPType* (*create_t)(std::string, std::string, int);
    typedef void (*destroy_t)(ESPType *esp);
    create_t mCreateESP;
    destroy_t mDestroyESP;
    std::shared_ptr<ESPType> mESP;

    std::vector<ScalarType> mParameters;
    std::vector<ScalarType> mPrevParameters;
    std::vector<ScalarType> mLocalData;

    Plato::TimersTree* mTimersTree;

    bool hasChanged();
};
