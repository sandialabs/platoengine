/*
 * XMLGeneratorParseAssembly.hpp
 *
 *  Created on: June 1, 2021
 */

#pragma once

#include "XMLGeneratorParseMetadata.hpp"
#include "XMLGeneratorParserUtilities.hpp"
#include "XMLGeneratorAssemblyMetadata.hpp"

namespace XMLGen
{

class ParseAssembly : public XMLGen::ParseMetadata<std::vector<XMLGen::Assembly>>
{
private:
    XMLGen::MetaDataTags mTags; /*!< map from plato input file tags to valid tokens-value pairs, i.e. map<tag, pair<tokens,value> > */
    std::vector<XMLGen::Assembly> mData; /*!< assembly metadata */

private:
    /******************************************************************************//**
     * \fn allocate
     * \brief Allocate map from valid tags to valid token-value pair
    **********************************************************************************/
    void allocate();

    /******************************************************************************//**
     * \fn insertCoreProperties
     * \brief Insert core assembly properties, e.g. identifiers, to map from plato \n
     * input file tags to valid tokens-value pairs
    **********************************************************************************/
    void insertCoreProperties();

    /******************************************************************************//**
     * \fn setMetaData
     * \brief Set XMLGen::Assembly metadata.
     * \param [in/out] aInputFile parsed input metadata
    **********************************************************************************/
    void setMetadata(XMLGen::Assembly& aMetadata);

    /******************************************************************************//**
     * \fn checkUniqueIDs
     * \brief Throw error if Assembly block identification numbers are not unique.
    **********************************************************************************/
    void checkUniqueIDs();

public:
    /******************************************************************************//**
     * \fn data
     * \brief Return Assembly metadata.
     * \return metadata
    **********************************************************************************/
    std::vector<XMLGen::Assembly> data() const override;

    /******************************************************************************//**
     * \fn parse
     * \brief Parse Assembly metadata.
     * \param [in] aInputFile input file metadata.
    **********************************************************************************/
    void parse(std::istream &aInputFile) override;

    /******************************************************************************//**
     * \fn setAssemblyIdentification
     * \brief Set the Assembly id.
     * \param [in] aMetadata assembly metadata.
    **********************************************************************************/
    void setAssemblyIdentification(XMLGen::Assembly& aMetadata);

    /******************************************************************************//**
     * \fn setType
     * \brief Set the Assembly type.
     * \param [in] aMetadata assembly metadata.
    **********************************************************************************/
    void setType(XMLGen::Assembly& aMetadata);

    /******************************************************************************//**
     * \fn setChildNodeset
     * \brief Set the Assembly child nodeset.
     * \param [in] aMetadata assembly metadata.
    **********************************************************************************/
    void setChildNodeset(XMLGen::Assembly& aMetadata);

    /******************************************************************************//**
     * \fn setParentBlock
     * \brief Set the Assembly parent block.
     * \param [in] aMetadata assembly metadata.
    **********************************************************************************/
    void setParentBlock(XMLGen::Assembly& aMetadata);

    /******************************************************************************//**
     * \fn setOffsetMetadata
     * \brief Set the Assembly offset vector.
     * \param [in] aMetadata assembly metadata.
    **********************************************************************************/
    void setOffsetMetadata(XMLGen::Assembly& aMetadata);

    /******************************************************************************//**
     * \fn setRhsValue
     * \brief Set the Assembly right-hand side value.
     * \param [in] aMetadata assembly metadata.
    **********************************************************************************/
    void setRhsValue(XMLGen::Assembly& aMetadata);

};
// class ParseAssembly

}
// namespace XMLGen
