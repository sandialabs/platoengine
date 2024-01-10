/*
 * XMLGeneratorParseMethodInputOptionsUtilities.hpp
 *
 *  Created on: Jan 18, 2022
 */

#pragma once

#include "XMLGeneratorParserUtilities.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \fn insert_topology_optimization_input_options
 * \brief Insert input options and corresponding defaults for topology optimization \n
 *        problems into the map from plato input deck keywords to the input value \n
 *        and default value pairs.
 * \param [in] aTags Plato input deck key-value map
**********************************************************************************/
void insert_topology_optimization_input_options(XMLGen::MetaDataTags &aTags);

/******************************************************************************//**
 * \fn insert_shape_optimization_input_options
 * \brief Insert input options and corresponding defaults for shape optimization \n
 *        problems into the map from plato input deck keywords to the input value \n
 *        and default value pairs.
 * \param [in] aTags Plato input deck key-value map
**********************************************************************************/
void insert_shape_optimization_input_options(XMLGen::MetaDataTags &aTags);

/******************************************************************************//**
 * \fn insert_general_optimization_input_options
 * \brief Insert input options and corresponding defaults for general optimization \n
 *        problem options into the map from plato input deck keywords to the input \n
 *        value and default value pairs.
 * \param [in] aTags Plato input deck key-value map
**********************************************************************************/
void insert_general_optimization_input_options(XMLGen::MetaDataTags &aTags);

/******************************************************************************//**
 * \fn insert_rol_input_options
 * \brief Insert input options and corresponding defaults for Rapid Optimization \n
 *        Library's (ROL) algorithms into the map from plato input deck keywords \n
 *        to the input value and default value pairs.
 * \param [in] aTags Plato input deck key-value map
**********************************************************************************/
void insert_rol_input_options(XMLGen::MetaDataTags &aTags);

/******************************************************************************//**
 * \fn insert_restart_input_options
 * \brief Insert input options and corresponding defaults for Plato's restart \n
 *        feature into the map from the plato input deck keyword to the input \n
 *        value and default value pairs.
 * \param [in] aTags Plato input deck key-value map
**********************************************************************************/
void insert_restart_input_options(XMLGen::MetaDataTags &aTags);

/******************************************************************************//**
 * \fn insert_derivative_checker_input_options
 * \brief Insert input options and corresponding defaults for Plato's derivative \n
          checker algorithm into the map from the plato input deck keyword to the \n
          input value and default value pairs.
 * \param [in] aTags Plato input deck key-value map
**********************************************************************************/
void insert_derivative_checker_input_options(XMLGen::MetaDataTags &aTags);

/******************************************************************************//**
 * \fn insert_general_dakota_input_options
 * \brief Insert input options and corresponding default values for Dakota's \n
          Driver problems into the map from the plato input deck \n
          keyword to the input value and default value pairs.
 * \param [in] aTags Plato input deck key-value map
**********************************************************************************/
void insert_general_dakota_input_options(XMLGen::MetaDataTags &aTags);

/******************************************************************************//**
 * \fn insert_dakota_multidim_param_study_input_options
 * \brief Insert input options and corresponding default values for Dakota's \n
          Multi-dimensional Parameter Study into the map from the plato input deck \n
          keyword to the input value and default value pairs.
 * \param [in] aTags Plato input deck key-value map
**********************************************************************************/
void insert_dakota_multidim_param_study_input_options(XMLGen::MetaDataTags &aTags);

/******************************************************************************//**
 * \fn insert_dakota_moga_input_options
 * \brief Insert input options and corresponding defaults for Dakota's Multi-Objective \n
          Genetic Algorithm (MOGA) into the map from the plato input deck keyword to the \n
          input value and default value pairs.
 * \param [in] aTags Plato input deck key-value map
**********************************************************************************/
void insert_dakota_moga_input_options(XMLGen::MetaDataTags &aTags);

/******************************************************************************//**
 * \fn insert_plato_filter_input_options
 * \brief Insert input options and corresponding defaults for Plato's filter \n
          technology used in topology optimization problems into the map from \n
          the plato input deck keyword to the input value and default value pairs.
 * \param [in] aTags Plato input deck key-value map
**********************************************************************************/
void insert_plato_filter_input_options(XMLGen::MetaDataTags &aTags);

/******************************************************************************//**
 * \fn insert_fixed_blocks_input_options
 * \brief Insert input options and corresponding defaults to enforce fixed block \n
          regions, i.e. frozen optimization regions, in topology optimization \n
          problems into the map from the plato input deck keyword to the input \n
          value and default value pairs.
 * \param [in] aTags Plato input deck key-value map
**********************************************************************************/
void insert_fixed_blocks_input_options(XMLGen::MetaDataTags &aTags);

/******************************************************************************//**
 * \fn insert_prune_and_refine_input_options
 * \brief Insert input options and corresponding defaults Plato's prune and refine \n
          algorithm into the map from the plato input deck keyword to the input \n
          value and default value pairs.
 * \param [in] aTags Plato input deck key-value map
**********************************************************************************/
void insert_prune_and_refine_input_options(XMLGen::MetaDataTags &aTags);

/******************************************************************************//**
 * \fn insert_plato_symmetry_filter_input_options
 * \brief Insert input options and corresponding defaults for Plato's symmetry \n
          enforcement filter technology used in topology optimization problems \n
          into the map from the plato input deck keyword to the input value and \n
          default value pairs.
 * \param [in] aTags Plato input deck key-value map
**********************************************************************************/
void insert_plato_symmetry_filter_input_options(XMLGen::MetaDataTags &aTags);

/******************************************************************************//**
 * \fn insert_plato_projection_filter_input_options
 * \brief Insert input options and corresponding defaults for Plato's projection \n
          filter technology used in topology optimization problems into the map \n
          from the plato input deck keyword to the input value and default value pairs.
 * \param [in] aTags Plato input deck key-value map
**********************************************************************************/
void insert_plato_projection_filter_input_options(XMLGen::MetaDataTags &aTags);

/******************************************************************************//**
 * \fn insert_plato_levelset_input_options
 * \brief Insert input options and corresponding default values for levelset based \n 
          topology optimization problems into the map from the plato input deck \n
          keyword to the input value and default value pairs.
 * \param [in] aTags Plato input deck key-value map
**********************************************************************************/
void insert_plato_levelset_input_options(XMLGen::MetaDataTags &aTags);

/******************************************************************************//**
 * \fn insert_amgx_input_options
 * \brief Insert input options and corresponding default values for AMGX solver \n
          into the map from the plato input deck keyword to the input value and \n
          default value pairs.
 * \param [in] aTags Plato input deck key-value map
**********************************************************************************/
void insert_amgx_input_options(XMLGen::MetaDataTags &aTags);

}
// namespace XMLGen
