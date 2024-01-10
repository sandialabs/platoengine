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

void ParseBlock::allocate()
{
    mTags.clear();

    mTags.insert({"block_id", { { {"block_id"}, ""}, "" } });
    mTags.insert({"name", { { {"name"}, ""}, "" } });
    mTags.insert({"material", { { {"material"}, ""}, "" } });
    mTags.insert({"element_type", { { {"element_type"}, ""}, "" } });
    mTags.insert({"sub_block", { { {"sub_block"}, ""}, "" } });
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
}

void ParseBlock::setElementType(XMLGen::Block& aMetadata)
{
    auto tItr = mTags.find("element_type");
    if (tItr != mTags.end() && !tItr->second.first.second.empty())
    {
        aMetadata.element_type = tItr->second.first.second;
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
        aMetadata.bounding_box = this->getBoundingBoxCoordinates(tCoordinates);
    }
}

std::vector<double> ParseBlock::getBoundingBoxCoordinates(const std::vector<std::string> &aCoordinates)
{
    if(aCoordinates.size() != 6 )
        THROWERR(std::string("Parse Block: Number of coordinate entries specifying bounding box for sub-block creation is not equal to 6."))
    
    std::vector<double> tBoundingBox(6, 0);
    for(unsigned int lBoundingBoxEntry = 0; lBoundingBoxEntry < aCoordinates.size(); ++lBoundingBoxEntry )
        tBoundingBox[lBoundingBoxEntry] = std::stod(aCoordinates[lBoundingBoxEntry]);
    
    if(tBoundingBox[0] >= tBoundingBox[3] || 
       tBoundingBox[1] >= tBoundingBox[4] || 
       tBoundingBox[2] >= tBoundingBox[5] )
        THROWERR(std::string("Parse Block: Bounding box coordinates must be ordered as: xmin ymin zmin xmax ymax zmax. Min and max values cannot be equal."))

    return tBoundingBox;
}

void ParseBlock::setMetaData(XMLGen::Block& aMetadata)
{
    this->setName(aMetadata);
    this->setMaterialID(aMetadata);
    this->setElementType(aMetadata);
    this->setBoundingBox(aMetadata);
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
