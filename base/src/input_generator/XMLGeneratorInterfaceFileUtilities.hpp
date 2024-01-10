/*
 * XMLGeneratorInterfaceFileUtilities.hpp
 *
 *  Created on: May 26, 2020
 */

#pragma once

#include "pugixml.hpp"
#include "XMLGeneratorDataStruct.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \fn write_interface_xml_file
 * \brief Write interface.xml file.
 * \param [in]  aMetaData Plato problem input metadata
**********************************************************************************/
void write_interface_xml_file
(const XMLGen::InputData& aMetaData);

/******************************************************************************//**
 * \fn append_performer_data
 * \brief Append performers to interface.xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aParentNode  parent xml node
**********************************************************************************/
void append_performer_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_console_data
 * \brief Append console data to interface.xml file.
 * \param [in]     aMetaData Plato problem input metadata
 * \param [in/out] aParentNode  parent xml node
**********************************************************************************/
void append_console_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_shared_data
 * \brief Append shared data to interface.xml file.
 * \param [in]     aMetaData Plato problem input metadata
 * \param [in/out] aDocument    parent xml document
**********************************************************************************/
void append_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_stages
 * \brief Append stages to interface.xml file.
 * \param [in]     aMetaData Plato problem input metadata
 * \param [in/out] aDocument    parent xml document
**********************************************************************************/
void append_stages
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_method_options
 * \brief Append options associated with a method, e.g. an optimization algorithm, \n 
          to the interface xml file.
 * \param [in]  aMetaData Plato problem input data
 * \param [out] aParentNode Parent xml node to add data to
**********************************************************************************/
void append_method_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_gradient_value_operation_for_multi_load_case
 * \brief Append the gradient value operation for multi-load case problems
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_gradient_value_operation_for_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode);

/******************************************************************************//**
 * \fn append_gradient_value_operation_for_non_multi_load_case
 * \brief Append the gradient value operation for non multi-load case problems
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_gradient_value_operation_for_non_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node         & aParentNode);

/******************************************************************************//**
 * \fn append_platoservice
 * \brief Append performer data to interface.xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml document
 * \param [in]  aBasePerformerID base performer id
 * \return cummulative performer id
**********************************************************************************/
int append_platoservice
(const XMLGen::InputData & aMetaData,
 pugi::xml_document      & aDocument,
 int                       aBasePerformerID = 0);

/******************************************************************************//**
 * \fn append_physics_performer
 * \brief Append performer data to interface.xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml document
 * \param [in]  aBasePerformerID base performer id
 * \return cummulative performer id
**********************************************************************************/
int append_physics_performer
(const XMLGen::InputData & aMetaData,
 pugi::xml_document      & aDocument,
 int                       aBasePerformerID = 0);

}
// namespace XMLGen
