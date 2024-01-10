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
 * Plato_Interface.cpp
 *
 *  Created on: March 20, 2017
 *
 */

// TODO:
// 4.  Implement shared pointers
// 5.  Performer stdout.  Redirect to file? to stringstream then communicate?
// 6.  Ndof SharedField
#include <cstdlib>
#include <stdlib.h>

#include "Plato_Interface.hpp"
#include "Plato_SharedField.hpp"
#include "Plato_SharedValue.hpp"
#include "Plato_Exceptions.hpp"
#include "Plato_Communication.hpp"
#include "Plato_SharedDataInfo.hpp"

namespace Plato
{

/******************************************************************************/
DataLayer::DataLayer(const Plato::SharedDataInfo & aSharedDataInfo, const Plato::CommunicationData & aCommData) :
        mSharedData(),
        mSharedDataMap()
/******************************************************************************/
{
    // create the shared fields
    //
    const int tNumSharedData = aSharedDataInfo.getNumSharedData();
    for(int tIndex = 0; tIndex < tNumSharedData; tIndex++)
    {
        std::string tMyName = aSharedDataInfo.getSharedDataName(tIndex);
        std::string tMyLayout = aSharedDataInfo.getSharedDataLayout(tIndex);

        std::shared_ptr<SharedData> tNewData;
        if(tMyLayout == "NODAL FIELD" )
        {
            const Plato::communication::broadcast_t tBroadcastType = aSharedDataInfo.getMyBroadcast(tIndex);
            tNewData = std::make_shared<SharedField>(tMyName, tBroadcastType, aCommData,  Plato::data::layout_t::SCALAR_FIELD);
        }
        else
        if(tMyLayout == "ELEMENT FIELD")
        {
            const Plato::communication::broadcast_t tBroadcastType = aSharedDataInfo.getMyBroadcast(tIndex);
            tNewData = std::make_shared<SharedField>(tMyName, tBroadcastType, aCommData, Plato::data::layout_t::ELEMENT_FIELD);
        }
        else
        if(tMyLayout == "GLOBAL")
        {
            const int tSize = aSharedDataInfo.getSharedDataSize(tMyName);
            const int tIsDynamic = aSharedDataInfo.getSharedDataDynamic(tMyName);
            assert(tSize > static_cast<int>(0));
            const std::vector<std::string>& tMyProviderNames = aSharedDataInfo.getProviderNames(tIndex);
            tNewData = std::make_shared<SharedValue>(tMyName, tMyProviderNames, aCommData, Plato::data::layout_t::SCALAR, tSize, tIsDynamic);
        }
        else
        if(tMyLayout == "PARAMETER")
        {
            const int tSize = aSharedDataInfo.getSharedDataSize(tMyName);
            if(tSize != 1)
            {
                std::cout << "Warning: Shared data parameter " << tMyName << " requested size " << tSize << ", but must be size 1." << std::endl;
            }
            const std::vector<std::string>& tMyProviderNames = aSharedDataInfo.getProviderNames(tIndex);
            tNewData = std::make_shared<SharedValue>(tMyName, tMyProviderNames, aCommData, Plato::data::layout_t::SCALAR_PARAMETER);
        }
        else
        {
            // TODO: THROW
            std::stringstream ss;
            ss << "Plato::DataLayer: Given unknown SharedData Layout ('" << tMyLayout << "').";
            throw ParsingException(ss.str());
        }

        mSharedData.push_back(tNewData);
        mSharedDataMap[tMyName] = tNewData;
    }
}

bool DataLayer::hasSharedData(const std::string& aName) const
{
    return mSharedDataMap.find(aName) != mSharedDataMap.end();
}

/******************************************************************************/
SharedData& DataLayer::getSharedData(const std::string & aName) const
/******************************************************************************/
{
    auto tIterator = mSharedDataMap.find(aName);
    if(tIterator != mSharedDataMap.end())
    {
        std::shared_ptr<SharedData> tData = tIterator->second.lock();
        assert(tData);
        return *tData;
    }
    else
    {
        std::stringstream ss;
        ss << "Plato::DataLayer: request for SharedData ('" << aName << "') that doesn't exist.";
        throw ParsingException(ss.str());
    }
}

/******************************************************************************/
const std::vector<std::shared_ptr<SharedData>> & DataLayer::getSharedData() const
/******************************************************************************/
{
    return mSharedData;
}

/******************************************************************************/
void DataLayer::initializeMPI(const Plato::CommunicationData& aCommData)
/******************************************************************************/
{
    for(auto& tSharedData : mSharedData)
    {
        tSharedData->initializeMPI(aCommData);
    }
}

} /* namespace Plato */
