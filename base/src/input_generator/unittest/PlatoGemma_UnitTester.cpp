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

#include "GemmaProblem.hpp"
#include "Stage.hpp"
#include "Operation.hpp"

namespace PlatoTestXMLGenerator
{

TEST(PlatoTestXMLGenerator, WriteGemmaInputDeckMatchedPowerBalance)
{
    XMLGen::InputData tInputMetaData;

    // define optimization parameters 
    XMLGen::OptimizationParameters tOptParams;
    std::vector<std::string> tDescriptors = {"slot_length", "slot_width", "slot_depth"};
    tOptParams.descriptors(tDescriptors);
    tOptParams.append("concurrent_evaluations", "2");
    tInputMetaData.set(tOptParams);

    // define scenario
    XMLGen::Scenario tScenario;
    tScenario.append("id", "1");
    tScenario.append("material", "1");
    tScenario.append("frequency_min", "10");
    tScenario.append("frequency_max", "100");
    tScenario.append("frequency_step", "5");
    tScenario.append("cavity_radius", "0.1016");
    tScenario.append("cavity_height", "0.1018");
    tScenario.append("physics", "electromagnetics");
    tInputMetaData.append(tScenario);

    // define material
    XMLGen::Material tMaterial;
    tMaterial.id("1");
    tMaterial.property("conductivity", "1e6");
    tInputMetaData.append(tMaterial);

    director::GemmaProblem tGemmaProblem(tInputMetaData);
    ASSERT_NO_THROW(tGemmaProblem.create_matched_power_balance_input_deck(tInputMetaData));

    auto tReadData = XMLGen::read_data_from_file("gemma_matched_power_balance_input_deck.yaml.template");
    std::string tGoldString = std::string("%YAML1.1---Gemma-dynamic:Global:Description:HigginscylinderSolutiontype:powerbalance") + 
        std::string("Powerbalance:Algorithm:matchedboundRadius:0.1016Height:0.1018Conductivity:1e6Slotlength:{slot_length}") + 
        std::string("Slotwidth:{slot_width}Slotdepth:{slot_depth}Startfrequencyrange:10Endfrequencyrange:100Frequencyintervalsize:5...");
    ASSERT_STREQ(tGoldString.c_str(), tReadData.str().c_str());
    Plato::system("rm -rf gemma_matched_power_balance_input_deck.yaml.template");
}

TEST(PlatoTestXMLGenerator, WriteGemmaInputDeckMatchedPowerBalanceToSubdirectories)
{
    XMLGen::InputData tInputMetaData;

    // define optimization parameters 
    XMLGen::OptimizationParameters tOptParams;
    std::vector<std::string> tDescriptors = {"slot_length", "slot_width", "slot_depth"};
    tOptParams.descriptors(tDescriptors);
    tOptParams.append("concurrent_evaluations", "2");
    tInputMetaData.set(tOptParams);

    // define scenario
    XMLGen::Scenario tScenario;
    tScenario.append("id", "1");
    tScenario.append("material", "1");
    tScenario.append("frequency_min", "50");
    tScenario.append("frequency_max", "300");
    tScenario.append("frequency_step", "7");
    tScenario.append("cavity_radius", "0.1016");
    tScenario.append("cavity_height", "0.1018");
    tScenario.append("physics", "electromagnetics");
    tInputMetaData.append(tScenario);

    // define material
    XMLGen::Material tMaterial;
    tMaterial.id("1");
    tMaterial.property("conductivity", "1e6");
    tInputMetaData.append(tMaterial);

    director::GemmaProblem tGemmaProblem(tInputMetaData);
    ASSERT_NO_THROW(tGemmaProblem.create_evaluation_subdirectories_and_gemma_input(tInputMetaData));

    auto tReadData = XMLGen::read_data_from_file("evaluations_0/gemma_matched_power_balance_input_deck.yaml.template");
    std::string tGoldString = std::string("%YAML1.1---Gemma-dynamic:Global:Description:HigginscylinderSolutiontype:powerbalance") + 
        std::string("Powerbalance:Algorithm:matchedboundRadius:0.1016Height:0.1018Conductivity:1e6Slotlength:{slot_length}") + 
        std::string("Slotwidth:{slot_width}Slotdepth:{slot_depth}Startfrequencyrange:50Endfrequencyrange:300Frequencyintervalsize:7...");
    ASSERT_STREQ(tGoldString.c_str(), tReadData.str().c_str());

    tReadData = XMLGen::read_data_from_file("evaluations_1/gemma_matched_power_balance_input_deck.yaml.template");
    tGoldString = std::string("%YAML1.1---Gemma-dynamic:Global:Description:HigginscylinderSolutiontype:powerbalance") + 
        std::string("Powerbalance:Algorithm:matchedboundRadius:0.1016Height:0.1018Conductivity:1e6Slotlength:{slot_length}") + 
        std::string("Slotwidth:{slot_width}Slotdepth:{slot_depth}Startfrequencyrange:50Endfrequencyrange:300Frequencyintervalsize:7...");
    ASSERT_STREQ(tGoldString.c_str(), tReadData.str().c_str());

    Plato::system("rm -rf evaluations_0");
    Plato::system("rm -rf evaluations_1");
}

TEST(PlatoTestXMLGenerator, WriteGemmaPlatoMainOperationsFile)
{
    XMLGen::InputData tInputMetaData;

    // define criteria
    XMLGen::Criterion tCriterion;
    tCriterion.id("2");
    tCriterion.type("system_call");
    tCriterion.append("data_group", "1");
    tCriterion.append("data_extraction_operation", "max");
    tCriterion.append("data_file", "matched_power_balance.dat");
    tInputMetaData.append(tCriterion);

    // define objective 
    XMLGen::Objective tObjective;
    tObjective.criteriaIDs.push_back("2");
    tInputMetaData.objective = tObjective;

    // define optimization parameters 
    XMLGen::OptimizationParameters tOptParams;
    std::vector<std::string> tDescriptors = {"slot_length", "slot_width", "slot_depth"};
    tOptParams.descriptors(tDescriptors);
    tOptParams.append("concurrent_evaluations", "2");
    tInputMetaData.set(tOptParams);

    // define services
    XMLGen::Service tServiceOne;
    tServiceOne.append("code", "gemma");
    tServiceOne.append("id", "1");
    tServiceOne.append("type", "system_call");
    tServiceOne.append("number_processors", "1");
    tInputMetaData.append(tServiceOne);
    XMLGen::Service tServiceTwo;
    tServiceTwo.append("code", "platomain");
    tServiceTwo.append("id", "2");
    tServiceTwo.append("type", "plato_app");
    tServiceTwo.append("number_processors", "2");
    tInputMetaData.append(tServiceTwo);

    director::GemmaProblem tGemmaProblem(tInputMetaData);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tGemmaProblem.write_plato_main_operations(tDocument));
    ASSERT_FALSE(tDocument.empty());

