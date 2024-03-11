 /*
 * XMLGeneratorPlatoGemma_UnitTester.cpp
 *
 *  Created on: Feb 28, 2022
 */

#include <gtest/gtest.h>

#include "pugixml.hpp"
#include <fstream>

#include "XMLGenerator_UnitTester_Tools.hpp"

#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorDataStruct.hpp"
#include "XMLGeneratorParserUtilities.hpp"
#include "XMLGeneratorServiceUtilities.hpp"

namespace PlatoTestXMLGenerator
{

TEST(PlatoTestXMLGenerator, move_file_to_subdirectories)
{
    std::ofstream tOutFile;
    tOutFile.open("dummy.txt", std::ofstream::out | std::ofstream::trunc);
    tOutFile << "hello";
    tOutFile.close();

    XMLGen::move_file_to_subdirectories("dummy.txt", {"dummy_dir"});

    auto tReadData = XMLGen::read_data_from_file("./dummy_dir/dummy.txt");
    auto tGoldString = std::string("hello");
    ASSERT_STREQ(tGoldString.c_str(), tReadData.str().c_str());
    Plato::system("rm -rf dummy_dir");
    Plato::system("rm -f dummy.txt");
}

}
// PlatoTestXMLGenerator