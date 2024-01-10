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
 * Plato_ReducedObjectiveROL.hpp
 *
 *  Created on: Feb 8, 2018
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cassert>
#include <algorithm>

#include "ROL_Objective.hpp"
#include "ROL_UpdateType.hpp"

#include "Plato_Interface.hpp"
#include "Plato_DistributedVectorROL.hpp"
#include "Plato_OptimizerEngineStageData.hpp"

namespace Plato
{
namespace
{
std::string updateType(const ROL::UpdateType aUpdateType)
{
    switch(aUpdateType)
    {
        case ROL::UpdateType::Initial:
            return "Initial";
        case ROL::UpdateType::Accept:
            return "Accept";
        case ROL::UpdateType::Revert:
            return "Revert";
        case ROL::UpdateType::Trial:
            return "Trial";
        case ROL::UpdateType::Temp:
            return "Temp";
        default:
            return "Unknown";
    }
}
}

/******************************************************************************//**
 * \brief PLATO Engine interface to a ROL reduced objective function
 * \tparam scalar type, e.g. double, float, etc.
**********************************************************************************/
template<typename ScalarType>
class ReducedObjectiveROL : public ROL::Objective<ScalarType>
{
public:
    /******************************************************************************//**
     * \brief Constructor
     * \param [in] aInputData XML input data
     * \param [in] aInterface PLATO Engine interface
     * \pre @a aInterface must not be `nullptr`
    **********************************************************************************/
    ReducedObjectiveROL(const Plato::OptimizerEngineStageData & aInputData, Plato::Interface* aInterface) :
            mInterface(aInterface),
            mEngineInputData(aInputData),
            mControl(numDesignVariables()),
            mGradient(numDesignVariables()),
            mUpdateFrequency(mEngineInputData.getProblemUpdateFrequency()),
            mHessianType(mEngineInputData.getHessianType())
    {
        assert(mInterface != nullptr);
        if(mDebugOutput)
        {
            mDebugLog.open("obj_output.txt", std::ios::out);
        }
    }

    ReducedObjectiveROL(const Plato::ReducedObjectiveROL<ScalarType> & aRhs) = delete;
    Plato::ReducedObjectiveROL<ScalarType> & operator=(const Plato::ReducedObjectiveROL<ScalarType> & aRhs) = delete;
    ReducedObjectiveROL(Plato::ReducedObjectiveROL<ScalarType> && aRhs) = delete;
    Plato::ReducedObjectiveROL<ScalarType> & operator=(Plato::ReducedObjectiveROL<ScalarType> && aRhs) = delete;

    /******************************************************************************//**
     * \brief Updates physics and enables continuation of app-based parameters.
     * \param [in] aControl design variables
     * \param [in] aFlag indicates if vector of design variables was updated
     * \param [in] aIteration outer loop optimization iteration
    **********************************************************************************/
    using ROL::Objective<ScalarType>::update;
    void update(const ROL::Vector<ScalarType> & /*aControl*/, ROL::UpdateType aUpdateType, int aIteration = -1) override
    {
        debugOutput("update() called with aUpdateType: " + updateType(aUpdateType) + " and aIteration: " + std::to_string(aIteration));

        if(aUpdateType != ROL::UpdateType::Accept)
        {
            mStateComputed = false;
        }
        mGradientComputed = false;

        const bool tNewIteration = aIteration != mLastIteration && aIteration != -1;
        if(tNewIteration)
        {
          callOutputStage();
        }

        const bool tUpdateIteration = mUpdateFrequency > 0 && tNewIteration && aIteration > 0 && aIteration % mUpdateFrequency == 0;
        if(tUpdateIteration)
        {
          callUpdateStage();
        }

        mLastIteration = aIteration;
    }

    /******************************************************************************//**
     * \brief Returns current objective value
     * \param [in] aControl design variables
     * \param [in] aTolerance inexactness tolerance
     * \return objective function value
    **********************************************************************************/
    ScalarType value(const ROL::Vector<ScalarType> & aControl, ScalarType & /*aTolerance*/) override
    {
        debugOutput("value() called");
        if(!mStateComputed)
        {
            computeValue(aControl);
        }

        return mObjectiveValue;
    }

