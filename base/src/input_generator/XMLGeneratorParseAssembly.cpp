/*
 * XMLGeneratorParseAssembly.cpp
 *
 *  Created on: June 1, 2021
 */

#include <algorithm>

#include "XMLGeneratorParseAssembly.hpp"

namespace XMLGen
{

void ParseAssembly::insertCoreProperties()
{
    mTags.insert({ "id", { { {"id"}, ""}, "" } });
    mTags.insert({ "type", { { {"type"}, ""}, "" } });
    mTags.insert({ "child_nodeset", { { {"child_nodeset"}, ""}, "" } });
    mTags.insert({ "parent_block", { { {"parent_block"}, ""}, "" } });
    mTags.insert({ "offset", { { {"offset"}, ""}, "0.0 0.0 0.0" } });
    mTags.insert({ "rhs_value", { { {"rhs_value"}, ""}, "0.0" } });
}

void ParseAssembly::allocate()
{
    mTags.clear();
    this->insertCoreProperties();
}


void ParseAssembly::setAssemblyIdentification(XMLGen::Assembly& aMetadata)
{
    if(aMetadata.id().empty())
    {
        auto tItr = mTags.find("id");
        if(tItr->second.first.second.empty())
        {
            THROWERR(std::string("Parse Assembly: assembly identification number is empty. ")
                + std::string("A unique assembly identification number must be assigned to an assembly block."))
        }
        aMetadata.id(tItr->second.first.second);
    }
}

void ParseAssembly::setType(XMLGen::Assembly& aMetadata)
{
    if(aMetadata.value("type").empty())
    {
        auto tItr = mTags.find("type");
        if(tItr->second.first.second.empty())
        {
            THROWERR("Parse Assembly: assembly type is empty.")
        }
        aMetadata.property("type", tItr->second.first.second);
    }
}

void ParseAssembly::setChildNodeset(XMLGen::Assembly& aMetadata)
{
    if(aMetadata.value("child_nodeset").empty())
    {
        auto tItr = mTags.find("child_nodeset");
        if(tItr->second.first.second.empty())
        {
            THROWERR("Parse Assembly: assembly child_nodeset is empty.")
        }
        aMetadata.property("child_nodeset", tItr->second.first.second);
    }
}

void ParseAssembly::setParentBlock(XMLGen::Assembly& aMetadata)
{
    if(aMetadata.value("parent_block").empty())
    {
        auto tItr = mTags.find("parent_block");
        if(tItr->second.first.second.empty())
        {
            THROWERR("Parse Assembly: assembly parent_block is empty.")
        }
        aMetadata.property("parent_block", tItr->second.first.second);
    }
}

void ParseAssembly::setRhsValue(XMLGen::Assembly& aMetadata)
{
    if(aMetadata.value("rhs_value").empty())
    {
        auto tItr = mTags.find("rhs_value");
        if(tItr->second.first.second.empty()) // set to default
        {
            if(tItr->second.second.empty())
            {
                THROWERR("Parse Assembly: assembly rhs_value default is empty.")
            }
            aMetadata.property("rhs_value", tItr->second.second);
        }
        else
        {
            aMetadata.property("rhs_value", tItr->second.first.second);
        }
    }
}

void ParseAssembly::setOffsetMetadata(XMLGen::Assembly& aMetadata)
{
    if(aMetadata.offset().size() == 0)
    {
        auto tItr = mTags.find("offset");
        if(tItr->second.first.second.empty()) // set to default
        {
            if(tItr->second.second.empty())
            {
                THROWERR("Parse Assembly: assembly offset default is empty.")
            }
            std::string tOffset = tItr->second.second;
            std::vector<std::string> tParsedOffset;
            char tOffsetBuffer[10000];
            strcpy(tOffsetBuffer, tOffset.c_str());
            XMLGen::parse_tokens(tOffsetBuffer, tParsedOffset);
            aMetadata.offset(tParsedOffset);
        } 
        else
        {
            std::string tOffset = tItr->second.first.second;
            std::vector<std::string> tParsedOffset;
            char tOffsetBuffer[10000];
            strcpy(tOffsetBuffer, tOffset.c_str());
            XMLGen::parse_tokens(tOffsetBuffer, tParsedOffset);
            aMetadata.offset(tParsedOffset);
        }
    }
}

void ParseAssembly::setMetadata(XMLGen::Assembly& aMetadata)
{
    this->setAssemblyIdentification(aMetadata);
    this->setType(aMetadata);
    this->setChildNodeset(aMetadata);
    this->setParentBlock(aMetadata);
    this->setOffsetMetadata(aMetadata);
    this->setRhsValue(aMetadata);
}

void ParseAssembly::checkUniqueIDs()
{
    std::vector<std::string> tIDs;
    for(auto& tMPC : mData)
    {
        tIDs.push_back(tMPC.id());
    }

    if(!XMLGen::unique(tIDs))
    {
        THROWERR("Parse Assembly: Assembly block identification numbers, i.e. IDs, are not unique.  Assembly block IDs must be unique.")
    }
}

std::vector<XMLGen::Assembly> ParseAssembly::data() const
{
    return mData;
}

void ParseAssembly::parse(std::istream &aInputFile)
{
    mData.clear();
    this->allocate();
    constexpr int MAX_CHARS_PER_LINE = 10000;
    std::vector<char> tBuffer(MAX_CHARS_PER_LINE);
    while (!aInputFile.eof())
    {
        // read an entire line into memory
        std::vector<std::string> tTokens;
        aInputFile.getline(tBuffer.data(), MAX_CHARS_PER_LINE);
        XMLGen::parse_tokens(tBuffer.data(), tTokens);
        XMLGen::to_lower(tTokens);

        std::string tMPCBlockID;
        if (XMLGen::parse_single_value(tTokens, { "begin", "assembly" }, tMPCBlockID))
        {
            XMLGen::Assembly tMetadata;
            tMetadata.id(tMPCBlockID);
            XMLGen::erase_tag_values(mTags);
            XMLGen::parse_input_metadata( { "end", "assembly" }, aInputFile, mTags);
            this->setMetadata(tMetadata);
            mData.push_back(tMetadata);
        }
    }
    this->checkUniqueIDs();
}

}
// namespace XMLGen
