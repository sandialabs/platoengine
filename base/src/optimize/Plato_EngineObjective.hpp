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
 * Plato_EngineObjective.hpp
 *
 *  Created on: Dec 21, 2017
 */

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <cassert>

#include "Plato_Interface.hpp"
#include "Plato_Criterion.hpp"
#include "Plato_MultiVector.hpp"
#include "Plato_DistributedVector.hpp"
#include "Plato_OptimizerEngineStageData.hpp"
#include "Plato_OptimizerInterface.hpp"

namespace Plato
{

template<typename ScalarType, typename OrdinalType = size_t>
class EngineObjective : public Plato::Criterion<ScalarType, OrdinalType>
{
public:
    /******************************************************************************//**
     * @brief Constructor
     * @param [in] aNumControls Number of control variables
     * @param [in] aInputData PLATO Engine input data
     * @param [in] aInterface PLATO Engine interface
    **********************************************************************************/
    explicit EngineObjective(const OrdinalType aNumControls,
                             const Plato::OptimizerEngineStageData & aInputData,
                             Plato::Interface* aInterface,
                             Plato::OptimizerInterface< ScalarType, OrdinalType > * aOptInterface) :
            mVector(std::vector<ScalarType>(aNumControls)),
            mControl(std::vector<ScalarType>(aNumControls)),
            mGradient(std::vector<ScalarType>(aNumControls)),
            mHessianTimesVector(std::vector<ScalarType>(aNumControls)),
            mInterface(aInterface),
            mEngineInputData(aInputData),
            mParameterList(std::make_shared<Teuchos::ParameterList>())
    {

        // This data is used to manage the serial and nesting
        // optimization when calling the value method.
        if( aOptInterface )
        {
            mOptimizerName  = aOptInterface->getOptimizerName();
            mOptimizerIndex = aOptInterface->getOptimizerIndex();
            mHasInnerLoop   = aOptInterface->getHasInnerLoop();
        }
    }

    /******************************************************************************//**
     * @brief Constructor
     * @param [in] aInputData PLATO Engine input data
     * @param [in] aInterface PLATO Engine interface
    **********************************************************************************/
    explicit EngineObjective(const Plato::OptimizerEngineStageData & aInputData,
                             Plato::Interface* aInterface,
                             Plato::OptimizerInterface< ScalarType, OrdinalType > * aOptInterface) :
            mVector(),
            mControl(),
            mGradient(),
            mHessianTimesVector(),
            mInterface(aInterface),
            mEngineInputData(aInputData),
            mParameterList(std::make_shared<Teuchos::ParameterList>())
    {
        // This data is used to manage the serial and nesting
        // optimization when calling the value method.
        if( aOptInterface )
        {
            mOptimizerName  = aOptInterface->getOptimizerName();
            mOptimizerIndex = aOptInterface->getOptimizerIndex();
            mHasInnerLoop   = aOptInterface->getHasInnerLoop();
        }
    }

    /******************************************************************************//**
     * @brief Destructor
    **********************************************************************************/
    virtual ~EngineObjective()
    {
    }

    /******************************************************************************//**
     * @brief Allocate control containers needed during optimization
     * @param [in] aNumControls number of controls
    **********************************************************************************/
    void allocateControlContainers(const OrdinalType& aNumControls)
    {
        mVector = std::vector<ScalarType>(aNumControls);
        mControl = std::vector<ScalarType>(aNumControls);
        mGradient = std::vector<ScalarType>(aNumControls);
        mHessianTimesVector = std::vector<ScalarType>(aNumControls);
    }

    /******************************************************************************//**
     * @brief Set PLATO Engine interface
     * @param [in] aInterface PLATO Engine interface
    **********************************************************************************/
    void setPlatoInterface(Plato::Interface* aInterface)
    {
        assert(aInterface != nullptr);
        mInterface = aInterface;
    }

    /******************************************************************************//**
     * @brief Directive to third-party applications to cache app-based data
    **********************************************************************************/
    void cacheData() override
    {
        assert(mInterface != nullptr);

        // Tell performers to cache the state
        std::vector<std::string> tStageNames;
        std::string tCacheStageName = mEngineInputData.getCacheStageName();
        if(tCacheStageName.empty() == false)
        {
            tStageNames.push_back(tCacheStageName);
            mInterface->compute(tStageNames, *mParameterList);
        }

        // Temporarily putting output here.
        tStageNames.clear();
        std::string tOutputStageName = mEngineInputData.getOutputStageName();
        if(tOutputStageName.empty() == false)
        {
            tStageNames.push_back(tOutputStageName);
            mInterface->compute(tStageNames, *mParameterList);
        }
    }

