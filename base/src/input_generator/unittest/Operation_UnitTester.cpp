 /*
 * Operation_UnitTester.cpp
 *
 *  Created on: April 12, 2022
 */
#include <gtest/gtest.h>

#include "pugixml.hpp"
#include "Stage.hpp"
#include "Operation.hpp"
#include "SharedData.hpp"
#include "Performer.hpp"
#include "XMLGeneratorDataStruct.hpp"
#include "XMLGeneratorParserUtilities.hpp"
#include "XMLGeneratorUtilities.hpp"
#include "XMLGenerator_UnitTester_Tools.hpp"

namespace PlatoTestXMLGenerator
{

TEST(OperationTest, WriteDefinitionGemmaMPIOperationWithConcurrency)
{
    int numRanks = 16;
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",numRanks,2);
    director::OperationGemmaMPISystemCall tGemma("match.yaml","", "2", tPerformer, 1);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tGemma.write_definition(tDocument,"1"));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "Command", 
        "ChDir",
        "OnChange",
        "NumRanks",
        "Argument",
        "AppendInput"};
    std::vector<std::string> tValues = {"SystemCallMPI", 
        "gemma_1", 
        "gemma", 
        "evaluations_1",
        "true",
        "2",
        "match.yaml",
        "false"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteDefinitionGemmaMPIOperationNoConcurrency)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,2);
    director::OperationGemmaMPISystemCall tGemma("match.yaml","" ,"2", tPerformer, 0);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tGemma.write_definition(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "Command", 
        "OnChange",
        "NumRanks",
        "Argument",
        "AppendInput"};
    std::vector<std::string> tValues = {"SystemCallMPI", 
        "gemma", 
        "gemma", 
        "true",
        "2",
        "match.yaml",
        "false"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteDefinitionGemmaMPIOperationWithTag)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,2);
    director::OperationGemmaMPISystemCall tGemma("match.yaml","path/", "2", tPerformer, 1);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tGemma.write_definition(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "Command", 
        "ChDir",
        "OnChange",
        "NumRanks",
        "Argument",
        "AppendInput"};
    std::vector<std::string> tValues = {"SystemCallMPI", 
        "gemma_{E}", 
        "path/gemma", 
        "evaluations_{E}",
        "true",
        "2",
        "match.yaml",
        "false"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteInterfaceGemmaMPIOperationWithTag)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,2);
    director::OperationGemmaMPISystemCall tGemma("match.yaml", "","2", tPerformer, 1);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tGemma.write_interface(tDocument));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    std::vector<std::string> tKeys = {
        "Name", 
        "PerformerName"
        };
    std::vector<std::string> tValues = {
        "gemma_{E}", 
        "plato_services_{E}"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteDefinitionApreproOperationWithConcurrency)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,2);
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};
    std::shared_ptr<director::SharedData> tSharedData = std::make_shared<director::SharedDataGlobal>("design_parameters","3",tPerformerMain,tUserPerformers,2);
    
    director::OperationAprepro tAprepro("match.yaml", {"l", "w", "d"},tSharedData, tPerformer, 2);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tAprepro.write_definition(tDocument,"2"));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "Command", 
        "ChDir",
        "OnChange",
        "Argument",
        "Argument",
        "Argument",
        "AppendInput",
        "Option",
        "Option",
        "Option",
        "Input"};
    std::vector<std::string> tValues = {"SystemCall", 
        "aprepro_2", 
        "aprepro", 
        "evaluations_2",
        "true",
        "-q",
        "match.yaml.template",
        "match.yaml",
        "true",
        "l=",
        "w=",
        "d=",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());
    tKeys = {"ArgumentName", "Layout", "Size"};
    tValues = {"parameters", "scalar", "3"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteDefinitionApreproOperationWithTag)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,2);
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};
    std::shared_ptr<director::SharedData> tSharedData = std::make_shared<director::SharedDataGlobal>("design_parameters","3",tPerformerMain,tUserPerformers,2);
    
    director::OperationAprepro tAprepro("match.yaml", {"l", "w", "d"},tSharedData, tPerformer, 2);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tAprepro.write_definition(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "Command", 
        "ChDir",
        "OnChange",
        "Argument",
        "Argument",
        "Argument",
        "AppendInput",
        "Option",
        "Option",
        "Option",
        "Input"};
    std::vector<std::string> tValues = {"SystemCall", 
        "aprepro_{E}", 
        "aprepro", 
        "evaluations_{E}",
        "true",
        "-q",
        "match.yaml.template",
        "match.yaml",
        "true",
        "l=",
        "w=",
        "d=",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());
    tKeys = {"ArgumentName", "Layout", "Size"};
    tValues = {"parameters", "scalar", "3"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteInterfaceApreproOperationWithConcurrency)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,2);
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};
    std::shared_ptr<director::SharedData> tSharedData = std::make_shared<director::SharedDataGlobal>("design_parameters","3",tPerformerMain,tUserPerformers,2);
    
    director::OperationAprepro tAprepro("match.yaml", {"l", "w", "d"}, tSharedData, tPerformer, 2);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tAprepro.write_interface(tDocument,"2"));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    std::vector<std::string> tKeys = {
        "Name", 
        "PerformerName", 
        "Input"};
    std::vector<std::string> tValues = {
        "aprepro_2", 
        "plato_services_2", 
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"parameters", "design_parameters_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteDefinitionApreproOperationNoConcurrency)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,0);
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};
    std::shared_ptr<director::SharedData> tSharedData = std::make_shared<director::SharedDataGlobal>("design_parameters","3",tPerformerMain,tUserPerformers,0);
    std::vector<std::string> vars = {"l","w","d"};

    director::OperationAprepro tAprepro("match.yaml", vars, tSharedData, tPerformer, 0);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tAprepro.write_definition(tDocument));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "Command", 
        "OnChange",
        "Argument",
        "Argument",
        "Argument",
        "AppendInput",
        "Option",
        "Option",
        "Option",
        "Input"};
    std::vector<std::string> tValues = {"SystemCall", 
        "aprepro", 
        "aprepro", 
        "true",
        "-q",
        "match.yaml.template",
        "match.yaml",
        "true",
        "l=",
        "w=",
        "d=",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());
    tKeys = {"ArgumentName", "Layout", "Size"};
    tValues = {"parameters", "scalar", "3"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteDefinitionHarvestDataOperationWithConcurrency)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,2);
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};
    std::shared_ptr<director::SharedData> tSharedData = std::make_shared<director::SharedDataGlobal>("criterion value","3",tPerformerMain,tUserPerformers,2);
    
    director::OperationHarvestDataFunction tHarvestData("power_balance.dat", "max", "1", tSharedData, tPerformer, 3);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tHarvestData.write_definition(tDocument,"0"));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "File", 
        "Operation",
        "Column",
        "Output"};
    std::vector<std::string> tValues = {"HarvestDataFromFile", 
        "harvest_data_0", 
        "./evaluations_0/power_balance.dat",
        "max",
        "1",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());
    tKeys = {"ArgumentName", "Layout", "Size"};
    tValues = {"criterion value", "scalar", "1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);
    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteInterfaceHarvestDataOperationWithTag)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,2);
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};
    std::shared_ptr<director::SharedData> tSharedData = std::make_shared<director::SharedDataGlobal>("criterion_1_service_1_scenario_1","3",tPerformerMain,tUserPerformers,3);
    
    director::OperationHarvestDataFunction tHarvestData("power_balance.dat", "max", "1", tSharedData, tPerformer, 3);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tHarvestData.write_interface(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    std::vector<std::string> tKeys = {
        "Name", 
        "PerformerName",
        "Output"};
    std::vector<std::string> tValues = {
        "harvest_data_{E}", 
        "plato_services_{E}", 
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"criterion value", "criterion_1_service_1_scenario_1_{E}"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);
    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteDefinitionCubitTet10OperationWithTag)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,2);
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};
    std::shared_ptr<director::SharedData> tSharedData = std::make_shared<director::SharedDataGlobal>("design_parameters","3",tPerformerMain,tUserPerformers,2);
    
    director::OperationCubitTet10Conversion tCubit("myFile.exo", {"1", "3", "4"},tSharedData, tPerformer, 2);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tCubit.write_definition(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "Command", 
        "ChDir",
        "OnChange",
        "Argument",
        "AppendInput",
        "Input"};
    std::vector<std::string> tValues = {"SystemCall", 
        "cubit_convert_to_tet10_{E}", 
        "cubit", 
        "evaluations_{E}",
        "true",
        " -input toTet10.jou -batch -nographics -nogui -noecho -nojournal -nobanner -information off",
        "false",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());
    tKeys = {"ArgumentName"};
    tValues = {"parameters"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteInterfaceCubitTet10Operation)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,2);
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};
    std::shared_ptr<director::SharedData> tSharedData = std::make_shared<director::SharedDataGlobal>("design_parameters","3",tPerformerMain,tUserPerformers,2);
    
    director::OperationCubitTet10Conversion tCubit("myFile.exo", {"1", "3", "4"},tSharedData, tPerformer, 2);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tCubit.write_interface(tDocument,"2"));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    std::vector<std::string> tKeys = {
        "Name", 
        "PerformerName", 
        "Input"};
    std::vector<std::string> tValues = {
        "cubit_convert_to_tet10_2", 
        "plato_services_2", 
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"parameters", "design_parameters_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteCubitTet10OperationToTet10FileNoConcurrency)
{
    Plato::system("rm -rf toTet10.jou");
    Plato::system("rm -rf evaluations_0");
    Plato::system("rm -rf evaluations_1");

    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,0);
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};
    std::shared_ptr<director::SharedData> tSharedData = std::make_shared<director::SharedDataGlobal>("design_parameters","3",tPerformerMain,tUserPerformers,0);
    
    director::OperationCubitTet10Conversion tCubit("myFile.exo", {"1", "3", "4"},tSharedData, tPerformer, 0);
    
    auto tReadData = XMLGen::read_data_from_file("toTet10.jou");
    auto tGold = std::string("importmesh\"myFile.exo\"no_geom") + 
        std::string("block1elementtypetetra10") + 
        std::string("block3elementtypetetra10") + 
        std::string("block4elementtypetetra10") + 
        std::string("setexodusnetcdf4off") + 
        std::string("setlargeexodusfileon") + 
        std::string("exportmesh\"myFile.exo\"overwrite");

    EXPECT_STREQ(tReadData.str().c_str(),tGold.c_str());

    ASSERT_THROW(tReadData = XMLGen::read_data_from_file("evaluations_0/toTet10.jou"),std::runtime_error);

    Plato::system("rm -rf toTet10.jou");
}

