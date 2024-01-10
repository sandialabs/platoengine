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
 * Plato_DriverFactory.hpp
 *
 *  Created on: April 19, 2017
 *
 */

#pragma once

#include <mpi.h>

#include "Plato_DriverInterface.hpp"

#include "Plato_OptimizerFactory.hpp"
#include "Plato_DakotaFactory.hpp"

namespace Plato
{

/******************************************************************************//**
 * \brief Construct interface to optimization algorithm
**********************************************************************************/
template<typename ScalarType, typename OrdinalType = size_t>
class DriverFactory
{
public:
    DriverFactory() = default;

    /******************************************************************************//**
     * \brief Construct interface to optimization algorithm
     * \param [in] aInterface PLATO Engine interface
     * \param [in] aLocalComm local MPI communicator
     * \param [in] aOptimizerIndex the index of a specific driver block
     * \return non-const pointer to the optimization algorithm's interface
    **********************************************************************************/
    std::unique_ptr<Plato::DriverInterface<ScalarType, OrdinalType>>
    create(Plato::Interface* aInterface,
           MPI_Comm aLocalComm,
           std::vector< size_t > /*aOptimizerIndex*/ = std::vector< size_t >()
           )
    {
      // Look for an Optimizer Driver.
      if( aInterface->getInputData().size<Plato::InputData>("Optimizer") >= 1 )
      {
        Plato::OptimizerFactory<ScalarType, OrdinalType> tOptimizerFactory;

        return tOptimizerFactory.create(aInterface, aLocalComm);
      }
      // Look for a Dakota Driver.
      else if( aInterface->getInputData().size<Plato::InputData>("DakotaDriver") >= 1 )
      {
        Plato::DakotaFactory<ScalarType, OrdinalType> tDakotaFactory;

        return tDakotaFactory.create(aInterface, aLocalComm);
      }

      // NEW DRIVERS GO HERE

      else
      {
        std::stringstream tStringStream;
        tStringStream
          << "Plato::DriverFactory: "
            << "No driver block found." << std::endl
            << "Valid options are\n"
            << "\t 'Optimizer' ... Plato driven by the Optimizer Driver\n"
#ifdef DAKOTAOPTIMIZER
            << "\t 'DakotaDriver' ... Plato driven by the Dakota Driver\n"
#endif
            << std::endl;

        Plato::ParsingException tParsingException(tStringStream.str());
        aInterface->registerException(tParsingException);
      }

      return nullptr;
    }
};
// class DriverFactory

} // namespace Plato
