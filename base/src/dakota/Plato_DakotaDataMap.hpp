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
 * Plato_DakotaDataMap.hpp
 *
 *  Created on: Oct 4, 2021
 */

#pragma once

#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <unordered_map>

// dakota includes
#include "ProblemDescDB.hpp"

// plato includes
#include "Plato_InputData.hpp"
#include "Plato_FreeFunctions.hpp"

namespace Plato
{

namespace dakota
{

/******************************************************************************//**
 * \struct SharedDataMetaData
 * \brief Contains shared data metadata.
**********************************************************************************/
struct SharedDataMetaData
{
    std::string mName; /*!< shared data name */
    std::vector<double> mValues; /*!< shared data values */
};
// struct SharedDataMetaData

}
// namespace dakota

class DakotaDataMap 
{
public:
    //
    //- Heading: Constructor and destructor
    //

    /******************************************************************************//**
     * \brief Main constructor - Takes in input data and parses it to populate \n 
     * data maps.
     * \param [in] aInputData Plato Engine (PE) input metadata.
    **********************************************************************************/
    DakotaDataMap(const Plato::InputData& aInputData);

    //
    //- Heading: Accessor functions
    //
    
    /******************************************************************************//**
     * \brief Get list of active Plato Engine stage tags. Tags are ordered by index,
     * i.e. criterion_value_0, criterion_value_1, criterion_gradient_0, etc, \n
     * \return list of active Plato Engine stage tags (standard vector)
    **********************************************************************************/
    std::vector<std::string> getStageTags() const;
        
    /******************************************************************************//**
     * \brief Get list of input shared data. 
     * \param [in] aStageTag stage tag
     * \param [in] aDataType  input data type, options are: continuous, discrete integer, 
     *   and discrete real. 
     * \return list of input shared data names (empty if not found)
    **********************************************************************************/
    std::vector<std::string> getInputSharedDataNames(const std::string& aStageTag, const std::string& aDataType) const;

    /******************************************************************************//**
     * \brief Check if stage has input shared data
     * \param [in] aStageTag stage tag
     * \return true if stage has input shared data
    **********************************************************************************/
    bool stageHasInputSharedData(const std::string& aStageTag) const;

    /******************************************************************************//**
     * \brief Get list of output shared_data names for a given stage tag. 
     *   Shared data is data shared by multiple application codes at runtime. For instance, \n
     *   the objective function value is own by the application code responsible of evaluating \n
     *   it and used by the optimization algorithm in PE.
     * \param [in] aStageTag stage tag, each stage has a set of input and output \n
     *   shared data.
     * \return list of output shared data names (empty if not found)
    **********************************************************************************/
    std::vector<std::string> getOutputSharedDataNames(const std::string& aStageTag) const;
        
    /******************************************************************************//**
     * \brief Check if stage has output shared data
     * \param [in] aStageTag stage tag
     * \return true if stage has output shared data
    **********************************************************************************/
    bool stageHasOutputSharedData(const std::string& aStageTag) const;

    /******************************************************************************//**
     * \brief Get Plato Engine (PE) stage name from stage tag. The stages are \n
     *   pre-defined for the optimization problems. A PE stage defines a workflow to \n
     *   be executed by PE at runtime. 
     * \param [in] aStageTag stage tag 
     * \return Plato Engine stage name 
    **********************************************************************************/
    std::string getStageName(const std::string& aStageTag) const;

    /******************************************************************************//**
     * \brief Store values of continuous variables shared data for stage \n 
     * \param [in] aSharedDataName shared data name associated with continuous variables array for stage
     * \param [in] aNumValues number of shared data values to store
     * \param [in] aData continuous data values from dakota
    **********************************************************************************/
    void setContinuousVarsSharedData(const std::string &aSharedDataName, 
                                     const size_t aNumValues,
                                     const Dakota::RealVector & aData);

    /******************************************************************************//**
     * \brief return shared data values
     * \param [in] aSharedDataName shared data name associated with continuous variables array for stage
    **********************************************************************************/
    std::vector<Dakota::Real> & getContinuousVarsSharedData(const std::string &aSharedDataName);

    /******************************************************************************//**
     * \brief Store values of discrete real variables shared data for stage \n 
     * \param [in] aSharedDataName shared data name associated with discrete real variables array for stage
     * \param [in] aNumValues number of shared data values to store
     * \param [in] aData discrete real data values from dakota
    **********************************************************************************/
    void setDiscreteRealVarsSharedData(const std::string &aSharedDataName, 
                                       const size_t aNumValues,
                                       const Dakota::RealVector & aData);

    /******************************************************************************//**
     * \brief return shared data values
     * \param [in] aSharedDataName shared data name associated with discrete real variables array for stage
    **********************************************************************************/
    std::vector<Dakota::Real> & getDiscreteRealVarsSharedData(const std::string &aSharedDataName);

    /******************************************************************************//**
     * \brief Store values of discrete integer variables shared data for stage \n 
     * \param [in] aSharedDataName shared data name associated with discrete integer variables array for stage
     * \param [in] aNumValues number of shared data values to store
     * \param [in] aData discrete integer data values from dakota
    **********************************************************************************/
    void setDiscreteIntegerVarsSharedData(const std::string &aSharedDataName, 
                                     const size_t aNumValues,
                                     const Dakota::IntVector & aData);

