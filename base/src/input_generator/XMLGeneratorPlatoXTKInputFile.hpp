/*
 * XMLGeneratorPlatoXTKInputFile.hpp
 *
 *  Created on: July 21 2021
 */

#pragma once

#include "pugixml.hpp"

#include "XMLGeneratorDataStruct.hpp"

namespace XMLGen
{
        void write_xtk_input_deck_file(const XMLGen::InputData & aInputData);

        // Takes a string of the form 0 1 2 to 0,1,2
        std::string format_input_string_for_xtk_xml(std::string aInputDeckString);

        void setup_union_set_parameters(const XMLGen::InputData & aInputData,
                                                pugi::xml_node & aXTKParamNodes);
}
// namespace XMLGen
