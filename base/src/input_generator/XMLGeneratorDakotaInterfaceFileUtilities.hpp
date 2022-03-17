/*
 * XMLGeneratorDakotaInterfaceFileUtilities.hpp
 *
 *  Created on: Jan 18, 2022
 */

#pragma once

#include "pugixml.hpp"
#include "XMLGeneratorDataStruct.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \fn write_dakota_interface_xml_file
 * \brief Write interface.xml file for dakota driver.
 * \param [in]  aMetaData Plato problem input metadata
**********************************************************************************/
void write_dakota_interface_xml_file
(const XMLGen::InputData & aMetaData);

/******************************************************************************//**
 * \fn append_dakota_performer_data
 * \brief Append performer data to interface.xml file.
 * \param [in]     aMetaData Plato problem input metadata
 * \param [in/out] aDocument    parent xml document
**********************************************************************************/
void append_dakota_performer_data
(const XMLGen::InputData & aMetaData,
 pugi::xml_document      & aDocument);

/******************************************************************************//**
 * \fn append_physics_performers_dakota_usecase
 * \brief Append performer data to interface.xml file.
 * \param [in]     aMetaData Plato problem input metadata
 * \param [in/out] aDocument    parent xml document
 * \param [in]     aPerformerId performer id
**********************************************************************************/
void append_physics_performers_dakota_usecase
(const XMLGen::InputData & aMetaData,
 pugi::xml_document      & aDocument,
 int                     & aPerformerId);

/******************************************************************************//**
 * \fn append_platoservices_dakota_usecase
 * \brief Append performer data to interface.xml file.
 * \param [in]     aMetaData Plato problem input metadata
 * \param [in/out] aDocument    parent xml document
 * \param [in]     aPerformerId performer id
**********************************************************************************/
void append_platoservices_dakota_usecase
(const XMLGen::InputData & aMetaData,
 pugi::xml_document      & aDocument,
 int                     & aPerformerId);

/******************************************************************************//**
 * \fn append_dakota_shared_data
 * \brief Append shared data to interface.xml file.
 * \param [in]     aMetaData Plato problem input metadata
 * \param [in/out] aDocument    parent xml document
**********************************************************************************/
void append_dakota_shared_data
(const XMLGen::InputData & aMetaData,
 pugi::xml_document      & aDocument);

/******************************************************************************//**
 * \fn append_concurrent_design_variables_shared_data
 * \brief Append parameters sensitivity shared data for shape optimization problems.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_concurrent_design_variables_shared_data
(const XMLGen::InputData & aMetaData,
 pugi::xml_document      & aDocument);

 /******************************************************************************//**
 * \fn append_dakota_criterion_shared_data
 * \brief Append dakota driver criteria shared data to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_dakota_criterion_shared_data
(const XMLGen::InputData & aMetaData,
 pugi::xml_document      & aDocument);

/******************************************************************************//**
 * \fn append_dakota_stages
 * \brief Append stages to interface.xml file.
 * \param [in]     aMetaData Plato problem input metadata
 * \param [in/out] aDocument    parent xml document
**********************************************************************************/
void append_dakota_stages
(const XMLGen::InputData & aMetaData,
 pugi::xml_document      & aDocument);

/******************************************************************************//**
 * \fn append_initialize_stage
 * \brief Append initialize stage to interface.xml file.
 * \param [in]     aMetaData Plato problem input metadata
 * \param [in/out] aDocument    parent xml document
**********************************************************************************/
void append_initialize_stage
(const XMLGen::InputData & aMetaData,
 pugi::xml_document      & aDocument);

/******************************************************************************//**
 * \fn append_design_parameters_input
 * \brief Append design parameters shared data as input.
 * \param [in/out] aParentNode    parent xml node
**********************************************************************************/
void append_design_parameters_input(pugi::xml_node & aParentNode);

/******************************************************************************//**
 * \fn append_concurrent_update_geometry_on_change_operation
 * \brief Append update geometry on change operation for concurrent design parameters.
 * \param [in/out] aParentNode    parent xml node
**********************************************************************************/
void append_concurrent_update_geometry_on_change_operation(pugi::xml_node & aParentNode);

/******************************************************************************//**
 * \fn append_concurrent_physics_performer_tet10_conversion_operation
 * \brief Append convert to tet10 on change operation for all physics performers with
  concurrent design parameters.
 * \param [in]     aMetaData Plato problem input metadata
 * \param [in/out] aParentNode    parent xml node
**********************************************************************************/
void append_concurrent_physics_performer_tet10_conversion_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_concurrent_physics_performer_decomp_operation
 * \brief Append decomp on change operation for all physics performers with
  concurrent design parameters.
 * \param [in]     aMetaData Plato problem input metadata
 * \param [in/out] aParentNode    parent xml node