TEST(OperationTest, WriteCubitTet10OperationToTet10File)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,2);
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};
    std::shared_ptr<director::SharedData> tSharedData = std::make_shared<director::SharedDataGlobal>("design_parameters","3",tPerformerMain,tUserPerformers,2);
    
    director::OperationCubitTet10Conversion tCubit("myFile.exo", {"1", "3", "4"},tSharedData, tPerformer, 2);
    
    auto tReadData = XMLGen::read_data_from_file("toTet10.jou");
    auto tGold = std::string("importmesh\"myFile.exo\"no_geom") + 
        std::string("block1elementtypetetra10") + 
        std::string("block3elementtypetetra10") + 
        std::string("block4elementtypetetra10") + 
        std::string("setexodusnetcdf4off") + 
        std::string("setlargeexodusfileon") + 
        std::string("exportmesh\"myFile.exo\"overwrite");

    EXPECT_STREQ(tReadData.str().c_str(),tGold.c_str());

    tReadData = XMLGen::read_data_from_file("evaluations_0/toTet10.jou");
    EXPECT_STREQ(tReadData.str().c_str(),tGold.c_str());

    tReadData = XMLGen::read_data_from_file("evaluations_1/toTet10.jou");
    EXPECT_STREQ(tReadData.str().c_str(),tGold.c_str());

    Plato::system("rm -rf toTet10.jou");
    Plato::system("rm -rf evaluations_0");
    Plato::system("rm -rf evaluations_1");

}