    // APREPRO OPERATIONS
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
        "aprepro_0", 
        "aprepro", 
        "evaluations_0",
        "true",
        "-q",
        "gemma_matched_power_balance_input_deck.yaml.template",
        "gemma_matched_power_balance_input_deck.yaml",
        "true",
        "slot_length=",
        "slot_width=",
        "slot_depth=",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());
    PlatoTestXMLGenerator::test_children({"ArgumentName", "Layout", "Size"}, {"parameters", "scalar", "3"}, tInput);
    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_FALSE(tOperation.empty());
    tValues = {"SystemCall", 
        "aprepro_1", 
        "aprepro", 
        "evaluations_1",
        "true",
        "-q",
        "gemma_matched_power_balance_input_deck.yaml.template",
        "gemma_matched_power_balance_input_deck.yaml",
        "true",
        "slot_length=",
        "slot_width=",
        "slot_depth=",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());
    PlatoTestXMLGenerator::test_children({"ArgumentName", "Layout", "Size"}, {"parameters", "scalar", "3"}, tInput);
    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());

    // GEMMA CALL OPERATIONS
    tOperation = tOperation.next_sibling("Operation");
    ASSERT_FALSE(tOperation.empty());
    tKeys = {"Function",
        "Name", 
        "Command", 
        "ChDir",
        "OnChange",
        "NumRanks",
        "Argument",
        "AppendInput"};
    tValues = {"SystemCallMPI", 
        "gemma_0", 
        "gemma", 
        "evaluations_0",
        "true",
        "1",
        "gemma_matched_power_balance_input_deck.yaml",
        "false"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_FALSE(tOperation.empty());
    tValues = {"SystemCallMPI", 
        "gemma_1", 
        "gemma", 
        "evaluations_1",
        "true",
        "1",
        "gemma_matched_power_balance_input_deck.yaml",
        "false"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    // HARVEST DATA OPERATIONS
    tOperation = tOperation.next_sibling("Operation");
    ASSERT_FALSE(tOperation.empty());
    tKeys = {"Function",
        "Name", 
        "File", 
        "Operation",
        "Column",
        "Output"};
    tValues = {"HarvestDataFromFile", 
        "harvest_data_0", 
        "./evaluations_0/matched_power_balance.dat",
        "max",
        "1",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());
    PlatoTestXMLGenerator::test_children({"ArgumentName", "Layout", "Size"}, {"criterion value", "scalar", "1"}, tOutput);
    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_FALSE(tOperation.empty());
    tValues = {"HarvestDataFromFile", 
        "harvest_data_1", 
        "./evaluations_1/matched_power_balance.dat",
        "max",
        "1",
        ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());
    PlatoTestXMLGenerator::test_children({"ArgumentName", "Layout", "Size"}, {"criterion value", "scalar", "1"}, tOutput);
    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());
}

