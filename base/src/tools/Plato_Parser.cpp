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
 * Plato_Parser.cpp
 *
 *  Created on: Dec 13, 2016
 *
 */

#include "Plato_Parser.hpp"

#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include "Plato_Exceptions.hpp"

namespace Plato
{

namespace
{

template <typename T>
T get_with_error_message(const Plato::InputData& aInputData,
                         const std::string_view aEntryName,
                         const std::string_view aErrorMessage)
{
    T tValue;
    try
    {
        tValue = aInputData.get<T>(std::string{aEntryName});
    }
    catch (int /*tErr*/)
    {
        const std::string tErrorMessage = "Error parsing input: " + std::string{aErrorMessage};
        throw Plato::ParsingException(tErrorMessage);
    }
    return tValue;
}

}  // namespace

void MathParser::addArrays(const decltype(mArrays)& aArrays) { mArrays = aArrays; }

void MathParser::addVariable(std::string aVarName, std::string aVarValue)
{
    // the te_variable struct stores pointers to name and value -- it doesn't
    // copy anything -- so the data have to be persistent.  Use shared_ptr's
    // so there's no cleanup.  The references in the te_variable instance aren't
    // counted, but mVariables, mValues, and mNames all go out of scope at the
    // same time.

    if (aVarValue.find("{") != std::string::npos) aVarValue = this->compute(aVarValue);

    auto tNewDatum = std::make_shared<double>(atof(aVarValue.c_str()));
    mValues.push_back(tNewDatum);

    // tinyexpr only works with lowercase variable names
    std::transform(aVarName.begin(), aVarName.end(), aVarName.begin(), ::tolower);
    auto tNewName = std::make_shared<std::string>(aVarName);
    mNames.push_back(tNewName);

    te_variable newVar = {tNewName->c_str(), tNewDatum.get(), 0, nullptr};

    mVariables.push_back(newVar);
}

std::string MathParser::compute(std::string aExpr)
{
    std::stringstream tRetval;

    // get substring between '{' and '}'
    size_t tBegin = aExpr.find("{");
    tBegin++;
    size_t tEnd = aExpr.find("}");
    std::string tSubExpr = aExpr.substr(tBegin, tEnd - tBegin);

    // tinyexpr doesn't handle arrays.  find/replace array values before compiling.
    size_t tSearchPos;
    if ((tSearchPos = tSubExpr.find("[")) != std::string::npos)
    {
        if (mArrays.size() == 0)
        {
            std::stringstream ss;
            ss << "Expression '" << tSubExpr << "' has Array operator (i.e., [*]) but no Arrays are defined.";
            throw Plato::ParsingException(ss.str());
        }

        size_t tIndexEnd = tSubExpr.find("]", tSearchPos);
        if (tIndexEnd == std::string::npos)
        {
            std::stringstream ss;
            ss << "Expression '" << tSubExpr << "' has opening '[' but no closing ']'";
            throw Plato::ParsingException(ss.str());
        }

        auto tIndexStart = tSearchPos + 1;
        auto tIndexLen = tIndexEnd - tIndexStart;
        int tArrayIndex = stoi(tSubExpr.substr(tIndexStart, tIndexLen));

        std::string tDelimiters(" +-()/*^");
        auto tNameStart = tSubExpr.find_last_of(tDelimiters, tSearchPos);
        tNameStart++;  // Array name starts immediately after the delimiter
        auto tNameLen = tSearchPos - tNameStart;
        std::string tArrayName = tSubExpr.substr(tNameStart, tNameLen);

        if (mArrays.count(tArrayName) == 0)
        {
            std::stringstream ss;
            ss << "Expression '" << tSubExpr << "' uses Array '" << tArrayName << "' that is not defined.";
            throw Plato::ParsingException(ss.str());
        }

        const auto& tStrVals = mArrays[tArrayName];
        if ((tStrVals.size() < (static_cast<size_t>(tArrayIndex + 1))) || (tArrayIndex < 0))
        {
            std::stringstream ss;
            ss << "Expression '," << tSubExpr << "' Array '" << tArrayName << "', value (" << tArrayIndex
               << " out of bounds.";
            throw Plato::ParsingException(ss.str());
        }

        auto tStrVal = tStrVals[tArrayIndex];
        auto tTotalLen = tIndexEnd - tNameStart;
        tSubExpr.replace(tNameStart, tTotalLen + 1, tStrVal);
    }

    // tinyexpr requires lowercase.
    std::transform(tSubExpr.begin(), tSubExpr.end(), tSubExpr.begin(), ::tolower);

    int error;
    te_expr* tExpr = te_compile(tSubExpr.c_str(), mVariables.data(), mVariables.size(), &error);

    if (tExpr)
    {
        double tVal = te_eval(tExpr);
        tRetval << std::setprecision(16) << tVal;
    }
    else
    {
        std::stringstream ss;
        ss << "Fatal error:  failed to evaluate expression: '" << aExpr << "'.";
        throw Plato::ParsingException(ss.str());
    }
    te_free(tExpr);
    return tRetval.str();
}

std::string MathParser::parse(std::string aExpr)
{
    std::stringstream tRetval;

    int error = 0;
    double tValue = te_interp(aExpr.c_str(), &error);

    if (error != 0)
    {
        std::stringstream ss;
        ss << "Fatal error:  failed to evaluate expression: '" << aExpr << "'.";
        throw Plato::ParsingException(ss.str());
    }

    tRetval << std::setprecision(16) << tValue;

    return tRetval.str();
}

InputData PugiParser::parseFile(const std::string& filename) const
{
    auto tInput = std::make_shared<pugi::xml_document>();
    pugi::xml_parse_result tResult = tInput->load_file(filename.c_str());
    if (!tResult)
    {
        std::stringstream ss;
        ss << "Error loading file '" << filename << "'.";
        throw Plato::ParsingException(ss.str());
    }
    return read(tInput);
}

InputData PugiParser::parseString(const std::string& inputString) const
{
    auto tInput = std::make_shared<pugi::xml_document>();
    pugi::xml_parse_result tResult = tInput->load_string(inputString.c_str());
    if (!tResult)
    {
        // throw
    }
    return read(tInput);
}

InputData PugiParser::read(std::shared_ptr<pugi::xml_document> doc) const
{
    preProcess(doc);

    InputData retInputData("Input Tree");

    addChildren(*(doc), retInputData);

    retInputData.set<decltype(doc)>("InputTree", doc);

    return retInputData;
}

void PugiParser::preProcess(std::shared_ptr<pugi::xml_document> doc) const
{
    // process 'includes'.  included files are pulled in verbatim.
    //
    for (auto tIncludeNode = doc->child("include"); tIncludeNode; tIncludeNode = tIncludeNode.next_sibling("include"))
    {
        std::string tFilename = tIncludeNode.attribute("filename").value();
        pugi::xml_document tInclude;
        pugi::xml_parse_result tResult = tInclude.load_file(tFilename.c_str());
        if (!tResult)
        {
            std::stringstream ss;
            ss << "Error loading file '" << tFilename << "'.";
            throw Plato::ParsingException(ss.str());
        }
        for (auto tNode = tInclude.first_child(); tNode; tNode = tNode.next_sibling())
        {
            doc->insert_copy_before(tNode, tIncludeNode);
        }
        tIncludeNode.set_name("Delete");
    }
    PugiParser::deleteNodesByName(*doc, "Delete");

    std::shared_ptr<Plato::MathParser> tMathParser(new MathParser);

    // parse 'Define' definitions
    //
    for (auto tDefineNode = doc->child("Define"); tDefineNode; tDefineNode = tDefineNode.next_sibling("Define"))
    {
        std::string tDefName = tDefineNode.attribute("name").value();
        std::string tDefValue = tDefineNode.attribute("value").value();
        tMathParser->addVariable(tDefName, tDefValue);
    }

    // parse 'Array' definitions
    //
    std::map<std::string, std::vector<std::string>> tArrays;
    for (auto tArrayNode = doc->child("Array"); tArrayNode; tArrayNode = tArrayNode.next_sibling("Array"))
    {
        std::string tVarName = tArrayNode.attribute("name").value();

        if (tArrays.count(tVarName) == 0)
        {
            // new variable.  add it.
            //
            tArrays[tVarName] = std::vector<std::string>(0);

            std::string tStrType = tArrayNode.attribute("type").value();
            std::transform(tStrType.begin(), tStrType.end(), tStrType.begin(), ::tolower);
            if (tStrType == "int" || tStrType == "")
            {
                // parse ints

                std::string tStrValues = tArrayNode.attribute("values").value();
                if (tStrValues != "")
                {
                    tArrays[tVarName] = tokenize(tStrValues, ',');
                }
                else
                {
                    // parse from=, to=, by=,
                    std::string tStrFrom = tArrayNode.attribute("from").value();
                    if (tStrFrom.find("{") != std::string::npos) tStrFrom = tMathParser->compute(tStrFrom);
                    int tIntFrom = stoi(tStrFrom);

                    std::string tStrTo = tArrayNode.attribute("to").value();
                    if (tStrTo.find("{") != std::string::npos) tStrTo = tMathParser->compute(tStrTo);
                    int tIntTo = stoi(tStrTo);

                    std::string tStrBy = tArrayNode.attribute("by").value();
                    int tIntBy = 1;
                    if (tStrBy != "")
                    {
                        if (tStrBy.find("{") != std::string::npos) tStrBy = tMathParser->compute(tStrBy);
                        tIntBy = stoi(tStrBy);
                    }
                    if (tIntBy == 0)
                    {
                        throw Plato::ParsingException("'by' keyword cannot be zero.");
                    }

                    // make sure that the range isn't bogus
                    int tIntRange = (tIntTo - tIntFrom) / tIntBy;
                    if (tIntRange < 0)
                    {
                        std::stringstream ss;
                        ss << "Range (from: " << tIntFrom << ", to: " << tIntTo << ", by: " << tIntBy
                           << ") doesn't end." << std::endl;
                        throw Plato::ParsingException(ss.str());
                    }

                    auto& tValues = tArrays[tVarName];
                    for (int i = tIntFrom; i <= tIntTo; i += tIntBy)
                    {
                        tValues.push_back(std::to_string(i));
                    }
                }
            }
            else if (tStrType == "real" || tStrType == "float" || tStrType == "double")
            {
                // parse reals

                std::string tStrValues = tArrayNode.attribute("values").value();
                if (tStrValues != "")
                {
                    tArrays[tVarName] = tokenize(tStrValues, ',');
                }
                else
                {
                    // parse from=
                    std::string tStrFrom = tArrayNode.attribute("from").value();
                    if (tStrFrom.find("{") != std::string::npos) tStrFrom = tMathParser->compute(tStrFrom);
                    double tRealFrom = atof(tStrFrom.c_str());

                    // parse to=
                    std::string tStrTo = tArrayNode.attribute("to").value();
                    if (tStrTo.find("{") != std::string::npos) tStrTo = tMathParser->compute(tStrTo);
                    double tRealTo = atof(tStrTo.c_str());

                    // parse by= or intervals=
                    int tNumReals = 0;
                    double tRealBy = 0.0;

                    std::string tStrBy = tArrayNode.attribute("by").value();
                    std::string tStrIntervals = tArrayNode.attribute("intervals").value();

                    // specify either 'by=' or 'intervals='
                    if ((tStrBy != "" && tStrIntervals != "") || (tStrBy == "" && tStrIntervals == ""))
                    {
                        throw Plato::ParsingException("Specify either 'by=' or 'intervals=' in Array definition");
                    }

                    if (tStrBy != "")
                    {
                        // parse 'by='
                        if (tStrBy.find("{") != std::string::npos) tStrBy = tMathParser->compute(tStrBy);
                        tRealBy = atof(tStrBy.c_str());
                        if (tRealBy == 0.0)
                        {
                            throw Plato::ParsingException("'by' keyword cannot be zero.");
                        }
                        tNumReals = (tRealTo - tRealFrom) / tRealBy;

                        // make sure that the range isn't bogus
                        if (tNumReals < 0)
                        {
                            std::stringstream ss;
                            ss << "Range (from: " << tRealFrom << ", to: " << tRealTo << ", by: " << tRealBy
                               << ") doesn't end." << std::endl;
                            throw Plato::ParsingException(ss.str());
                        }
                    }

                    if (tStrIntervals != "")
                    {
                        // parse 'intervals='
                        if (tStrIntervals.find("{") != std::string::npos)
                            tStrIntervals = tMathParser->compute(tStrIntervals);
                        tNumReals = stoi(tStrIntervals);

                        // make sure that the range isn't bogus
                        if (tNumReals <= 0)
                        {
                            throw Plato::ParsingException("'intervals' value must be positive.");
                        }
                        tRealBy = (tRealTo - tRealFrom) / tNumReals;
                    }

                    auto& tValues = tArrays[tVarName];
                    for (int i = 0; i <= tNumReals; i++)
                    {
                        double val = tRealFrom + i * tRealBy;
                        tValues.push_back(std::to_string(val));
                    }
                }
            }
        }
        else
        {
            std::stringstream ss;
            ss << "Plato::Parser: Multiple variable definitions for '" << tVarName << "'." << std::endl;
            throw Plato::ParsingException(ss.str());
        }
    }

    tMathParser->addArrays(tArrays);

    int numMods;
    do
    {
        // find 'For' elements and expand them.  The for_each member function in
        // the ForWalker class is called for each node in the tree:
        ForWalker tForWalker(tArrays, numMods);
        doc->traverse(tForWalker);
    } while (numMods != 0);

    PugiParser::deleteNodesByName(*doc, "Delete");

    do
    {
        // find arithmetic expressions and evaluate them.  The for_each member function in
        // the MathWalker class is called for each node in the tree:
        MathWalker tMathWalker(tMathParser);
        doc->traverse(tMathWalker);
    } while (numMods != 0);

    //    std::cout << "Document: \n";
    //    doc->save(std::cout);
}

void PugiParser::deleteNodesByName(pugi::xml_node aNode, std::string aNodeName)
{
    while (aNode.remove_child(aNodeName.c_str()));
    for (auto& tNode : aNode.children())
    {
        deleteNodesByName(tNode, aNodeName);
    }
}

bool PugiParser::MathWalker::for_each(pugi::xml_node& aNode)
{
    if (0 != strcmp(aNode.name(), ""))
    {
        std::string tStrName(aNode.name());
        std::string tStrNameEval = evalExpr(tStrName);
        aNode.set_name(tStrNameEval.c_str());
    }

    if (0 != strcmp(aNode.value(), ""))
    {
        std::string tStrValue(aNode.value());
        std::string tStrValueEval = evalExpr(tStrValue);
        aNode.set_value(tStrValueEval.c_str());
    }

    return true;
}

std::string PugiParser::MathWalker::evalExpr(std::string aString)
{
    std::string tString(aString);

    size_t tSearchPos = 0;
    while ((tSearchPos = tString.find('{', tSearchPos)) != std::string::npos)
    {
        evalSubExpr(tString, tSearchPos + 1);
    }
    return tString;
}

void PugiParser::MathWalker::evalSubExpr(std::string& aString, size_t aSearchPos)
{
    size_t tOpenerPos = aString.find('{', aSearchPos);
    size_t tCloserPos = aString.find('}', aSearchPos);

    if (tOpenerPos == std::string::npos && tCloserPos == std::string::npos)
    {
        std::stringstream ss;
        ss << "Failed to evaluate '" << aString << "'. Malformed expression?";
        throw Plato::ParsingException(ss.str());
    }

    if (tOpenerPos < tCloserPos)
    {
        evalSubExpr(aString, tOpenerPos + 1);
    }
    else
    {
        size_t tStrLen = tCloserPos - aSearchPos;
        std::string tExpression = aString.substr(aSearchPos, tStrLen);
        std::string tParsed = mMathParser->compute(tExpression);
        aString.replace(aSearchPos - 1, tStrLen + 2, tParsed);
    }
}

bool PugiParser::ForWalker::for_each(pugi::xml_node& aNode)
{
    if (std::string(aNode.name()) == "For")
    {
        // verify 'var' input
        std::string tStrVar = aNode.attribute("var").value();
        if (tStrVar.empty())
        {
            throw Plato::ParsingException("'For' element missing 'var' attribute");
        }

        // tokenize to account for compound 'For' loops (i.e., For var='I,J', etc.)
        auto tStrVars = tokenize(tStrVar, ',');

        // verify 'in' input
        std::string tStrIn = aNode.attribute("in").value();
        if (tStrIn.empty())
        {
            throw Plato::ParsingException("'For' element missing 'in' attribute");
        }
        // tokenize
        auto tStrIns = tokenize(tStrIn, ',');

        // compound 'var' and 'in' must be same length
        if (tStrVars.size() != tStrIns.size())
        {
            throw Plato::ParsingException("Compound 'For' element with unequal length 'var' and 'in'.");
        }

        // verify requested variables exists and check that lengths are equal
        int tNumVals = -1;
        for (auto tStr : tStrIns)
        {
            if (mVarMap.count(tStr) == 0)
            {
                std::stringstream ss;
                ss << "'For' element requested undefined variable: '" << tStr << "'." << std::endl;
                throw Plato::ParsingException(ss.str());
            }
            int tNewLen = mVarMap.at(tStr).size();
            if (tNumVals != -1 && tNumVals != tNewLen)
            {
                throw Plato::ParsingException("Arrays in compound For statements must be equal length.");
            }
            tNumVals = tNewLen;
        }

        for (int iVal = 0; iVal < tNumVals; iVal++)
        {
            // copy pattern above For node.
            for (auto toCopy = aNode.first_child(); toCopy; toCopy = toCopy.next_sibling())
            {
                auto copied = aNode.parent().insert_copy_before(toCopy, aNode);

                for (size_t iVar = 0; iVar < tStrVars.size(); iVar++)
                {
                    std::string tVal = mVarMap.at(tStrIns[iVar])[iVal];
                    std::string tVar = tStrVars[iVar];

                    // find/replace var in new copy.
                    PugiParser::recursiveFindReplace(copied, tVar, tVal);
                }
            }
        }
        aNode.set_name("Delete");

        mNumMods++;
    }
    return true;  // continue traversal
}

std::vector<std::string> PugiParser::tokenize(std::string aString, const char aDelimiter)
{
    std::vector<std::string> tTokens;
    {
        std::istringstream tStream(aString);
        std::string tToken;
        while (std::getline(tStream, tToken, aDelimiter))
        {
            tTokens.push_back(tToken);
        }
    }
    return tTokens;
}

void PugiParser::recursiveFindReplace(pugi::xml_node aNode, std::string aFind, std::string aReplace)
{
    for (auto tNode : aNode.children())
    {
        tNode.set_name(findReplace(tNode.name(), aFind, aReplace).c_str());
        tNode.set_value(findReplace(tNode.value(), aFind, aReplace).c_str());
        recursiveFindReplace(tNode, aFind, aReplace);
    }
}

std::string PugiParser::findReplace(std::string aString, std::string aFind, std::string aReplace)
{
    // find aFind in aString and replace with aReplace.
    // only look for aFind between '{' and '}' delimiters
    // tokenify the string between '{' and '}' based on arithmetic ops and delimiters (i.e., +-()/*^{}[]) and find based
    // on tokens the delimiters '{' and '}' are not removed.

    std::string tString(aString);

    size_t tSearchPos = 0;

    // for each opening '{':
    while ((tSearchPos = tString.find('{', tSearchPos)) != std::string::npos)
    {
        // find closing '}':
        size_t tEndPos = tString.find('}', tSearchPos);
        if (tEndPos == std::string::npos)
        {
            std::stringstream ss;
            ss << "Opening '{' without closing '}' in '" << aString;
            throw Plato::ParsingException(ss.str());
        }

        // search between tSearchPos and tEndPos for aFind
        std::string tDelimiters(" +-()/*^{}[]");
        size_t tNextPos;
        do
        {
            tSearchPos++;
            tNextPos = tString.find_first_of(tDelimiters, tSearchPos);
            size_t tStrLen = tNextPos - tSearchPos;
            if (tString.substr(tSearchPos, tStrLen) == aFind)
            {
                tString.replace(tSearchPos, tStrLen, aReplace);
            }
            tSearchPos = tString.find_first_of(tDelimiters, tSearchPos);
            tEndPos = tString.find('}', tSearchPos);
        } while (tSearchPos != tEndPos);
    }
    return tString;
}

void PugiParser::addChildren(const pugi::xml_node& node, InputData& inputData) const
{
    for (pugi::xml_node& child : node.children())
    {
        if (std::string(child.child_value()) != "")
        {
            inputData.add<std::string>(child.name(), child.child_value());
        }
        else
        {
            Plato::InputData newData(child.name());
            addChildren(child, newData);
            inputData.add<Plato::InputData>(child.name(), newData);
        }
    }
}

InputData inputDataFromPugiParsedFile(const std::string& aFileName)
{
    Plato::PugiParser tParser;
    return tParser.parseFile(aFileName);
}

namespace Get
{
/******************************************************************************/
std::string String(Plato::InputData& aNode, const std::string& aFieldname, const std::string& aDefault)
/******************************************************************************/
{
    if (aNode.size<std::string>(aFieldname) == 0)
    {
        aNode.set<std::string>(aFieldname, aDefault);
    }
    return String(aNode, aFieldname);
}
/******************************************************************************/
std::string String(const Plato::InputData& aNode, const std::string& aFieldName, bool aToUpper)
/******************************************************************************/
{
    std::string tStrVal;
    if (aNode.size<std::string>(aFieldName))
    {
        tStrVal = aNode.get<std::string>(aFieldName);
    }
    else
        return tStrVal;

    if (aToUpper == true)
    {
        Plato::Parse::toUppercase(tStrVal);
    }
    return tStrVal;
}
/******************************************************************************/
Plato::InputData InputData(Plato::InputData& aNode, const std::string& aFieldName, unsigned int aIndex)
/******************************************************************************/
{
    if (aNode.size<Plato::InputData>(aFieldName) > aIndex)
    {
        // requested entry exists.  return it.
        return aNode.get<Plato::InputData>(aFieldName, aIndex);
    }
    else if (aNode.size<Plato::InputData>(aFieldName) == aIndex)
    {
        // requested entry is one past then end of the list.  add new entry then return it.
        aNode.add<Plato::InputData>(aFieldName, Plato::InputData(aFieldName));
        return aNode.get<Plato::InputData>(aFieldName, aIndex);
    }
    else
    {
        // requested entry is more than one past then end of the list.  return empty InputData but don't add it.
        return Plato::InputData(aFieldName);
    }
}
/******************************************************************************/
int Int(const Plato::InputData& aNode, const std::string& aFieldname, int aDefaultValue)
/******************************************************************************/
{
    if (aNode.size<std::string>(aFieldname))
    {
        const int tBase = 10;
        return std::strtol(aNode.get<std::string>(aFieldname).c_str(), nullptr, tBase);
    }
    else
    {
        return aDefaultValue;
    }
}
/******************************************************************************/
std::vector<double> Doubles(const Plato::InputData& aInputData, const std::string& aKeyword)
/******************************************************************************/
{
    std::vector<double> tValues;
    auto tStrValues = aInputData.getByName<std::string>(aKeyword);
    for (auto tStrValue : tStrValues)
    {
        double tValue = std::strtold(tStrValue.c_str(), nullptr);
        tValues.push_back(tValue);
    }
    return (tValues);
}
/******************************************************************************/
std::vector<int> Ints(const Plato::InputData& aInputData, const std::string& aKeyword)
/******************************************************************************/
{
    std::vector<int> tValues;
    auto tStrValues = aInputData.getByName<std::string>(aKeyword);
    for (auto tStrValue : tStrValues)
    {
        double tValue = std::stoi(tStrValue.c_str());
        tValues.push_back(tValue);
    }
    return (tValues);
}
/******************************************************************************/
double Double(const Plato::InputData& aNode, const std::string& aFieldname)
/******************************************************************************/
{
    if (aNode.size<std::string>(aFieldname))
    {
        return atof(aNode.get<std::string>(aFieldname).c_str());
    }
    else
    {
        return 0.0;
    }
}
/******************************************************************************/
bool Bool(const Plato::InputData& aNode, const std::string& aFieldname, bool aDefaultValue)
/******************************************************************************/
{
    if (aNode.size<std::string>(aFieldname))
    {
        std::string strval = aNode.get<std::string>(aFieldname);
        return Parse::boolFromString(strval);
    }
    else
    {
        return aDefaultValue;
    }
}

}  // namespace Get

namespace Parse
{

/******************************************************************************/
std::string getString(pugi::xml_node& aNode, const std::string& aFieldname, const std::string& aDefault)
/******************************************************************************/
{
    if (numChildren(aNode, aFieldname) == 0)
    {
        pugi::xml_node newNode = aNode.append_child(aFieldname.c_str());
        newNode.append_child(pugi::node_pcdata).set_value(aDefault.c_str());
    }
    return getString(aNode, aFieldname);
}

/******************************************************************************/
std::string getStringValue(const pugi::xml_node& aNode, bool aToUpper)
/******************************************************************************/
{
    std::string tStrValue = aNode.child_value();
    if (aToUpper == true)
    {
        Plato::Parse::toUppercase(tStrValue);
    }
    return tStrValue;
}

/******************************************************************************/
std::string getString(const pugi::xml_node& aNode, const std::string& aFieldName, bool aToUpper)
/******************************************************************************/
{
    std::string tStrVal = aNode.child_value(aFieldName.c_str());
    if (aToUpper == true)
    {
        Plato::Parse::toUppercase(tStrVal);
    }
    return tStrVal;
}

/******************************************************************************/
std::vector<std::string> getStrings(const pugi::xml_node& aXML_Node, const std::string& aKeyword)
/******************************************************************************/
{
    std::vector<std::string> tValueNames;
    for (pugi::xml_node tNode = aXML_Node.child(aKeyword.c_str()); tNode; tNode = tNode.next_sibling(aKeyword.c_str()))
    {
        const std::string tValue = Plato::Parse::getStringValue(tNode);
        tValueNames.push_back(tValue);
    }
    return (tValueNames);
}

/******************************************************************************/
int getInt(const pugi::xml_node& aNode, const std::string& aFieldname)
/******************************************************************************/
{
    const int tBase = 10;
    return std::strtol(aNode.child_value(aFieldname.c_str()), nullptr, tBase);
}

/******************************************************************************/
double getDouble(const pugi::xml_node& aNode)
/******************************************************************************/
{
    std::string tValue = aNode.child_value();
    return (std::strtold(tValue.c_str(), nullptr));
}

/******************************************************************************/
std::vector<double> getDoubles(const pugi::xml_node& aXML_Node, const std::string& aKeyword)
/******************************************************************************/
{
    std::vector<double> tValues;
    for (pugi::xml_node tNode = aXML_Node.child(aKeyword.c_str()); tNode; tNode = tNode.next_sibling(aKeyword.c_str()))
    {
        const double tValue = Plato::Parse::getDouble(tNode);
        tValues.push_back(tValue);
    }
    return (tValues);
}

/******************************************************************************/
double getDouble(const pugi::xml_node& aNode, const std::string& aFieldname)
/******************************************************************************/
{
    return atof(aNode.child_value(aFieldname.c_str()));
}

/******************************************************************************/
bool getBool(const pugi::xml_node& aNode, const std::string& aFieldname)
/******************************************************************************/
{
    std::string strval = aNode.child_value(aFieldname.c_str());
    return boolFromString(strval);
}

/******************************************************************************/
bool boolFromString(const std::string& strval)
/******************************************************************************/
{
    if (strval.size() == 0)
    {
        return false;
    }
    std::string strCopy(strval);
    Plato::Parse::toUppercase(strCopy);
    if (strCopy == "FALSE")
    {
        return false;
    }
    else if (strCopy == "TRUE")
    {
        return true;
    }

    std::stringstream ss;
    ss << "Plato::Parser: expected a boolean ('True','False'), not '" << strval << "'.";
    throw Plato::ParsingException(ss.str());
}

/******************************************************************************/
int numChildren(const pugi::xml_node& aNodeIn, const std::string& aFieldname)
/******************************************************************************/
{
    int tCount = 0;
    for (pugi::xml_node aNode = aNodeIn.child(aFieldname.c_str()); aNode;
         aNode = aNode.next_sibling(aFieldname.c_str()))
    {
        tCount++;
    }
    return tCount;
}

/******************************************************************************/
void toUppercase(std::string& aString)
/******************************************************************************/
{
    std::transform(aString.begin(), aString.end(), aString.begin(), ::toupper);
}

/******************************************************************************/
void loadFile(pugi::xml_document& aInput)
/******************************************************************************/
{
    const char* tInputChar = std::getenv("PLATO_APP_FILE");
    pugi::xml_parse_result tResult = aInput.load_file(tInputChar);
    if (!tResult)
    {
        // throw
    }
}

std::string keyword(const Plato::InputData& aNode, const std::string& aKeyword, const std::string& aDefault)
{
    auto tOutput = Plato::Get::String(aNode, aKeyword);
    std::transform(tOutput.begin(), tOutput.end(), tOutput.begin(), ::tolower);
    tOutput = tOutput.empty() ? aDefault : tOutput;
    return tOutput;
}
// function parseKeyword

/******************************************************************************/
void parseArgumentNameInputs(const Plato::InputData& aInputNode, std::vector<std::string>& aInputs)
/******************************************************************************/
{
    assert(aInputs.empty() == true);
    auto tInputs = aInputNode.getByName<Plato::InputData>("Input");
    for (auto tInput = tInputs.begin(); tInput != tInputs.end(); ++tInput)
    {
        std::string tSharedDataName = tInput->get<std::string>("ArgumentName");
        aInputs.push_back(tSharedDataName);
    }
}

/******************************************************************************/
void parseArgumentNameOutputs(const Plato::InputData& aOutputNode, std::vector<std::string>& aOutputs)
/******************************************************************************/
{
    assert(aOutputs.empty() == true);
    auto tOutputs = aOutputNode.getByName<Plato::InputData>("Output");
    for (auto tOutput = tOutputs.begin(); tOutput != tOutputs.end(); ++tOutput)
    {
        std::string tSharedDataName = tOutput->get<std::string>("ArgumentName");
        aOutputs.push_back(tSharedDataName);
    }
}

/******************************************************************************/
void parseSharedDataNameInputs(const Plato::InputData& aInputNode, std::vector<std::string>& aInputs)
/******************************************************************************/
{
    assert(aInputs.empty() == true);
    auto tInputs = aInputNode.getByName<Plato::InputData>("Input");
    for (auto tInput = tInputs.begin(); tInput != tInputs.end(); ++tInput)
    {
        std::string tSharedDataName = tInput->get<std::string>("SharedDataName");
        aInputs.push_back(tSharedDataName);
    }
}

/******************************************************************************/
void parseSharedDataNameOutputs(const Plato::InputData& aOutputNode, std::vector<std::string>& aOutputs)
/******************************************************************************/
{
    assert(aOutputs.empty() == true);
    auto tOutputs = aOutputNode.getByName<Plato::InputData>("Output");
    for (auto tOutput = tOutputs.begin(); tOutput != tOutputs.end(); ++tOutput)
    {
        std::string tSharedDataName = tOutput->get<std::string>("SharedDataName");
        aOutputs.push_back(tSharedDataName);
    }
}

} /* namespace Parse */

} /* namespace Plato */
