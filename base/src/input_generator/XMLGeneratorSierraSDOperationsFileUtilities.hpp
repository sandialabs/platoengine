/*
 * XMLGeneratorSierraSDOperationsFileUtilities.hpp
 *
 *  Created on: Feb 1, 2021
 */

#pragma once

#include "pugixml.hpp"
#include "XMLGeneratorDataStruct.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \fn write_sierra_sd_operation_xml_file
 * \brief Write SierraSD operations to plato_analyze_operation.xml file.
 * \param [in] aMetaData Plato problem input data
**********************************************************************************/
void write_sierra_sd_operation_xml_file
(const XMLGen::InputData& aXMLMetaData);

void append_internal_energy_gradient_operation_shape
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument);

void append_internal_energy_gradient_operation_topology
(const XMLGen::Scenario& aScenario,
 pugi::xml_document& aDocument);

}
// namespace XMLGen
