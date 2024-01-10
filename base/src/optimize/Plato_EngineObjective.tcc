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
 * Plato_EngineObjective.tcc
 *
 *  Created on: Dec 21, 2017
 */

#pragma once

#include <string>
#include <vector>

#include "Plato_Interface.hpp"
#include "Plato_Console.hpp"
#include "Plato_MultiVector.hpp"
#include "Plato_OptimizerFactory.hpp"

namespace Plato
{

/******************************************************************************//**
 * @brief Evaluate one or multiple third-party application objective functions
 * @param [in] aControl const reference to 2D container of optimization variables
**********************************************************************************/

template<typename ScalarType, typename OrdinalType>
ScalarType
Plato::EngineObjective<ScalarType, OrdinalType>::
value(const Plato::MultiVector<ScalarType, OrdinalType> & aControl)
{
    assert(mInterface != nullptr);

    // Check to see if there is a nested inner loop which should be
    // executed before the outer loop.
    if( this->mHasInnerLoop )
    {
        MPI_Comm tLocalComm;
        mInterface->getLocalComm(tLocalComm);

        // For the inner loop use the factory to create the optimizer.
        Plato::OptimizerFactory<ScalarType, OrdinalType> tOptimizerFactory;

        // Get the optimizer block index for this objective (i.e. the
        // outer loop). To read the first optimizer block for the
        // inner loop add an additional index, 0 which increases the
        // inner loop depth. By passing the specific optimizer index
        // to the factory the first inner optimizer will be created.
        std::vector< size_t > tOptimizerIndex = this->mOptimizerIndex;
        tOptimizerIndex.push_back(0);

        // Create the first inner loop optimizer.
        std::unique_ptr<Plato::OptimizerInterface<ScalarType, OrdinalType>>
            tOptimizer = tOptimizerFactory.create(mInterface, tLocalComm,
                                                  tOptimizerIndex );

        // Do the optimization and check for another serial optimizer.
        while( tOptimizer != nullptr )
        {
            tOptimizer->run();

            // Check for another serial optimizer block. Note the
            // index does not need to passed as the factory stores the
            // index of last optimizer block read and will
            // automatically increment it to get the next serial
            // optimizer block.
            tOptimizer = tOptimizerFactory.create(mInterface, tLocalComm);
        }
    }

    // Normal evaluation of the objective.

    // If there is a name then report what optimizer is being computed.
    if( !mOptimizerName.empty() )
    {
        std::stringstream tConsoleStream;
        tConsoleStream << "Computing Engine Objective for Optimizer : "
                       << this->mOptimizerName << "  "
                       << "Optimizer Index " << " {";

        for( size_t i=0; i<this->mOptimizerIndex.size(); ++i )
        {
            tConsoleStream << this->mOptimizerIndex[i];

            if( i < this->mOptimizerIndex.size()-1 )
              tConsoleStream << ", ";
            else
              tConsoleStream << "}";
        }

        Plato::Console::Alert(tConsoleStream.str());
    }

    // ********* Set view to each control vector entry ********* //
    this->setControls(aControl);

    // ********* Set view to objective function value ********* //
    std::string tObjectiveValueName =
      mEngineInputData.getObjectiveValueOutputName();
    ScalarType tObjectiveValue = 0;

    mParameterList->set(tObjectiveValueName, &tObjectiveValue);

    // ********* Compute objective function value ********* //
    std::string tMyStageName = mEngineInputData.getObjectiveValueStageName();
    assert(tMyStageName.empty() == false);

    std::vector<std::string> tStageNames;
    tStageNames.push_back(tMyStageName);

    mInterface->compute(tStageNames, *mParameterList);

    return (tObjectiveValue);
}

} // namespace Plato
