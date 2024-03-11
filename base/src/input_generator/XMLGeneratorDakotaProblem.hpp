/*
 * XMLGeneratorDakotaProblem.hpp
 *
 *  Created on: March 25, 2022
 */

#pragma once

namespace XMLGen
{

struct InputData;

namespace dakota
{

/******************************************************************************//**
 * \fn write_performer_input_deck_file
 * \brief Write the input deck file for the performer in the metada
 * \param [in] aMetaData input metadata
**********************************************************************************/
void write_performer_input_deck_file
(XMLGen::InputData& aMetaData);

/******************************************************************************//**
 * \fn write_performer_operation_xml_file
 * \brief Write the operations file for the performer in the metada
 * \param [in] aInputData input metadata
**********************************************************************************/
void write_performer_operation_xml_file
(const XMLGen::InputData& aMetaData);

/******************************************************************************//**
 * \fn plato_esp_problem
 * \brief Write input files needed to solve problems with dakota and engineering sketch pad (esp).
 * \param [in] aInputData input metadata
 * \param [in] aPreProcessedMetaData pre process metadata for problems with \n 
 *    concurrent app evaluations
**********************************************************************************/
void plato_esp_problem
(XMLGen::InputData& aMetaData,
 const std::vector<XMLGen::InputData>& aPreProcessedMetaData);

/******************************************************************************//**
 * \fn write_problem
 * \brief Write input files needed to solve problems with dakota.
 * \param [in] aInputData input metadata
 * \param [in] aPreProcessedMetaData pre process metadata for problems with \n 
 *    concurrent app evaluations
**********************************************************************************/
void write_problem
(XMLGen::InputData& aMetaData,
 const std::vector<XMLGen::InputData>& aPreProcessedMetaData);

}
// namespace dakota

}
// namespace XMLGen