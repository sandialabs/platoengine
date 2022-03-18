/*
 * XMLGeneratorParseBlock.hpp
 *
 *  Created on: Mar 17, 2022
 */

#pragma once

#include "XMLGeneratorParseMetadata.hpp"
#include "XMLGeneratorDataStruct.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \class ParseBlock
 * \brief Parse inputs in block function blocks and store values in a vector of XMLGen::Block.
**********************************************************************************/
class ParseBlock : public XMLGen::ParseMetadata<std::vector<XMLGen::Block>>
{
private:
    XMLGen::MetaDataTags mTags; /*!< map from plato input file tags to valid tokens-value pairs, i.e. map<tag, pair<tokens,value> > */
    std::vector<XMLGen::Block> mData; /*!< block metadata */

private:
    /******************************************************************************//**
     * \fn allocate
     * \brief Allocate map from valid tags to valid tokens-value pair
    **********************************************************************************/
    void allocate();

    /******************************************************************************//**
     * \fn setName
     * \brief Set block name.
     * \param [in/out] aInputFile parsed input metadata
    **********************************************************************************/
    void setName(XMLGen::Block& aMetadata);

    /******************************************************************************//**
     * \fn setMaterialID
     * \brief Set block material ID.
     * \param [in/out] aInputFile parsed input metadata
    **********************************************************************************/
    void setMaterialID(XMLGen::Block& aMetadata);

    /******************************************************************************//**
     * \fn setElementType
     * \brief Set block element type.
     * \param [in/out] aInputFile parsed input metadata
    **********************************************************************************/
    void setElementType(XMLGen::Block& aMetadata);

    /******************************************************************************//**
     * \fn setBoundingBox
     * \brief Set block sub-block bounding box.
     * \param [in/out] aInputFile parsed input metadata
    **********************************************************************************/
    void setBoundingBox(XMLGen::Block& aMetadata);

    /******************************************************************************//**
     * \fn setMetaData
     * \brief Set block metadata.
     * \param [in] aMetadata block metadata
    **********************************************************************************/
    void setMetaData(XMLGen::Block& aMetadata);

    /******************************************************************************//**
     * \fn checkUniqueIDs
     * \brief Throw error if Block block identification numbers are not unique.
    **********************************************************************************/
    void checkUniqueIDs();

public:
    /******************************************************************************//**
     * \fn data
     * \brief Return block metadata.
     * \return container of block metadata
    **********************************************************************************/
    std::vector<XMLGen::Block> data() const override;

    /******************************************************************************//**
     * \fn parse
     * \brief Parse block metadata.
     * \param [in] aInputFile input metadata
    **********************************************************************************/
    void parse(std::istream& aInputFile) override;
};
// class ParseBlock

}
// namespace XMLGen
