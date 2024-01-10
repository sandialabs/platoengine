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
 * Plato_SystemCallOperation.cpp
 *
 *  Created on: Feb 11, 2022
 */
#include "Plato_SystemCallOperation.hpp"
#include "PlatoApp.hpp"

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
BOOST_CLASS_EXPORT_IMPLEMENT(Plato::SystemCallOperation)

namespace Plato
{

namespace Private
{

void set_system_call_metadata(
    PlatoApp* aPlatoApp, 
    const Plato::InputData& aInputData,
    Plato::SystemCallMetadata& aMetaData)
{
    try
    {
        for(Plato::InputData& tInputNode : aInputData.getByName<Plato::InputData>("Input"))
        {
            std::string tInputName = Plato::Get::String(tInputNode, "ArgumentName");
            aMetaData.mInputArgumentMap[tInputName] = aPlatoApp->getValue(tInputName);
        }
    }
    catch(std::exception& aException)
    {
        THROWERR(std::string("Error while constructing the map from input argument names to data. Exception message is '") + aException.what() + "'.")
    }
}

}
// namespace Private

SystemCallOperation::SystemCallOperation(PlatoApp* aPlatoApp, Plato::InputData & aNode)
    : Plato::LocalOp(aPlatoApp),
      mInputData(aNode)
{
    mSystemCall = std::make_unique<SystemCall>(aNode);
}

void SystemCallOperation::operator()()
{
    Plato::Private::set_system_call_metadata(mPlatoApp, mInputData, mMetaData);
    (*mSystemCall)(mMetaData);
}

void SystemCallOperation::getArguments(std::vector<Plato::LocalArg> & aLocalArgs)
{
    mSystemCall->getArguments(aLocalArgs);
}




// ******************************** SystemCallMPIOperation ******************************** //




SystemCallMPIOperation::SystemCallMPIOperation(PlatoApp* aPlatoApp, Plato::InputData & aNode)
    : Plato::LocalOp(aPlatoApp),
      mInputData(aNode)
{
    mSystemCall = std::make_unique<SystemCallMPI>(aNode,aPlatoApp->getComm());
}

void SystemCallMPIOperation::operator()()
{
    Plato::Private::set_system_call_metadata(mPlatoApp, mInputData, mMetaData);
    (*mSystemCall)(mMetaData);
}

void SystemCallMPIOperation::getArguments(std::vector<Plato::LocalArg> & aLocalArgs)
{
    mSystemCall->getArguments(aLocalArgs);
}

}
// namespace Plato