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
 * Plato_Operation.cpp
 *
 *  Created on: March 30, 2018
 *
 */

#include <iostream>
#include <algorithm>
#include <sstream>

#include "Plato_Exceptions.hpp"
#include "Plato_Operation.hpp"
#include "Plato_Performer.hpp"
#include "Plato_SharedData.hpp"
#include "Plato_Utils.hpp"
#include "Plato_OperationInputDataMng.hpp"

namespace Plato {

/******************************************************************************/
void
Operation::
addArgument(const std::string & tArgumentName,
            const std::string & tSharedDataName,
            const std::vector<std::shared_ptr<Plato::SharedData>>& aSharedData,
            std::vector<std::shared_ptr<SharedData>>& aLocalData)
/******************************************************************************/
{
    bool tFoundData = false;
    for(const auto& tSharedData : aSharedData)
    {
        if(tSharedData->myName() == tSharedDataName)
        {
            m_argumentNames.insert({tSharedDataName, tArgumentName});
            addIfDoesNotExist(tSharedData, aLocalData);
            tFoundData = true;
            break;
        }
    }
    if(!tFoundData)
    {
        std::stringstream tErrorMessage;
        tErrorMessage << "Plato::Operation: requested field ('" << tSharedDataName << "') that doesn't exist.";
        throw Plato::ParsingException(tErrorMessage.str());
    }
}

void Operation::addIfDoesNotExist(const std::shared_ptr<Plato::SharedData>& aDataToAdd,
        std::vector<std::shared_ptr<Plato::SharedData>>& aLocalData)
{
    const bool tDataExists = std::any_of(aLocalData.cbegin(), aLocalData.cend(), 
    [&aDataToAdd](const std::shared_ptr<SharedData>& aCurData)
    { 
      return aCurData == aDataToAdd;
    });
    if(!tDataExists)
    {
      aLocalData.push_back(aDataToAdd);
    }
}

/******************************************************************************/
void
Operation::
sendInput()
/******************************************************************************/
{
  for( auto& sd : m_inputData )
    sd->transmitData();
}

/******************************************************************************/
void
Operation::
sendOutput()
/******************************************************************************/
{
  for( auto& sd : m_outputData )
    sd->transmitData();
}

/******************************************************************************/
void 
Operation::
sendParameters()
/******************************************************************************/
{
  for( const auto& sd : m_parameters)
  {
    sd.second->transmitData();
  }
}

/******************************************************************************/
std::vector<std::string>
Operation::
getInputDataNames() const
/******************************************************************************/
{
  std::vector<std::string> names;
  for(const auto& sf : m_inputData)
  {
    names.push_back(sf->myName());
  }
  return names;
}

/******************************************************************************/
std::vector<std::string>
Operation::
getOutputDataNames() const
/******************************************************************************/
{
  std::vector<std::string> names;
  for(const auto& sf : m_outputData)
  {
    names.push_back(sf->myName());
  }
  return names;
}

/******************************************************************************/
void
Operation::
compute()
/******************************************************************************/
{
  if(m_performer)
  {
     for( const auto& p : m_parameters )
     {
        m_performer->importData(p.first, *(p.second));
     }
     computeImpl();
  }
}

/******************************************************************************/
void
Operation::
computeImpl()
/******************************************************************************/
{
  m_performer->compute(m_operationName);
}

/******************************************************************************/
void
Operation::
importData(const std::string& aSharedDataName, SharedData& aImportData)
/******************************************************************************/
{
  if(m_performer){
    auto range = m_argumentNames.equal_range(aSharedDataName);
    for( auto it = range.first; it != range.second; ++it ){
      m_performer->importData(it->second, aImportData);
    }
  }
}

/******************************************************************************/
void
Operation::
exportData(const std::string& aSharedDataName, SharedData& aExportData)
/******************************************************************************/
{
  if(m_performer){
    auto range = m_argumentNames.equal_range(aSharedDataName);
    for( auto it = range.first; it != range.second; ++it ){
      m_performer->exportData(it->second, aExportData);
    }
  }
}

/******************************************************************************/
std::string
Operation::
getPerformerName() const
/******************************************************************************/
{
    if(m_performer){
        return m_performer->myName();
    } else {
        return "NO PERFORMER SET!";
    }
}

/******************************************************************************/
const std::string&
Operation::
getOperationName() const
/******************************************************************************/
{
    return m_operationName;
}

/******************************************************************************/
void Operation::setPerformer(std::shared_ptr<Performer> aPerformer)
/******************************************************************************/
{ 
    assert(aPerformer);
    if(m_performerName == aPerformer->myName())
    {
        m_performer = std::move(aPerformer);
        setComputeFunctionOnNewPerformer();
    }
}

/******************************************************************************/
bool Operation::hasParameter(const std::string& aParamName) const
/******************************************************************************/
{
    return m_parameters.count(aParamName) > 0;
}

/******************************************************************************/
/******************************************************************************/
} // End namespace Plato
