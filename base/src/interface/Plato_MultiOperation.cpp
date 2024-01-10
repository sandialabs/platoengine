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
 * Plato_MultiOperation.cpp
 *
 *  Created on: March 30, 2018
 *
 */

#include <iostream>
#include <algorithm>
#include <sstream>

#include "Plato_Exceptions.hpp"
#include "Plato_MultiOperation.hpp"
#include "Plato_Performer.hpp"
#include "Plato_SharedData.hpp"
#include "Plato_Utils.hpp"
#include "Plato_OperationInputDataMng.hpp"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
BOOST_CLASS_EXPORT_IMPLEMENT(Plato::MultiOperation)

namespace Plato {

/******************************************************************************/
MultiOperation::
MultiOperation(const Plato::OperationInputDataMng & aOperationDataMng,
               const std::shared_ptr<Plato::Performer> aPerformer,
               const std::vector<std::shared_ptr<Plato::SharedData>>& aSharedData)
/******************************************************************************/
{
    initialize(aOperationDataMng, aPerformer, aSharedData);
}

/******************************************************************************/
void
MultiOperation::
initialize(const Plato::OperationInputDataMng & aOperationDataMng,
           const std::shared_ptr<Plato::Performer> aPerformer,
           const std::vector<std::shared_ptr<Plato::SharedData>>& aSharedData)
/******************************************************************************/
{
    // Clear local data
    m_performer = nullptr;
    m_parameters.clear();
    m_inputData.clear();
    m_outputData.clear();
    m_argumentNames.clear();

    const int tNumSubOperations = aOperationDataMng.getNumOperations();
    for(int tSubOperationIndex = 0; tSubOperationIndex < tNumSubOperations; tSubOperationIndex++)
    {
        const std::string tPerformerName =
          aOperationDataMng.getPerformerName(tSubOperationIndex);
        const std::string& tOperationName = aOperationDataMng.getOperationName(tPerformerName);

        auto tAllParamsData = aOperationDataMng.get<Plato::InputData>("Parameters");
        if( tAllParamsData.size<Plato::InputData>(tPerformerName) )
        {
            auto tParamsData = tAllParamsData.get<Plato::InputData>(tPerformerName);
            for( auto tParamData : tParamsData.getByName<Plato::InputData>("Parameter") )
            {
                auto tArgName  = Plato::Get::String(tParamData,"ArgumentName");
                auto tArgValue = Plato::Get::Double(tParamData,"ArgumentValue");
                if(std::shared_ptr<Plato::SharedData> tSharedData = Utils::byName(aSharedData, tArgName); tSharedData != nullptr)
                {
                    tSharedData->setMyContext(tOperationName);
                    m_parameters.emplace(std::move(tArgName), std::move(tSharedData));
                }
                else if(tPerformerName == aPerformer->myName())
                {
                    m_parameters.emplace(std::move(tArgName), std::make_shared<Parameter>(tArgName, tOperationName, tArgValue));
                }
            }
        }
    }

    // collect arrays of all input and output SharedData
    //
    for(int tSubOperationIndex = 0; tSubOperationIndex < tNumSubOperations; tSubOperationIndex++)
    {
        const std::string tPerformerName = aOperationDataMng.getPerformerName(tSubOperationIndex);

        // Get the input shared data.
        const int tNumInputs = aOperationDataMng.getNumInputs(tPerformerName);
        for(int tInputIndex = 0; tInputIndex < tNumInputs; tInputIndex++)
        {
            const std::string & tSharedDataName =
              aOperationDataMng.getInputSharedData(tPerformerName, tInputIndex);
            std::shared_ptr<Plato::SharedData> tSharedData = Utils::byName(aSharedData, tSharedDataName);
            if(tSharedData != nullptr)
            {
                addIfDoesNotExist(tSharedData, m_inputData);
            }
            else
            {
                std::stringstream tErrorMessage;
                tErrorMessage << "Plato::Operation: requested field ('" << tSharedDataName << "') that doesn't exist.";
                throw Plato::ParsingException(tErrorMessage.str());
            }
        }

        // Get the output shared data.
        const int tNumOutputs = aOperationDataMng.getNumOutputs(tPerformerName);
        for(int tOutputIndex = 0; tOutputIndex < tNumOutputs; tOutputIndex++)
        {
            const std::string & tSharedDataName =
              aOperationDataMng.getOutputSharedData(tPerformerName, tOutputIndex);
            std::shared_ptr<Plato::SharedData> tSharedData = Utils::byName(aSharedData, tSharedDataName);
            if(tSharedData != nullptr)
            {
                addIfDoesNotExist(tSharedData, m_outputData);
            }
            else
            {
                std::stringstream tErrorMessage;
                tErrorMessage << "Plato::Operation: requested field ('" << tSharedDataName << "') that doesn't exist.";
                throw Plato::ParsingException(tErrorMessage.str());
            }
        }
    }

    // Loop on the sub-operation specs to See if any of the locally available
    // performers are involved in this operation.
    //
    for(int tSubOperationIndex = 0; tSubOperationIndex < tNumSubOperations; tSubOperationIndex++)
    {
        const std::string tPerformerName =
          aOperationDataMng.getPerformerName(tSubOperationIndex);

        if(aPerformer->myName() == tPerformerName)
        {
            m_performer = aPerformer;
            m_performerName = tPerformerName;
        }

        if(m_performer != nullptr)
        {
            // A local performer is participating in this operation,
            // so parse the input and output argument multimaps
            //
            m_operationName = aOperationDataMng.getOperationName(tPerformerName);

            // Get the inputs.
            const int tNumInputs = aOperationDataMng.getNumInputs(tPerformerName);
            for(int tInputIndex = 0; tInputIndex < tNumInputs; tInputIndex++)
            {
                const std::string & tArgumentName = aOperationDataMng.getInputArgument(tPerformerName, tInputIndex);
                const std::string & tSharedDataName = aOperationDataMng.getInputSharedData(tPerformerName, tInputIndex);
                for(const auto& tSharedData : m_inputData)
                {
                    if(tSharedData->myName() == tSharedDataName)
                    {
                        m_argumentNames.insert(std::pair<std::string, std::string>(tSharedDataName, tArgumentName));
                        break;
                    }
                }
            }

            // Get the outputs.
            const int tNumOutputs = aOperationDataMng.getNumOutputs(tPerformerName);
            for(int tOutputIndex = 0; tOutputIndex < tNumOutputs; tOutputIndex++)
            {
                const std::string & tArgumentName = aOperationDataMng.getOutputArgument(tPerformerName, tOutputIndex);
                const std::string & tSharedDataName = aOperationDataMng.getOutputSharedData(tPerformerName, tOutputIndex);
                for(const auto& tSharedData : m_outputData)
                {
                    if(tSharedData->myName() == tSharedDataName)
                    {
                        m_argumentNames.insert(std::pair<std::string, std::string>(tSharedDataName, tArgumentName));
                        break;
                    }
                }
            }
            break;
        }
    }

}

/******************************************************************************/
void MultiOperation::
update(const Plato::OperationInputDataMng & aOperationDataMng,
       const std::shared_ptr<Plato::Performer> aPerformer,
       const std::vector<std::shared_ptr<Plato::SharedData>>& aSharedData)
/******************************************************************************/
{
    // If the shared data is recreated then the operation must be
    // updated so to have the new links to the shared data.
    initialize(aOperationDataMng, aPerformer, aSharedData);
}

} // End namespace Plato
