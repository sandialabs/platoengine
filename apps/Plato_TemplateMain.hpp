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

#pragma once

#include "Plato_Interface.hpp"
#include "Plato_Exceptions.hpp"

#ifndef NDEBUG
#include <cfenv>
#endif

static void safeExit(){
    MPI_Finalize();
    exit(0);
}

namespace Plato {
/******************************************************************************/

template <typename AppType>
int Main(int aArgc, char *aArgv[], [[maybe_unused]] bool aEnableDebugExceptions = true)
/******************************************************************************/
{
#ifndef NDEBUG
    feclearexcept(FE_ALL_EXCEPT);
    if ( aEnableDebugExceptions )
        feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif

    MPI_Init(&aArgc, (char***) &aArgv);

    ::Plato::Interface* tPlatoInterface = nullptr;
    try
    {
        tPlatoInterface = new ::Plato::Interface();
    }
    catch(::Plato::ParsingException& e)
    {
        std::cout << e.message() << std::endl;
        safeExit();
    }
    catch(...)
    {
        safeExit();
    }

    MPI_Comm tLocalComm;
    tPlatoInterface->getLocalComm(tLocalComm);

    AppType *tApp = nullptr;
    try
    {
        if(aArgc > static_cast<int>(1))
        {
            tApp = new AppType(aArgc, aArgv, tLocalComm);
        }
        else
        {
            throw std::logic_error("Input file argument missing");
        }
    }
    catch(...)
    {
        tApp = nullptr;
        tPlatoInterface->Catch();
    }

    try
    {
        tPlatoInterface->registerApplication(tApp);
    }
    catch(...)
    {
        safeExit();
    }

    try
    {
      tPlatoInterface->perform();
    }
    catch(...)
    {
        safeExit();
    }

    if(tApp)
    {
        delete tApp;
    }
    if(tPlatoInterface)
    {
        delete tPlatoInterface;
    }
    safeExit();

    return 0;
}

}
