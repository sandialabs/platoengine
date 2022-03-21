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
 * Plato_DakotaDataMap.cpp
 *
 *  Created on: Oct 4, 2021
 */

#include "Plato_Macros.hpp"
#include "Plato_Parser.hpp"
#include "Plato_DakotaDataMap.hpp"

namespace Plato
{

DakotaDataMap::DakotaDataMap
(const Plato::InputData &aInputData)
{
    Plato::InputData tDakotaDriver;
    try
    {
        tDakotaDriver = aInputData.get<Plato::InputData>("DakotaDriver");
    }
    catch(int aValue)
    {
        throw std::runtime_error(ERRMSG("Input Data does not contain Dakota Driver block"));
    }

    auto tStageNodes = tDakotaDriver.getByName<Plato::InputData>("Stage");

    if(tStageNodes.size() == 0)
        throw std::runtime_error(ERRMSG("Dakota Driver block contains no stages"));

    for (auto &tStageNode : tStageNodes)
    {
        this->parseStageAttributes(tStageNode);
        this->parseStageInputs(tStageNode);
        this->parseStageOutputs(tStageNode);
    }

}

void DakotaDataMap::parseStageAttributes(Plato::InputData &aStageNode)
{
    auto tStageTags = this->getStageTags();
    auto tStageName = this->parseMyStageName(aStageNode);
    auto tLowerStageTag = this->parseMyStageTag(aStageNode);

    this->verifyStageTagsAreUnique(tLowerStageTag);
    this->verifyStageNamesAreUnique(tStageName);

    mMapFromStageTagToStageName[tLowerStageTag] = tStageName;
}

std::string DakotaDataMap::parseMyStageName(Plato::InputData &aStageNode) const
{
    auto tStageName = Plato::Get::String(aStageNode, "StageName");
    if (tStageName.empty())
    {
        THROWERR(std::string("Error while parsing inputs for the Plato-Dakota driver. The 'StageName' ") + "keyword is empty. A stage name must be provided to know which stage to perform at runtime.")
    }
    return tStageName;
}

std::string DakotaDataMap::parseMyStageTag(Plato::InputData &aStageNode) const
{
    auto tLowerStageTag = Plato::tolower(Plato::Get::String(aStageNode, "StageTag"));
    
    if (tLowerStageTag.empty())
        THROWERR(std::string("Error while parsing inputs for the Plato-Dakota driver. The 'StageTag' keyword is empty. A stage ") + "tag must be defined. Options are: 'criterion_value_#', 'criterion_gradient_#, 'initialize', or 'finalize'.")
            
    auto tTagTokens = Plato::tokenize(tLowerStageTag);
    if(this->isCriterionTag(tTagTokens))
        this->checkValidCriterionId(tTagTokens,tLowerStageTag);
    else if(tLowerStageTag != "initialize" && tLowerStageTag != "finalize")
        throw std::runtime_error(ERRMSG("Unexpected stage tag \"" + tLowerStageTag + "\" in DakotaDriver block. Options are: 'criterion_value_#', 'criterion_gradient_#, 'initialize', or 'finalize'"));

    return tLowerStageTag;
}

bool DakotaDataMap::isCriterionTag(const std::vector<std::string> &aTokens) const
{
    bool tReturn = false;
    if (aTokens.size() == 3 && aTokens[0] == "criterion")
    {
      if (aTokens[1] == "value" || aTokens[1] == "gradient") 
        tReturn = true;
    }

    return tReturn;
}

void DakotaDataMap::checkValidCriterionId(const std::vector<std::string> &aTokens, const std::string &aTag) const
{
    char* p;
    long converted = std::strtol(aTokens[2].c_str(), &p, 10);
    if (*p)
        throw std::runtime_error(ERRMSG("Unexpected stage tag \"" + aTag + "\" in DakotaDriver block. Options are: 'criterion_value_#', 'criterion_gradient_#, 'initialize', or 'finalize'"));
}

void DakotaDataMap::parseStageInputs(Plato::InputData &aStageNode)
{
    auto tStageTag = this->parseMyStageTag(aStageNode);
    auto tInputNodes = aStageNode.getByName<Plato::InputData>("Input");
    for (auto &tInputNode : tInputNodes)
    {
        auto tSharedDataName = this->parseMyInputSharedDataName(tInputNode);
        auto tInputDataTag = this->parseMyInputSharedDataTag(tInputNode);
        mMapFromStageTagToMapFromInputDataTagToSharedDataNames[tStageTag][tInputDataTag].push_back(tSharedDataName);
    }
}

std::string DakotaDataMap::parseMyInputSharedDataName(Plato::InputData &aInputNode) const
{
    auto tSharedDataName = Plato::Get::String(aInputNode, "SharedDataName");
    if (tSharedDataName.empty())
    {
        THROWERR(std::string("Error while parsing inputs for the Plato-Dakota driver. A 'SharedDataName' ") + "keyword inside the 'Input' block is empty. Shared data must be named to know at runtime which " + "application owns and uses the shared data.")
    }
    return tSharedDataName;
}

std::string DakotaDataMap::parseMyInputSharedDataTag(Plato::InputData &aInputNode) const
{
    auto tInputSharedDataTag = Plato::tolower(Plato::Get::String(aInputNode, "Tag"));
    if (tInputSharedDataTag.empty())
    {
        THROWERR(std::string("Error while parsing inputs for the Plato-Dakota driver. The 'Tag' keyword for a shared ") + "data input in the 'Input' block is empty. A tag must be assigned to the input shared data. Options are: " + "'continuous', 'discrete integer', or 'discrete real'.")
    }
    if(tInputSharedDataTag != "continuous" && tInputSharedDataTag != "discrete real" && tInputSharedDataTag != "discrete integer")
        throw std::runtime_error(ERRMSG("Unexpected input shared data tag \"" + tInputSharedDataTag + "\" in DakotaDriver block stage. Options are: 'continuous', 'discrete real, or 'discrete integer'"));

    return tInputSharedDataTag;
}

void DakotaDataMap::parseStageOutputs(Plato::InputData &aStageNode)
{
    auto tStageTag = this->parseMyStageTag(aStageNode);
    auto tOutputNodes = aStageNode.getByName<Plato::InputData>("Output");
    for (auto &tOutputNode : tOutputNodes)
    {
        auto tSharedDataName = this->parseMyOutputSharedDataName(tOutputNode);
        mMapFromStageTagToOutputSharedDataNames[tStageTag].push_back(tSharedDataName);
    }

    mMapFromStageTagToOutputSharedDataMetadata[tStageTag].resize(tOutputNodes.size());
}

std::string DakotaDataMap::parseMyOutputSharedDataName(Plato::InputData &aOutputNode) const
{
    auto tSharedDataName = Plato::Get::String(aOutputNode, "SharedDataName");
    if (tSharedDataName.empty())
    {
        THROWERR(std::string("Error while parsing inputs for the Plato-Dakota driver. A 'SharedDataName' ") + "keyword inside the 'Output' block is empty. A shared data name must be provided to know the users " + "and owners of the shared data at runtime.")
    }
    return tSharedDataName;
}

std::vector<std::string> DakotaDataMap::getStageTags() const
{
    std::vector<std::string> tStageTags;
    for (auto &tPair : mMapFromStageTagToStageName)
    {
        tStageTags.push_back(tPair.first);
    }
    return tStageTags;
}

std::vector<std::string> DakotaDataMap::getInputSharedDataNames(const std::string &aStageTag, const std::string &aDataType) const
{
    std::vector<std::string> tSharedDataNames;
    
    auto tStageMapItr = mMapFromStageTagToMapFromInputDataTagToSharedDataNames.find(aStageTag);
    if (tStageMapItr != mMapFromStageTagToMapFromInputDataTagToSharedDataNames.end())
    {
        auto tLowerDataType = Plato::tolower(aDataType);
        auto tDataItr = tStageMapItr->second.find(tLowerDataType);
        if (tDataItr != tStageMapItr->second.end())
        {
            tSharedDataNames = tDataItr->second;
        }
    }
    return tSharedDataNames;
}

bool DakotaDataMap::stageHasInputSharedData(const std::string &aStageTag) const
{
    auto tStageMapItr = mMapFromStageTagToMapFromInputDataTagToSharedDataNames.find(aStageTag);
    if (tStageMapItr == mMapFromStageTagToMapFromInputDataTagToSharedDataNames.end())
    {
        return false;
    }
    return true;
}

std::vector<std::string> DakotaDataMap::getOutputSharedDataNames(const std::string &aStageTag) const
{
    std::vector<std::string> tSharedDataNames;
    auto tItr = mMapFromStageTagToOutputSharedDataNames.find(aStageTag);
    if (tItr != mMapFromStageTagToOutputSharedDataNames.end())
    {
        tSharedDataNames = tItr->second;
    }
    return tSharedDataNames;
}

bool DakotaDataMap::stageHasOutputSharedData(const std::string &aStageTag) const
{
    auto tItr = mMapFromStageTagToOutputSharedDataNames.find(aStageTag);
    if (tItr == mMapFromStageTagToOutputSharedDataNames.end())
    {
        return false;
    }
    return true;
}

std::string DakotaDataMap::getStageName(const std::string &aStageTag) const
{
    auto tItr = mMapFromStageTagToStageName.find(aStageTag);
    if (tItr == mMapFromStageTagToStageName.end())
    {
        return "";
    }
    return tItr->second;
}

void DakotaDataMap::setContinuousVarsSharedData(const std::string &aSharedDataName, const size_t aNumValues, const Dakota::RealVector & aData)
{
    if (mMapFromSharedDataNameToContinuousVars[aSharedDataName].empty())
    {
        mMapFromSharedDataNameToContinuousVars[aSharedDataName].resize(aNumValues);
    }

    if(mMapFromSharedDataNameToContinuousVars[aSharedDataName].size() < aNumValues)
        throw std::out_of_range(ERRMSG("Number of stored continuous variables smaller than requested size"));

    if(aData.length() < aNumValues)
        throw std::out_of_range(ERRMSG("Expected number of stored variables is larger than input vector size"));

    for (size_t tIndex = 0; tIndex < aNumValues; tIndex++)
    {
        mMapFromSharedDataNameToContinuousVars[aSharedDataName][tIndex] = aData[tIndex];
    }
}

std::vector<Dakota::Real> & DakotaDataMap::getContinuousVarsSharedData(const std::string &aSharedDataName) 
{
    return mMapFromSharedDataNameToContinuousVars[aSharedDataName];
}

void DakotaDataMap::setDiscreteRealVarsSharedData(const std::string &aSharedDataName, const size_t aNumValues, const Dakota::RealVector & aData)
{
    if (mMapFromSharedDataNameToDiscreteRealVars[aSharedDataName].empty())
    {
        mMapFromSharedDataNameToDiscreteRealVars[aSharedDataName].resize(aNumValues);
    }

    if(mMapFromSharedDataNameToDiscreteRealVars[aSharedDataName].size() < aNumValues)
        throw std::out_of_range(ERRMSG("Number of stored discrete real variables smaller than requested size"));

    if(aData.length() < aNumValues)
        throw std::out_of_range(ERRMSG("Expected number of stored variables is larger than input vector size"));

    for (size_t tIndex = 0; tIndex < aNumValues; tIndex++)
    {
        mMapFromSharedDataNameToDiscreteRealVars[aSharedDataName][tIndex] = aData[tIndex];
    }
}

std::vector<Dakota::Real> & DakotaDataMap::getDiscreteRealVarsSharedData(const std::string &aSharedDataName)
{
    return mMapFromSharedDataNameToDiscreteRealVars[aSharedDataName];
}

void DakotaDataMap::setDiscreteIntegerVarsSharedData(const std::string &aSharedDataName, const size_t aNumValues, const Dakota::IntVector & aData)
{
    if (mMapFromSharedDataNameToDiscreteIntegerVars[aSharedDataName].empty())
    {
        mMapFromSharedDataNameToDiscreteIntegerVars[aSharedDataName].resize(aNumValues);
    }

    if(mMapFromSharedDataNameToDiscreteIntegerVars[aSharedDataName].size() < aNumValues)
        throw std::out_of_range(ERRMSG("Number of stored discrete integer variables smaller than requested size"));

    if(aData.length() < aNumValues)
        throw std::out_of_range(ERRMSG("Expected number of stored variables is larger than input vector size"));

    for (size_t tIndex = 0; tIndex < aNumValues; tIndex++)
    {
        mMapFromSharedDataNameToDiscreteIntegerVars[aSharedDataName][tIndex] = aData[tIndex];
    }
}

std::vector<Dakota::Real> & DakotaDataMap::getDiscreteIntegerVarsSharedData(const std::string &aSharedDataName) 
{
    return mMapFromSharedDataNameToDiscreteIntegerVars[aSharedDataName];
}

void DakotaDataMap::setOutputVarsSharedData(const std::string &aStageTag, const Plato::dakota::SharedDataMetaData & aMetaData, const size_t aIndex)
{
    if(mMapFromStageTagToOutputSharedDataMetadata[aStageTag].size() == 0)
        throw std::runtime_error(ERRMSG("Attempted to set data for stage tag \"" + aStageTag + "\" but no data allocation exists"));

    if(mMapFromStageTagToOutputSharedDataMetadata[aStageTag].size() <= aIndex)
        throw std::out_of_range(ERRMSG("Index out of range of data allocation for shared data \"" + aStageTag + "\""));

    mMapFromStageTagToOutputSharedDataMetadata[aStageTag][aIndex] = aMetaData;
}

Plato::dakota::SharedDataMetaData & DakotaDataMap::getOutputVarsSharedData(const std::string &aStageTag, const size_t aIndex)
{
    if(mMapFromStageTagToOutputSharedDataMetadata[aStageTag].size() == 0)
        throw std::runtime_error(ERRMSG("Attempted to get data for stage tag \"" + aStageTag + "\" but no data allocation exists"));

    if(mMapFromStageTagToOutputSharedDataMetadata[aStageTag].size() <= aIndex)
        throw std::out_of_range(ERRMSG("Index out of range of data allocation for shared data \"" + aStageTag + "\""));

    return mMapFromStageTagToOutputSharedDataMetadata[aStageTag][aIndex];
}

void DakotaDataMap::verifyStageTagsAreUnique(const std::string& aStageTag) const
{
    if(mMapFromStageTagToStageName.find(aStageTag) != mMapFromStageTagToStageName.end())
        throw std::runtime_error(ERRMSG("StageTag \"" + aStageTag + "\" is used more than once in DakotaDriver block"));
}

void DakotaDataMap::verifyStageNamesAreUnique(const std::string& aStageName) const
{
    for(auto tIter = mMapFromStageTagToStageName.begin(); tIter != mMapFromStageTagToStageName.end(); ++tIter)
    {
        if(tIter->second == aStageName)
            throw std::runtime_error(ERRMSG("StageName \"" + aStageName + "\" is used more than once in DakotaDriver block"));
    }
}

}
// namespace Plato