TEST(PlatoTestXMLGenerator, WriteGemmaPlatoInterfaceFile)
{
    XMLGen::InputData tInputMetaData;

    // define criteria
    XMLGen::Criterion tCriterion;
    tCriterion.id("2");
    tCriterion.type("system_call");
    tCriterion.append("data_group", "1");
    tCriterion.append("data_extraction_operation", "max");
    tCriterion.append("data_file", "matched_power_balance.dat");
    tInputMetaData.append(tCriterion);

    // define objective 
    XMLGen::Objective tObjective;
    tObjective.criteriaIDs.push_back("2");
    tInputMetaData.objective = tObjective;

    // define optimization parameters 
    XMLGen::OptimizationParameters tOptParams;
    std::vector<std::string> tDescriptors = {"slot_length", "slot_width", "slot_depth"};
    tOptParams.descriptors(tDescriptors);
    tOptParams.append("concurrent_evaluations", "2");
    tOptParams.append("verbose", "true");
    tInputMetaData.set(tOptParams);

    // define services
    XMLGen::Service tServiceOne;
    tServiceOne.append("code", "gemma");
    tServiceOne.append("id", "1");
    tServiceOne.append("type", "system_call");
    tServiceOne.append("number_processors", "1");
    tInputMetaData.append(tServiceOne);
    XMLGen::Service tServiceTwo;
    tServiceTwo.append("code", "platomain");
    tServiceTwo.append("id", "2");
    tServiceTwo.append("type", "plato_app");
    tServiceTwo.append("number_processors", "2");
    tInputMetaData.append(tServiceTwo);

    director::GemmaProblem tGemmaProblem(tInputMetaData);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tGemmaProblem.write_interface(tDocument));
    ASSERT_FALSE(tDocument.empty());

    // INCLUDE DEFINES
    auto tInclude = tDocument.child("include");
    ASSERT_FALSE(tInclude.empty());
    PlatoTestXMLGenerator::test_attributes({"filename"}, {"defines.xml"}, tInclude);
    tInclude = tInclude.next_sibling("include");
    ASSERT_TRUE(tInclude.empty());

    // CONSOLE
    auto tConsole = tDocument.child("Console");
    ASSERT_FALSE(tConsole.empty());
    ASSERT_STREQ("Console", tConsole.name());
    auto tChild = tConsole.child("Enabled");
    ASSERT_STREQ("false", tChild.child_value());
    tChild = tConsole.child("Verbose");
    ASSERT_STREQ("true", tChild.child_value());

    // PERFORMERS
    auto tPerformer = tDocument.child("Performer");
    ASSERT_FALSE(tPerformer.empty());
    PlatoTestXMLGenerator::test_children({"PerformerID", "Name", "Code" }, {"0","platomain_1", "platomain"}, tPerformer);

    auto tPerformerForNode = tPerformer.next_sibling("For");
    ASSERT_FALSE(tPerformerForNode.empty());
    PlatoTestXMLGenerator::test_attributes({"var", "in"}, {"E", "Performers"}, tPerformerForNode);
    auto tInnerPerformer = tPerformerForNode.child("Performer");
    ASSERT_FALSE(tInnerPerformer.empty());
    PlatoTestXMLGenerator::test_children({"PerformerID","Name", "Code" }, {"{E+1}", "plato_services_{E}", "plato_services"}, tInnerPerformer);

    tInnerPerformer = tInnerPerformer.next_sibling("Performer");
    ASSERT_TRUE(tInnerPerformer.empty());

    tPerformer = tPerformer.next_sibling("Performer");
    ASSERT_TRUE(tPerformer.empty());

    // SHARED DATA
    auto tSharedDataForNode = tPerformerForNode.next_sibling("For");
    ASSERT_FALSE(tSharedDataForNode.empty());
    auto tInnerSharedData = tSharedDataForNode.child("SharedData");
    ASSERT_FALSE(tInnerSharedData.empty());
    std::vector<std::string> tKeys = {"Name",
        "Type", 
        "Layout",
        "Size", 
        "OwnerName",
        "UserName",
        "UserName"};
    std::vector<std::string> tValues = {"design_parameters_{E}", 
        "Scalar", 
        "Global", 
        "3",
        "platomain_1",
        "platomain_1",
        "plato_services_{E}"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInnerSharedData);
    tInnerSharedData = tInnerSharedData.next_sibling("SharedData");
    ASSERT_TRUE(tInnerSharedData.empty());

    tSharedDataForNode = tSharedDataForNode.next_sibling("For");
    ASSERT_FALSE(tSharedDataForNode.empty());
    tInnerSharedData = tSharedDataForNode.child("SharedData");
    ASSERT_FALSE(tInnerSharedData.empty());
    tKeys = {"Name",
        "Type", 
        "Layout",
        "Size", 
        "OwnerName",
        "UserName"};
    tValues = {"criterion_X_service_X_scenario_X_{E}", 
        "Scalar", 
        "Global", 
        "1",
        "plato_services_{E}",
        "platomain_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInnerSharedData);
    tInnerSharedData = tInnerSharedData.next_sibling("SharedData");
    ASSERT_TRUE(tInnerSharedData.empty());

    // INITIALIZE STAGE
    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    ASSERT_STREQ("Stage", tStage.name());
    auto tName = tStage.child("Name");
    ASSERT_STREQ("Initialize Input", tName.child_value());
    auto tForNode = tStage.child("For");
    PlatoTestXMLGenerator::test_attributes({"var", "in"}, {"E", "Parameters"}, tForNode);
    auto tInputs = tForNode.child("Input");
    ASSERT_FALSE(tInputs.empty());
    ASSERT_STREQ("Input", tInputs.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"design_parameters_{E}"}, tInputs);

    auto tOuterOperation = tStage.child("Operation");
    ASSERT_FALSE(tOuterOperation.empty());
    tForNode = tOuterOperation.child("For");
    ASSERT_FALSE(tForNode.empty());
    PlatoTestXMLGenerator::test_attributes({"var", "in"}, {"E", "Parameters"}, tForNode);
    auto tOperation = tForNode.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    tKeys = {"Name", "PerformerName", "Input"};
    tValues = {"aprepro_{E}", "plato_services_{E}", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tOpInputs = tOperation.child("Input");
    ASSERT_FALSE(tOpInputs.empty());
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"parameters", "design_parameters_{E}"}, tOpInputs);

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());

    tOuterOperation = tOuterOperation.next_sibling("Operation");
    ASSERT_TRUE(tOuterOperation.empty());

    // CRITERION 0 STAGE
    tStage = tStage.next_sibling("Stage");
    ASSERT_FALSE(tStage.empty());
    ASSERT_STREQ("Stage", tStage.name());
    tName = tStage.child("Name");
    ASSERT_STREQ("Compute Criterion 0 Value", tName.child_value());

    tOuterOperation = tStage.child("Operation");
    ASSERT_FALSE(tOuterOperation.empty());
    tForNode = tOuterOperation.child("For");
    ASSERT_FALSE(tForNode.empty());
    PlatoTestXMLGenerator::test_attributes({"var", "in"}, {"E", "Parameters"}, tForNode);
    tOperation = tForNode.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    tKeys = {"Name", "PerformerName"};
    tValues = {"gemma_{E}", "plato_services_{E}"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());

    tOuterOperation = tOuterOperation.next_sibling("Operation");
    ASSERT_FALSE(tOuterOperation.empty());
    tForNode = tOuterOperation.child("For");
    ASSERT_FALSE(tForNode.empty());
    PlatoTestXMLGenerator::test_attributes({"var", "in"}, {"E", "Parameters"}, tForNode);
    tOperation = tForNode.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    tKeys = {"Name", "PerformerName", "Output"};
    tValues = {"harvest_data_{E}", "plato_services_{E}", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tOpOutputs = tOperation.child("Output");
    ASSERT_FALSE(tOpOutputs.empty());
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"criterion value", "criterion_X_service_X_scenario_X_{E}"}, tOpOutputs);

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());

    tForNode = tOuterOperation.next_sibling("For");
    ASSERT_FALSE(tForNode.empty());
    PlatoTestXMLGenerator::test_attributes({"var", "in"}, {"E", "Parameters"}, tForNode);
    auto tOutputs = tForNode.child("Output");
    ASSERT_FALSE(tOutputs.empty());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"criterion_X_service_X_scenario_X_{E}"}, tOutputs);

    tForNode = tForNode.next_sibling("For");
    ASSERT_TRUE(tForNode.empty());

    tStage = tStage.next_sibling("Stage");
    ASSERT_TRUE(tStage.empty());

    // DAKOTA DRIVER
    auto tDriver = tDocument.child("DakotaDriver");
    ASSERT_FALSE(tDriver.empty());
    ASSERT_STREQ("DakotaDriver", tDriver.name());
    tStage = tDriver.child("Stage");
    ASSERT_FALSE(tStage.empty());
    tKeys = {"StageTag", "StageName", "For"};
    tValues = {"initialize", "Initialize Input", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tStage);
    tForNode = tStage.child("For");
    ASSERT_FALSE(tForNode.empty());
    PlatoTestXMLGenerator::test_attributes({"var", "in"}, {"E", "Parameters"}, tForNode);
    tInputs = tForNode.child("Input");
    ASSERT_FALSE(tInputs.empty());
    tKeys = {"Tag", "SharedDataName"};
    tValues = {"continuous", "design_parameters_{E}"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInputs);

    tInputs = tInputs.next_sibling("Input");
    ASSERT_TRUE(tInputs.empty());

    tForNode = tForNode.next_sibling("For");
    ASSERT_TRUE(tForNode.empty());

    tStage = tStage.next_sibling("Stage");
    ASSERT_FALSE(tStage.empty());
    tKeys = {"StageTag", "StageName", "For"};
    tValues = {"criterion_value_0", "Compute Criterion 0 Value", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tStage);
    tForNode = tStage.child("For");
    ASSERT_FALSE(tForNode.empty());
    PlatoTestXMLGenerator::test_attributes({"var", "in"}, {"E", "Parameters"}, tForNode);
    tOutputs = tForNode.child("Output");
    ASSERT_FALSE(tOutputs.empty());
    tKeys = {"SharedDataName"};
    tValues = {"criterion_X_service_X_scenario_X_{E}"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutputs);

    tOutputs = tOutputs.next_sibling("Output");
    ASSERT_TRUE(tOutputs.empty());

    tForNode = tForNode.next_sibling("For");
    ASSERT_TRUE(tForNode.empty());

    tStage = tStage.next_sibling("Stage");
    ASSERT_TRUE(tStage.empty());
}

