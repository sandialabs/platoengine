/*
 * XMLGeneratorOutputUtilities.hpp
 *
 *  Created on: Jan 17, 2022
 */

#pragma once

#include "pugixml.hpp"

#include "XMLGeneratorDataStruct.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \fn append_output_qoi_to_deterministic_write_output_operation
 * \brief Append output Quantities of Interests (QoIs) to the deterministic write \n
          output operation to be written in the interface xml file.
 * \param [in]  aMetaData       Plato problem input metadata
 * \param [in]  aOutputMetaData Plato problem output metadata
 * \param [out] aParentNode     parent xml node
**********************************************************************************/
void append_output_qoi_to_deterministic_write_output_operation
(const XMLGen::InputData& aMetaData,
 const XMLGen::Output& aOutputMetadata,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_output_qoi_to_nondeterministic_write_output_operation
 * \brief Append output Quantities of Interests (QoIs) to the nondeterministic \n 
          write output operation to be written in the interface xml file.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_output_qoi_to_nondeterministic_write_output_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_default_qoi_to_output_operation_in_interface_file
 * \brief Append default Quantities of Interests (QoIs) to the write output \n 
          operation to be written in the interface xml file.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_default_qoi_to_output_operation_in_interface_file
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_deterministic_qoi_to_output_operation_in_interface_file
 * \brief Append deterministic Quantities of Interests (QoIs) to the write output \n 
          operation to be written in the interface xml file.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_deterministic_qoi_to_output_operation_in_interface_file
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_objective_gradient_qoi_to_output_operation
 * \brief Append objective gradient Quantities of Interests (QoIs) to the write \n
          output operation to be written in the interface xml file.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_objective_gradient_qoi_to_output_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_constraint_gradient_qoi_to_output_operation
 * \brief Append constraint gradient Quantities of Interests (QoIs) to the write \n
          output operation to be written in the interface xml file.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_constraint_gradient_qoi_to_output_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

 /******************************************************************************//**
 * \fn append_deterministic_qoi_to_output_operation_in_interface_file_for_multi_load_case
 * \brief Append quantities of interest to output stage for multi load case problems.
 * \param [in] aMetaData    Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_deterministic_qoi_to_output_operation_in_interface_file_for_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_deterministic_qoi_to_output_operation_in_interface_file_for_non_multi_load_case
 * \brief Append quantities of interest to output stage for non multi load case problems.
 * \param [in]  aMetaData   Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_deterministic_qoi_to_output_operation_in_interface_file_for_non_multi_load_case 
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_nondeterministic_qoi_to_output_operation_in_interface_file
 * \brief Append nondeterministic Quantities of Interests (QoIs), e.g. samples, \n
          to the write output operation to be written in the interface xml file.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_nondeterministic_qoi_to_output_operation_in_interface_file
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_qoi_statistics_to_output_operation_in_interface_file
 * \brief Append statistics of nondeterministic Quantities of Interests (QoIs) \n
          to the write output operation to be written in the interface xml file.
 * \param [in]  aMetaData   Plato problem input metadata
 * \param [out] aParentNode parent xml node
**********************************************************************************/
void append_qoi_statistics_to_output_operation_in_interface_file
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

}
// namespace XMLGen