/*
 * XMLGeneratorParserUtilities.cpp
 *
 *  Created on: Jun 16, 2020
 */

#include <string>
#include <numeric>
#include <sstream>
#include <iostream>
#include <iterator>
#include <algorithm>

#include "XMLG_Macros.hpp"
#include "XMLGeneratorValidInputKeys.hpp"
#include "XMLGeneratorParserUtilities.hpp"

namespace XMLGen
{

std::string get_compound_scenario_id(const std::vector<std::string> &aScenarioIDs)
{
    std::string tReturn = "";
    for(auto tScenarioID : aScenarioIDs)
    {
        tReturn += tScenarioID;
    }
    return tReturn;
}

std::string get_concretized_criterion_identifier_string(ConcretizedCriterion aConcretizedCriterion)
{
    std::string tCriterionID = std::get<0>(aConcretizedCriterion);
    std::string tServiceID = std::get<1>(aConcretizedCriterion);
    std::string tScenarioID = std::get<2>(aConcretizedCriterion);

    std::string tIdentifierString = "";
    tIdentifierString += "criterion_" + tCriterionID + "_service_" + tServiceID + "_scenario_" + tScenarioID;

    return tIdentifierString;
}

bool unique(const std::vector<std::string>& aInput)
{
    auto tCopy = aInput;
    std::sort(tCopy.begin(), tCopy.end());
    auto tItr = std::unique( tCopy.begin(), tCopy.end() );
    return (tItr == tCopy.end());
}

void erase_tag_values(XMLGen::MetaDataTags& aTags)
{
    for (auto &tTag : aTags)
    {
        tTag.second.first.second.clear();
    }
}

void parse_input_metadata
(const std::vector<std::string>& aStopKeys,
 std::istream& aInputFile,
 XMLGen::MetaDataTags& aTags)
{
    constexpr int tMAX_CHARS_PER_LINE = 10000;
    std::vector<char> tBuffer(tMAX_CHARS_PER_LINE);
    while (!aInputFile.eof())
    {
        std::vector<std::string> tTokens;
        aInputFile.getline(tBuffer.data(), tMAX_CHARS_PER_LINE);
        XMLGen::parse_tokens(tBuffer.data(), tTokens);
        XMLGen::to_lower(tTokens);

        std::string tID;
        if (XMLGen::parse_single_value(tTokens, aStopKeys, tID))
        {
            break;
        }
        XMLGen::parse_tag_values(tTokens, aTags);
    }
}

void parse_input_metadata_with_valid_keyword_checking
(const std::vector<std::string>& aStopKeys,
 std::istream& aInputFile,
 XMLGen::MetaDataTags& aTags,
 std::vector<std::string> &aValidKeywords)
{
    constexpr int tMAX_CHARS_PER_LINE = 10000;
    std::vector<char> tBuffer(tMAX_CHARS_PER_LINE);
    while (!aInputFile.eof())
    {
        std::vector<std::string> tTokens;
        aInputFile.getline(tBuffer.data(), tMAX_CHARS_PER_LINE);
        XMLGen::parse_tokens(tBuffer.data(), tTokens);
        XMLGen::to_lower(tTokens);

        std::string tID;
        if (XMLGen::parse_single_value(tTokens, aStopKeys, tID))
        {
            break;
        }
        if(!XMLGen::check_for_valid_keyword(tTokens, aValidKeywords))
        {
            THROWERR(std::string("Check Keyword: keyword '") + tTokens[0] + std::string("' is not supported"))
        }
        XMLGen::parse_tag_values(tTokens, aTags);
    }
}

void parse_input_metadata_unlowered
(const std::vector<std::string>& aStopKeys,
 const std::vector<std::string>& aFindKeys,
 std::istream& aInputFile,
 XMLGen::MetaDataTags& aTags,
 std::string& aOutputString)
{
    constexpr int tMAX_CHARS_PER_LINE = 10000;
    std::vector<char> tBuffer(tMAX_CHARS_PER_LINE);
    while (!aInputFile.eof())
    {
        std::vector<std::string> tTokens;
        aInputFile.getline(tBuffer.data(), tMAX_CHARS_PER_LINE);
        XMLGen::parse_tokens(tBuffer.data(), tTokens);
        std::vector<std::string> tUnLoweredTokens = tTokens;
        XMLGen::to_lower(tTokens);

        std::string tID;
        int tIndex;
        if (XMLGen::parse_single_value(tTokens, aStopKeys, tID))
        {
            break;
        }
        else if (XMLGen::parse_single_value_index(tTokens, aFindKeys, tIndex))
        {
            aOutputString = tUnLoweredTokens[tIndex];
            if (aOutputString == "")
            {
                THROWERR(std::string("Parse input metadata unlowered: Can't find specified token for desired unlowered key"))
            }
        }
        else
        {
            XMLGen::parse_tag_values(tTokens, aTags);
        }
    }
}

bool parse_single_value
(const std::vector<std::string> &aTokens,
 const std::vector<std::string> &aTargetTokens,
 std::string &aKeyword)
{
    aKeyword = "";
    if(aTargetTokens.size() < 1 || aTokens.size() < 1 || aTokens.size() < aTargetTokens.size())
    {
        return false;
    }

    for(auto& tTargetToken : aTargetTokens)
    {
        auto tIndex = &tTargetToken - &aTargetTokens[0];
        if(aTokens[tIndex].compare(tTargetToken))
        {
            return false;
        }
    }

    if(aTokens.size() == (aTargetTokens.size() + 1u))
    {
        auto tIndex = aTargetTokens.size();
        aKeyword = aTokens[tIndex];
    }

    return true;
}
// function parse_single_value

bool parse_single_value_index
(const std::vector<std::string> &aTokens,
 const std::vector<std::string> &aTargetTokens,
 int &aIndex)
{
    if(aTargetTokens.size() < 1 || aTokens.size() < 1 || aTokens.size() < aTargetTokens.size())
    {
        return false;
    }

    for(auto& tTargetToken : aTargetTokens)
    {
        auto tIndex = &tTargetToken - &aTargetTokens[0];
        if(aTokens[tIndex].compare(tTargetToken))
        {
            return false;
        }
    }

    if(aTokens.size() == (aTargetTokens.size() + 1u))
    {
        auto tIndex = aTargetTokens.size();
        aIndex = tIndex;
    }

    return true;
}
// function parse_single_value_index

bool parse_target_keyword_index
(const std::vector<std::string> &aTokens, 
 const std::string &aTarget,
 int &aIndex)
{
    auto tMyIter = std::find(aTokens.begin(),aTokens.end(),aTarget);
    if( tMyIter != aTokens.end() )
    {
        aIndex = tMyIter - aTokens.begin();
        return true;
    }
    return false;
} 

// function parse_target_keyword_index
std::string to_upper(const std::string &aInput)
{
    std::string tOutput = aInput;
    std::transform(tOutput.begin(), tOutput.end(), tOutput.begin(), ::toupper);
    return tOutput;
}
// function to_upper

std::string to_lower(const std::string &aInput)
{
    std::string tOutput = aInput;
    std::transform(tOutput.begin(), tOutput.end(), tOutput.begin(), ::tolower);
    return tOutput;
}
// function to_lower

void to_lower(std::vector<std::string>& aInput)
{
    for(auto& tToken : aInput)
    {
        auto tIndex = &tToken - &aInput[0];
        aInput[tIndex] = XMLGen::to_lower(tToken);
    }
}
// function to_lower

bool parse_tokens(char *aBuffer, std::vector<std::string> &aTokens)
{
    const std::string tDELIMITER = " \t";
    constexpr int tMAX_TOKENS_PER_LINE = 5000;
    const char* tToken[tMAX_TOKENS_PER_LINE] = {}; // initialize to 0

    // strip any # comments to end of buffer
    const std::string tCOMMENT_DELIMETER = "#";
    std::string tBuffer(aBuffer);
    std::size_t tFound = tBuffer.find(tCOMMENT_DELIMETER);
    if(tFound!=std::string::npos)
        tBuffer=tBuffer.substr(0,tFound);

    // parse the line stripped of comments
    tToken[0] = std::strtok(&tBuffer[0], tDELIMITER.c_str()); // first token

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

std::string transform_keyword_values(const std::vector<std::string>& aTokens)
{
    std::ostringstream tOutput;
    for(auto& tToken : aTokens)
    {
        tOutput << tToken;
        auto tIndex = &tToken - &aTokens[0];
        if(static_cast<size_t>(tIndex) != (aTokens.size() - 1u))
        {
            tOutput << " ";
        }
    }
    return (tOutput.str());
}
// function transform_keyword_values

void is_input_keyword_empty
(const std::vector<std::string>& aInputTokens,
 const std::vector<std::string>& aTargetKey)
{
    if(aTargetKey.size() == aInputTokens.size())
    {
        auto tKeyword = XMLGen::transform_keyword_values(aTargetKey);
        THROWERR(std::string("Is Input Keyword Empty: Input keyword '") + tKeyword + "' value is empty.")
    }
}
// function is_input_keyword_empty

bool check_for_valid_keyword
(const std::vector<std::string>& aInputTokens,
 const std::vector<std::string>& aValidKeywords)
{
    for (auto& tKeyword : aValidKeywords)
    {
        if(tKeyword == aInputTokens[0])
        {
            return true;
        }
    }
    return false;
}

bool tokens_match
(const std::vector<std::string>& aInputTokens,
 const std::vector<std::string>& aTargetKey)
{
    if(aTargetKey.size() > aInputTokens.size())
    {
        return false;
    }

    std::vector<size_t> tMatch(aTargetKey.size());
    for (auto& tToken : aTargetKey)
    {
        auto tIndex = &tToken - &aTargetKey[0];
        tMatch[tIndex] = tToken == aInputTokens[tIndex] ? 1 : 0;
    }
    auto tSum = std::accumulate(tMatch.begin(), tMatch.end(), 0);
    auto tFoundMatch = static_cast<size_t>(tSum) == tMatch.size() ? true : false;
    return tFoundMatch;
}
// function tokens_match

void parse_tag_values(const std::vector<std::string>& aTokens, XMLGen::MetaDataTags& aTags)
{
    for (auto& tTag : aTags)
    {
        auto tFoundMatch = XMLGen::tokens_match(aTokens, tTag.second.first.first);
        if (tFoundMatch)
        {
            XMLGen::is_input_keyword_empty(aTokens, tTag.second.first.first);
            auto tBeginItr = aTokens.begin();
            auto tBeginIndex = tTag.second.first.first.size();
            std::vector<std::string> tTokenList;
            for (auto tItr = std::next(tBeginItr, tBeginIndex); tItr != aTokens.end(); tItr++)
            {
                tTokenList.push_back(tItr.operator*());
            }
            tTag.second.first.second = XMLGen::transform_keyword_values(tTokenList);
            break;
        }
    }
}
// function parse_tag_values

bool is_integer(const std::string& aInput)
{
    auto tIsDigit = true;
    for(auto tValue : aInput)
    {
        if(std::isdigit(tValue) == false)
        {
            tIsDigit = false;
            break;
        }
    }
    return tIsDigit;
}
// function is_integer

void split(const std::string& aInput, std::vector<std::string>& aOutput, bool aToLower)
{
    std::istringstream tInputSS(aInput);
    std::copy(std::istream_iterator<std::string>(tInputSS),
              std::istream_iterator<std::string>(),
              std::back_inserter(aOutput));

    if(aToLower) { XMLGen::to_lower(aOutput); }
}
// function split

std::string check_data_layout_keyword(const std::string& aInput)
{
    XMLGen::ValidLayoutKeys tValidKeys;
    auto tValue = tValidKeys.value(aInput);
    if(tValue.empty())
    {
        THROWERR("Check Data Layout Keyword: data layout keyword '" + aInput + "' is not supported.")
    }
    return tValue;
}
// check_data_layout_keyword

std::string check_output_keyword(const std::string& aInput)
{
    XMLGen::ValidOutputToLayoutKeys tValidKeys;
    auto tValue = tValidKeys.key(aInput);
    if(tValue.empty())
    {
        THROWERR("Check Output Keyword: output keyword '" + aInput + "' is not supported.")
    }
    return tValue;
}
// check_output_keyword

std::string return_output_qoi_data_layout(const std::string& aInput)
{
    XMLGen::ValidOutputToLayoutKeys tValidKeys;
    auto tValue = tValidKeys.value(aInput);
    if(tValue.empty())
    {
        THROWERR("Check Output Keyword: output keyword '" + aInput + "' is not supported.")
    }
    return tValue;
}
// return_output_qoi_data_layout

std::string check_data_layout(const std::string& aInput)
{
    XMLGen::ValidLayoutKeys tValidKeys;
    auto tValidLayout = tValidKeys.value(aInput);
    if(tValidLayout.empty())
    {
        THROWERR("Data layout '" + aInput + "' is not supported by Plato Engine.")
    }
    return tValidLayout;
}
// function check_data_layout

std::string check_code_keyword(const std::string& aInput)
{
    XMLGen::ValidCodeKeys tValidKeys;
    auto tValue = tValidKeys.value(aInput);
    if(tValue.empty())
    {
        THROWERR(std::string("Keyword 'code' with tag '") + tValue + "' is not supported by Plato Engine.")
    }
    return (tValue);
}
// function check_code_keyword

bool transform_boolean_key(const std::string& aInput)
{
    XMLGen::ValidBoolKeys tValidKeys;
    auto tValue = tValidKeys.value(aInput);
    if(tValue.empty())
    {
        THROWERR(std::string("Check Boolean Keyword: boolean keyword with value '")
            + aInput + "' is not supported. " + "Supported values are 'true' or 'false'.")
    }
    auto tFlag = tValue.compare("true") == 0 ? true : false;
    return tFlag;
}
// function transform_boolean_key

bool check_run_type(const std::string& aInput)
{
    XMLGen::ValidRunTypes tValidTypes;
    for(auto& tCurRunType : tValidTypes.mKeys)
    {
        if(tCurRunType == aInput)
        {
            return true;
        }
    }
    return false;
}

std::string check_physics_keyword(const std::string& aInput)
{
    XMLGen::ValidPhysicsKeys tValidKeys;
    auto tValue = tValidKeys.physics(aInput);
    if(tValue.empty())
    {
        THROWERR(std::string("Check Physics Keyword: keyword 'physics' with tag '") + aInput + "' is not supported.")
    }
    return tValue;
}
// function check_physics_keyword

std::string check_spatial_dimensions_keyword(const std::string& aInput)
{
    auto tLowerInput = XMLGen::to_lower(aInput);
    XMLGen::ValidSpatialDimsKeys tValidKeys;
    auto tItr = std::find(tValidKeys.mKeys.begin(), tValidKeys.mKeys.end(), tLowerInput);
    if(tItr == tValidKeys.mKeys.end())
    {
        THROWERR(std::string("Check Dimensions Keyword: keyword 'dimensions' with tag '") + tLowerInput + "' is not supported.")
    }
    return (tItr.operator*());
}
// function check_spatial_dimensions_keyword

void is_metadata_block_id_valid(const std::vector<std::string>& tTokens)
{
    if(tTokens.size() > 3u)
    {
        XMLGen::ValidPlatoInputFileMetaDataBlockKeys tValidKeys;
        auto tLowerInput = XMLGen::to_lower(tTokens[1]);
        auto tInputToken = XMLGen::transform_keyword_values(tTokens);
        THROWERR(std::string("\nIs Metadata Block ID Valid? - Input meta data block id '") + tInputToken + "' is invalid. Supported examples include:\n"
        + "  * begin metadata_block_name ID, where ID can be an integer, e.g. '1', or a string with the following formats, e.g. 'name' or 'name1_name2'. \n"
        + "  * Space indented formats, e.g. 'name1 name2', are not supported.\n"
        + "  * These four examples are valid alternatives: 1) begin " + tLowerInput + " 1, 2) begin " + tLowerInput + " plato, 3) begin "
        + tLowerInput + " plato_1,\n"
        + "    and 4) begin " + tLowerInput + " plato_is_the_best_optimization_based_design_tool.\n")
    }
}


}
