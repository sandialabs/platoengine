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

#ifdef DAKOTADRIVER
  #include "Plato_DakotaDriver.hpp"
#endif

namespace Plato
{

/******************************************************************************//**
 * \brief Construct interface to optimization algorithm
**********************************************************************************/
template<typename ScalarType, typename OrdinalType = size_t>
class DakotaFactory
{
public:
    /******************************************************************************//**
     * \brief Constructor
    **********************************************************************************/
    DakotaFactory()
    {
    }

    /******************************************************************************//**
     * \brief Destructuor
    **********************************************************************************/
    ~DakotaFactory()
    {
    }

    /******************************************************************************//**
     * \brief Construct interface to optimization algorithm
     * \param [in] aInterface PLATO Engine interface
     * \param [in] aLocalComm local MPI communicator
     * \param [in] aOptimizerIndex the index of a specific driver block
     * \return non-const pointer to the optimization algorithm's interface
    **********************************************************************************/
    Plato::DriverInterface<ScalarType, OrdinalType>*
    create(Plato::Interface* aInterface,
           MPI_Comm aLocalComm,
           std::vector< size_t > aOptimizerIndex = std::vector< size_t >()
           )
    {
#ifndef DAKOTADRIVER
        // Wanted Dakota but it is not present.
        std::stringstream tDakotaNotBuiltStream;
        tDakotaNotBuiltStream
          << "Plato::DakotaFactory: "
          << "Plato was not compiled with Dakota";
        Plato::ParsingException tDakotaNotBuiltException(tDakotaNotBuiltStream.str());
        aInterface->registerException(tDakotaNotBuiltException);
#endif

        int nBlocks = aInterface->getInputData().size<Plato::InputData>("DakotaDriver");

        if( nBlocks == 0 )
        {
            std::stringstream tStringStream;
            tStringStream
              << "Plato::DakotaFactory: "
              << "No Dakota Driver block was found. ";

            Plato::ParsingException tParsingException(tStringStream.str());
            aInterface->registerException(tParsingException);
        }

        else if( nBlocks > 1 )
        {
            std::stringstream tStringStream;
            tStringStream
              << "Plato::DakotaFactory: "
              << "More than one Dakota Driver block was found. "
              << "Only one block is permitted. ";

            Plato::ParsingException tParsingException(tStringStream.str());
            aInterface->registerException(tParsingException);
        }

        else if( nBlocks == 1 )
        {
          Plato::DriverInterface<ScalarType, OrdinalType>* tDriver = nullptr;

#ifdef DAKOTADRIVER
          try
          {
              tDriver = new Plato::DakotaDriver<ScalarType, OrdinalType>(aInterface, aLocalComm);
              feclearexcept(FE_ALL_EXCEPT);
          }
          catch(...)
          {
              aInterface->Catch();
              tDriver = nullptr;
          }
#endif
          return tDriver;
      }

      return nullptr;
    }

private:
    DakotaFactory(const Plato::DakotaFactory<ScalarType, OrdinalType>&);

    Plato::DakotaFactory<ScalarType, OrdinalType> & operator=(const Plato::DakotaFactory<ScalarType, OrdinalType>&);
};
// class DakotaFactory

} // namespace Plato
