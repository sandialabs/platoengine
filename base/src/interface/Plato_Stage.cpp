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
 * Plato_Stage.cpp
 *
 *  Created on: April 24, 2017
 *
 */

#include "Plato_Stage.hpp"
#include "Plato_Operation.hpp"
#include "Plato_OperationFactory.hpp"
#include "Plato_SharedData.hpp"
#include "Plato_Performer.hpp"
#include "Plato_Exceptions.hpp"
#include "Plato_Utils.hpp"
#include "Plato_StageInputDataMng.hpp"
#include "Plato_OperationInputDataMng.hpp"

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>

namespace Plato
{
namespace{
std::vector<std::string> getSharedDataNames(const std::vector<std::shared_ptr<SharedData>>& aSharedData)
{
    std::vector<std::string> tNames;
    std::transform(aSharedData.cbegin(), aSharedData.cend(), std::back_inserter(tNames), 
    [](const std::shared_ptr<SharedData>& tSharedData)
    {
        return tSharedData->myName();
    });
    return tNames;
}   
}

/******************************************************************************/
Stage::Stage(const Plato::StageInputDataMng & aStageInputData,
             const std::shared_ptr<Plato::Performer> aPerformer,
             const std::vector<std::shared_ptr<Plato::SharedData>>& aSharedData) :
        m_name(aStageInputData.getStageName())
/******************************************************************************/
{
    initializeSharedData(aStageInputData, aSharedData);

    const int tNumOperations = aStageInputData.getNumOperations(m_name);
    for(int tOperationIndex = 0; tOperationIndex < tNumOperations; tOperationIndex++)
    {
        const Plato::OperationInputDataMng & tOperationDataMng = aStageInputData.getOperationInputData(m_name, tOperationIndex);
        m_operations.push_back(OperationFactory::create(tOperationDataMng, aPerformer, aSharedData));
    }
}

/******************************************************************************/
void Stage::update(const Plato::StageInputDataMng & aStageInputData,
		   const std::shared_ptr<Plato::Performer> aPerformer,
		   const std::vector<std::shared_ptr<Plato::SharedData>>& aSharedData)
/******************************************************************************/
{
    // If the shared data is recreated then the stage and its
    // operations must be updated so to have the new links to the
    // shared data.

    // Clear the input and output data.
    m_inputData.clear();
    m_outputData.clear();

    initializeSharedData(aStageInputData, aSharedData);

    // Update the operations.
    const size_t num_operations = m_operations.size();
    for(size_t tOperationIndex = 0u; tOperationIndex < num_operations; tOperationIndex++)
    {
        const Plato::OperationInputDataMng & tOperationDataMng =
	        aStageInputData.getOperationInputData(m_name, tOperationIndex);

        m_operations[tOperationIndex]->update(tOperationDataMng, aPerformer, aSharedData);
    }
}

/******************************************************************************/
void Stage::initializeSharedData(const Plato::StageInputDataMng & aStageInputData,
                                 const std::vector<std::shared_ptr<Plato::SharedData>>& aSharedData)
/******************************************************************************/
{
    // Get the input shared data.
    int tNumInputs = aStageInputData.getNumInputs();
    for(int tInputIndex = 0; tInputIndex < tNumInputs; tInputIndex++)
    {
        std::string tSharedDataName = aStageInputData.getInput(m_name, tInputIndex);
        std::shared_ptr<Plato::SharedData> tSharedData = Utils::byName(aSharedData, tSharedDataName);
        if(tSharedData)
        {
            m_inputData.push_back(tSharedData);
        }
        else
        {
            std::stringstream tErrorMessage;
            tErrorMessage << "While parsing Stage '" << m_name << "', requested SharedData ('" << tSharedDataName << "') which doesn't exist.";
            throw Plato::ParsingException(tErrorMessage.str());
        }
    }

    // Get the output shared data.
    int tNumOutputs = aStageInputData.getNumOutputs();
    for(int tOutputIndex = 0; tOutputIndex < tNumOutputs; tOutputIndex++)
    {
        std::string tSharedDataName = aStageInputData.getOutput(m_name, tOutputIndex);
        std::shared_ptr<Plato::SharedData> tSharedData = Utils::byName(aSharedData, tSharedDataName);
        if(tSharedData)
        {
            m_outputData.push_back(std::move(tSharedData));
        }
        else
        {
            std::stringstream tErrorMessage;
            tErrorMessage << "While parsing Stage '" << m_name << "', requested SharedData ('" << tSharedDataName << "') which doesn't exist.";
            throw Plato::ParsingException(tErrorMessage.str());
        }
    }
}

/******************************************************************************/
std::vector<std::string> Stage::getInputDataNames() const
/******************************************************************************/
{
    return getSharedDataNames(m_inputData);
}

/******************************************************************************/
std::vector<std::string> Stage::getOutputDataNames() const
/******************************************************************************/
{
    return getSharedDataNames(m_outputData);
}

/******************************************************************************/
void Stage::begin()
/******************************************************************************/
{
    for(auto& tSharedData : m_inputData)
    {
        tSharedData->transmitData();
    }
    // reset to first operation
    currentOperationIndex = 0;
}

/******************************************************************************/
Plato::Operation*
Stage::getNextOperation()
/******************************************************************************/
{
    Plato::Operation* tOperation = nullptr;
    if(static_cast<int>(m_operations.size()) > currentOperationIndex)
    {
        tOperation = m_operations[currentOperationIndex].get();
    }
    currentOperationIndex++;
    return tOperation;
}

/******************************************************************************/
void Stage::end()
/******************************************************************************/
{
    for(auto& tSharedData : m_outputData)
    {
        tSharedData->transmitData();
    }
}

/******************************************************************************/
void Stage::setPerformerOnOperations(std::shared_ptr<Performer> aPerformer)
/******************************************************************************/
{
    assert(aPerformer);
    for(auto& operation : m_operations)
    {
        if(operation)
        {
            operation->setPerformer(std::move(aPerformer));
        }
    }
}

bool Stage::hasParameter(const std::string& aParameterName) const
{
    return std::any_of(m_operations.cbegin(), m_operations.cend(), 
    [&aParameterName](const std::unique_ptr<Operation>& aOperation)
    {
        return aOperation->hasParameter(aParameterName);
    });
}

} // End namespace Plato
