/*
 * XMLGeneratorUtilities_UnitTester.cpp
 *
 *  Created on: Jul 7, 2022
 */

#include <gtest/gtest.h>

#include "XMLGenerator_UnitTester_Tools.hpp"
#include "XMLGeneratorUtilities.hpp"

namespace PlatoTestXMLGenerator
{

TEST(PlatoTestXMLGenerator, StageOperation_count_number_of_reinitializations_needed_SierraTF)
{
    XMLGen::InputData aXMLMetaData;

    XMLGen::Service tService;
    tService.id("1");
    tService.code("sierra_tf");
    aXMLMetaData.append(tService);
    tService.id("2");
    aXMLMetaData.append(tService);

    XMLGen::Objective tObjective;
    tObjective.serviceIDs.push_back("1");
    tObjective.serviceIDs.push_back("2");
    aXMLMetaData.objective = tObjective;

    int tNum = XMLGen::count_number_of_reinitializations_needed(aXMLMetaData, tObjective);
    ASSERT_TRUE(tNum == 0);
}

}
// namespace PlatoTestXMLGenerator