TEST(PlatoTestXMLGenerator, WriteGemmaMPIRunFile)
{
    XMLGen::InputData tInputMetaData;

    // define criteria
    XMLGen::Criterion tCriterion;
    tCriterion.id("2");
    tCriterion.type("system_call");
    tCriterion.append("data_group", "1");
    tCriterion.append("data_extraction_operation", "max");
    tCriterion.append("data_file", "matched_power_balance.dat");
    tInputMetaData.append(tCriterion);

    // define objective 
    XMLGen::Objective tObjective;
    tObjective.criteriaIDs.push_back("2");
    tInputMetaData.objective = tObjective;

    // define optimization parameters 
    XMLGen::OptimizationParameters tOptParams;
    std::vector<std::string> tDescriptors = {"slot_length", "slot_width", "slot_depth"};
    tOptParams.descriptors(tDescriptors);
    tOptParams.append("concurrent_evaluations", "2");
    tInputMetaData.set(tOptParams);

    // define services
    XMLGen::Service tServiceOne;
    tServiceOne.append("code", "gemma");
    tServiceOne.append("id", "1");
    tServiceOne.append("type", "system_call");
    tServiceOne.append("number_processors", "13");
    tInputMetaData.append(tServiceOne);
    XMLGen::Service tServiceTwo;
    tServiceTwo.append("code", "platomain");
    tServiceTwo.append("id", "2");
    tServiceTwo.append("type", "plato_app");
    tServiceTwo.append("number_processors", "2");
    tInputMetaData.append(tServiceTwo);

    director::GemmaProblem tGemmaProblem(tInputMetaData);

    tGemmaProblem.write_mpirun("mpifile");

    std::string tLine;
    std::ifstream tInFile("mpifile");

    EXPECT_NO_THROW(std::getline(tInFile,tLine));
    ASSERT_EQ("mpirun --oversubscribe -np 1 -x PLATO_PERFORMER_ID=0 \\",tLine);
    EXPECT_NO_THROW(std::getline(tInFile,tLine));
    ASSERT_EQ("-x PLATO_INTERFACE_FILE=interface.xml \\",tLine);
    EXPECT_NO_THROW(std::getline(tInFile,tLine));
    ASSERT_EQ("-x PLATO_APP_FILE=plato_main_operations.xml \\",tLine);
    EXPECT_NO_THROW(std::getline(tInFile,tLine));
    ASSERT_EQ("PlatoMain plato_main_input_deck.xml \\",tLine);
    
    EXPECT_NO_THROW(std::getline(tInFile,tLine));
    ASSERT_EQ(": -np 13 -x PLATO_PERFORMER_ID=1 \\",tLine);
    EXPECT_NO_THROW(std::getline(tInFile,tLine));
    ASSERT_EQ("-x PLATO_INTERFACE_FILE=interface.xml \\",tLine);
    EXPECT_NO_THROW(std::getline(tInFile,tLine));
    ASSERT_EQ("-x PLATO_APP_FILE=plato_main_operations.xml \\",tLine);
    EXPECT_NO_THROW(std::getline(tInFile,tLine));
    ASSERT_EQ("PlatoEngineServices plato_main_input_deck.xml \\",tLine);
    
    EXPECT_NO_THROW(std::getline(tInFile,tLine));
    ASSERT_EQ(": -np 13 -x PLATO_PERFORMER_ID=2 \\",tLine);
    EXPECT_NO_THROW(std::getline(tInFile,tLine));
    ASSERT_EQ("-x PLATO_INTERFACE_FILE=interface.xml \\",tLine);
    EXPECT_NO_THROW(std::getline(tInFile,tLine));
    ASSERT_EQ("-x PLATO_APP_FILE=plato_main_operations.xml \\",tLine);
    EXPECT_NO_THROW(std::getline(tInFile,tLine));
    ASSERT_EQ("PlatoEngineServices plato_main_input_deck.xml \\",tLine);
    
    std::getline(tInFile,tLine);
    EXPECT_TRUE(tInFile.eof());

    tInFile.close();
    Plato::system("rm -f mpifile");

}


