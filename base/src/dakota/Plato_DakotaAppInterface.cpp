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
 * Plato_DakotaAppInterface.cpp
 *
 *  Created on: Aug 14, 2021
 */

// Dakota includes
#include "PRPMultiIndex.hpp"
#include "DakotaResponse.hpp"
#include "DakotaVariables.hpp"
#include "ParamResponsePair.hpp"

#include "Plato_Macros.hpp"
#include "Plato_Parser.hpp"
#include "Plato_DakotaAppInterface.hpp"

namespace Plato
{

DakotaAppInterface::DakotaAppInterface
(const Dakota::ProblemDescDB &aProblemDataBase, 
 Plato::Interface *aInterface) : 
     Dakota::DirectApplicInterface(aProblemDataBase),
     mInterface(aInterface),
     mDataMap(aInterface->getInputData())
{
}

int DakotaAppInterface::derived_map_if(const Dakota::String &/*if_name*/)
{
    THROWERR(std::string("derived_map_if is not supported with Plato Dakota Driver, Plato should always use wait_local_evaluations"))
    return 0; /* can be used the same way the updateProblem in Plato is used. */
}

int DakotaAppInterface::derived_map_ac(const Dakota::String &/*aAnalysisDriverName*/)
{
    THROWERR(std::string("derived_map_ac is not supported with Plato Dakota Driver, Plato should always use wait_local_evaluations"))
    return 0;
}

int DakotaAppInterface::derived_map_of(const Dakota::String &/*of_name*/)
{
    THROWERR(std::string("derived_map_of is not supported with Plato Dakota Driver, Plato should always use wait_local_evaluations"))
    return 0; /* can be used to call output functions in other apps. the function is called once per iteration */
}

void DakotaAppInterface::wait_local_evaluations(Dakota::PRPQueue &aParamResponsePairQueue)
{
    this->setAllStageData(aParamResponsePairQueue);
    this->evaluateStages();
    this->setDakotaOutputData(aParamResponsePairQueue);
}

void DakotaAppInterface::derived_map_asynch(const Dakota::ParamResponsePair& /*pair*/)
{
  // virtual function overwritten to enable call to wait_local_evaluations
}

void DakotaAppInterface::set_communicators_checks(int /*max_eval_concurrency*/)
{
  // virtual function overwritten to enable call to wait_local_evaluations
}

void DakotaAppInterface::setAllStageData(const Dakota::PRPQueue &aParamResponsePairQueue)
{
    this->setInitializeStageData(aParamResponsePairQueue);

    this->setEvaluationFlags(aParamResponsePairQueue);
    this->setCriterionStageData(aParamResponsePairQueue);

    this->setFinalizeStageData(aParamResponsePairQueue);
}

void DakotaAppInterface::setEvaluationFlags(const Dakota::PRPQueue &aParamResponsePairQueue)
{
    size_t tNumCriteria = this->determineNumberOfCriteria(aParamResponsePairQueue);

    resetEvaluationFlags(tNumCriteria,mEvaluationFlags);
    this->setNewEvaluationFlagsForPRPQueue(tNumCriteria,aParamResponsePairQueue);
}

void DakotaAppInterface::setNewEvaluationFlagsForPRPQueue(const size_t& tNumCriteria,
                                                          const Dakota::PRPQueue& aParamResponsePairQueue)
{
    for(size_t tCriterion = 0; tCriterion < tNumCriteria; tCriterion++)
    {
        for (auto tPRP : aParamResponsePairQueue)
        {
            this->setLocalData(tPRP);
            auto tDirectFnASV = Dakota::DirectApplicInterface::directFnASV[tCriterion];
            setCriterionEvaluationFlagsForPRP(tDirectFnASV,tCriterion,mEvaluationFlags);
        }
    }
}

void DakotaAppInterface::setLocalData(const Dakota::ParamResponsePair& aPRP)
{
    const Dakota::Variables &tVariables = aPRP.variables();
    const Dakota::ActiveSet &tActiveSet = aPRP.active_set();
    Dakota::DirectApplicInterface::set_local_data(tVariables, tActiveSet);
}

size_t DakotaAppInterface::determineNumberOfCriteria(const Dakota::PRPQueue &aParamResponsePairQueue)
{
    Dakota::PRPQueueIter tFirstPRPIter = aParamResponsePairQueue.begin();
    this->setLocalData(*tFirstPRPIter);
    return Dakota::DirectApplicInterface::directFnASV.size();
}

void DakotaAppInterface::setInitializeStageData(const Dakota::PRPQueue &aParamResponsePairQueue)
{
    std::string tStageTag = "initialize";
    this->setStageData(tStageTag,aParamResponsePairQueue);
}

void DakotaAppInterface::setStageData(const std::string& aStageTag,
                                      const Dakota::PRPQueue& aParamResponsePairQueue)
{
    auto tStageName = mDataMap.getStageName(aStageTag);
    if (!tStageName.empty())
    {
        size_t tPrpIndex = 0;
        for (Dakota::PRPQueueIter tParamRespPairIter = aParamResponsePairQueue.begin();
                tParamRespPairIter != aParamResponsePairQueue.end(); tParamRespPairIter++, tPrpIndex++)
        {
            this->setLocalData(*tParamRespPairIter);
            this->setStageSharedData(aStageTag,tPrpIndex);
        }
    }
}

void DakotaAppInterface::setCriterionStageData(const Dakota::PRPQueue &aParamResponsePairQueue)
{
    auto tNumCriteria = mEvaluationFlags.size();

    size_t tPrpIndex = 0;
    for (Dakota::PRPQueueIter tParamRespPairIter = aParamResponsePairQueue.begin();
             tParamRespPairIter != aParamResponsePairQueue.end(); tParamRespPairIter++, tPrpIndex++)
    {
        this->setCriterionStageDataForPRP(tNumCriteria, tPrpIndex, *tParamRespPairIter);
    }
}

void DakotaAppInterface::setCriterionStageDataForPRP(const size_t& aNumCriteria,
                                                     const size_t& aPrpIndex,
                                                     const Dakota::ParamResponsePair &aPRP)
{
    this->setLocalData(aPRP);
    auto tNumActiveCriteria = Dakota::DirectApplicInterface::directFnASV.size();

    if (tNumActiveCriteria != aNumCriteria)
        THROWERR(std::string("In setCriterionStageData: Number of active criteria for PRP ") + std::to_string(aPrpIndex) + std::string(" is different from PRP 0"))

    for (size_t tCriterionIndex=0; tCriterionIndex < tNumActiveCriteria; tCriterionIndex++)
    {
        short tASV = mEvaluationFlags[tCriterionIndex];
        std::vector<std::string> tStageTags = makeStageTags(tASV,tCriterionIndex);

        for(auto tStageTag : tStageTags)
            this->setStageSharedData(tStageTag,aPrpIndex);
    }
}

void DakotaAppInterface::setFinalizeStageData(const Dakota::PRPQueue &aParamResponsePairQueue)
{
    std::string tStageTag = "finalize";
    this->setStageData(tStageTag,aParamResponsePairQueue);
}

void DakotaAppInterface::setStageSharedData(const std::string aStageTag, const size_t aPrpIndex)
{
    if ( mDataMap.stageHasInputSharedData(aStageTag) )
    {
        this->setStageContinuousVarsInputSharedData(aStageTag, aPrpIndex);
        this->setStageDiscreteRealVarsInputSharedData(aStageTag, aPrpIndex);
        this->setStageDiscreteIntegerVarsInputSharedData(aStageTag, aPrpIndex);
    }
    if ( mDataMap.stageHasOutputSharedData(aStageTag) )
    {
        allocateStageOutputSharedData(aStageTag, aPrpIndex, mDataMap, mParameterList);
    }
}

void DakotaAppInterface::setStageContinuousVarsInputSharedData(const std::string &aStageTag,const size_t aPrpIndex)
{
    auto tDataNames = mDataMap.getInputSharedDataNames(aStageTag,"continuous");
    
    if (tDataNames.size() > 0)
    {
        if(tDataNames.size() <= aPrpIndex)
            throw std::out_of_range(ERRMSG("PRP Index out of range of continuous shared data names"));

        std::string tName = tDataNames[aPrpIndex];
        mDataMap.setContinuousVarsSharedData(tName,Dakota::DirectApplicInterface::numACV,Dakota::DirectApplicInterface::xC);
        mParameterList.set(tName, mDataMap.getContinuousVarsSharedData(tName).data());
    }
}

void DakotaAppInterface::setStageDiscreteRealVarsInputSharedData(const std::string &aStageTag, const size_t aPrpIndex)
{
    auto tDataNames = mDataMap.getInputSharedDataNames(aStageTag,"discrete real");
    
    if (tDataNames.size() > 0)
    {
        if(tDataNames.size() <= aPrpIndex)
            throw std::out_of_range(ERRMSG("PRP Index out of range of discrete real shared data names"));

        std::string tName = tDataNames[aPrpIndex];
        mDataMap.setDiscreteRealVarsSharedData(tName,Dakota::DirectApplicInterface::numADRV,Dakota::DirectApplicInterface::xDR);
        mParameterList.set(tName, mDataMap.getDiscreteRealVarsSharedData(tName).data());
    }
}

void DakotaAppInterface::setStageDiscreteIntegerVarsInputSharedData(const std::string &aStageTag, const size_t aPrpIndex)
{
    auto tDataNames = mDataMap.getInputSharedDataNames(aStageTag,"discrete integer");
    
    if (tDataNames.size() > 0)
    {
        if(tDataNames.size() <= aPrpIndex)
            throw std::out_of_range(ERRMSG("PRP Index out of range of discrete integer shared data names"));

        std::string tName = tDataNames[aPrpIndex];
        mDataMap.setDiscreteIntegerVarsSharedData(tName,Dakota::DirectApplicInterface::numADIV,Dakota::DirectApplicInterface::xDI);
        mParameterList.set(tName, mDataMap.getDiscreteIntegerVarsSharedData(tName).data());
    }
}

void DakotaAppInterface::evaluateStages()
{
    std::vector<std::string> tStageNames = setStageNamesForEvaluation(mDataMap);
    mInterface->compute(tStageNames, mParameterList);
}

void DakotaAppInterface::setDakotaOutputData(const Dakota::PRPQueue &aParamResponsePairQueue)
{
    auto tNumCriteria = mEvaluationFlags.size();

    size_t tPrpIndex = 0;
    for (Dakota::PRPQueueIter tParamRespPairIter = aParamResponsePairQueue.begin();
             tParamRespPairIter != aParamResponsePairQueue.end(); tParamRespPairIter++, tPrpIndex++)
    {
        this->setLocalData(*tParamRespPairIter);

        auto tNumActiveCriteria = Dakota::DirectApplicInterface::directFnASV.size();
        if (tNumActiveCriteria != tNumCriteria)
        {
            THROWERR(std::string("In setDakotaOutputData: Number of active criteria for PRP ") + std::to_string(tPrpIndex) + std::string(" is different from PRP 0"))
        }

        auto tEvaluationID = tParamRespPairIter->eval_id();
        Dakota::Response tResponse = tParamRespPairIter->response();
        Dakota::RealVector tDakotaFunVals = tResponse.function_values_view();
        Dakota::RealMatrix tDakotaFunGrads = tResponse.function_gradients_view();

        for (size_t tCriterion=0; tCriterion < tNumActiveCriteria; tCriterion++)
        {
            if (mEvaluationFlags[tCriterion] == GET_VALUE) // value
            {
                setValueOutputs(tCriterion,tPrpIndex,tDakotaFunVals,mDataMap);
            }

            if (mEvaluationFlags[tCriterion] == GET_GRADIENT) // gradient
            {
                setGradientOutputs(tCriterion,tPrpIndex,tDakotaFunGrads,Dakota::DirectApplicInterface::numDerivVars,mDataMap);
            }

            if (mEvaluationFlags[tCriterion] == GET_GRADIENT_AND_VALUE) // value and gradient
            {
                setValueOutputs(tCriterion,tPrpIndex,tDakotaFunVals,mDataMap);
                setGradientOutputs(tCriterion,tPrpIndex,tDakotaFunGrads,Dakota::DirectApplicInterface::numDerivVars,mDataMap);
            }
        
        }
        Dakota::DirectApplicInterface::completionSet.insert(tEvaluationID);
    }
}

}
// namespace Plato
