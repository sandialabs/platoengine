/*
 * XMLGeneratorStagesUtilities.hpp
 *
 *  Created on: Jan 14, 2022
 */

#pragma once

#include "pugixml.hpp"

#include "XMLGeneratorDataStruct.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \fn append_design_volume_stage
 * \brief Append design volume stage to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_design_volume_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_initial_guess_stage
 * \brief Append initial guess stage to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_initial_guess_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_lower_bound_stage
 * \brief Append lower bound stage to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_lower_bound_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_upper_bound_stage
 * \brief Append upper bound stage to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_upper_bound_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_plato_main_output_stage
 * \brief Append plato main output stage to interface.xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_plato_main_output_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_cache_state_stage
 * \brief Append cache state stage to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_cache_state_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_update_problem_stage
 * \brief Append update problem stage to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_update_problem_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_constraint_value_stage
 * \brief Append constraint value stage to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_constraint_value_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_constraint_gradient_stage
 * \brief Append constraint gradient stage to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_constraint_gradient_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_constraint_stage_for_topology_problem
 * \brief Append constaint stage for topology optimization problems.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_constraint_stage_for_topology_problem 
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_constraint_stage_for_shape_problem
 * \brief Append constaint stage for shape optimization problems.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_constraint_stage_for_shape_problem 
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_constraint_gradient_stage_for_topology_problem
 * \brief Append constaint gradient stage for topology optimization problems.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_constraint_gradient_stage_for_topology_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_constraint_gradient_stage_for_shape_problem
 * \brief Append constaint gradient stage for shape optimization problems.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_constraint_gradient_stage_for_shape_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

 /******************************************************************************//**
 * \fn append_constraint_gradient_stage_for_topology_levelset_problem
 * \brief Append constaint gradient stage for topology levelset optimization problems.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_constraint_gradient_stage_for_topology_levelset_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_objective_value_stage
 * \brief Append objective value stage to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_objective_value_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_objective_value_stage_for_topology_problem
 * \brief Append objective value stage for topology optimization problems.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_objective_value_stage_for_topology_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_objective_value_stage_for_shape_problem
 * \brief Append objective value stage for shape optimization problems.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_objective_value_stage_for_shape_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_objective_gradient_stage
 * \brief Append objective gradient stage to interface xml file.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_objective_gradient_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_objective_gradient_stage_for_topology_problem
 * \brief Append objective gradient stage for topology optimization problems.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_objective_gradient_stage_for_topology_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_objective_gradient_stage_for_shape_problem
 * \brief Append objective gradient stage for shape optimization problems
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_objective_gradient_stage_for_shape_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

/******************************************************************************//**
 * \fn append_objective_gradient_stage_for_topology_problem
 * \brief Append objective gradient stage for topology optimization problem with levelset discretization.
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] aDocument parent xml node
**********************************************************************************/
void append_objective_gradient_stage_for_topology_levelset_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

}
// namespace XMLGen