    /******************************************************************************//**
     * \brief return shared data values
     * \param [in] aSharedDataName shared data name associated with discrete integer variables array for stage
    **********************************************************************************/
    std::vector<Dakota::Real> & getDiscreteIntegerVarsSharedData(const std::string &aSharedDataName);

    /******************************************************************************//**
     * \brief Store output metadata for stage
     * \param [in] aStageTag stage tag where metadata is to be stored
     * \param [in] aMetaData metadata for output
    **********************************************************************************/
    void setOutputVarsSharedData(const std::string &aStageTag, 
                                 const Plato::dakota::SharedDataMetaData & aMetaData,
                                 const size_t aIndex);

    /******************************************************************************//**
     * \brief return shared data meta data
     * \param [in] aStageTag stage tag for where metadata is stored
     * \param [in] aPrpIndex index in prp queue
    **********************************************************************************/
    Plato::dakota::SharedDataMetaData & getOutputVarsSharedData(const std::string &aStageTag,
                                                                const size_t aIndex);

private:
    /******************************************************************************//**
     * \brief Parse stage attributes (name and type) at runtime.
     * \param [in] aStageNode stage input metadata.
    **********************************************************************************/
    void parseStageAttributes(Plato::InputData& aStageNode);

    /******************************************************************************//**
     * \brief Parse stage name at runtime.
     * \param [in] aStageNode stage input metadata.
     * \return stage name (standard string)
    **********************************************************************************/
    std::string parseMyStageName(Plato::InputData& aStageNode) const;

    /******************************************************************************//**
     * \brief Parse stage type at runtime.
     * \param [in] aStageNode stage input metadata.
     * \return stage type (standard string)
    **********************************************************************************/
    std::string parseMyStageTag(Plato::InputData& aStageNode) const;

    /******************************************************************************//**
     * \brief Check if stage tag corresponds to a criterion
     * \param [in] aTokens stage tag tokens
     * \return true if tag corresponds to criterion
    **********************************************************************************/
    bool isCriterionTag(const std::vector<std::string>& aTokens) const;

    /******************************************************************************//**
     * \brief Check if stage tag has a number as ID
     * \param [in] aTokens stage tag tokens
     * \param [in] aTag full stage tag
    **********************************************************************************/
    void checkValidCriterionId(const std::vector<std::string>& aTokens, const std::string& aTag) const;

    /******************************************************************************//**
     * \brief Parse stage's input shared data name at runtime.
     * \param [in] aStageNode stage metadata.
    **********************************************************************************/
    void parseStageInputs(Plato::InputData& aStageNode);

    /******************************************************************************//**
     * \brief Parse input shared data name at runtime.
     * \param [in] aInputNode input shared data metadata.
     * \return shared data name (standard string)
    **********************************************************************************/
    std::string parseMyInputSharedDataName(Plato::InputData& aInputNode) const;

    /******************************************************************************//**
     * \brief Parse input shared data tag at runtime.
     * \param [in] aInputNode input shared data metadata.
     * \return shared data tag (standard string)
    **********************************************************************************/
    std::string parseMyInputSharedDataTag(Plato::InputData& aInputNode) const;

    /******************************************************************************//**
     * \brief Parse stage's output shared data name at runtime.
     * \param [in] aStageNode stage metadata.
    **********************************************************************************/
    void parseStageOutputs(Plato::InputData& aStageNode);

    /******************************************************************************//**
     * \brief Parse output shared data name.
     * \param [in] aOutputNode output shared data metadata.
     * \return shared data name (standard string)
    **********************************************************************************/
    std::string parseMyOutputSharedDataName(Plato::InputData& aOutputNode) const;

    void verifyStageTagsAreUnique(const std::string& aStageTag) const;
    void verifyStageNamesAreUnique(const std::string& aStageName) const;

private:
    // maps for dakota stages stage, input, and output data
    std::unordered_map< std::string, std::string > mMapFromStageTagToStageName; /*!< map from stage tag to stage names */
    std::unordered_map< std::string, std::unordered_map<std::string, std::vector<std::string> > > mMapFromStageTagToMapFromInputDataTagToSharedDataNames; /*!< map from stage tag to input shared data names */
    std::unordered_map< std::string, std::vector<std::string> > mMapFromStageTagToOutputSharedDataNames; /*!< map from stage tag to output shared_data names */
    std::unordered_map< std::string, std::vector<Plato::dakota::SharedDataMetaData> > mMapFromStageTagToOutputSharedDataMetadata; /*!< map from stage tag to output shared data metadata */
    // maps from shared data name to input variables (continuous, discrete real, and discrete integer types)
    std::map< std::string, std::vector<Dakota::Real> > mMapFromSharedDataNameToContinuousVars; /*!< map from shared data name to continuous variables arrays */
    std::map< std::string, std::vector<Dakota::Real> > mMapFromSharedDataNameToDiscreteRealVars; /*!< map from shared data name to discrete real variables arrays */
    std::map< std::string, std::vector<Dakota::Real> > mMapFromSharedDataNameToDiscreteIntegerVars; /*!< map from shared data name to discrete integer variables arrays */

};
// class DakotaDataMap
}
// namespace Plato
