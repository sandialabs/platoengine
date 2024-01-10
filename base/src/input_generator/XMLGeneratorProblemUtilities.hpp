/*
 * XMLGeneratorProblemUtilities.hpp
 *
 *  Created on: March 25, 2022
 */

#pragma once

namespace XMLGen
{

struct InputData;

namespace Problem
{

 /******************************************************************************//**
 * \fn create_subdirectory_for_evaluation
 * \brief create subdirectory and copy required files for performer
 * \param [in] aCsmFileName csm file name
 * \param [in] aEvaluation evaluation ID
**********************************************************************************/
void create_subdirectory_for_evaluation
(const std::string& aCsmFileName,
 int aEvaluation);

/******************************************************************************//**
 * \fn create_concurrent_evaluation_subdirectories
 * \brief create directories for concurrent evaluation performers
 * \param [in] aMetaData input metadata
**********************************************************************************/
void create_concurrent_evaluation_subdirectories
(const XMLGen::InputData& aMetaData);

/******************************************************************************//**
 * \fn move_input_deck_to_subdirectory
 * \brief move performer input deck to subdirectory
 * \param [in] aInputFileName performer input file name
**********************************************************************************/
void move_input_deck_to_subdirectory
(const std::string& aInputFileName,
 int aEvaluation);

/******************************************************************************//**
 * \fn write_plato_services_performer_input_deck_files
 * \brief Write the input deck file for the performer in the metada
 * \param [in] aMetaData input metadata
**********************************************************************************/
void write_plato_services_performer_input_deck_files
(XMLGen::InputData& aMetaData);

}
// namespace Problem

}
// namespace XMLGen