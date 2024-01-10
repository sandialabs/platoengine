/*
 * XMLGeneratorProblem.hpp
 *
 *  Created on: Nov 20, 2020
 */

#pragma once

namespace XMLGen
{

struct InputData;

namespace Problem
{

/******************************************************************************//**
 * \fn write_performer_operation_xml_file_gradient_based_problem
 * \brief Write the operations file for the performer in the metada
 * \param [in] aInputData input metadata
**********************************************************************************/
void write_performer_operation_xml_file_gradient_based_problem
(const XMLGen::InputData& aMetaData);

/******************************************************************************//**
 * \fn write_performer_input_deck_file_gradient_based_problem
 * \brief Write the input deck file for the performer in the metada
 * \param [in] aInputData input metadata
**********************************************************************************/
void write_performer_input_deck_file_gradient_based_problem
(const XMLGen::InputData& aMetaData);

/******************************************************************************//**
 * \fn write_optimization_problem
 * \brief Write input files needed to solve optimization problems with Plato Analyze.
 * \param [in] aInputData input metadata
**********************************************************************************/
void write_optimization_problem
(const XMLGen::InputData& aMetaData,
 const std::vector<XMLGen::InputData>& aPreProcessedMetaData);

/******************************************************************************//**
 * \fn write_dakota_problem
 * \brief Write input files needed to solve problems with dakota.
 * \param [in] aInputData input metadata
 * \param [in] aPreProcessedMetaData pre process metadata for problems with \n 
 *    concurrent app evaluations
**********************************************************************************/
void write_dakota_problem
(XMLGen::InputData& aMetaData,
 const std::vector<XMLGen::InputData>& aPreProcessedMetaData);

}
// namespace Problem

}
// namespace XMLGen
