/*
 * XMLGeneratorSharedDataUtilities.hpp
 *
 *  Created on: Jan 14, 2022
 */

#pragma once

#include "pugixml.hpp"

#include "XMLGeneratorDataStruct.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \fn append_design_variables_shared_data
 * \brief Append design variables shared data to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_design_variables_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_control_shared_data
 * \brief Append control shared data to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_control_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_design_parameters_shared_data
 * \brief Append design parameters shared data for shape optimization problems.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_design_parameters_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

 /******************************************************************************//**
 * \fn append_lower_bounds_shared_data
 * \brief Append lower bounds shared data to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_lower_bounds_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_upper_bounds_shared_data
 * \brief Append upper bounds shared data to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_upper_bounds_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_design_volume_shared_data
 * \brief Append design volume shared data to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_design_volume_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_initial_control_shared_data
 * \brief Append initial control shared data - used in levelset topology optimization use cases.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
 void append_initial_control_shared_data
 (const XMLGen::InputData &aMetaData,
  pugi::xml_document &aDocument);

/******************************************************************************//**
 * \fn append_parameter_sensitivity_shared_data
 * \brief Append parameters sensitivity shared data for shape optimization problems.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_parameter_sensitivity_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

 /******************************************************************************//**
 * \fn append_gradient_based_criterion_shared_data
 * \brief Append gradient-based criteria shared data to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_gradient_based_criterion_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_gradient_based_constraint_shared_data
 * \brief Append gradient-based constraint shared data to inteface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_gradient_based_constraint_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

 /******************************************************************************//**
 * \fn append_gradient_based_objective_shared_data
 * \brief Append gradient-based objective shared data to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_gradient_based_objective_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_normalization_shared_data
 * \brief Append normalization variable shared data to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_normalization_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_qoi_shared_data
 * \brief Append quantities of interest shared data to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_qoi_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aParentNode);

 /******************************************************************************//**
 * \fn append_qoi_shared_data_for_multi_load_case
 * \brief Append quantities of interest shared data for multi load case problems.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_qoi_shared_data_for_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aParentNode);

/******************************************************************************//**
 * \fn append_qoi_shared_data_for_non_multi_load_case
 * \brief Append quantities of interest shared data for non multi load case problems.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_qoi_shared_data_for_non_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aParentNode);

/******************************************************************************//**
 * \fn append_topology_shared_data
 * \brief Append topology shared data to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_topology_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_helmholtz_topology_shared_data
 * \brief Append topology shared data for helmholtz filter to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_helmholtz_topology_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_projection_helmholtz_topology_shared_data
 * \brief Append topology and filtered control shared data for helmholtz filter
 * with projection to interface.xml file.
 * \param [in]  aMetaData            Plato problem input metadata
 * \param [out] aTopologyNode        topology shared data parent xml node
 * \param [out] aFilteredControlNode filtered control shared data xml node
**********************************************************************************/
void append_projection_helmholtz_topology_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aTopologyNode,
 pugi::xml_node& aFilteredControlNode);

/******************************************************************************//**
 * \fn get_filter_constraint_criterion_gradient_input_shared_data_name
 * \brief Get the name for the input shared data for constraint gradient filtering.
 * \param [in] aConstraint  constraint metadata 
 * \return shared data name for filtered constraint gradient
**********************************************************************************/
std::string get_filter_constraint_criterion_gradient_input_shared_data_name
(const XMLGen::Constraint &aConstraint);

/******************************************************************************//**
 * \fn get_filter_objective_criterion_gradient_input_shared_data_name
 * \brief Get the name for the input shared data for objective gradient filtering.
 * \param [in] aMetaData  Plato problem input data
**********************************************************************************/
std::string get_filter_objective_criterion_gradient_input_shared_data_name
(const XMLGen::InputData& aMetaData);

}
// namespace XMLGen