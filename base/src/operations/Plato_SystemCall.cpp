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
 * Plato_SystemCall.cpp
 *
 *  Created on: Jun 29, 2019
 */

#include <cstdlib>
#include <sstream>
#include <unistd.h>
#include <iomanip>
#include <math.h>

#include "Plato_Utils.hpp"
#include "Plato_Macros.hpp"
#include "Plato_Parser.hpp"
#include "Plato_Console.hpp"
#include "Plato_InputData.hpp"
#include "Plato_Exceptions.hpp"
#include "Plato_SystemCall.hpp"
#include "Plato_FreeFunctions.hpp"
#include "Plato_OperationsUtilities.hpp"

namespace Plato
{
/******************************************************************************/
void SystemCall::getArguments(std::vector<Plato::LocalArg> & aLocalArgs)
/******************************************************************************/
{
    for(auto& tPair : mInputArgumentNameToAttributesMap) 
    {
        auto tSize = tPair.second.second;
        auto tArgumentName = tPair.first;
        aLocalArgs.push_back(Plato::LocalArg(Plato::data::layout_t::SCALAR, tArgumentName, tSize));
    }
}

/******************************************************************************/
SystemCall::SystemCall(const Plato::InputData & aNode)
/******************************************************************************/
{
    // set basic info
    mName = Plato::Get::String(aNode, "Name");
    mPrint = Plato::Get::Bool(aNode, "Print");
    mChDir = Plato::Get::String(aNode, "ChDir");
    mOnChange = Plato::Get::Bool(aNode, "OnChange");
    mAppendInput = Plato::Get::Bool(aNode, "AppendInput");
    mStringCommand = Plato::Get::String(aNode, "Command");
    mPrecision = Plato::Get::Int(aNode, "ParameterPrecision", 16);
    
    this->setInputSharedDataNames(aNode);
    this->setArguments(aNode);
    this->setOptions(aNode);
}

void SystemCall::setInputSharedDataNames(const Plato::InputData& aNode)
{
    for(Plato::InputData& tInputNode : aNode.getByName<Plato::InputData>("Input"))
    {
        auto tSize = this->getSize(tInputNode);
        auto tLayout = this->getLayout(tInputNode);
        auto tArgumentName = Plato::Get::String(tInputNode, "ArgumentName");
        mInputArgumentNameToAttributesMap[tArgumentName] = std::make_pair(tLayout, tSize);
        mInputNames.push_back(tArgumentName);
    }
}

int SystemCall::getSize(const Plato::InputData& aInputNode)
{
    auto tSize = Plato::Get::Int(aInputNode, "Size");
    if(tSize <= 0)
    {
        auto tName = Plato::Get::String(aInputNode, "ArgumentName");
        THROWERR(std::string("Invalid size '") + std::to_string(tSize) + "' for argument with name '" + 
            tName + "'. " + "Size has to be a positive integer greater than zero.")
    }
    return tSize;
}

std::string SystemCall::getLayout(const Plato::InputData& aInputNode)
{
    std::vector<std::string> tSupportedLayout = {"scalar"};
    auto tLayout = Plato::Get::String(aInputNode, "Layout");
    if(tLayout.empty())
    {
        auto tName = Plato::Get::String(aInputNode, "ArgumentName");
        THROWERR(std::string("Data layout was not defined for argument with name '") + tName + "'.")
    }

    auto tLowerLayout = Plato::tolower(tLayout);
    if(std::find(tSupportedLayout.begin(), tSupportedLayout.end(),tLowerLayout) == tSupportedLayout.end())
    {
        auto tName = Plato::Get::String(aInputNode, "ArgumentName");
        auto tMsg = std::string("Data layout '") + tLayout + "' of argument '" + tName + "' is not supported by SystemCall Operation. Supported Layouts: "; 

        std::string tLayoutsMsg;
        for(auto tMyLayout : tSupportedLayout)
        {
            auto tDelimiter = (&tMyLayout - &tSupportedLayout[0] >= (int)(tSupportedLayout.size())) ? "." : ", ";
            tLayoutsMsg += tMyLayout + tDelimiter;
            
        }
        THROWERR(tMsg + tLayoutsMsg);
    }
    return tLowerLayout;
}

void SystemCall::setArguments(const Plato::InputData& aNode)
{
    // set arguments
    for(auto& tStrValue : aNode.getByName<std::string>("Argument"))
    {
        mArguments.push_back(tStrValue);
    }
}

void SystemCall::setOptions(const Plato::InputData& aNode)
{
    auto tNumOptions = aNode.getByName<std::string>("Option").size();
    mOptions = std::vector<std::string>(tNumOptions, "");
    
    size_t tIndex = 0;
    for(auto tStrValue : aNode.getByName<std::string>("Option"))
    {
        mOptions[tIndex++] = tStrValue;
    }
}

void SystemCall::areNumOptionsGreaterThanNumParams(const Plato::SystemCallMetadata& aMetaData)
{
    if(mOptions.empty())
    {return;}
    
    auto tNumOptions = mOptions.size();
    auto tTotalNumParameters = this->countTotalNumParameters(aMetaData);
    if(tNumOptions > tTotalNumParameters)
    {
        THROWERR(std::string("Number of options cannot exceed the number of total input parameters. ") 
            + "The number of options is set to '" + std::to_string(tNumOptions) 
            + "' while the number of input parameters is set to '" + std::to_string(tTotalNumParameters) + ".")
    }
}

size_t SystemCall::countTotalNumParameters(const Plato::SystemCallMetadata& aMetaData) const
{
    size_t tTotalNumParameters = 0;
    for(auto& tInputArgumentPair : aMetaData.mInputArgumentMap)
    {
        tTotalNumParameters += tInputArgumentPair.second->size();
    }
    return tTotalNumParameters;
}

bool SystemCall::checkForLocalParameterChanges(const Plato::SystemCallMetadata& aMetaData)
{
    if(aMetaData.mInputArgumentMap.size() != mInputNames.size())
    {
        THROWERR(std::string("Mismatch between map from input argument name to data and array of input argument names. ") 
            + "The size of the map from input argument names to data is '" + std::to_string(aMetaData.mInputArgumentMap.size()) 
            + "' while the size of the array of input argument names is '" + std::to_string(mInputNames.size()) + "'.")
    }
    
    bool tDidParametersChanged = false;
    for(size_t tIndexJ=0; tIndexJ<mInputNames.size(); ++tIndexJ)
    {
        bool tLocalChanged = false;
        Plato::Console::Status("PlatoMain: On Change SystemCall -- \"" + mInputNames[tIndexJ] + "\" Values:");
        auto tInputArgument = aMetaData.mInputArgumentMap.at(mInputNames[tIndexJ]);
        for(size_t tIndexI=0; tIndexI<tInputArgument->size(); ++tIndexI)
        {
            double tSavedValue = mSavedParameters[tIndexJ][tIndexI];
            double tCurrentValue = tInputArgument->data()[tIndexI];
            if(tSavedValue != tCurrentValue)
            {
                double tPercentChange = (fabs(tSavedValue-tCurrentValue)/fabs(tSavedValue))*100.0;
                char tPercentChangeString[20];
                sprintf(tPercentChangeString, "%.1lf", tPercentChange);
                Plato::Console::Status("Saved: " + std::to_string(mSavedParameters[tIndexJ][tIndexI]) 
                                       + "; \tCurrent: " + std::to_string(tInputArgument->data()[tIndexI]) 
                                       + " \t-- CHANGED " + tPercentChangeString + "%");
                tLocalChanged = true;
                tDidParametersChanged = true;
            }
            else
            {
                Plato::Console::Status("Saved: " + std::to_string(mSavedParameters[tIndexJ][tIndexI]) + "; \tCurrent: " 
                                       + std::to_string(tInputArgument->data()[tIndexI]) + " \t-- NO CHANGE");
            }
        }
        if(tLocalChanged == true)
        {
            mSavedParameters[tIndexJ] = *(tInputArgument);
        }
    }
    return tDidParametersChanged;
}

void SystemCall::performSystemCall(const Plato::SystemCallMetadata& aMetaData)
{
    std::vector<std::string> tArguments;
    for(auto& tArgumentName : mArguments)
    {
        tArguments.push_back(tArgumentName);
    }
    if(mAppendInput)
    {
        this->appendOptionsAndValues(aMetaData,tArguments);
    }

    auto tCWD = Plato::Utils::current_working_directory();
    Plato::Utils::change_directory(mChDir);
    this->executeCommand(tArguments);
    if(tCWD != mChDir && !mChDir.empty()) { Plato::Utils::change_directory(tCWD); }
}

void SystemCall::appendOptionsAndValues(const Plato::SystemCallMetadata& aMetaData, std::vector<std::string>& aArguments)
{
    for(auto& tInputName : mInputNames)
    {
        auto tInputArgument = aMetaData.mInputArgumentMap.at(tInputName);
        for(size_t tIndex=0; tIndex<tInputArgument->size(); ++tIndex)
        {
            std::stringstream tDataString;
            if(mOptions.empty())
                tDataString << std::setprecision(mPrecision) << tInputArgument->data()[tIndex];
            else
                tDataString << std::setprecision(mPrecision) << mOptions[tIndex] << tInputArgument->data()[tIndex];
            aArguments.push_back(tDataString.str());
        }
    }
}

void SystemCall::saveParameters(const Plato::SystemCallMetadata& aMetaData)
{
    if(aMetaData.mInputArgumentMap.size() != mInputNames.size())
    {
        THROWERR(std::string("Mismatch between map from input argument name to data and array of input argument names. ") 
            + "The size of the map from input argument names to data is '" + std::to_string(aMetaData.mInputArgumentMap.size()) 
            + "' while the size of the array of input argument names is '" + std::to_string(mInputNames.size()) + "'.")
    }
    
    for(size_t tIndexJ=0; tIndexJ<mInputNames.size(); ++tIndexJ)
    {
        Plato::Console::Status("PlatoMain: On Change SystemCall -- \"" + mInputNames[tIndexJ] + "\" Values:");
        auto tInputArgument = aMetaData.mInputArgumentMap.at(mInputNames[tIndexJ]);
        std::vector<double> tCurVector(tInputArgument->size());
        for(size_t tIndexI=0; tIndexI<tInputArgument->size(); ++tIndexI)
        {
            tCurVector[tIndexI] = tInputArgument->data()[tIndexI];
            Plato::Console::Status("Saved: Not set yet; Current: " + std::to_string(tCurVector[tIndexI]));
        }
        mSavedParameters.push_back(tCurVector);
    }
}

/******************************************************************************/
void SystemCall::operator()(const Plato::SystemCallMetadata& aMetaData)
/******************************************************************************/
{
    this->areNumOptionsGreaterThanNumParams(aMetaData);

    bool tChanged = true;
    if(mOnChange)
    {
        // If we haven't saved any parameters yet save them now and set "changed" to true.
        if(mSavedParameters.size() == 0)
        {
            this->saveParameters(aMetaData);
        }
        else
        {
            tChanged = this->checkForLocalParameterChanges(aMetaData);
        }
    }

    if(tChanged)
    {
        Plato::Console::Status("PlatoMain: SystemCall -- Condition met. Calling.");
        this->performSystemCall(aMetaData);
    }
    else
    {
        Plato::Console::Status("PlatoMain: SystemCall -- Condition not met. Not calling.");
    }
}

void SystemCall::executeCommand(const std::vector<std::string> &aArguments)
{
    mCommandPlusArguments = mStringCommand;
    for(const auto &tArgument : aArguments) 
    {
        mCommandPlusArguments += " " + tArgument;
    }

    // make system call and throw error if exit status non-zero
    Plato::system_with_throw(mCommandPlusArguments.c_str());
    if (mPrint)
    {
        Plato::Console::Status("Executed command: " + mCommandPlusArguments);
    }
}










SystemCallMPI::SystemCallMPI(const Plato::InputData & aNode, const MPI_Comm& aComm) : 
    SystemCall(aNode),
    mComm(aComm)
{
    mNumRanks = Plato::Get::Int(aNode, "NumRanks");
}

void SystemCallMPI::executeCommand(const std::vector<std::string> &aArguments)
{
    std::vector<char*> tArgumentPointers;
    mCommandPlusArguments = mStringCommand;
    for(const auto &tArgument : aArguments) {
        mCommandPlusArguments += " " + tArgument;
        const int tLengthWithNullTerminator = tArgument.length() + 1;
        char *tPointer = new char [tLengthWithNullTerminator];
        std::copy_n(tArgument.c_str(), tLengthWithNullTerminator, tPointer);
        tArgumentPointers.push_back(tPointer);
    }
    tArgumentPointers.push_back(nullptr);

    MPI_Comm tIntercom;
    auto tExitStatus = MPI_Comm_spawn(mStringCommand.c_str(),
                                      tArgumentPointers.data(),
                                      mNumRanks, MPI_INFO_NULL,
                                      0,
                                      mComm,
                                      &tIntercom, MPI_ERRCODES_IGNORE);
    MPI_Barrier(tIntercom);

    if (tExitStatus != MPI_SUCCESS)
    {
        THROWERR(std::string("System call '") + mStringCommand + std::string("' exited with exit status: ") + std::to_string(tExitStatus))
    }

    for(auto tPointer : tArgumentPointers) {
        delete [] tPointer;
    }
}

}

