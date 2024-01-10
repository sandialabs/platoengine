/*
 * XMLGeneratorDakotaDriverInputFileUtilities.hpp
 *
 *  Created on: Jan 24, 2022
 */

#pragma once

#include "pugixml.hpp"
#include "XMLGeneratorDataStruct.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \fn write_dakota_driver_input_deck
 * \brief Write plato_dakota_input_deck.i file for dakota driver.
 * \param [in]  aMetaData Plato problem input metadata
**********************************************************************************/
void write_dakota_driver_input_deck
(const XMLGen::InputData & aMetaData);

/******************************************************************************//**
 * \fn check_dakota_workflow
 * \brief check that valid dakota workflow is specified
 * \param [in]  aMetaData Plato problem input metadata
**********************************************************************************/
void check_dakota_workflow
(const XMLGen::InputData & aMetaData);

/******************************************************************************//**
 * \fn append_dakota_driver_title
 * \brief append title line
 * \param [in]  aMetaData Plato problem input metadata
 * \param [in/out] fp pointer to dakota driver input file
**********************************************************************************/
void append_dakota_driver_title
(const XMLGen::InputData & aMetaData,
 FILE*                   & fp);

/******************************************************************************//**
 * \fn append_dakota_driver_environment_block
 * \brief append environment block
 * \param [in]  aMetaData Plato problem input metadata
 * \param [in/out] fp pointer to dakota driver input file
**********************************************************************************/
void append_dakota_driver_environment_block
(const XMLGen::InputData & aMetaData,
 FILE*                   & fp);

/******************************************************************************//**
 * \fn append_dakota_driver_method_blocks
 * \brief append method blocks
 * \param [in]  aMetaData Plato problem input metadata
 * \param [in/out] fp pointer to dakota driver input file
**********************************************************************************/
void append_dakota_driver_method_blocks
(const XMLGen::InputData & aMetaData,
 FILE*                   & fp);

/******************************************************************************//**
 * \fn append_multidim_parameter_study_method_block
 * \brief append method block for multidim parameter study
 * \param [in]  aMetaData Plato problem input metadata
 * \param [in/out] fp pointer to dakota driver input file
**********************************************************************************/
void append_multidim_parameter_study_method_block
(const XMLGen::InputData & aMetaData,
 FILE*                   & fp);

/******************************************************************************//**
 * \fn append_surrogate_based_global_method_block
 * \brief append method block for surrogate based global
 * \param [in]  aMetaData Plato problem input metadata
 * \param [in/out] fp pointer to dakota driver input file
**********************************************************************************/
void append_surrogate_based_global_method_block
(const XMLGen::InputData & aMetaData,
 FILE*                   & fp);

/******************************************************************************//**
 * \fn append_moga_method_block
 * \brief append method block for multi objective genetic algorithm (moga)
 * \param [in]  aMetaData Plato problem input metadata
 * \param [in/out] fp pointer to dakota driver input file
**********************************************************************************/
void append_moga_method_block
(const XMLGen::InputData & aMetaData,
 FILE*                   & fp);

/******************************************************************************//**
 * \fn append_sampling_method_block
 * \brief append method block for sampling
 * \param [in]  aMetaData Plato problem input metadata
 * \param [in/out] fp pointer to dakota driver input file
**********************************************************************************/
void append_sampling_method_block
(const XMLGen::InputData & aMetaData,
 FILE*                   & fp);

/******************************************************************************//**
 * \fn append_dakota_driver_model_blocks
 * \brief append model blocks
 * \param [in]  aMetaData Plato problem input metadata
 * \param [in/out] fp pointer to dakota driver input file
**********************************************************************************/
void append_dakota_driver_model_blocks
(const XMLGen::InputData & aMetaData,
 FILE*                   & fp);

/******************************************************************************//**
 * \fn append_surrogate_model_block
 * \brief append model block for surrogate model
 * \param [in/out] fp pointer to dakota driver input file
**********************************************************************************/
void append_surrogate_model_block
(const XMLGen::InputData & aMetaData,
 FILE*                   & fp);

/******************************************************************************//**
 * \fn append_single_model_block
 * \brief append model block for single model
 * \param [in/out] fp pointer to dakota driver input file
**********************************************************************************/
void append_single_model_block
(FILE*                   & fp);

/******************************************************************************//**
 * \fn append_dakota_driver_variables_block
 * \brief append variables block
 * \param [in]  aCsmFileName Name of csm file
 * \param [in/out] fp pointer to dakota driver input file
**********************************************************************************/
void append_dakota_driver_variables_block
(const XMLGen::InputData & aMetaData,
 FILE*        & fp);

/******************************************************************************//**
 * \fn get_variable_strings_from_csm_file
 * \brief get variables strings for dakota input deck from csm file
 * \param [in]  aVariableStrings vector containing strings for variables 
 * \param [in]  aCsmFileName Name of csm file
 * \param [out] number of variables
**********************************************************************************/
size_t get_variable_strings_from_csm_file
(std::vector<std::string>& aVariablesStrings,
 const std::string& aCsmFileName,
 const XMLGen::InputData& aMetaData);

/******************************************************************************//**
 * \fn get_variable_strings_from_optimization_parameters
 * \brief get variables strings for dakota input deck from csm file
 * \param [in]  aVariableStrings vector containing strings for variables 
 * \param [in]  aMetaData Plato problem input metadata
 * \param [out] number of variables
**********************************************************************************/
size_t get_variable_strings_from_optimization_parameters
(std::vector<std::string>& aVariablesStrings,
 const XMLGen::InputData& aMetaData);

/******************************************************************************//**
 * \fn append_dakota_driver_interface_block
 * \brief append interface block
 * \param [in]  aMetaData Plato problem input metadata
 * \param [in/out] fp pointer to dakota driver input file
**********************************************************************************/
void append_dakota_driver_interface_block
(const XMLGen::InputData & aMetaData,
 FILE*                   & fp);

/******************************************************************************//**
 * \fn append_dakota_driver_responses_block
 * \brief append responses block
 * \param [in]  aMetaData Plato problem input metadata
 * \param [in/out] fp pointer to dakota driver input file
**********************************************************************************/
void append_dakota_driver_responses_block
(const XMLGen::InputData & aMetaData,
 FILE*                   & fp);

/******************************************************************************//**
 * \fn append_surrogate_based_global_responses_block
 * \brief append responses block for surrogate based global optimization workflow
 * \param [in]  aMetaData Plato problem input metadata
 * \param [in/out] fp pointer to dakota driver input file
**********************************************************************************/
void append_surrogate_based_global_responses_block
(const XMLGen::InputData & aMetaData,
 FILE*                   & fp);

}
// namespace XMLGen
