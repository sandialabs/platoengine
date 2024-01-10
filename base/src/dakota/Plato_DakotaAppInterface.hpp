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
 * Plato_DakotaAppInterface.hpp
 *
 *  Created on: Aug 14, 2021
 */

#pragma once

#include <vector>
#include <string>
#include <iterator>
#include <algorithm>

// dakota includes
#include "ProblemDescDB.hpp"
#include "DirectApplicInterface.hpp"

// plato includes
#include "Plato_InputData.hpp"
#include "Plato_Interface.hpp"
#include "Plato_FreeFunctions.hpp"
#include "Plato_DakotaDataMap.hpp"
#include "Plato_DakotaAppInterfaceUtilities.hpp"

namespace Plato
{

class DakotaAppInterface : public Dakota::DirectApplicInterface
{
public:
    //
    //- Heading: Constructor and destructor
    //

    /******************************************************************************//**
     * \brief Main constructor - used in Plato-Dakota use cases driven by Plato Engine (PE).
     * \param [in] aProblemDataBase Dakota problem metadata.
     * \param [in] aInterface PE interface metadata.
    **********************************************************************************/
    DakotaAppInterface(const Dakota::ProblemDescDB &aProblemDataBase, Plato::Interface *aInterface);

    /******************************************************************************//**
     * \brief Execute the input filter portion of a direct evaluation invocation.
     * \param [in] aInputFilterName input filter name
    **********************************************************************************/
    int derived_map_if(const Dakota::String &aInputFilterName) override;

    /******************************************************************************//**
     * \brief Execute an analysis code portion of a direct evaluation invocation.
     * \param [in] aAnalysisDriverName analysis driver name
    **********************************************************************************/
    int derived_map_ac(const Dakota::String &aAnalysisDriverName) override;

    /******************************************************************************//**
     * \brief Execute the output filter portion of a direct evaluation invocation.
     * \param [in] aOutputFilterName output filter name
    **********************************************************************************/
    int derived_map_of(const Dakota::String &aOutputFilterName) override;

    /******************************************************************************//**
     * \brief Evaluate the batch of jobs contained in the parameter-response pair queue.
     * \param [in] aParamResponsePairQueue parameter-response pair queue metadata
    **********************************************************************************/
    void wait_local_evaluations(Dakota::PRPQueue& aParamResponsePairQueue) override;

    /******************************************************************************//**
     * \brief no op hides base error, job batching occurs within wait_local_evaluations
     * \param [in] aParamResponsePairQueue parameter-response pair queue metadata
    **********************************************************************************/
    void derived_map_asynch(const Dakota::ParamResponsePair& pair) override;

    /******************************************************************************//**
     * \brief need to override function so that asynchronous evaluation doesn't throw error.
     * \param [in] max_eval_concurrency maximum evaluation concurrency
    **********************************************************************************/
    void set_communicators_checks(int max_eval_concurrency) override;

private:
    /******************************************************************************//**
     * \brief Set input shared data and initialize output shared data for all stages
     * \param [in] aParamResponsePairQueue parameter-response pair queue metadata
    **********************************************************************************/
    void setAllStageData(const Dakota::PRPQueue& aParamResponsePairQueue);

    /******************************************************************************//**
     * \brief Set input shared data and initialize output shared data for aStageTag
     * \param [in] aStageTag 
     * \param [in] aParamResponsePairQueue parameter-response pair queue metadata
    **********************************************************************************/
    void setStageData(const std::string& aStageTag,
                      const Dakota::PRPQueue& aParamResponsePairQueue);

    /******************************************************************************//**
     * \brief For each criterion sets the stage type (value, gradient or both) \n
     * that will be called 
     * \param [in] aParamResponsePairQueue queue of parameter response paiers
    **********************************************************************************/
    void setEvaluationFlags(const Dakota::PRPQueue &aParamResponsePairQueue);

    /******************************************************************************//**
     * \brief Use first parameter-response pair to determine number of active
     * criteria
     * \param [in] aParamResponsePairQueue parameter-response pair queue metadata
    **********************************************************************************/
    size_t determineNumberOfCriteria(const Dakota::PRPQueue& aParamResponsePairQueue);

    /******************************************************************************//**
     * \brief Set input shared data and initialize output shared data for initialize 
     * stage
     * \param [in] aParamResponsePairQueue parameter-response pair queue metadata
    **********************************************************************************/
    void setInitializeStageData(const Dakota::PRPQueue& aParamResponsePairQueue);

