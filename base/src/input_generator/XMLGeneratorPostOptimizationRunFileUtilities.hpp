/*
 * XMLGeneratorPostOptimizationRunFileUtilities.hpp
 *
 *  Created on: Aug 26, 2021
 */

#pragma once

#include "pugixml.hpp"

#include "XMLGeneratorDataStruct.hpp"
#include "XMLGeneratorRunMetadata.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \fn write_post_optimization_run_files
 * \brief Write files needed for post optimization runs.
 * \param [in] aMetaData  input meta data
**********************************************************************************/
void write_post_optimization_run_files
(const XMLGen::InputData& aMetaData);

/******************************************************************************//**
 * \fn build_post_optimization_run_input_deck_name
 * \brief Build the name of the input deck file for the run.
 * \param [in] aRun The run we are building the filename for.
**********************************************************************************/
std::string build_post_optimization_run_input_deck_name
(const XMLGen::Run &aRun);


}
// namespace XMLGen
