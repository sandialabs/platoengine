/*
 * XMLGeneratorSierraSDUtilities.hpp
 *
 *  Created on: Jan 17, 2022
 */

#pragma once

#include "XMLGeneratorDataStruct.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \fn do_tet10_conversion
 * \brief determine whether we are using tet10 conversion
 * \param [in]     aMetaData Plato problem input data
**********************************************************************************/
bool do_tet10_conversion(const XMLGen::InputData& aMetaData);

/******************************************************************************//**
 * \fn get_salinas_service_id
 * \brief Get the id of the Sierra/SD service
 * \param [in]     aMetaData Plato problem input metadata
**********************************************************************************/
std::string get_salinas_service_id(const XMLGen::InputData& aMetaData);

/******************************************************************************//**
 * \fn extract_metadata_for_writing_sd_input_deck
 * \brief Extract service, scenario, and criterion input metadata for Sierra-SD runs.
 * \param [in] aMetaData   Plato problem input metadata
 * \param [out] aService   service input metadata 
 * \param [out] aScenario  scenario input metadata 
 * \param [out] aCriterion criterion input metadata 
**********************************************************************************/
bool extract_metadata_for_writing_sd_input_deck
(const XMLGen::InputData &aMetaData,
 XMLGen::Service &aService,
 XMLGen::Scenario &aScenario,
 XMLGen::Criterion &aCriterion);

}
// namespace XMLGen