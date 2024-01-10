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
 * Plato_OptimizerFactory.hpp
 *
 *  Created on: April 19, 2017
 *
 */

#pragma once

#include <mpi.h>

#include "Plato_OptimizerInterface.hpp"
#include "Plato_SOParameterStudiesInterface.hpp"
#include "Plato_ROLInterface.hpp"
#include "Plato_StochasticROLInterface.hpp"

#include <memory>

namespace Plato
{

/******************************************************************************//**
 * \brief Construct interface to optimization algorithm
**********************************************************************************/
template<typename ScalarType, typename OrdinalType = size_t>
class OptimizerFactory final
{
public:
    OptimizerFactory() = default;
    OptimizerFactory(const Plato::OptimizerFactory<ScalarType, OrdinalType>&) = delete;
    Plato::OptimizerFactory<ScalarType, OrdinalType> & operator=(const Plato::OptimizerFactory<ScalarType, OrdinalType>&) = delete;
    OptimizerFactory(Plato::OptimizerFactory<ScalarType, OrdinalType>&&) = delete;
    Plato::OptimizerFactory<ScalarType, OrdinalType> & operator=(Plato::OptimizerFactory<ScalarType, OrdinalType>&&) = delete;

    /******************************************************************************//**
     * \brief Construct interface to optimization algorithm
     * \param [in] aInterface PLATO Engine interface
     * \param [in] aLocalComm local MPI communicator
     * \param [in] aOptimizerIndex the index of a specific optimizer block
     * \return non-const pointer to the optimization algorithm's interface
    **********************************************************************************/
    std::unique_ptr<Plato::OptimizerInterface<ScalarType, OrdinalType>>
    create(Plato::Interface* aInterface,
           MPI_Comm aLocalComm,
           std::vector< size_t > aOptimizerIndex = std::vector< size_t >()
           )
    {

      std::unique_ptr<Plato::OptimizerInterface<ScalarType, OrdinalType>> tOptimizer;
      try
      {
        Plato::InputData tOptimizerNode;

        // The optimizer block index is a vector of indices. The size
        // of the vector less 1 denotes the number of nesting
        // levels. Each index is the serial index of the optimizer
        // block.

        // A basic run with one optimizer block would have a vector of
        // {0}. A simple nested run with one nested optimizer block
        // would have a vector of {0} for the outer optimizer block
        // and {0,0} for the inner optimizer block.

        // A more complicated vector for the inner most optimizer
        // block would be {2,1,0} which denotes the outer most third
        // serial optimizer block {2}, of which its second serial
        // inner loop optimizer block is wanted {2,1}, of which its
        // first inner loop optimizer block is wanted {2,1,0},

        // Requesting a specific optimizer block via an argument.
        if( aOptimizerIndex.size() )
        {
            tOptimizerNode =
                Plato::getOptimizerNode(aInterface, aOptimizerIndex);

            if( tOptimizerNode.empty() )
            {
                std::stringstream tStringStream;
                tStringStream
                  << "Plato::OptimizerFactory: "
                  << "requesting an 'Optimizer' definition that does not exist";
                Plato::ParsingException tParsingException(tStringStream.str());
                aInterface->registerException(tParsingException);
            }
        }
        // A previous optimizer block index was specified. Increment
        // the last index which was stored and attempt to serially
        // read the next optimizer block.
        else if( mOptimizerIndex.size() )
        {
            aOptimizerIndex = mOptimizerIndex;

            ++aOptimizerIndex[aOptimizerIndex.size()-1];

            tOptimizerNode =
                Plato::getOptimizerNode(aInterface, aOptimizerIndex);

            if( tOptimizerNode.empty() )
                return (nullptr);
        }
        // No previously read optimizer so start a new index vector
        // and attempt to read the very first optimizer block.
        else
        {
            aOptimizerIndex.push_back(0);

            tOptimizerNode =
                Plato::getOptimizerNode(aInterface, aOptimizerIndex);

            if( tOptimizerNode.empty() )
            {
                std::stringstream tStringStream;
                tStringStream
                  << "Plato::OptimizerFactory: "
                  << "missing 'Optimizer' definition";
                Plato::ParsingException tParsingException(tStringStream.str());
                aInterface->registerException(tParsingException);
            }
        }

        // For the optimizer block get the package which denotes the
        // optimizer.
        std::string tOptPackage = Plato::Get::String(tOptimizerNode, "Package");

        if( tOptPackage == "SOParameterStudies" )
        {
          try {
            tOptimizer = std::make_unique<Plato::SOParameterStudiesInterface<ScalarType, OrdinalType>>(aInterface, aLocalComm);
          } catch(...){aInterface->Catch();}
        }
       else if( tOptPackage == "ROL AugmentedLagrangian" )
       {
         try {
          Plato::optimizer::algorithm_t tType = Plato::optimizer::algorithm_t::ROL_AUGMENTED_LAGRANGIAN;
           tOptimizer = std::make_unique<Plato::ROLInterface<ScalarType, OrdinalType>>(aInterface, aLocalComm,tType);
           
         } catch(...){aInterface->Catch();}
       }
       else if( tOptPackage == "ROL BoundConstrained" )
       {
         try {
           Plato::optimizer::algorithm_t tType = Plato::optimizer::algorithm_t::ROL_BOUND_CONSTRAINED;
           tOptimizer = std::make_unique<Plato::ROLInterface<ScalarType, OrdinalType>>(aInterface, aLocalComm,tType);
           
         } catch(...){aInterface->Catch();}
       }
       else if( tOptPackage == "ROL LinearConstraint" )
       {
         try {
           Plato::optimizer::algorithm_t tType = Plato::optimizer::algorithm_t::ROL_LINEAR_CONSTRAINT;
           tOptimizer = std::make_unique<Plato::ROLInterface<ScalarType, OrdinalType>>(aInterface, aLocalComm,tType);
         } catch(...){aInterface->Catch();}
       }
       else if( tOptPackage == "ROL Stochastic" )
       {
         try {
           Plato::optimizer::algorithm_t tType = Plato::optimizer::algorithm_t::ROL_LINEAR_CONSTRAINT;
           tOptimizer = std::make_unique<Plato::StochasticROLInterface<ScalarType, OrdinalType>>(aInterface, aLocalComm, tType);
         } catch(...){aInterface->Catch();}
       }
       else
       {
          std::stringstream tStringStream;
          tStringStream
            << "Plato::OptimizerFactory: "
            << tOptPackage << " Unknown." << std::endl
            << "Valid options are\n"
            << "\t SOParameterStudies ... Shape Optimization Parameter Study Toolkit\n"
            << "\t ROL AugmentedLagrangian... Rapid Optimization Library Augmented Lagrangian\n"
            << "\t ROL BoundConstrained... Rapid Optimization Library Bound Constrained\n"
            << "\t ROL LinearConstraint... Rapid Optimization Library LinearConstraint\n"
            << std::endl;

          // Dump a console message as exception handling cannot be
          // done because of the serial / recursive nature of the
          // optimizers.
          Console::Status("OptimizerFactory: " + tStringStream.str());

          throw Plato::ParsingException(tStringStream.str());
        }

        // Find the optional name - helpful when there are multiple
        // optimizer blocks.
        if( tOptimizerNode.size<std::string>("Name") )
        {
          std::string tName = Plato::Get::String(tOptimizerNode, "Name");

          tOptimizer->setOptimizerName( tName );
        }

        // Check to see if this optimizer is the last top level serial
        // optimizer which will issue a finialize/terminate operation.
        if( aOptimizerIndex.size() == 1 &&
            aOptimizerIndex[0] ==
            aInterface->getInputData().size<Plato::InputData>("Optimizer") - 1 )
        {
          tOptimizer->lastOptimizer( true );
        }

        // The index and inner loop boolean are used by the engine
        // objective to manage addtional optimizers.
        tOptimizer->setOptimizerIndex( aOptimizerIndex );

        const size_t nNestedOptimizers =
          tOptimizerNode.size<Plato::InputData>("Optimizer");

        tOptimizer->setHasInnerLoop(nNestedOptimizers > 0);

        // Store the index of the current optimizer block so to be
        // able to read additional serial optimizer blocks.
        mOptimizerIndex = aOptimizerIndex;

        const bool tSaveToXML = std::getenv("PLATO_SAVE_TO_XML") != nullptr;
      
        if(tSaveToXML)
        {
          Plato::saveToXML(*tOptimizer, Plato::XMLFileName{"save_optimizer.xml"}, Plato::XMLNodeName{"Optimizer"});
        }
      }

      catch(...)
      {
        aInterface->Catch();
        // Exception handling cannot be done because of the serial /
        // recursive nature of the optimizers so finalize (terminate).
        aInterface->finalize();
      }

      return tOptimizer;
    }

private:

    std::vector<size_t> mOptimizerIndex;
};
// class OptimizerFactory

} // namespace Plato

// With some of the the optimizer interfaces such as the MMA it is
// possible to have nested optimization. This nesting creates a
// circular dependency:
// OptimizerFactory->MMA->EngineObjective->OptimizerFactory
//
// Because of this circular dependency it is necessary to break from
// the traditional template class definition where everything is
// defined as part of the class definition in a single header file.
// Instead the EngineObjective::value method (which calls the
// OptimizerFactory) is defined and SEPARTELY and AFTER the
// OptimizerFactory class has been defined. As such, the definition of
// EngineObjective::value is included here.

#include "Plato_EngineObjective.tcc"
