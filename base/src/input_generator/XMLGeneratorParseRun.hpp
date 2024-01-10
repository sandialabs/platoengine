/*
 * XMLGeneratorParseRun.hpp
 *
 *  Created on: Aug 24, 2021
 */

#pragma once

#include "XMLGeneratorParseMetadata.hpp"
#include "XMLGeneratorParserUtilities.hpp"
#include "XMLGeneratorRunMetadata.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \class ParseRun
 * \brief Parse inputs in run block and store values in XMLGen::Run.
**********************************************************************************/
class ParseRun : public XMLGen::ParseMetadata<std::vector<XMLGen::Run>>
{
private:
    std::vector<XMLGen::Run> mData; 
    XMLGen::MetaDataTags mTags;

private:
    /******************************************************************************//**
     * \fn allocate
     * \brief Allocate map from valid tags to valid tokens-value pair
    **********************************************************************************/
    void allocate();

    /******************************************************************************//**
     * \fn setTags
     * \brief Set run metadata.
     * \param [in] aRun run metadata
    **********************************************************************************/
    void setTags(XMLGen::Run& aRun);

    /******************************************************************************//**
     * \fn finalize
     * \brief If required keywords are not defined, set values to default.
    **********************************************************************************/
    void finalize();

public:
    /******************************************************************************//**
     * \fn data
     * \brief Return run metadata.
     * \return metadata
    **********************************************************************************/
    std::vector<XMLGen::Run> data() const override;

    /******************************************************************************//**
     * \fn parse
     * \brief Parse Services metadata.
     * \param [in] aInputFile input file metadata.
    **********************************************************************************/
    void parse(std::istream &aInputFile) override;

    /******************************************************************************//**
     * \fn checkType
     * \brief Check if the Run has a valid type
     * \param [in] aRun Input run metadata.
    **********************************************************************************/
    void checkType(XMLGen::Run& aRun);

    /******************************************************************************//**
     * \fn checkCriterionOrCommand
     * \brief Check if the Run has a valid criterion or command
     * \param [in] aRun Input run metadata.
    **********************************************************************************/
    void checkCriterionOrCommand(XMLGen::Run& aRun);

};

}
// namespace XMLGen