TEST(PlatoTestXMLGenerator, WriteGemmaDefinesFile)
{
    XMLGen::InputData tInputMetaData;

    // define criteria
    XMLGen::Criterion tCriterion;
    tCriterion.id("2");
    tCriterion.type("system_call");
    tCriterion.append("data_group", "1");
    tCriterion.append("data_extraction_operation", "max");
    tCriterion.append("data_file", "matched_power_balance.dat");
    tInputMetaData.append(tCriterion);

    // define objective 
    XMLGen::Objective tObjective;
    tObjective.criteriaIDs.push_back("2");
    tInputMetaData.objective = tObjective;

    // define optimization parameters 
    XMLGen::OptimizationParameters tOptParams;
    std::vector<std::string> tDescriptors = {"slot_length", "slot_width", "slot_depth"};
    tOptParams.descriptors(tDescriptors);
    tOptParams.append("concurrent_evaluations", "2");
    tInputMetaData.set(tOptParams);

    // define services
    XMLGen::Service tServiceOne;
    tServiceOne.append("code", "gemma");
    tServiceOne.append("id", "1");
    tServiceOne.append("type", "system_call");
    tServiceOne.append("number_processors", "1");
    tInputMetaData.append(tServiceOne);
    XMLGen::Service tServiceTwo;
    tServiceTwo.append("code", "platomain");
    tServiceTwo.append("id", "2");
    tServiceTwo.append("type", "plato_app");
    tServiceTwo.append("number_processors", "2");
    tInputMetaData.append(tServiceTwo);

    director::GemmaProblem tGemmaProblem(tInputMetaData);

    tGemmaProblem.write_defines();

    std::string tLine;
    std::ifstream tInFile("defines.xml");

    EXPECT_NO_THROW(std::getline(tInFile,tLine));
    ASSERT_EQ("<?xml version=\"1.0\"?>",tLine);
    
    EXPECT_NO_THROW(std::getline(tInFile,tLine));
    ASSERT_EQ("<Array name=\"Parameters\" type=\"int\" from=\"0\" to=\"2\" />",tLine);
    
    EXPECT_NO_THROW(std::getline(tInFile,tLine));
    ASSERT_EQ("<Array name=\"Performers\" type=\"int\" from=\"0\" to=\"1\" />",tLine);
    
    std::getline(tInFile,tLine);
    EXPECT_TRUE(tInFile.eof());
    tInFile.close();
    //Plato::system("rm -f defines.xml");
}



