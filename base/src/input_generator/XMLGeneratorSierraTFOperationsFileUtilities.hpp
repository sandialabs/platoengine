/*
 * XMLGeneratorSierraTFOperationsFileUtilities.hpp
 *
 *  Created on: July 5, 2022
 */

#pragma once

#include "pugixml.hpp"
#include "XMLGeneratorDataStruct.hpp"

namespace XMLGenSierraTF
{

/******************************************************************************//**
 * \fn write_sierra_tf_operation_xml_file
 * \brief Write SierraTF operations file
 * \param [in] aMetaData Plato problem input data
**********************************************************************************/
void write_sierra_tf_operation_xml_file
(const XMLGen::InputData& aXMLMetaData);

}
// namespace XMLGenSierraTF