    /******************************************************************************//**
     * \brief Set input shared data and initialize output shared data for stages
     * involving entire prp queue 
     * \param [in] aParamResponsePairQueue parameter-response pair queue metadata
    **********************************************************************************/
    void setCriterionStageData(const Dakota::PRPQueue& aParamResponsePairQueue);

    /******************************************************************************//**
     * \brief Set input shared data and initialize output shared data for stages
     * involving a specific prp 
     * \param [in] aNumCriteria The number of criteria associated with the first
     * prp in the queue. We currently require that each PRP has the same number
     * \param [in] aPrpIndex index of PRP in the PRPQueue
     * \param [in] aPRP parameter-response pair
    **********************************************************************************/
    void setCriterionStageDataForPRP(const size_t& aNumCriteria,
                                     const size_t& aPrpIndex,
                                     const Dakota::ParamResponsePair &aPRP);

    void setSingleCriterionStageDataForPRP(const size_t& aCriterionIndex,
                                           const size_t& aPrpIndex,
                                           std::vector<DakotaEvaluationType>& aEvaluationFlags);

    /******************************************************************************//**
     * \brief Set input shared data and initialize output shared data for finalize 
     * stage
     * \param [in] aParamResponsePairQueue parameter-response pair queue metadata
    **********************************************************************************/
    void setFinalizeStageData(const Dakota::PRPQueue& aParamResponsePairQueue);

    /******************************************************************************//**
     * \brief Sets input and output shared data for the criterion
     * \param [in] aStageTag tag for stage
     * \param [in] aPrpIndex index in prp queue
     * \param [in] aDataMap maps from plato stage shared data (inputs & outputs) to dakota data
    **********************************************************************************/
    void setStageSharedData(const std::string aStageTag, const size_t aPrpIndex);

    /******************************************************************************//**
     * \brief Set inputs for current Plato Engine stage by copying the active \n
     *   continuous optimization parameter values into the corresponding input shared data arrays. 
     * \param [in] aStageTag stage tag, e.g. criterion_value_0
     * \param [in] aPrpIndex index in prp queue
    **********************************************************************************/
    void setStageContinuousVarsInputSharedData(const std::string& aStageTag, const size_t aPrpIndex);

    /******************************************************************************//**
     * \brief Set inputs for current Plato Engine stage by copying the active \n
     *   discrete real optimization parameter values into the corresponding input shared data arrays. 
     * \param [in] aStageTag stage tag, e.g. criterion_value_0
     * \param [in] aPrpIndex index in prp queue
    **********************************************************************************/
    void setStageDiscreteRealVarsInputSharedData(const std::string& aStageTag, const size_t aPrpIndex);

    /******************************************************************************//**
     * \brief Set inputs for current Plato Engine stage by copying the active \n
     *   discrete integer optimization parameter values into the corresponding input shared data arrays. 
     * \param [in] aStageTag stage tag, e.g. criterion_value_0
     * \param [in] aPrpIndex index in prp queue
    **********************************************************************************/
    void setStageDiscreteIntegerVarsInputSharedData(const std::string& aStageTag, const size_t aPrpIndex);

    /******************************************************************************//**
     * \brief Create list of stage names and evaluate using Plato Interface.
    **********************************************************************************/
    void evaluateStages();

    /******************************************************************************//**
     * \brief Set dakota output data from output shared data and finalize criteria
    **********************************************************************************/
    void setDakotaOutputData(const Dakota::PRPQueue &aParamResponsePairQueue);

    /******************************************************************************//**
     * \brief Set local data to populate dakota data structures for input PRP
     * \param [in] aPRP Dakota parameter response pair
    **********************************************************************************/
    void setLocalData(const Dakota::ParamResponsePair& aPRP);

    /******************************************************************************//**
     * \brief Set local data to populate dakota data structures for input PRP
     * \param [in] aPRP Dakota parameter response pair
    **********************************************************************************/
    void setNewEvaluationFlagsForPRPQueue(const size_t& tNumCriteria,
                                          const Dakota::PRPQueue& aParamResponsePairQueue);

private:
    Plato::Interface *mInterface; /*!< provides access to functions in plato engine */
    Teuchos::ParameterList mParameterList = Teuchos::ParameterList(); /*!< list of input and output shared data for evaluating plato stages */
    Plato::DakotaDataMap mDataMap; /*!< maps from plato stage shared data (inputs & outputs) to dakota data*/
    std::vector<Plato::DakotaEvaluationType> mEvaluationFlags; /*!< flag of evaluation type (value, gradient, or gradient and value) for different criteria  */

};
// class DakotaAppInterface

}
// namespace Plato
