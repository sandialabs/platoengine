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
 * Plato_ParseCSMUtilities.cpp
 *
 */

#include "Plato_ParseCSMUtilities.hpp"
#include "Plato_Macros.hpp"

#include <cstring>
#include <string>
#include <iostream>

namespace Plato
{

namespace ParseCSM
{

const int MAX_CHARS_PER_LINE = 10000;

void getValuesFromCSMFile
(const std::string& aCSMFileName,
 std::vector<double>& aInitialValues,
 std::vector<double>& aLowerBounds,
 std::vector<double>& aUpperBounds)
{
    std::ifstream tInputStream;
    tInputStream.open(aCSMFileName.c_str());
    if(tInputStream.good())
    {
        getValuesFromStream(tInputStream, aInitialValues, aLowerBounds, aUpperBounds);
        tInputStream.close();
    }
}

void getValuesFromStream
(std::istream& aStream,
 std::vector<double>& aInitialValues,
 std::vector<double>& aLowerBounds,
 std::vector<double>& aUpperBounds)
{
    char tBuffer[MAX_CHARS_PER_LINE];

    bool tFileHasDespmtr = false;
    while(!aStream.eof())
    {
        aStream.getline(tBuffer, MAX_CHARS_PER_LINE);
        char* tCharPointer = std::strtok(tBuffer, " ");

        if(tCharPointer && tCharPointer[0] == '#')
            continue;

        if(tCharPointer && std::strcmp(tCharPointer, "despmtr") == 0)
        {
            tFileHasDespmtr = true;
            bool tFoundLboundInLine = false;
            bool tFoundUboundInLine = false;
            bool tFoundInitialInLine = false;
            while (tCharPointer)
            {
                tCharPointer = strtok(NULL, " ");
                if (!tFoundLboundInLine)
                    tFoundLboundInLine = addValueIfTokenIsKeyword(tCharPointer,"lbound",aLowerBounds);
                if (!tFoundUboundInLine)
                    tFoundUboundInLine = addValueIfTokenIsKeyword(tCharPointer,"ubound",aUpperBounds);
                if (!tFoundInitialInLine)
                    tFoundInitialInLine = addValueIfTokenIsKeyword(tCharPointer,"initial",aInitialValues);
            }
            if (!tFoundLboundInLine)
                THROWERR(std::string("No lower bound specified for design parameter. Designate lower bound with 'lbound' keyword."))
            if (!tFoundUboundInLine)
                THROWERR(std::string("No upper bound specified for design parameter. Designate upper bound with 'ubound' keyword."))
            if (!tFoundInitialInLine)
                THROWERR(std::string("No initial value specified for design parameter. Designate initial value with 'initial' keyword."))
        }
    }
    if (!tFileHasDespmtr)
        THROWERR(std::string("CSM file does not contain design parameters. Designate design parameters with despmtr keyword."))

    checkParameterValues(aInitialValues, aLowerBounds, aUpperBounds);
}

bool addValueIfTokenIsKeyword
(char* aLineToken,
 const std::string& aKeyword,
 std::vector<double>& aValues)
{
    if( aLineToken && std::strcmp(aLineToken, aKeyword.c_str()) == 0 )
    {
        aLineToken = std::strtok(0, " ");
        aValues.push_back(std::atof(aLineToken));
        return true;
    }
    return false;
}

void checkParameterValues
(std::vector<double>& aInitialValues,
 std::vector<double>& aLowerBounds,
 std::vector<double>& aUpperBounds)
{
    for (size_t iParam = 0; iParam < aInitialValues.size(); iParam++)
    {
        if (aLowerBounds[iParam] > aUpperBounds[iParam])
            THROWERR(std::string("Lower bound for parameter ") + std::to_string(iParam) + std::string(" is greater than upper bound."))

        if (aInitialValues[iParam] < aLowerBounds[iParam])
            THROWERR(std::string("Initial value for parameter ") + std::to_string(iParam) + std::string(" is less than than lower bound."))

        if (aInitialValues[iParam] > aUpperBounds[iParam])
            THROWERR(std::string("Initial value for parameter ") + std::to_string(iParam) + std::string(" is greater than than upper bound."))
    }
}

void getDescriptorsFromCSMFile
(const std::string& aCSMFileName,
 std::vector<std::string>& aDescriptors)
{
    std::ifstream tInputStream;
    tInputStream.open(aCSMFileName.c_str());
    if(tInputStream.good())
    {
        getDescriptorsFromStream(tInputStream, aDescriptors);
        tInputStream.close();
    }
}

void getDescriptorsFromStream
(std::istream& aStream,
 std::vector<std::string>& aDescriptors)
{
    char tBuffer[MAX_CHARS_PER_LINE];

    while(!aStream.eof())
    {
        aStream.getline(tBuffer, MAX_CHARS_PER_LINE);
        char* tCharPointer = std::strtok(tBuffer, " ");

        if(tCharPointer && tCharPointer[0] == '#')
            continue;

        if(tCharPointer && std::strcmp(tCharPointer, "despmtr") == 0)
        {
            tCharPointer = strtok(NULL, " ");
            aDescriptors.push_back(tCharPointer);
        }
    }
}

}

} 