TEST(PlatoTestXMLGenerator, WriteGemmaPlatoMainInput)
{
    XMLGen::InputData tInputMetaData;

    // define criteria
    XMLGen::Criterion tCriterion;
    tCriterion.id("2");
    tCriterion.type("system_call");
    tCriterion.append("data_group", "1");
    tCriterion.append("data_extraction_operation", "max");
    tCriterion.append("data_file", "matched_power_balance.dat");
    tInputMetaData.append(tCriterion);

    // define objective 
    XMLGen::Objective tObjective;
    tObjective.criteriaIDs.push_back("2");
    tInputMetaData.objective = tObjective;

    // define optimization parameters 
    XMLGen::OptimizationParameters tOptParams;
    std::vector<std::string> tDescriptors = {"slot_length", "slot_width", "slot_depth"};
    tOptParams.descriptors(tDescriptors);
    tOptParams.append("concurrent_evaluations", "2");
    tInputMetaData.set(tOptParams);

    // define services
    XMLGen::Service tServiceOne;
    tServiceOne.append("code", "gemma");
    tServiceOne.append("id", "1");
    tServiceOne.append("type", "system_call");
    tServiceOne.append("number_processors", "1");
    tInputMetaData.append(tServiceOne);
    XMLGen::Service tServiceTwo;
    tServiceTwo.append("code", "platomain");
    tServiceTwo.append("id", "2");
    tServiceTwo.append("type", "plato_app");
    tServiceTwo.append("number_processors", "2");
    tInputMetaData.append(tServiceTwo);

    director::GemmaProblem tGemmaProblem(tInputMetaData);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tGemmaProblem.write_plato_main_input(tDocument));
    ASSERT_FALSE(tDocument.empty());

    //tDocument.save_file("main.xml", "  ");

    auto tOutput = tDocument.child("output");
    ASSERT_FALSE(tOutput.empty());
    
    std::vector<std::string> tKeys = {"file", "format"};
    std::vector<std::string> tValues = {"platomain", "exodus"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tOutput = tOutput.next_sibling("output");
    ASSERT_TRUE(tOutput.empty());
}


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