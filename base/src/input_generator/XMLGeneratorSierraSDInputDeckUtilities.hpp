/*
 * XMLGeneratorSierraSDInputDeckUtilities.hpp
 *
 *  Created on: Feb 6, 2021
 */

#pragma once

#include <fstream>

#include "pugixml.hpp"

#include "XMLGeneratorDataStruct.hpp"

namespace XMLGen
{

void write_sierra_sd_input_deck
(const XMLGen::InputData& aXMLMetaData);

void write_sierra_sd_modal_input_deck
(const std::string& aFilename,
 const XMLGen::Run& aRun,
 const XMLGen::InputData& aXMLMetaData);

void augment_sierra_sd_input_deck_with_plato_problem_description(const XMLGen::InputData &aXMLMetaData, std::istream &inputDeck, std::ostream &outfile);

void write_inverse_methods_block
(const XMLGen::Criterion & aCriterion,
 std::ostream &outfile);

void write_inverse_method_objective
(const XMLGen::Criterion &aCriterion, 
 std::ostream &outfile);

void write_match_nodesets
(const XMLGen::Criterion &aCriterion, 
 std::ostream &outfile);

void write_modes_to_exclude
(const XMLGen::Criterion &aCriterion, 
 std::ostream &outfile);

}
// namespace XMLGen