    /******************************************************************************//**
     * @brief Enable third-party applications to perform continuation on app-based data
     * @param [in] aControl const reference to 2D container of optimization variables
    **********************************************************************************/
    void updateProblem(const Plato::MultiVector<ScalarType, OrdinalType> & aControl) override
    {
        assert(mInterface != nullptr);

        this->setControls(aControl);

        // Tell performers to cache the state
        std::vector<std::string> tUpdateProblemNames =
            mEngineInputData.getUpdateProblemStageNames();

        mInterface->compute(tUpdateProblemNames, *mParameterList);
    }

    /******************************************************************************//**
     * @brief Evaluate one or multiple third-party application objective functions
     * @param [in] aControl const reference to 2D container of optimization variables
     *
     * With some of the the optimizer interfaces such as the MMA it is
     * possible to have nested optimization. This nesting creates a
     * circular dependency:
     * DriverFactory->MMA->EngineObjective->OptimizerFactory

     * Because of this circular dependency it is necessary to break
     * from the traditional template class definition where everything
     * is defined as part of the class definition in a single header
     * file. Instead the EngineObjective::value method (which calls
     * the OptimizerFactory) is defined SEPARATELY and AFTER the
     * OptimizerFactory class has been defined. As such, the
     * definition of EngineObjective::value is in a separate file,
     * Plato_EngineObjective.tcc and is included in
     * Plato_DriverFactory.hpp
    **********************************************************************************/
    ScalarType value(const Plato::MultiVector<ScalarType, OrdinalType> & aControl) override;

    /******************************************************************************//**
     * @brief Compute the gradient of one or multiple third-party application objectives
     * @param [in] aControl const reference to 2D container of optimization variables
     * @param [in] aOutput reference to 2D container of the gradient with respect to the
     *                     optimization variables
    **********************************************************************************/
    void gradient(const Plato::MultiVector<ScalarType, OrdinalType> & aControl,
                  Plato::MultiVector<ScalarType, OrdinalType> & aOutput) override
    {
        assert(mInterface != nullptr);
        // ********* Set view to each control vector entry ********* //
        this->setControls(aControl);

        // ********* Set view to each output vector ********* //
        std::string tGradientOutputName = mEngineInputData.getObjectiveGradientOutputName();
        std::fill(mGradient.begin(), mGradient.end(), static_cast<ScalarType>(0));
        mParameterList->set(tGradientOutputName, mGradient.data());

        // ********* Compute objective function gradient ********* //
        std::string tMyStageName = mEngineInputData.getObjectiveGradientStageName();
        assert(tMyStageName.empty() == false);
        std::vector<std::string> tStageNames;
        tStageNames.push_back(tMyStageName);
        mInterface->compute(tStageNames, *mParameterList);
        this->copy(mGradient, aOutput);
    }

    /******************************************************************************//**
     * @brief Compute the application of a vector to the Hessian of one or multiple
     *        third-party application objectives
     * @param [in] aControl const reference to 2D container of optimization variables
     * @param [in] aVector const reference to 2D container of descent directions
     * @param [in] aOutput reference to 2D container of the application of a vector
     *                     to the Hessian
    **********************************************************************************/
    void hessian(const Plato::MultiVector<ScalarType, OrdinalType> & aControl,
                 const Plato::MultiVector<ScalarType, OrdinalType> & aVector,
                 Plato::MultiVector<ScalarType, OrdinalType> & aOutput) override
    {
        assert(mInterface != nullptr);
        // ********* Set view to each control and descent direction vector entry ********* //
        const OrdinalType tControlVectorIndex = 0;
        assert(mVector.size() == mControl.size());
        assert(aVector[tControlVectorIndex].size() == mVector.size());
        assert(aControl[tControlVectorIndex].size() == mControl.size());
        for(OrdinalType tControlIndex = 0; tControlIndex < mControl.size(); tControlIndex++)
        {
            mVector[tControlIndex] = aVector(tControlVectorIndex, tControlIndex);
            mControl[tControlIndex] = aControl(tControlVectorIndex, tControlIndex);
        }
        std::string tControlName = mEngineInputData.getControlName(tControlVectorIndex);
        mParameterList->set(tControlName, mControl.data());
        std::string tDescentDirectionlName = mEngineInputData.getDescentDirectionName(tControlVectorIndex);
        mParameterList->set(tDescentDirectionlName, mVector.data());

        // ********* Set view to each output vector ********* //
        std::string tHessianOutputName = mEngineInputData.getObjectiveHessianOutputName();
        std::fill(mHessianTimesVector.begin(), mHessianTimesVector.end(), static_cast<ScalarType>(0));
        mParameterList->set(tHessianOutputName, mHessianTimesVector.data());

        // ********* Apply vector to Hessian operator ********* //
        std::string tMyStageName = mEngineInputData.getObjectiveHessianStageName();
        assert(tMyStageName.empty() == false);
        std::vector<std::string> tStageNames;
        tStageNames.push_back(tMyStageName);
        mInterface->compute(tStageNames, *mParameterList);
        this->copy(mHessianTimesVector, aOutput);
    }

