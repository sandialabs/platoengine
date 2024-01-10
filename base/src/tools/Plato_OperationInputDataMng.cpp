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
 * Plato_OperationInputDataMng.cpp
 *
 *  Created on: Oct 10, 2017
 */

#include <cassert>

#include "Plato_OperationInputDataMng.hpp"

namespace Plato
{

/*****************************************************************************/
OperationInputDataMng::OperationInputDataMng() :
        mPerformerName(),
        mOperationMap(),
        mInputDataMap(),
        mOutputDataMap()
/*****************************************************************************/
{
}

/*****************************************************************************/
OperationInputDataMng::~OperationInputDataMng()
/*****************************************************************************/
{
}

/*****************************************************************************/
int OperationInputDataMng::getNumPerformers() const
/*****************************************************************************/
{
    assert(mPerformerName.size() == mOperationMap.size());
    return (mPerformerName.size());
}

/*****************************************************************************/
int OperationInputDataMng::getNumOperations() const
/*****************************************************************************/
{
    return (mOperationMap.size());
}

/*****************************************************************************/
bool OperationInputDataMng::hasSubOperations() const
/*****************************************************************************/
{
    bool tHasSubOperations = false;
    if(mPerformerName.size() > 1u)
    {
        tHasSubOperations = true;
    }
    return (tHasSubOperations);
}
/*****************************************************************************/
const std::string & OperationInputDataMng::getPerformerName() const
/*****************************************************************************/
{
    assert(mPerformerName.empty() == false);
    assert(1u == mPerformerName.size());
    const size_t tOPERATION_INDEX = 0;
    return (mPerformerName[tOPERATION_INDEX]);
}

/*****************************************************************************/
const std::string & OperationInputDataMng::getPerformerName(const int & aOperationIndex) const
/*****************************************************************************/
{
    assert(mPerformerName.empty() == false);
    assert(aOperationIndex >= static_cast<int>(0));
    assert(aOperationIndex < static_cast<int>(mPerformerName.size()));
    return (mPerformerName[aOperationIndex]);
}

/*****************************************************************************/
const std::string & OperationInputDataMng::getOperationName(const int & aOperationIndex) const
/*****************************************************************************/
{
    assert(mPerformerName.empty() == false);
    assert(aOperationIndex >= static_cast<int>(0));
    assert(aOperationIndex < static_cast<int>(mPerformerName.size()));
    const std::string & tPerformerName = mPerformerName[aOperationIndex];
    std::map<std::string, std::string>::const_iterator tIterator;
    tIterator = mOperationMap.find(tPerformerName);
    return (tIterator->second);
}

/*****************************************************************************/
const std::string & OperationInputDataMng::getOperationName(const std::string & aPerformerName) const
/*****************************************************************************/
{
    assert(mOperationMap.empty() == false);
    std::map<std::string, std::string>::const_iterator tIterator;
    tIterator = mOperationMap.find(aPerformerName);
    assert(tIterator != mOperationMap.end());
    return (tIterator->second);
}

/*****************************************************************************/
boost::optional<OperationType> OperationInputDataMng::getOperationType(const std::string & aPerformerName) const
/*****************************************************************************/
{
    // todo: We can probably handle the error checking of the optional on addInput
    return operationTypeIgnoreSpaces(getOperationName(aPerformerName));
}

/*****************************************************************************/
int OperationInputDataMng::getNumInputs(const int & aOperationIndex) const
/*****************************************************************************/
{
    assert(aOperationIndex >= static_cast<int>(0));
    assert(aOperationIndex < static_cast<int>(mPerformerName.size()));
    const std::string & tOperationName = mPerformerName[aOperationIndex];
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::const_iterator tIterator;
    tIterator = mInputDataMap.find(tOperationName);
    return (tIterator->second.size());
}

/*****************************************************************************/
int OperationInputDataMng::getNumInputs(const std::string & aPerformerName) const
/*****************************************************************************/
{
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::const_iterator tIterator;
    tIterator = mInputDataMap.find(aPerformerName);
    return (tIterator->second.size());
}

/*****************************************************************************/
const std::vector<std::pair<std::string, std::string>> & OperationInputDataMng::getInputs(const int & aOperationIndex) const
/*****************************************************************************/
{
    assert(mInputDataMap.empty() == false);
    assert(aOperationIndex >= static_cast<int>(0));
    assert(aOperationIndex < static_cast<int>(mPerformerName.size()));
    const std::string & tOperationName = mPerformerName[aOperationIndex];
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::const_iterator tIterator;
    tIterator = mInputDataMap.find(tOperationName);
    return (tIterator->second);
}

/*****************************************************************************/
const std::vector<std::pair<std::string, std::string>> & OperationInputDataMng::getInputs(const std::string & aPerformerName) const
/*****************************************************************************/
{
    assert(mInputDataMap.empty() == false);
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::const_iterator tIterator;
    tIterator = mInputDataMap.find(aPerformerName);
    assert(tIterator != mInputDataMap.end());
    return (tIterator->second);
}

/*****************************************************************************/
const std::string & OperationInputDataMng::getInputSharedData(const int & aOperationIndex, const int & aInputDataIndex) const
/*****************************************************************************/
{
    assert(mInputDataMap.empty() == false);
    assert(aOperationIndex >= static_cast<int>(0));
    assert(aOperationIndex < static_cast<int>(mPerformerName.size()));
    const std::string & tOperationName = mPerformerName[aOperationIndex];
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::const_iterator tIterator;
    tIterator = mInputDataMap.find(tOperationName);
    assert(aInputDataIndex >= static_cast<int>(0));
    assert(aInputDataIndex < static_cast<int>(tIterator->second.size()));
    return (tIterator->second[aInputDataIndex].first);
}

/*****************************************************************************/
const std::string & OperationInputDataMng::getInputSharedData(const std::string & aPerformerName, const int & aInputDataIndex) const
/*****************************************************************************/
{
    assert(mInputDataMap.empty() == false);
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::const_iterator tIterator;
    tIterator = mInputDataMap.find(aPerformerName);
    assert(tIterator != mInputDataMap.end());
    assert(aInputDataIndex >= static_cast<int>(0));
    assert(aInputDataIndex < static_cast<int>(tIterator->second.size()));
    return (tIterator->second[aInputDataIndex].first);
}

/*****************************************************************************/
const std::string & OperationInputDataMng::getInputArgument(const int & aOperationIndex, const int & aInputDataIndex) const
/*****************************************************************************/
{
    assert(mInputDataMap.empty() == false);
    assert(aOperationIndex >= static_cast<int>(0));
    assert(aOperationIndex < static_cast<int>(mPerformerName.size()));
    const std::string & tOperationName = mPerformerName[aOperationIndex];
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::const_iterator tIterator;
    tIterator = mInputDataMap.find(tOperationName);
    assert(aInputDataIndex >= static_cast<int>(0));
    assert(aInputDataIndex < static_cast<int>(tIterator->second.size()));
    return (tIterator->second[aInputDataIndex].second);
}
/*****************************************************************************/
const std::string & OperationInputDataMng::getInputArgument(const std::string & aPerformerName, const int & aInputDataIndex) const
/*****************************************************************************/
{
    assert(mInputDataMap.empty() == false);
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::const_iterator tIterator;
    tIterator = mInputDataMap.find(aPerformerName);
    assert(tIterator != mInputDataMap.end());
    assert(aInputDataIndex >= static_cast<int>(0));
    assert(aInputDataIndex < static_cast<int>(tIterator->second.size()));
    return (tIterator->second[aInputDataIndex].second);
}

/*****************************************************************************/
int OperationInputDataMng::getNumOutputs(const int & aOperationIndex) const
/*****************************************************************************/
{
    assert(aOperationIndex >= static_cast<int>(0));
    assert(aOperationIndex < static_cast<int>(mPerformerName.size()));
    const std::string & tOperationName = mPerformerName[aOperationIndex];
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::const_iterator tIterator;
    tIterator = mOutputDataMap.find(tOperationName);
    return (tIterator->second.size());
}

/*****************************************************************************/
int OperationInputDataMng::getNumOutputs(const std::string & aPerformerName) const
/*****************************************************************************/
{
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::const_iterator tIterator;
    tIterator = mOutputDataMap.find(aPerformerName);
    return (tIterator->second.size());
}

/*****************************************************************************/
const std::vector<std::pair<std::string, std::string>> & OperationInputDataMng::getOutputs(const int & aOperationIndex) const
/*****************************************************************************/
{
    assert(mOutputDataMap.empty() == false);
    assert(aOperationIndex >= static_cast<int>(0));
    assert(aOperationIndex < static_cast<int>(mPerformerName.size()));
    const std::string & tOperationName = mPerformerName[aOperationIndex];
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::const_iterator tIterator;
    tIterator = mOutputDataMap.find(tOperationName);
    return (tIterator->second);
}

/*****************************************************************************/
const std::vector<std::pair<std::string, std::string>> & OperationInputDataMng::getOutputs(const std::string & aPerformerName) const
/*****************************************************************************/
{
    assert(mOutputDataMap.empty() == false);
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::const_iterator tIterator;
    tIterator = mOutputDataMap.find(aPerformerName);
    assert(tIterator != mOutputDataMap.end());
    return (tIterator->second);
}

/*****************************************************************************/
const std::string & OperationInputDataMng::getOutputSharedData(const int & aOperationIndex, const int & aInputDataIndex) const
/*****************************************************************************/
{
    assert(mOutputDataMap.empty() == false);
    assert(aOperationIndex >= static_cast<int>(0));
    assert(aOperationIndex < static_cast<int>(mPerformerName.size()));
    const std::string & tOperationName = mPerformerName[aOperationIndex];
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::const_iterator tIterator;
    tIterator = mOutputDataMap.find(tOperationName);
    assert(aInputDataIndex >= static_cast<int>(0));
    assert(aInputDataIndex < static_cast<int>(tIterator->second.size()));
    return (tIterator->second[aInputDataIndex].first);
}

/*****************************************************************************/
const std::string & OperationInputDataMng::getOutputSharedData(const std::string & aPerformerName, const int & aInputDataIndex) const
/*****************************************************************************/
{
    assert(mOutputDataMap.empty() == false);
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::const_iterator tIterator;
    tIterator = mOutputDataMap.find(aPerformerName);
    assert(tIterator != mOutputDataMap.end());
    assert(aInputDataIndex >= static_cast<int>(0));
    assert(aInputDataIndex < static_cast<int>(tIterator->second.size()));
    return (tIterator->second[aInputDataIndex].first);
}

/*****************************************************************************/
const std::string & OperationInputDataMng::getOutputArgument(const int & aOperationIndex, const int & aInputDataIndex) const
/*****************************************************************************/
{
    assert(mOutputDataMap.empty() == false);
    assert(aOperationIndex >= static_cast<int>(0));
    assert(aOperationIndex < static_cast<int>(mPerformerName.size()));
    const std::string & tOperationName = mPerformerName[aOperationIndex];
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::const_iterator tIterator;
    tIterator = mOutputDataMap.find(tOperationName);
    assert(aInputDataIndex >= static_cast<int>(0));
    assert(aInputDataIndex < static_cast<int>(tIterator->second.size()));
    return (tIterator->second[aInputDataIndex].second);
}

/*****************************************************************************/
const std::string & OperationInputDataMng::getOutputArgument(const std::string & aPerformerName, const int & aInputDataIndex) const
/*****************************************************************************/
{
    assert(mOutputDataMap.empty() == false);
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::const_iterator tIterator;
    tIterator = mOutputDataMap.find(aPerformerName);
    assert(tIterator != mOutputDataMap.end());
    assert(aInputDataIndex >= static_cast<int>(0));
    assert(aInputDataIndex < static_cast<int>(tIterator->second.size()));
    return (tIterator->second[aInputDataIndex].second);
}

/*****************************************************************************/
void OperationInputDataMng::addInput(const std::string & aPerformerName,
                                     const std::string & aOperationName,
                                     const std::string & aSharedDataName,
                                     const std::string & aArgumentName)
/*****************************************************************************/
{
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::iterator tIterator;
    tIterator = mInputDataMap.find(aPerformerName);
    if(tIterator == mInputDataMap.end())
    {
        mPerformerName.push_back(aPerformerName);
        mOperationMap[aPerformerName] = aOperationName;

        tIterator->second.push_back(std::make_pair(aSharedDataName, aArgumentName));

        std::vector<std::pair<std::string, std::string>> tOutputData;
        mOutputDataMap[aPerformerName] = tOutputData;
    }
    else
    {
        tIterator->second.push_back(std::make_pair(aSharedDataName, aArgumentName));
    }
}

/*****************************************************************************/
void OperationInputDataMng::addInputs(const std::string & aPerformerName,
                                      const std::string & aOperationName,
                                      const std::vector<std::string> & aSharedDataNames,
                                      const std::vector<std::string> & aArgumentNames)
/*****************************************************************************/
{
    assert(aSharedDataNames.size() == aArgumentNames.size());

    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::iterator tIterator;
    tIterator = mInputDataMap.find(aPerformerName);
    if(tIterator == mInputDataMap.end())
    {
        mPerformerName.push_back(aPerformerName);
        mOperationMap[aPerformerName] = aOperationName;

        std::vector<std::pair<std::string, std::string>> tInputData;
        for(size_t tDataIndex = 0; tDataIndex < aArgumentNames.size(); tDataIndex++)
        {
            const std::string & tArgumentName = aArgumentNames[tDataIndex];
            const std::string & tSharedDataName = aSharedDataNames[tDataIndex];
            tInputData.push_back(std::make_pair(tSharedDataName, tArgumentName));
        }
        mInputDataMap[aPerformerName] = tInputData;

        std::vector<std::pair<std::string, std::string>> tOutputData;
        mOutputDataMap[aPerformerName] = tOutputData;
    }
    else
    {
        for(size_t tDataIndex = 0; tDataIndex < aArgumentNames.size(); tDataIndex++)
        {
            const std::string & tArgumentName = aArgumentNames[tDataIndex];
            const std::string & tSharedDataName = aSharedDataNames[tDataIndex];
            tIterator->second.push_back(std::make_pair(tSharedDataName, tArgumentName));
        }
    }
}

/*****************************************************************************/
void OperationInputDataMng::addOutput(const std::string & aPerformerName,
                                      const std::string & aOperationName,
                                      const std::string & aSharedDataName,
                                      const std::string & aArgumentName)
/*****************************************************************************/
{
    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::iterator tIterator;
    tIterator = mOutputDataMap.find(aPerformerName);
    if(tIterator == mOutputDataMap.end())
    {
        mPerformerName.push_back(aPerformerName);
        mOperationMap[aPerformerName] = aOperationName;

        tIterator->second.push_back(std::make_pair(aSharedDataName, aArgumentName));

        std::vector<std::pair<std::string, std::string>> tInputData;
        mInputDataMap[aPerformerName] = tInputData;
    }
    else
    {
        tIterator->second.push_back(std::make_pair(aSharedDataName, aArgumentName));
    }
}

/*****************************************************************************/
void OperationInputDataMng::addOutputs(const std::string & aPerformerName,
                                       const std::string & aOperationName,
                                       const std::vector<std::string> & aSharedDataNames,
                                       const std::vector<std::string> & aArgumentNames)
/*****************************************************************************/
{
    assert(aSharedDataNames.size() == aArgumentNames.size());

    std::map<std::string, std::vector<std::pair<std::string, std::string>>>::iterator tIterator;
    tIterator = mOutputDataMap.find(aPerformerName);
    if(tIterator == mOutputDataMap.end())
    {
        mPerformerName.push_back(aPerformerName);
        mOperationMap[aPerformerName] = aOperationName;

        std::vector<std::pair<std::string, std::string>> tOutputData;
        for(size_t tDataIndex = 0; tDataIndex < aArgumentNames.size(); tDataIndex++)
        {
            const std::string & tArgumentName = aArgumentNames[tDataIndex];
            const std::string & tSharedDataName = aSharedDataNames[tDataIndex];
            tOutputData.push_back(std::make_pair(tSharedDataName, tArgumentName));
        }
        mOutputDataMap[aPerformerName] = tOutputData;

        std::vector<std::pair<std::string, std::string>> tInputData;
        mInputDataMap[aPerformerName] = tInputData;
    }
    else
    {
        for(size_t tDataIndex = 0; tDataIndex < aArgumentNames.size(); tDataIndex++)
        {
            const std::string & tArgumentName = aArgumentNames[tDataIndex];
            const std::string & tSharedDataName = aSharedDataNames[tDataIndex];
            tIterator->second.push_back(std::make_pair(tSharedDataName, tArgumentName));
        }
    }
}

}
