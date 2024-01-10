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
 * Plato_Performer.cpp
 *
 *  Created on: April 23, 2017
 *
 */

#include "Plato_Performer.hpp"
#include "Plato_Application.hpp"
#include "Plato_Console.hpp"

namespace Plato
{

Performer::Performer(const std::string & aMyName, const int & aCommID) :
        mApplication(nullptr),
        mName(aMyName),
        mCommID(aCommID)
{
}

void Performer::finalize()
{
    if(mApplication)
    {
        mApplication->finalize();
    }
}

void Performer::compute(const std::string & aOperationName)
{
    if(mApplication)
    {
        Console::Status("Operation: (" + mName + ") " + aOperationName);
        mApplication->compute(aOperationName);
    }
}

void Performer::computeCriterionValue()
{
    if(mApplication)
    {
        Console::Status("Operation: (" + mName + ") Compute Objective");
        mApplication->computeCriterionValue();
    }
}

void Performer::computeCriterionGradient()
{
    if(mApplication)
    {
        Console::Status("Operation: (" + mName + ") Compute Gradient");
        mApplication->computeCriterionGradient();
    }
}

void Performer::computeCriterionHessianTimesVector()
{
    if(mApplication)
    {
        Console::Status("Operation: (" + mName + ") Compute Hessian");
        mApplication->computeCriterionHessianTimesVector();
    }
}

void Performer::importData(const std::string & aArgumentName, const SharedData & aImportData)
{
    if(mApplication)
    {
        mApplication->importData(aArgumentName, aImportData);
    }
}

void Performer::exportData(const std::string & aArgumentName, SharedData & aExportData)
{
    if(mApplication)
    {
        mApplication->exportData(aArgumentName, aExportData);
    }
}

void Performer::setApplication(Application* aApplication)
{
    mApplication = aApplication;
}

Application * Performer::getApplication() const
{
    return mApplication;
}

const std::string& Performer::myName()
{
    return mName;
}

int Performer::myCommID()
{
    return mCommID;
}

bool Performer::usesConstrainedOperationInterface() const 
{
    if(mApplication)
    {
        return mApplication->usesConstrainedOperationInterface();
    }
    else 
    {
        return false;
    }
}

std::vector<OperationType> Performer::supportedOperationTypes() const
{
    if(mApplication)
    {
        return mApplication->supportedOperationTypes();
    }
    else
    {
        return {};
    }
}

auto Performer::computeFunction(OperationType aOperation) -> std::function<void(Performer&)>
{
    switch(aOperation)
    {
        case OperationType::kCriterionValue:
            return std::mem_fn(&Performer::computeCriterionValue);
            break;
        case OperationType::kCriterionGradient:
            return std::mem_fn(&Performer::computeCriterionGradient);
            break;
        case OperationType::kCriterionHessian:
        default:
            return std::mem_fn(&Performer::computeCriterionHessianTimesVector);
            break;
    }
}

}