    /******************************************************************************//**
     * @brief set the optimizer name - Optional.
    **********************************************************************************/
    void setOptimizerName( std::string val ) { mOptimizerName = val; }

    /******************************************************************************//**
     * @brief set the index of the optimizer.
    **********************************************************************************/
    void setOptimizerIndex( std::vector<size_t> val ) { mOptimizerIndex = val; }

    /******************************************************************************//**
     * @brief set the inner loop boolean if the optimizer has an inner
     * loop
    **********************************************************************************/
    void setHasInnerLoop( bool val ) { this->mHasInnerLoop = val; }

private:
    /******************************************************************************//**
     * @brief Copy elements in standard vector into a Plato multi-vector
     * @param [in] aFrom input data
     * @param [out] aTo output data
    **********************************************************************************/
    void copy(const std::vector<ScalarType> & aFrom, Plato::MultiVector<ScalarType, OrdinalType> & aTo)
    {
        const OrdinalType tVectorIndex = 0;
        Plato::Vector<ScalarType, OrdinalType> & tToVector = aTo[tVectorIndex];
        assert(tToVector.size() == aFrom.size());
        for(OrdinalType tIndex = 0; tIndex < aFrom.size(); tIndex++)
        {
            tToVector[tIndex] = aFrom[tIndex];
        }
    }

    /******************************************************************************//**
     * @brief Make control variables available to the parameter list
     * @param [in] aControl design variables
    **********************************************************************************/
    void setControls(const Plato::MultiVector<ScalarType, OrdinalType> & aControl)
    {
        // ********* Set view to each control vector entry ********* //
        const OrdinalType tControlVectorIndex = 0;
        assert(aControl[tControlVectorIndex].size() == mControl.size());
        for(OrdinalType tControlIndex = 0; tControlIndex < mControl.size(); tControlIndex++)
        {
            mControl[tControlIndex] = aControl(tControlVectorIndex, tControlIndex);
        }
        std::string tControlName = mEngineInputData.getControlName(tControlVectorIndex);
        mParameterList->set(tControlName, mControl.data());
    }

private:
    std::vector<ScalarType> mVector; /*!< local copy of a descent direction vector */
    std::vector<ScalarType> mControl; /*!< local copy of the control variables */
    std::vector<ScalarType> mGradient; /*!< local copy of the gradient vector */
    std::vector<ScalarType> mHessianTimesVector; /*!< local copy of the application of a vector to the Hessian */

    Plato::Interface* mInterface; /*!< PLATO Engine interface */
    Plato::OptimizerEngineStageData mEngineInputData; /*!< Parsed input data */
    std::shared_ptr<Teuchos::ParameterList> mParameterList; /*!< parameter list with data to be communicated through the PLATO Engine interface */

    /******************************************************************************//**
     * @brief string containing the optimizer name - Optional
    **********************************************************************************/
    std::string mOptimizerName{""};

    /******************************************************************************//**
     * @brief a vector of indices decribing this optimizer's location
     * in the input - used when there are multiple optimizers. See
     * Plato_OptimizerUtilities.hpp and the function getOptimizerNode().
    **********************************************************************************/
    std::vector<size_t> mOptimizerIndex;

    /******************************************************************************//**
     * @brief boolean indicating an inner optimizer loop is present
    **********************************************************************************/
    bool mHasInnerLoop{false};

private:
    EngineObjective(const Plato::EngineObjective<ScalarType, OrdinalType>&);
    Plato::EngineObjective<ScalarType, OrdinalType> & operator=(const Plato::EngineObjective<ScalarType, OrdinalType>&);
}; // class EngineObjective

} // namespace Plato