    /******************************************************************************//**
     * \brief Returns current gradient value
     * \param [out] aGradient objective function gradient
     * \param [in] aControl design variables
     * \param [in] aTolerance inexactness tolerance
    **********************************************************************************/
    void gradient(ROL::Vector<ScalarType> & aGradient, const ROL::Vector<ScalarType> & aControl, ScalarType & /*aTolerance*/) override
    {
        debugOutput("gradient() called");
        if(!mStateComputed)
        {
            computeValue(aControl);
        }

        if(!mGradientComputed)
        {
            computeGradient(aControl);
        }

        // ********* Set output gradient vector ********* //
        Plato::DistributedVectorROL<ScalarType> & tOutputGradient =
                dynamic_cast<Plato::DistributedVectorROL<ScalarType>&>(aGradient);
        std::copy(mGradient.begin(), mGradient.end(), tOutputGradient.vector().begin());
    }
    /******************************************************************************//**
     * \brief Returns current hessian applied to a vector
     * \param [out] aHessVec objective function Hessian applied to a vector
     * \param [in] aVector design variable direction vector
     * \param [in] aControl design variables
     * \param [in] aTolerance inexactness tolerance
     **********************************************************************************/
    void hessVec(ROL::Vector<ScalarType> & aHessVec, const ROL::Vector<ScalarType> & aVector, const ROL::Vector<ScalarType> & aControl, ScalarType & aTolerance) override
    {
        debugOutput("hessVec() called");
        if(mHessianType == "zero")
        {
            aHessVec.zero(); // Zero
        }
        else if(mHessianType == "finite_difference")
        {
            ROL::Objective<ScalarType>::hessVec(aHessVec,aVector,aControl,aTolerance); // Finite differences
        }
        else
        {
            std::cout << "Error: Unexpected hessian type!" << std::endl;
        }
    }

protected:
    void debugOutput(const std::string& aOutput) const
    {
        if(mDebugOutput)
        {
            mDebugLog << aOutput << std::endl;
        }
    }

private:
    std::size_t numDesignVariables() const
    {
        constexpr size_t tCONTROL_INDEX = 0;
        const std::vector<std::string> tControlNames = mEngineInputData.getControlNames();
        const std::string tMyControlName = tControlNames[tCONTROL_INDEX];
        return mInterface->size(tMyControlName);
    }

    void setViewToControlVector(const ROL::Vector<ScalarType> & aControl)
    {
      const Plato::DistributedVectorROL<ScalarType> & tControl =
              dynamic_cast<const Plato::DistributedVectorROL<ScalarType>&>(aControl);
      const std::vector<ScalarType> & tControlData = tControl.vector();
      assert(tControlData.size() == mControl.size());
      std::copy(tControlData.begin(), tControlData.end(), mControl.begin());
      constexpr size_t tCONTROL_VECTOR_INDEX = 0;
      const std::string tControlName = mEngineInputData.getControlName(tCONTROL_VECTOR_INDEX);
      mParameterList.set(tControlName, mControl.data());
    }

    void computeValue(const ROL::Vector<ScalarType> & aControl)
    {
      debugOutput("  computeValue() called");

      setViewToControlVector(aControl);
        
      mParameterList.set(mEngineInputData.getObjectiveValueOutputName(), &mObjectiveValue);

      mInterface->compute({mEngineInputData.getObjectiveValueStageName()}, mParameterList);

      cacheState();
    }

    void computeGradient(const ROL::Vector<ScalarType> & aControl)
    {
      debugOutput("  computeGradient() called");

      setViewToControlVector(aControl);

      std::fill(mGradient.begin(), mGradient.end(), static_cast<ScalarType>(0));
      mParameterList.set(mEngineInputData.getObjectiveGradientOutputName(), mGradient.data());

      mInterface->compute({mEngineInputData.getObjectiveGradientStageName()}, mParameterList);

      mGradientComputed = true;
    }

    void callOutputStage()
    {
      const std::string tOutputStageName = mEngineInputData.getOutputStageName();
      if(tOutputStageName.empty() == false)
      {
          const std::vector<std::string> tStageNames = {tOutputStageName};
          mInterface->compute(tStageNames, mParameterList);
      }
    }

    void callUpdateStage()
    {
      debugOutput("  callUpdateStage() called");
      std::vector<std::string> tStageNames = mEngineInputData.getUpdateProblemStageNames();
      if(tStageNames.size())
      {
        mInterface->compute(tStageNames, mParameterList);
      }
    }

    void cacheState()
    {
        debugOutput("  cacheState() called");
        std::vector<std::string> tStageNames;
        std::string tCacheStageName = mEngineInputData.getCacheStageName();
        if(tCacheStageName.empty() == false)
        {
            debugOutput("      caching state with stage " + tCacheStageName);
            tStageNames.push_back(tCacheStageName);
            mInterface->compute(tStageNames, mParameterList);
        }

        mStateComputed = true;
    }

protected:
    Interface* interface() const
    {
        return mInterface;
    }

    const OptimizerEngineStageData& engineInputData() const
    {
        return mEngineInputData;
    }

    void unsetComputedStateFlags()
    {
        mStateComputed = false;
        mGradientComputed = false;
    }
private:
    Plato::Interface* mInterface; /*!< PLATO Engine interface */
    Plato::OptimizerEngineStageData mEngineInputData; /*!< XML input data */
    Teuchos::ParameterList mParameterList; /*!< parameter list used in-memory to transfer data through PLATO Engine */

    std::vector<ScalarType> mControl; /*!< design variables */
    std::vector<ScalarType> mGradient; /*!< objective function gradient */
    ScalarType mObjectiveValue = 0;

    int mLastIteration = -1;
    int mUpdateFrequency = 0;

    bool mStateComputed = false;
    bool mGradientComputed = false;
    bool mDebugOutput = false;
    std::string mHessianType;

    mutable std::ofstream mDebugLog;
};
// class ReducedObjectiveROL

}
// namespace Plato