TEST(OperationTest, WriteDefinitionCubitSubBlockOperationWithTag)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,2);
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};
    std::shared_ptr<director::SharedData> tSharedData = std::make_shared<director::SharedDataGlobal>("design_parameters","3",tPerformerMain,tUserPerformers,2);
    
    director::OperationCubitSubBlock tCubit("myFile.exo", {"1", "3", "4"},tSharedData, tPerformer, 2);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tCubit.write_definition(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "Command", 
        "ChDir",
        "OnChange",
        "Argument",
        "AppendInput",
        "Input"};
    std::vector<std::string> tValues = {"SystemCall", 
        "create_sub_block_{E}", 
        "cubit", 
        "evaluations_{E}",
        "true",
        " -input subBlock.jou -batch -nographics -nogui -noecho -nojournal -nobanner -information off",
        "false",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());
    tKeys = {"ArgumentName"};
    tValues = {"design_parameters_{E}"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteInterfaceCubitSubBlockOperation)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,2);
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};
    std::shared_ptr<director::SharedData> tSharedData = std::make_shared<director::SharedDataGlobal>("design_parameters","3",tPerformerMain,tUserPerformers,2);
    
    director::OperationCubitSubBlock tCubit("myFile.exo", {"1", "3", "4"},tSharedData, tPerformer, 2);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tCubit.write_interface(tDocument,"2"));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    std::vector<std::string> tKeys = {
        "Name", 
        "PerformerName", 
        "Input"};
    std::vector<std::string> tValues = {
        "create_sub_block_2", 
        "plato_services_2", 
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"design_parameters_2", "design_parameters_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteCubitSubBlockOperationSubBlockFile)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,2);
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};
    std::shared_ptr<director::SharedData> tSharedData = std::make_shared<director::SharedDataGlobal>("design_parameters","3",tPerformerMain,tUserPerformers,2);
    
    director::OperationCubitSubBlock tCubit("myFile.exo", {"-0.872", "-5", "-5", "0.872", "10", "10"},tSharedData, tPerformer, 2);
    
    auto tReadData = XMLGen::read_data_from_file("subBlock.jou");
    auto tGold = std::string("importmesh\"myFile.exo\"no_geom") + 
        std::string("deleteblockall") + 
        std::string("block2tetwithx_coord>=-0.872andy_coord>=-5andz_coord>=-5andx_coord<=0.872andy_coord<=10andz_coord<=10") + 
        std::string("block1tetall") + 
        std::string("setexodusnetcdf4off") + 
        std::string("setlargeexodusfileon") + 
        std::string("exportmesh\"myFile.exo\"overwrite");

    EXPECT_STREQ(tReadData.str().c_str(),tGold.c_str());

    tReadData = XMLGen::read_data_from_file("evaluations_0/subBlock.jou");
    EXPECT_STREQ(tReadData.str().c_str(),tGold.c_str());

    tReadData = XMLGen::read_data_from_file("evaluations_1/subBlock.jou");
    EXPECT_STREQ(tReadData.str().c_str(),tGold.c_str());

    Plato::system("rm -rf subBlock.jou");
    Plato::system("rm -rf evaluations_0");
    Plato::system("rm -rf evaluations_1");

}

