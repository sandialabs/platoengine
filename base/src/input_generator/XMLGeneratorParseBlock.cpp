/*
 * XMLGeneratorParseBlock.cpp
 *
 *  Created on: Mar 17, 2022
 */

#include <iostream>
#include <algorithm>

#include "XMLGeneratorParseBlock.hpp"
#include "XMLGeneratorParserUtilities.hpp"
#include "XMLGeneratorValidInputKeys.hpp"

namespace XMLGen
{

void ParseBlock::setTags(XMLGen::Block& aMetadata)
{
    XMLGen::ValidBlockKeys tValidKeys;
    for(auto& tTag : mTags)
    {
        auto tLowerKey = XMLGen::to_lower(tTag.first);
        auto tItr = std::find(tValidKeys.mKeys.begin(), tValidKeys.mKeys.end(), tLowerKey);
        if(tItr == tValidKeys.mKeys.end())
        {
            THROWERR(std::string("Check Keyword: keyword '") + tTag.first + std::string("' is not supported"))
        }
/*
        if(tTag.second.first.second.empty())
        {
            auto tDefaultValue = tTag.second.second;
            //aMetadata.append(tTag.first, tDefaultValue);
            if (tTag.first.empty())
            {
                THROWERR(std::string("XML Generator Block Metadata: Parameter with tag '") + tTag.first + "' is empty.")
            }
            auto tTag2 = Plato::tolower(tTag.first);
            aMetaData[tTag2].push_back(tDefaultValue);
        }
        else
        {
            auto tInputValue = tTag.second.first.second;
            //aMetadata.append(tTag.first, tInputValue);
            if (tTag.first.empty())
            {
                THROWERR(std::string("XML Generator Block Metadata: Parameter with tag '") + tTag.first + "' is empty.")
            }
            auto tTag2 = Plato::tolower(tTag.first);
            aMetaData[tTag2].push_back(tInputValue);
        }*/
    }
}

void ParseBlock::insertTag(std::string keyword, std::string defaultValue)
{
    mTags.insert({keyword, { { {keyword}, ""}, defaultValue } });
}

void ParseBlock::allocate()
{
    mTags.clear();

    insertTag("block_id");
    insertTag("name");
    insertTag("material");
    insertTag("element_type");
    insertTag("sub_block", "");
}

void ParseBlock::setName(XMLGen::Block& aMetadata)
{
    auto tItr = mTags.find("name");
    if (tItr != mTags.end() && !tItr->second.first.second.empty())
    {
        aMetadata.name = tItr->second.first.second;
    }
    else
        aMetadata.name = "block_" + aMetadata.block_id;
}

void ParseBlock::setMaterialID(XMLGen::Block& aMetadata)
{
    auto tItr = mTags.find("material");
    if (tItr != mTags.end() && !tItr->second.first.second.empty())
    {
        aMetadata.material_id = tItr->second.first.second;
    }
    else
    {
        THROWERR(std::string("Parse Block: block 'material' keyword is empty. ")
            + "At least one material ID must be defined for a block.")
    }
}

void ParseBlock::setElementType(XMLGen::Block& aMetadata)
{
    auto tItr = mTags.find("element_type");
    if (tItr != mTags.end() && !tItr->second.first.second.empty())
    {
        aMetadata.name = tItr->second.first.second;
    }
}

void ParseBlock::setBoundingBox(XMLGen::Block& aMetadata)
{
    auto tItr = mTags.find("sub_block");
    std::string tValues = tItr->second.first.second;
    if (tItr != mTags.end() && !tValues.empty())
    {
        std::vector<std::string> tCoordinates;
        char tValuesBuffer[10000];
        strcpy(tValuesBuffer, tValues.c_str());
        XMLGen::parse_tokens(tValuesBuffer, tCoordinates);
        aMetadata.bounding_box = tCoordinates;
    }
}

void ParseBlock::setMetaData(XMLGen::Block& aMetadata)
{
    this->setName(aMetadata);
    this->setMaterialID(aMetadata);
    this->setElementType(aMetadata);
    this->setBoundingBox(aMetadata);
    this->setTags(aMetadata);
}

void ParseBlock::checkUniqueIDs()
{
    std::vector<std::string> tIDs;
    for(auto& tBlock : mData)
    {
        tIDs.push_back(tBlock.block_id);
    }

    if(!XMLGen::unique(tIDs))
    {
        THROWERR("Parse Block: Block block identification numbers, i.e. IDs, are not unique. Block block IDs must be unique.")
    }
}

std::vector<XMLGen::Block> ParseBlock::data() const
{
    return mData;
}

void ParseBlock::parse(std::istream& aInputFile)
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

        std::string tBlockID;
        if (XMLGen::parse_single_value(tTokens, { "begin", "block" }, tBlockID))
        {
            if (tBlockID != "")
            {
                XMLGen::Block tMetadata;
                tMetadata.block_id = tBlockID;
                XMLGen::erase_tag_values(mTags);
                XMLGen::parse_input_metadata( { "end", "block" }, aInputFile, mTags);
                this->setMetaData(tMetadata);
                mData.push_back(tMetadata);
            }
        }
    }
    this->checkUniqueIDs();
}

}
// namespace XMLGen
