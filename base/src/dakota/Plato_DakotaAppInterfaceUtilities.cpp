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
 * Plato_DakotaAppInterfaceUtilities.hpp
 *
 *  Created on: Jan 28, 2022
 */

#include "Plato_DakotaAppInterfaceUtilities.hpp"

#include <boost/numeric/conversion/cast.hpp>

namespace Plato
{

void resetEvaluationFlags(const size_t& aNumCriteria, std::vector<DakotaEvaluationType>& aEvaluationFlags)
{
    aEvaluationFlags.resize(aNumCriteria);

    for(size_t tFlagIndex = 0; tFlagIndex < aNumCriteria; tFlagIndex++)
    {
        aEvaluationFlags[tFlagIndex] = NO_DATA;
    }
}

void setCriterionEvaluationFlagsForPRP(const short& aDirectFnASV,
                                       const size_t& aCriterionIndex,
                                       std::vector<DakotaEvaluationType>& aEvaluationFlags)
{
    if(aDirectFnASV > 7)
        throw std::out_of_range(ERRMSG("Active set out of range"));

    if(aDirectFnASV > 3)
        throw std::runtime_error(ERRMSG("Active set requested hessian but Plato does not yet support hessian calculations"));
            
    if(aCriterionIndex >= aEvaluationFlags.size())
        throw std::out_of_range(ERRMSG("Criterion Index out of range of evaluation flags"));

    if(aEvaluationFlags[aCriterionIndex] == NO_DATA)
        aEvaluationFlags[aCriterionIndex] = static_cast<DakotaEvaluationType>(aDirectFnASV);

    if(aEvaluationFlags[aCriterionIndex] == GET_VALUE && (aDirectFnASV == GET_GRADIENT || aDirectFnASV == GET_GRADIENT_AND_VALUE))
        aEvaluationFlags[aCriterionIndex] = GET_GRADIENT_AND_VALUE;

    if(aEvaluationFlags[aCriterionIndex] == GET_GRADIENT && (aDirectFnASV == GET_VALUE || aDirectFnASV == GET_GRADIENT_AND_VALUE))
        aEvaluationFlags[aCriterionIndex] = GET_GRADIENT_AND_VALUE;
}

std::vector<std::string> makeStageTags(const short& aASV, const size_t& aCriterionIndex)
{
    if(aASV > 7)
        throw std::out_of_range(ERRMSG("Active set out of range"));

    if(aASV > 3)
        throw std::runtime_error(ERRMSG("Active set requested hessian but Plato does not yet support hessian calculations"));

    std::vector<std::string> tStageTags;

    if (aASV == GET_VALUE) // value
    {
        std::string tStageTag = "criterion_value_" + std::to_string(aCriterionIndex);
        tStageTags.push_back(tStageTag);
    }

    if (aASV == GET_GRADIENT) // gradient
    {
        std::string tStageTag = "criterion_gradient_" + std::to_string(aCriterionIndex);
        tStageTags.push_back(tStageTag);
    }

    if (aASV == GET_GRADIENT_AND_VALUE) // value and gradient
    {
        std::string tStageTag = "criterion_value_" + std::to_string(aCriterionIndex);
        tStageTags.push_back(tStageTag);
        tStageTag = "criterion_gradient_" + std::to_string(aCriterionIndex);
        tStageTags.push_back(tStageTag);
    }

    return tStageTags;
}

void allocateStageOutputSharedData(const std::string &aStageTag, const size_t aPrpIndex, Plato::DakotaDataMap& aDataMap, Teuchos::ParameterList& aParameterList)
{
    auto tSharedDataNames = aDataMap.getOutputSharedDataNames(aStageTag);

    if (tSharedDataNames.size() == 0)
        throw std::runtime_error(ERRMSG("Stage does not have output defined"));

    if(tSharedDataNames.size() <= aPrpIndex)
        throw std::out_of_range(ERRMSG("PRP Index out of range of output shared data names"));

    auto tSharedDataName = tSharedDataNames[aPrpIndex];
    Plato::dakota::SharedDataMetaData tMetaData;
    tMetaData.mName = tSharedDataName;
    tMetaData.mValues.resize(1u,0.0);
    aDataMap.setOutputVarsSharedData(aStageTag,tMetaData,aPrpIndex);
    aParameterList.set(tSharedDataName, aDataMap.getOutputVarsSharedData(aStageTag,aPrpIndex).mValues.data());
}

std::vector<std::string> setStageNamesForEvaluation(const Plato::DakotaDataMap& aDataMap)
{
    std::vector<std::string> tStageNames;
    setInitializeStageNameForEvaluation(aDataMap,tStageNames);
    setCriterionStageNamesForEvaluation(aDataMap,tStageNames);
    setFinalizeStageNameForEvaluation(aDataMap,tStageNames);

    return tStageNames;
}

void setInitializeStageNameForEvaluation(const Plato::DakotaDataMap& aDataMap, std::vector<std::string> & aStageNames)
{
    auto tStageName = aDataMap.getStageName("initialize");
    if (!tStageName.empty())
    {
        aStageNames.push_back(tStageName);
    }
}

void setCriterionStageNamesForEvaluation(const Plato::DakotaDataMap& aDataMap, std::vector<std::string> & aStageNames)
{
    std::vector<std::string> tStageTagList = aDataMap.getStageTags();
    for (std::string tStageTag : tStageTagList)
    {
        auto tTagTokens = Plato::tokenize(tStageTag);
        if (tTagTokens[0] == "criterion")
        {
            std::string tStageName = aDataMap.getStageName(tStageTag);
            aStageNames.push_back(tStageName);
        }
    }
}

void setFinalizeStageNameForEvaluation(const Plato::DakotaDataMap& aDataMap, std::vector<std::string> & aStageNames)
{
    auto tStageName = aDataMap.getStageName("finalize");
    if (!tStageName.empty())
    {
        aStageNames.push_back(tStageName);
    }
}

void setValueOutputs(const size_t aCriterionIndex, const size_t aPrpIndex, Dakota::RealVector & aDakotaVals, Plato::DakotaDataMap& aDataMap)
{
    std::string tStageTag = "criterion_value_" + std::to_string(aCriterionIndex);

    if(boost::numeric_cast<size_t>(aDakotaVals.length()) <= aCriterionIndex)
        throw std::out_of_range(ERRMSG("In setValueOutputs: Criterion index out of range of dakota function values"));

    aDakotaVals[aCriterionIndex] = aDataMap.getOutputVarsSharedData(tStageTag,aPrpIndex).mValues.front();
}

void setGradientOutputs(const size_t aCriterionIndex, const size_t aPrpIndex, Dakota::RealMatrix & aDakotaGrads, const size_t aGradientLength, Plato::DakotaDataMap& aDataMap)
{
    std::string tStageTag = "criterion_gradient_" + std::to_string(aCriterionIndex);

    if(boost::numeric_cast<size_t>(aDakotaGrads.numRows()) <= aCriterionIndex)
        throw std::out_of_range(ERRMSG("In setGradientOutputs: Criterion index out of range of dakota gradient values"));

    if(boost::numeric_cast<size_t>(aDakotaGrads.numCols()) < aGradientLength)
        throw std::out_of_range(ERRMSG("In setGradientOutputs: Requested gradient length larger than allocated dakota gradient length"));

    auto tValues = aDataMap.getOutputVarsSharedData(tStageTag,aPrpIndex).mValues;
    if(tValues.size() < aGradientLength)
        throw std::out_of_range(ERRMSG("In setGradientOutputs: Requested gradient length larger than allocated output data gradient length"));

    for (size_t tVarIndex = 0; tVarIndex < aGradientLength; tVarIndex++)
    {
        aDakotaGrads[aCriterionIndex][tVarIndex] = tValues[tVarIndex];
    }
}

} //namespace Plato
