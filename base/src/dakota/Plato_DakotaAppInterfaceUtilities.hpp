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
#pragma once

// Dakota includes
#include "PRPMultiIndex.hpp"
#include "DakotaResponse.hpp"
#include "DakotaVariables.hpp"
#include "ParamResponsePair.hpp"

#include "Plato_Macros.hpp"
#include "Plato_Parser.hpp"
#include "Plato_DakotaDataMap.hpp"

namespace Plato
{

/******************************************************************************//**
 * \brief enum for dakota active set evaluation types.
**********************************************************************************/
enum DakotaEvaluationType
{ 
  NO_DATA, 
  GET_VALUE, 
  GET_GRADIENT,
  GET_GRADIENT_AND_VALUE, 
  GET_HESSIAN,
  GET_HESSIAN_AND_VALUE,
  GET_HESSIAN_AND_GRADIENT,
  GET_HESSIAN_AND_GRADIENT_AND_VALUE 
};

void resetEvaluationFlags(const size_t& aNumCriteria, 
                          std::vector<DakotaEvaluationType>& aEvaluationFlags);

void setCriterionEvaluationFlagsForPRP(const short& aDirectFnASV,
                                       const size_t& aCriterion,
                                       std::vector<DakotaEvaluationType>& aEvaluationFlags);

std::vector<std::string> makeStageTags(const short& aASV, const size_t& aCriterionIndex);

void allocateStageOutputSharedData(const std::string &aStageTag,
                              const size_t aPrpIndex,
                              Plato::DakotaDataMap& aDataMap,
                              Teuchos::ParameterList& aParameterList);

std::vector<std::string> setStageNamesForEvaluation(const Plato::DakotaDataMap& aDataMap);

void setInitializeStageNameForEvaluation(const Plato::DakotaDataMap& aDataMap, std::vector<std::string> & aStageNames);

void setCriterionStageNamesForEvaluation(const Plato::DakotaDataMap& aDataMap, std::vector<std::string> & aStageNames);

void setFinalizeStageNameForEvaluation(const Plato::DakotaDataMap& aDataMap, std::vector<std::string> & aStageNames);

void setValueOutputs(const size_t aCriterionIndex, const size_t aPrpIndex, Dakota::RealVector & aDakotaVals, Plato::DakotaDataMap& aDataMap);

void setGradientOutputs(const size_t aCriterionIndex, const size_t aPrpIndex, Dakota::RealMatrix & aDakotaGrads, const size_t aNumDerivVars, Plato::DakotaDataMap& aDataMap);

} //namespace Plato
