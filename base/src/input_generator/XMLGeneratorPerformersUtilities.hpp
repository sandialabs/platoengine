/*
 * XMLGeneratorPerformersUtilities.hpp
 *
 *  Created on: Jan 14, 2022
 */

#pragma once

#include "pugixml.hpp"

#include "XMLGeneratorDataStruct.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \fn append_plato_main_performer
 * \brief Append Plato Main performer information to PUGI XML document.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument pugi::xml_document
 * \param [in]  aBasePerformerID base performer id (default=0)
 * \return cummulative performer id
**********************************************************************************/
int append_plato_main_performer
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aDocument,
 int aBasePerformerID = 0);

/******************************************************************************//**
 * \fn append_uniperformer_usecase
 * \brief Append uni-performer use case metadata to interface XML file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aParentNode  parent xml node
**********************************************************************************/
void append_uniperformer_usecase
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_multiperformer_usecase
 * \brief Append multi-performer uses case metadata to interface XML file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument pugi::xml_document
**********************************************************************************/
void append_multiperformer_usecase
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aNode);

}
// namespace XMLGen