TEST(OperationTest, WriteCubitSubBlockOperationSubBlockFileNoConcurrency)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,0);
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};
    std::shared_ptr<director::SharedData> tSharedData = std::make_shared<director::SharedDataGlobal>("design_parameters","3",tPerformerMain,tUserPerformers,0);
    
    director::OperationCubitSubBlock tCubit("myFile.exo", {"-0.872", "-5", "-5", "0.872", "10", "10"},tSharedData, tPerformer, 0);
    
    auto tReadData = XMLGen::read_data_from_file("subBlock.jou");
    auto tGold = std::string("importmesh\"myFile.exo\"no_geom") + 
        std::string("deleteblockall") + 
        std::string("block2tetwithx_coord>=-0.872andy_coord>=-5andz_coord>=-5andx_coord<=0.872andy_coord<=10andz_coord<=10") + 
        std::string("block1tetall") + 
        std::string("setexodusnetcdf4off") + 
        std::string("setlargeexodusfileon") + 
        std::string("exportmesh\"myFile.exo\"overwrite");

    EXPECT_STREQ(tReadData.str().c_str(),tGold.c_str());

    ASSERT_THROW(tReadData = XMLGen::read_data_from_file("evaluations_0/subBlock.jou"),std::runtime_error);

    Plato::system("rm -rf subBlock.jou");

}