**********************************************************************************/
void append_concurrent_physics_performer_decomp_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_concurrent_reinitialize_on_change_operation
 * \brief Append reinitialize on change operation for all physics performers with
  concurrent design parameters.
 * \param [in]     aMetaData Plato problem input metadata
 * \param [in/out] aParentNode    parent xml node
**********************************************************************************/
void append_concurrent_reinitialize_on_change_operation
(const XMLGen::InputData & aMetaData,
 pugi::xml_node          & aParentNode);

/******************************************************************************//**
 * \fn append_criterion_value_stages
 * \brief Append criterion value stages to interface.xml file.
 * \param [in]     aMetaData Plato problem input metadata
 * \param [in/out] aDocument    parent xml document
**********************************************************************************/
void append_criterion_value_stages
(const XMLGen::InputData & aMetaData,
 pugi::xml_document      & aDocument);

/******************************************************************************//**
 * \fn append_objective_criterion_value_stages
 * \brief Append objective criteria value operations to interface.xml file.
 * \param [in]     aMetaData Plato problem input metadata
 * \param [in/out] aDocument    parent xml document
 * \param [in]     aCriterionNumber  criterion ordering number
**********************************************************************************/
void append_objective_criterion_value_stages
(const XMLGen::InputData & aMetaData,
 pugi::xml_document      & aDocument,
 int                     & aCriterionNumber);

/******************************************************************************//**
 * \fn append_concurrent_criterion_value_operation
 * \brief Append parallel criterion value operation to interface.xml file.
 * \param [in/out] aParentNode       parent xml node
 * \param [in]     aService          Service computing criterion
 * \param [in]     aIdentifierString criterion identifier string
**********************************************************************************/
void append_concurrent_criterion_value_operation
(pugi::xml_node    & aParentNode,
 XMLGen::Service   & aService,
 const std::string & aIdentifierString);

/******************************************************************************//**
 * \fn append_constraint_criterion_value_stages
 * \brief Append constraint criteria value operations to interface.xml file.
 * \param [in]     aMetaData Plato problem input metadata
 * \param [in/out] aDocument    parent xml document
 * \param [in]     aCriterionNumber  criterion ordering number
**********************************************************************************/
void append_constraint_criterion_value_stages
(const XMLGen::InputData & aMetaData,
 pugi::xml_document      & aDocument,
 int                     & aCriterionNumber);

/******************************************************************************//**
 * \fn append_dakota_driver_options
 * \brief Append dakota driver options/stages to interface.xml file.
 * \param [in]     aMetaData Plato problem input metadata
 * \param [in/out] aDocument    parent xml document
**********************************************************************************/
void append_dakota_driver_options
(const XMLGen::InputData & aMetaData,
 pugi::xml_document      & aDocument);

/******************************************************************************//**
 * \fn append_initialize_stage_options
 * \brief Append initialize stage options for dakota driver in interface.xml file.
 * \param [in/out] aParentNode    parent xml node
**********************************************************************************/
void append_initialize_stage_options(pugi::xml_node & aParentNode);

/******************************************************************************//**
 * \fn append_objective_criteria_stage_options
 * \brief Append objective criteria stage options for dakota driver in interface.xml file.
 * \param [in]     aMetaData Plato problem input metadata
 * \param [in/out] aParentNode       parent xml node
 * \param [in]     aCriterionNumber  criterion ordering number
**********************************************************************************/
void append_objective_criteria_stage_options
(const XMLGen::InputData & aMetaData,
 pugi::xml_node          & aParentNode,
 int                     & aCriterionNumber);

/******************************************************************************//**
 * \fn append_criterion_stage_options
 * \brief Append criterion stage options for dakota driver in interface.xml file.
 * \param [in/out] aParentNode       parent xml node
 * \param [in]     aIdentifierString criterion identifier string
 * \param [in]     aCriterionNumber  criterion ordering number
**********************************************************************************/
void append_criterion_stage_options
(pugi::xml_node    & aParentNode,
 const std::string & aIdentifierString,
 int                 aCriterionNumber);

/******************************************************************************//**
 * \fn append_constraint_criteria_stage_options
 * \brief Append constraint criteria stage options for dakota driver in interface.xml file.
 * \param [in]     aMetaData Plato problem input metadata
 * \param [in/out] aParentNode       parent xml node
 * \param [in]     aCriterionNumber  criterion ordering number
**********************************************************************************/
void append_constraint_criteria_stage_options
(const XMLGen::InputData & aMetaData,
 pugi::xml_node          & aParentNode,
 int                     & aCriterionNumber);

}
// namespace XMLGen
