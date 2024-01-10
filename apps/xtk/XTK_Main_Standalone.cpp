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
 * XTK_Main_Standalone.cpp
 *
 *  Created on: May 28, 2019
 *      Author: doble
 */


#include "communicator.hpp"

#include "Plato_XTK_Application.hpp"
#include "Plato_XTK_Local_Op_Generate_Model.hpp"
#include "Plato_Interface.hpp"
#include "Plato_Exceptions.hpp"
#include <iostream>

#include <cl_Logger.hpp>
#include <cl_Communication_Manager.hpp>
// moris::Comm_Manager gMorisComm;
// moris::Logger       gLogger;

#ifndef NDEBUG
#include <fenv.h>
#endif

void safeExit(){
    MPI_Finalize();
    exit(0);
}

/******************************************************************************/
int main(int aArgc, char *aArgv[])
/******************************************************************************/
{
#ifndef NDEBUG
    feclearexcept(FE_ALL_EXCEPT);
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif

    MPI_Init(&aArgc, (char***) &aArgv);

        // gLogger.initialize(2);

    // initialize local communicator
    MPI_Comm tLocalComm = MPI_COMM_WORLD;
    
    // Create XTK  application and register it with the Plato interface
    Plato::XTKApplication *tXTKApp = nullptr;
    if(aArgc > static_cast<int>(1))
    {
        tXTKApp = new Plato::XTKApplication(aArgc, aArgv, tLocalComm);

        Plato::InputData tDummyInput;
        PlatoXTK::Generate_Model_Op tGenerateModelOp(tXTKApp, tDummyInput);

        tGenerateModelOp();
    }
    else
    {
        safeExit();
    }

 
    // Delete things if they were created successfully

    if(tXTKApp)
    {
        delete tXTKApp;
    }

    safeExit();
}