TEST(OperationTest, WriteDefinitionDecompOperationWithTag)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,2);    
    director::OperationDecomp tDecomp("myFile.exo", 16 , tPerformer, 2);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tDecomp.write_definition(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "Command", 
        "ChDir",
        "Argument",
        "Argument",
        "Argument",
        "Argument"};
    std::vector<std::string> tValues = {"SystemCall", 
        "decomp_mesh_{E}", 
        "decomp", 
        "evaluations_{E}",
        " -p ",
        "16",
        " ",
        "myFile.exo"};
    
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteInterfaceDecompOperationInterfaceFile)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,2);    
    director::OperationDecomp tDecomp("myFile.exo", 16 , tPerformer, 2);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tDecomp.write_interface(tDocument,"2"));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    std::vector<std::string> tKeys = {
        "Name", 
        "PerformerName"};
    std::vector<std::string> tValues = {
        "decomp_mesh_2", 
        "plato_services_2"};
    
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteDefinitionSetBoundsOperationLower)
{
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,0);
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};

    std::shared_ptr<director::SharedData> tInputSharedData = std::make_shared<director::SharedDataGlobal>("Lower Bound Value","1",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData = std::make_shared<director::SharedDataNodalField>("Lower Bound Vector",tPerformerMain,tUserPerformers);

    director::OperationSetBounds tSetBounds("Compute Lower Bounds",director::lower,"solid","density",tInputSharedData,tOutputSharedData,tPerformerMain);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tSetBounds.write_definition(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "UseCase", 
        "Discretization",
        "Input",
        "Output"};
    std::vector<std::string> tValues = {"SetLowerBounds", 
        "Compute Lower Bounds", 
        "solid", 
        "density",
        "",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());

    tKeys = {"ArgumentName"};
    tValues = {"Lower Bound Value"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"ArgumentName"};
    tValues = {"Lower Bound Vector"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteInterfaceSetBoundsInterfaceFileUpper)
{
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,0);
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};

    std::shared_ptr<director::SharedData> tInputSharedData = std::make_shared<director::SharedDataGlobal>("Upper Bound Value","1",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData = std::make_shared<director::SharedDataNodalField>("Upper Bound Vector",tPerformerMain,tUserPerformers);

    director::OperationSetBounds tSetBounds("Compute Upper Bounds",director::upper,"solid","density",tInputSharedData,tOutputSharedData,tPerformerMain);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tSetBounds.write_interface(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {
        "Name", 
        "PerformerName",
        "Input",
        "Output"};
    std::vector<std::string> tValues = {
        "Compute Upper Bounds", 
        "platomain",
        "",
        ""};
    
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());

    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"Upper Bound Value","Upper Bound Value"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"ArgumentName","SharedDataName"};
    tValues = {"Upper Bound Vector","Upper Bound Vector"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteDefinitionFilterOperation)
{
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain};

    std::shared_ptr<director::SharedData> tInputSharedData = std::make_shared<director::SharedDataNodalField>("Field",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData = std::make_shared<director::SharedDataNodalField>("Filtered Field",tPerformerMain,tUserPerformers);

    director::OperationFilter tFilter("Filter Control",director::value,tInputSharedData,tOutputSharedData,tPerformerMain);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tFilter.write_definition(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "Gradient", 
        "Input",
        "Output"};
    std::vector<std::string> tValues = {"Filter", 
        "Filter Control", 
        "False", 
        "",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());

    tKeys = {"ArgumentName"};
    tValues = {"Field"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"ArgumentName"};
    tValues = {"Filtered Field"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteInterfaceFilterOperationGradient)
{
      std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain};

    std::shared_ptr<director::SharedData> tInputSharedData = std::make_shared<director::SharedDataNodalField>("Field",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData = std::make_shared<director::SharedDataNodalField>("Filtered Field",tPerformerMain,tUserPerformers);

    director::OperationFilter tFilter("Filter Control",director::gradient,tInputSharedData,tOutputSharedData,tPerformerMain);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tFilter.write_interface(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {
        "Name", 
        "PerformerName",
        "Input",
        "Output"};
    std::vector<std::string> tValues = {
        "Filter Control", 
        "platomain",
        "",
        ""};
    
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());

    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"Field","Field"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"ArgumentName","SharedDataName"};
    tValues = {"Filtered Field","Filtered Field"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteDefinitionInitializeUniformOperation)
{
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain};

    std::shared_ptr<director::SharedData> tOutputSharedData = std::make_shared<director::SharedDataNodalField>("Initialized Field",tPerformerMain,tUserPerformers);

    director::OperationInitializeUniformField tInitialize("Initialize Field",0.25,tOutputSharedData,tPerformerMain);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tInitialize.write_definition(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "Method", 
        "Uniform",
        "Output"};
    std::vector<std::string> tValues = {"InitializeField", 
        "Initialize Field", 
        "Uniform", 
        "",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tUniform = tOperation.child("Uniform");
    ASSERT_FALSE(tUniform.empty());

    tKeys = {"Value"};
    tValues = {"0.250000"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tUniform);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"ArgumentName"};
    tValues = {"Initialized Field"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteInterfaceInitializeUniformOperation)
{
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain};

    std::shared_ptr<director::SharedData> tOutputSharedData = std::make_shared<director::SharedDataNodalField>("Initialized Field",tPerformerMain,tUserPerformers);

    director::OperationInitializeUniformField tInitialize("Initialize Field",0.25,tOutputSharedData,tPerformerMain);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tInitialize.write_interface(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {
        "Name", 
        "PerformerName",
        "Output"};
    std::vector<std::string> tValues = {
        "Initialize Field", 
        "platomain",
        ""};
    
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"ArgumentName","SharedDataName"};
    tValues = {"Initialized Field","Initialized Field"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteDefinitionCopyFieldOperation)
{
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain};

    std::shared_ptr<director::SharedData> tInputSharedData = std::make_shared<director::SharedDataNodalField>("Input Field",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData = std::make_shared<director::SharedDataNodalField>("Output Field",tPerformerMain,tUserPerformers);

    director::OperationCopy tCopy("Copy Field",director::copyfield,tInputSharedData,tOutputSharedData,tPerformerMain);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tCopy.write_definition(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "Input",
        "Output"};
    std::vector<std::string> tValues = {"CopyField", 
        "Copy Field", 
        "",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());

    tKeys = {"ArgumentName"};
    tValues = {"Input Field"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"ArgumentName"};
    tValues = {"Output Field"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteInterfaceCopyValueOperation)
{
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain};

    std::shared_ptr<director::SharedData> tInputSharedData = std::make_shared<director::SharedDataNodalField>("Input Value",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData = std::make_shared<director::SharedDataNodalField>("Output Value",tPerformerMain,tUserPerformers);

    director::OperationCopy tCopy("Copy Value",director::copyvalue,tInputSharedData,tOutputSharedData,tPerformerMain);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tCopy.write_interface(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {
        "Name", 
        "PerformerName",
        "Input",
        "Output"};
    std::vector<std::string> tValues = {
        "Copy Value", 
        "platomain",
        "",
        ""};
    
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());

    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"Input Value","Input Value"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"ArgumentName","SharedDataName"};
    tValues = {"Output Value","Output Value"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteDefinitionComputeCriterionOperationValue)
{
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("plato_analyze","plato_analyze",0,1,2);
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain};

    std::shared_ptr<director::SharedData> tInputSharedData = std::make_shared<director::SharedDataNodalField>("Topology",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData = std::make_shared<director::SharedDataNodalField>("Objective Value",tPerformerMain,tUserPerformers);

    director::OperationComputeCriterion tObjective("Compute Objective Value",director::value,"My Objective",tInputSharedData,tOutputSharedData,tPerformerMain,0.01);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tObjective.write_definition(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "Criterion",
        "Target",
        "Input",
        "Output"};
    std::vector<std::string> tValues = {"ComputeCriterionValue", 
        "Compute Objective Value", 
        "My Objective",
        "0.010000",
        "",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());

    tKeys = {"ArgumentName"};
    tValues = {"Topology"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"Argument","ArgumentName"};
    tValues = {"Value","Objective Value"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteDefinitionComputeCriterionOperationValueNoTargetGiven)
{
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("plato_analyze","plato_analyze",0,1,2);
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain};

    std::shared_ptr<director::SharedData> tInputSharedData = std::make_shared<director::SharedDataNodalField>("Topology",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData = std::make_shared<director::SharedDataNodalField>("Objective Value",tPerformerMain,tUserPerformers);

    director::OperationComputeCriterion tObjective("Compute Objective Value",director::value,"My Objective",tInputSharedData,tOutputSharedData,tPerformerMain);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tObjective.write_definition(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "Criterion",
        "Target",
        "Input",
        "Output"};
    std::vector<std::string> tValues = {"ComputeCriterionValue", 
        "Compute Objective Value", 
        "My Objective",
        "0.000000",
        "",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());

    tKeys = {"ArgumentName"};
    tValues = {"Topology"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"Argument","ArgumentName"};
    tValues = {"Value","Objective Value"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteInterfaceComputeCriterionOperationValue)
{
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("plato_analyze","plato_analyze",0,1,2);
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain};

    std::shared_ptr<director::SharedData> tInputSharedData = std::make_shared<director::SharedDataNodalField>("Topology",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData = std::make_shared<director::SharedDataNodalField>("Objective Value",tPerformerMain,tUserPerformers);

    director::OperationComputeCriterion tObjective("Compute Objective Value",director::value,"My Objective",tInputSharedData,tOutputSharedData,tPerformerMain,0.01);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tObjective.write_interface(tDocument,"2"));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {
        "Name", 
        "PerformerName",
        "Input",
        "Output"};
    std::vector<std::string> tValues = {
        "Compute Objective Value", 
        "plato_analyze_2",
        "",
        ""};
    
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());

    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"Topology","Topology"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"ArgumentName","SharedDataName"};
    tValues = {"Objective Value","Objective Value"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteDefinitionComputeCriterionOperationGradient)
{
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("plato_analyze","plato_analyze",0,1,2);
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain};

    std::shared_ptr<director::SharedData> tInputSharedData = std::make_shared<director::SharedDataNodalField>("Topology",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData = std::make_shared<director::SharedDataNodalField>("Objective Gradient",tPerformerMain,tUserPerformers);

    director::OperationComputeCriterion tObjective("Compute Objective Gradient",director::gradient,"My Objective",tInputSharedData,tOutputSharedData,tPerformerMain);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tObjective.write_definition(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    //tDocument.save_file("testint.xml","  ");

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "Criterion",
        "Input",
        "Output"};
    std::vector<std::string> tValues = {"ComputeCriterionGradient", 
        "Compute Objective Gradient", 
        "My Objective",
        "",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());

    tKeys = {"ArgumentName"};
    tValues = {"Topology"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"Argument","ArgumentName"};
    tValues = {"Gradient","Objective Gradient"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteInterfaceComputeCriterionOperationGradient)
{
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("plato_analyze","plato_analyze",0,1,2);
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain};

    std::shared_ptr<director::SharedData> tInputSharedData = std::make_shared<director::SharedDataNodalField>("Topology",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData = std::make_shared<director::SharedDataNodalField>("Objective Gradient",tPerformerMain,tUserPerformers);

    director::OperationComputeCriterion tObjective("Compute Objective Gradient",director::gradient,"My Objective",tInputSharedData,tOutputSharedData,tPerformerMain,0.01);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tObjective.write_interface(tDocument,"2"));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {
        "Name", 
        "PerformerName",
        "Input",
        "Output"};
    std::vector<std::string> tValues = {
        "Compute Objective Gradient", 
        "plato_analyze_2",
        "",
        ""};
    
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());

    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"Topology","Topology"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"ArgumentName","SharedDataName"};
    tValues = {"Objective Gradient","Objective Gradient"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteDefinitionDesignVolumeOperation)
{
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain};

    std::shared_ptr<director::SharedData> tOutputSharedData = std::make_shared<director::SharedDataNodalField>("Design Volume",tPerformerMain,tUserPerformers);

    director::OperationDesignVolume tDesignVolume("Design Volume",tOutputSharedData,tPerformerMain);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tDesignVolume.write_definition(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "Output"};
    std::vector<std::string> tValues = {"DesignVolume", 
        "Design Volume", 
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"ArgumentName"};
    tValues = {"Design Volume"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteInterfaceDesignVolumeOperation)
{
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain};

    std::shared_ptr<director::SharedData> tOutputSharedData = std::make_shared<director::SharedDataNodalField>("Design Volume",tPerformerMain,tUserPerformers);

    director::OperationDesignVolume tDesignVolume("Design Volume",tOutputSharedData,tPerformerMain);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tDesignVolume.write_interface(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {
        "Name", 
        "PerformerName",
        "Output"};
    std::vector<std::string> tValues = {
        "Design Volume", 
        "platomain",
        ""};
    
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"ArgumentName","SharedDataName"};
    tValues = {"Design Volume","Design Volume"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteDefinitionComputeVolumeSIMPOperation)
{
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain};

    std::shared_ptr<director::SharedData> tInputSharedData = std::make_shared<director::SharedDataNodalField>("Topology",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData1 = std::make_shared<director::SharedDataNodalField>("Volume",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData2 = std::make_shared<director::SharedDataNodalField>("Volume Gradient",tPerformerMain,tUserPerformers);
    std::vector<std::shared_ptr<director::SharedData>> tOutputs = {tOutputSharedData1 , tOutputSharedData2};

    director::OperationComputeVolumeSIMP tSIMP("Compute Volume",tInputSharedData,tOutputs,tPerformerMain);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tSIMP.write_definition(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    //tDocument.save_file("test.xml","  ");
    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "PenaltyModel",
        "Input",
        "Output",
        "Output",
        "SIMP"};
    std::vector<std::string> tValues = {"ComputeVolume", 
        "Compute Volume", 
        "SIMP",
        "",
        "",
        "",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    
    auto tIO = tOperation.child("Input");
    ASSERT_FALSE(tIO.empty());

    tKeys = {"ArgumentName"};
    tValues = {"Topology"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tIO);

    tIO = tIO.next_sibling("Output");
    ASSERT_FALSE(tIO.empty());

    tKeys = {"ArgumentName"};
    tValues = {"Volume"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tIO);

    tIO = tIO.next_sibling("Output");
    ASSERT_FALSE(tIO.empty());

    tKeys = {"ArgumentName"};
    tValues = {"Volume Gradient"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tIO);

    tIO = tIO.next_sibling("Output");
    ASSERT_TRUE(tIO.empty());

    auto tSIMPNode = tOperation.child("SIMP");
    tKeys = {"PenaltyExponent", "MinimumValue"};
    tValues = {"1.0","0.0"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSIMPNode);

    tSIMPNode = tOperation.next_sibling("SIMP");
    ASSERT_TRUE(tSIMPNode.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteInterfaceComputeVolumeSIMPOperation)
{
      std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain};

    std::shared_ptr<director::SharedData> tInputSharedData = std::make_shared<director::SharedDataNodalField>("Topology",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData1 = std::make_shared<director::SharedDataNodalField>("Volume",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData2 = std::make_shared<director::SharedDataNodalField>("Volume Gradient",tPerformerMain,tUserPerformers);
    std::vector<std::shared_ptr<director::SharedData>> tOutputs = {tOutputSharedData1 , tOutputSharedData2};

    director::OperationComputeVolumeSIMP tSIMP("Compute Volume",tInputSharedData,tOutputs,tPerformerMain);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tSIMP.write_interface(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    //tDocument.save_file("test.xml","  ");

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {
        "Name", 
        "PerformerName",
        "Input",
        "Output",
        "Output"};
    std::vector<std::string> tValues = {
        "Compute Volume", 
        "platomain",
        "",
        "",
        ""};
    
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());

    tKeys = {"ArgumentName","SharedDataName"};
    tValues = {"Topology","Topology"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"ArgumentName","SharedDataName"};
    tValues = {"Volume","Volume"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tOutput = tOutput.next_sibling("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"ArgumentName","SharedDataName"};
    tValues = {"Volume Gradient","Volume Gradient"};
    
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteDefinitionWriteOutputOperation)
{
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain};
    std::shared_ptr<director::SharedData> tOutputSharedData1 = std::make_shared<director::SharedDataNodalField>("displacement X",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData2 = std::make_shared<director::SharedDataNodalField>("displacement Y",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData3 = std::make_shared<director::SharedDataNodalField>("displacement Z",tPerformerMain,tUserPerformers);
    std::vector<std::shared_ptr<director::SharedData>> tOutputs = {tOutputSharedData1, tOutputSharedData2, tOutputSharedData3};

    director::OperationWriteOutput tWriteOutput("Write Output",tOutputs,tPerformerMain);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tWriteOutput.write_definition(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    //tDocument.save_file("test.xml","  ");
    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {"Function",
        "Name", 
        "Output",
        "Output",
        "Output"};
    std::vector<std::string> tValues = {"WriteOutput", 
        "Write Output", 
        "",
        "",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    
    auto tIO = tOperation.child("Output");
    ASSERT_FALSE(tIO.empty());

    tKeys = {"ArgumentName"};
    tValues = {"displacement X"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tIO);

    tIO = tIO.next_sibling("Output");
    ASSERT_FALSE(tIO.empty());

    tKeys = {"ArgumentName"};
    tValues = {"displacement Y"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tIO);

    tIO = tIO.next_sibling("Output");
    ASSERT_FALSE(tIO.empty());

    tKeys = {"ArgumentName"};
    tValues = {"displacement Z"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tIO);

    tIO = tIO.next_sibling("Output");
    ASSERT_TRUE(tIO.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(OperationTest, WriteInterfaceWriteOutputOperation)
{
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain};
    std::shared_ptr<director::SharedData> tOutputSharedData1 = std::make_shared<director::SharedDataNodalField>("displacement X",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData2 = std::make_shared<director::SharedDataNodalField>("displacement Y",tPerformerMain,tUserPerformers);
    std::shared_ptr<director::SharedData> tOutputSharedData3 = std::make_shared<director::SharedDataNodalField>("displacement Z",tPerformerMain,tUserPerformers);
    std::vector<std::shared_ptr<director::SharedData>> tOutputs = {tOutputSharedData1, tOutputSharedData2, tOutputSharedData3};

    director::OperationWriteOutput tWriteOutput("Write Output",tOutputs,tPerformerMain);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tWriteOutput.write_interface(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    //tDocument.save_file("test.xml","  ");

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());

    std::vector<std::string> tKeys = {
        "Name", 
        "PerformerName",
        "Output",
        "Output",
        "Output"};
    std::vector<std::string> tValues = {
        "Write Output", 
        "platomain",
        "",
        "",
        ""};
    
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"ArgumentName","SharedDataName"};
    tValues = {"displacement X","displacement X"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tOutput = tOutput.next_sibling("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"ArgumentName","SharedDataName"};
    tValues = {"displacement Y","displacement Y"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tOutput = tOutput.next_sibling("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys = {"ArgumentName","SharedDataName"};
    tValues = {"displacement Z","displacement Z"};
    
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}


} 