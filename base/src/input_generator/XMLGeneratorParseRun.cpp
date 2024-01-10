/*
 * XMLGeneratorParseRun.cpp
 *
 *  Created on: Jun 18, 2020
 */

#include <algorithm>

#include "XMLGeneratorParseRun.hpp"
#include "XMLGeneratorValidInputKeys.hpp"
#include "XMLGeneratorParserUtilities.hpp"

namespace XMLGen
{

std::vector<XMLGen::Run> ParseRun::data() const
{
    return mData;
}

void ParseRun::setTags(XMLGen::Run& aRun)
{
    for(auto& tTag : mTags)
    {
        if(tTag.second.first.second.empty())
        {
            auto tDefaultValue = tTag.second.second;
            aRun.append(tTag.first, tDefaultValue);
        }
        else
        {
            auto tInputValue = tTag.second.first.second;
            aRun.append(tTag.first, tInputValue);
        }
    }
}

void ParseRun::allocate()
{
    mTags.clear();
    mTags.insert({ "type", { { {"type"}, ""}, "" } });
    mTags.insert({ "criterion", { { {"criterion"}, ""}, "" } });
    mTags.insert({ "service", { { {"service"}, ""}, "" } });
    mTags.insert({ "command", { { {"command"}, ""}, "" } });
}

void ParseRun::finalize()
{
}

void ParseRun::checkType(XMLGen::Run& aRun)
{
    auto tType = aRun.value("type");
    if (tType.empty())
    {
        THROWERR("Parse Run: 'type' keyword is empty.")
    }
    if(!XMLGen::check_run_type(tType))
    {
        THROWERR("Parse Run: value of 'type' is invalid.")
    }
}

void ParseRun::checkCriterionOrCommand(XMLGen::Run& aRun)
{
    auto tCriterion = aRun.value("criterion");
    auto tCommand = aRun.value("command");
    if (tCriterion.empty() && tCommand.empty())
    {
        THROWERR("Parse Run: Both 'criterion' and 'command' keywords are empty. You must specify one or the other.")
    }
}

void ParseRun::parse(std::istream &aInputFile)
{
    mData.clear();
    this->allocate();
    constexpr int MAX_CHARS_PER_LINE = 10000;
    std::vector<char> tBuffer(MAX_CHARS_PER_LINE);
    XMLGen::ValidRunParameterKeys tValidKeys;
    while (!aInputFile.eof())
    {
        // read an entire line into memory
        std::vector<std::string> tTokens;
        aInputFile.getline(tBuffer.data(), MAX_CHARS_PER_LINE);
        XMLGen::parse_tokens(tBuffer.data(), tTokens);
        XMLGen::to_lower(tTokens);

        std::string tRunBlockID;
        if (XMLGen::parse_single_value(tTokens, { "begin", "run" }, tRunBlockID))
        {
            XMLGen::Run tRun;
            XMLGen::erase_tag_values(mTags);
            XMLGen::parse_input_metadata_with_valid_keyword_checking( { "end", "run" }, aInputFile, mTags, tValidKeys.mKeys);
            this->setTags(tRun);
            tRun.id(tRunBlockID);
            checkType(tRun);
            checkCriterionOrCommand(tRun);
            mData.push_back(tRun);
        }
    }
    this->finalize();
}

}
// namespace XMLGen
