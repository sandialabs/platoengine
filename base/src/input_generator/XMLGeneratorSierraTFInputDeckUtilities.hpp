/*
 * XMLGeneratorSierraTFInputDeckUtilities.hpp
 *
 *  Created on: July 6, 2022
 */

#pragma once

#include <fstream>

#include "pugixml.hpp"

#include "XMLGeneratorDataStruct.hpp"

namespace XMLGen
{

void write_sierra_tf_inverse_input_file
(const XMLGen::InputData& aXMLMetaData);

}
// namespace XMLGen
