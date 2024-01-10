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
 * Plato_OperationsUtilities.cpp
 *
 *  Created on: Jun 27, 2019
 */

#include <fstream>

#include "Plato_Parser.hpp"
#include "Plato_InputData.hpp"
#include "Plato_Exceptions.hpp"
#include "Plato_SharedData.hpp"
#include "Plato_OperationsUtilities.hpp"

namespace Plato
{

std::string getLayout(const Plato::data::layout_t & aInput)
{
    std::string tOutput;
    switch(aInput)
    {
        case Plato::data::SCALAR:
        {
            tOutput = std::string("SCALAR");
            break;
        }
        case Plato::data::SCALAR_FIELD:
        {
            tOutput = std::string("SCALAR FIELD");
            break;
        }
        case Plato::data::VECTOR_FIELD:
        {
            tOutput = std::string("VECTOR FIELD");
            break;
        }
        case Plato::data::TENSOR_FIELD:
        {
            tOutput = std::string("TENSOR FIELD");
            break;
        }
        case Plato::data::ELEMENT_FIELD:
        {
            tOutput = std::string("ELEMENT FIELD");
            break;
        }
        case Plato::data::SCALAR_PARAMETER:
        {
            tOutput = std::string("SCALAR PARAMETER");
            break;
        }
        case Plato::data::UNDEFINED:
        default:
        {
            tOutput = std::string("UNDEFINED");
            break;
        }
    }
    return(tOutput);
}

Plato::data::layout_t getLayout(const std::string & aLayoutStr)
{

    Plato::data::layout_t tLayout;
    if(aLayoutStr == "NODAL FIELD" || aLayoutStr == "SCALAR FIELD")
    {
        tLayout = Plato::data::layout_t::SCALAR_FIELD;
    }
    else if(aLayoutStr == "ELEMENT FIELD")
    {
        tLayout = Plato::data::layout_t::ELEMENT_FIELD;
    }
    else if(aLayoutStr == "VALUE" || aLayoutStr == "GLOBAL" || aLayoutStr == "SCALAR")
    {
        tLayout = Plato::data::layout_t::SCALAR;
    }
    else
    {
        std::stringstream tError;
        tError << std::endl << " PlatoApp: " << std::endl;
        tError << "   Unknown layout specified: '" << aLayoutStr << "'" << std::endl;
        Plato::ParsingException tParsingException(tError.str());
        throw tParsingException;
    }
    return (tLayout);
}

Plato::data::layout_t getLayout(const Plato::InputData& aNode, Plato::data::layout_t aDefaultLayout)
{
    auto tLayoutString = Plato::Get::String(aNode, "Layout", true);
    Plato::data::layout_t tLayout = aDefaultLayout;
    if(!tLayoutString.empty())
    {
        tLayout = Plato::getLayout(tLayoutString);
    }
    return (tLayout);
}

Plato::data::layout_t getLayout(const Plato::InputData& aNode)
{
    auto tLayoutString = Plato::Get::String(aNode, "Layout", true);
    if(tLayoutString.empty())
    {
        Plato::ParsingException tParsingException(" PlatoApp: required 'Layout' not specified");
        throw tParsingException;
    }
    return (Plato::getLayout(tLayoutString));
}

void zero(const size_t& aLength, double* aData)
{
    for(size_t tIndex = 0; tIndex < aLength; tIndex++)
    {
        aData[tIndex] = 0.0;
    }
}

void split(const std::string & aInput, std::vector<std::string> & aOutput)
{
    std::string tSegment;
    std::stringstream tArgument(aInput);
    while(std::getline(tArgument, tSegment, '_'))
    {
       aOutput.push_back(tSegment);
    }
}

bool parse_tokens(char *aBuffer, std::vector<std::string> &aTokens)
{
    const std::string tDELIMITER = " \t";
    constexpr int tMAX_TOKENS_PER_LINE = 5000;
    const char* tToken[tMAX_TOKENS_PER_LINE] = {}; // initialize to 0

    // parse the line
    tToken[0] = std::strtok(aBuffer, tDELIMITER.c_str()); // first token

    // If there is a comment...
    if(tToken[0] && std::strlen(tToken[0]) > 1 && tToken[0][0] == '/' && tToken[0][1] == '/')
    {
        aTokens.clear();
        return true;
    }

    int tN = 0;
    if (tToken[0]) // zero if line is blank
    {
        for (tN = 1; tN < tMAX_TOKENS_PER_LINE; tN++)
        {
            tToken[tN] = std::strtok(0, tDELIMITER.c_str()); // subsequent tokens
            if (!tToken[tN])
            {
                break; // no more tokens
            }
        }
    }
    for(int tIndex=0; tIndex<tN; ++tIndex)
    {
        aTokens.push_back(tToken[tIndex]);
    }

    return true;
}
// function parse_tokens

void read_table(const std::string& aFileName, std::vector<std::vector<double>>& aTable)
{
    constexpr int MAX_CHARS_PER_LINE = 10000;
    std::vector<char> tBuffer(MAX_CHARS_PER_LINE);
    std::ifstream tFile(aFileName); //taking file as inputstream
    while (!tFile.eof())
    {
        std::vector<std::string> tTokens;
        tFile.getline(tBuffer.data(), MAX_CHARS_PER_LINE);
        Plato::parse_tokens(tBuffer.data(), tTokens);
        
        bool tPushBack = false;
        std::vector<double> tRow(tTokens.size(), 0.0);
        for(auto& tToken : tTokens)
        {
            auto tIndex = &tToken - &tTokens[0];
            try
            {
                tRow[tIndex] = std::stod(tToken);
                tPushBack = true;
            }
            catch (std::exception& e)
            {
                std::cout << "Token: '" << tTokens[tIndex] << "' is not a number. String to double conversion will be skipped.\n";
                continue;
            }
        }

        if(tPushBack)
        {
            aTable.push_back(tRow);
        }
    }
}
// function read_table

}
// namespace Plato
