/*
 * XMLGeneratorGradBasedOptimizerOptions.hpp
 *
 *  Created on: Jan 17, 2022
 */

#pragma once

#include "pugixml.hpp"

#include "XMLGeneratorDataStruct.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \fn append_grad_based_optimizer_options
 * \brief Append gradeitn-based optimizer options to interface xml file.
 * \param [in]  aMetaData Plato problem input data
 * \param [out] aParentNode  parent node to append data to
**********************************************************************************/
void append_grad_based_optimizer_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_optimizer_update_problem_stage_options
 * \brief Append update problem options to optimization block in the interface xml file.
 * \param [in]  aMetaData Plato problem input data
 * \param [out] aParentNode  parent node to append data to
**********************************************************************************/
void append_optimizer_update_problem_stage_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_grad_based_optimizer_parameters
 * \brief Append optimization algorithm's options to interface xml file.
 * \param [in]  aMetaData Plato problem input data
 * \param [out] aParentNode  parent node to append data to
**********************************************************************************/
void append_grad_based_optimizer_parameters
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_optimizer_options
 * \brief Append optimizer options (such as Hessian type, num iterations, and update frequency) 
 *   to interface xml file.
 * \param [in]  aMetaData Plato problem input data
 * \param [out] aParentNode  parent node to append data to
**********************************************************************************/
void append_optimizer_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_reset_algorithm_on_update_option
 * \brief Append option specifying whether to reset the algorithm at the update frequency
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_reset_algorithm_on_update_option
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_rol_gradient_check_options
 * \brief Append options related to the derivative checker
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_rol_gradient_check_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn generate_rol_input_file
 * \brief Create the ROL input xml file
 * \param [in] aMetaData  Plato problem input data
**********************************************************************************/
void generate_rol_input_file
(const XMLGen::InputData& aMetaData);

/******************************************************************************//**
 * \fn append_rol_input_file
 * \brief Append the name of the ROL input xml file.
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_rol_input_file
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_rol_step_block
 * \brief Append option specifying whether to reset the algorithm at the update frequency
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_rol_step_block
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParent);

/******************************************************************************//**
 * \fn get_subproblem_model
 * \brief Map plato input deck keyword for the subproblem model to the \n 
          corresponding rol input deck keyword
 * \param [in]  aSubproblemModelString  Plato problem input data
 * return rol's keyword for subproblem model
**********************************************************************************/
 std::string get_subproblem_model(const std::string &aSubproblemModelString);

/******************************************************************************//**
 * \fn append_derivative_checker_options
 * \brief Append derivative checker options to interface xml file.
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_rol_gradient_check_flags
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_optimizer_cache_stage_options
 * \brief Append cache state options to optimization block in the interface xml file.
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_optimizer_cache_stage_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_grad_based_optimizer_output_options
 * \brief Append optimization output options to optimization block in the \n
          interface xml file.
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_grad_based_optimizer_output_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_grad_based_optimizer_variables_options
 * \brief Append gradient-based optimizer variables options to optimization \n 
          block in the interface xml file.
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_grad_based_optimizer_variables_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_grad_based_optimizer_objective_options
 * \brief Append gradient-based objective options to optimization block in \n 
          the interface xml file.
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_grad_based_optimizer_objective_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_grad_based_optimizer_constraint_options
 * \brief Append gradient-based constraint options to optimization blcok in \n
          the interface xml file.
 * \param [in]  aMetaData  Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_grad_based_optimizer_constraint_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_bound_constraints_options_topology_optimization
 * \brief Append optimizer bound constraint options to optimization block in \n 
          the interface xml file.
 * \param [in]  aValues     upper and lower bounds, i.e. {upper_value, lower_value}
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_bound_constraints_options_topology_optimization
(const std::vector<std::string>& aValues,
 pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn generate_target_value_entries
 * \brief Append constraint target values to optimization block in the interface xml file.
 * \param [in]  aMetaData   Plato problem input data
 * \param [in]  aConstraint constraint input metadata
 * \param [out] aKeyToValueMap key to value map of constraint target values
**********************************************************************************/
void generate_target_value_entries
(const XMLGen::InputData& aMetaData,
 const XMLGen::Constraint &aConstraint,
 std::map<std::string, std::string> &aKeyToValueMap);

/******************************************************************************//**
 * \fn append_rol_tolerances
 * \brief Append convergence tolerances to the rol input file.
 * \param [in]  aMetaData   Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_rol_tolerances(const XMLGen::InputData& aMetaData,
                           pugi::xml_node &aParent);

/******************************************************************************//**
 * \fn append_initial_trust_region_radius
 * \brief Append initial trust region radius to the rol input file.
 * \param [in]  aMetaData   Plato problem input data
 * \param [out] aParentNode Parent to add data to
**********************************************************************************/
void append_initial_trust_region_radius(const XMLGen::InputData& aMetaData,
                           pugi::xml_node &aParent);

}
//  namespace XMLGen
