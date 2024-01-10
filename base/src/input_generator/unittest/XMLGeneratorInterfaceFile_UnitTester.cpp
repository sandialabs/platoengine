/*
 * XMLGeneratorInterfaceFile_UnitTester.cpp
 *
 *  Created on: Jul 24, 2020
 */

#include <gtest/gtest.h>

#include "XMLGenerator_UnitTester_Tools.hpp"

#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorValidInputKeys.hpp"
#include "XMLGeneratorOutputUtilities.hpp"
#include "XMLGeneratorStagesUtilities.hpp"
#include "XMLGeneratorSharedDataUtilities.hpp"
#include "XMLGeneratorPerformersUtilities.hpp"
#include "XMLGeneratorPlatoAnalyzeUtilities.hpp"
#include "XMLGeneratorLaunchScriptUtilities.hpp"
#include "XMLGeneratorInterfaceFileUtilities.hpp"
#include "XMLGeneratorStagesOperationsUtilities.hpp"
#include "XMLGeneratorGradBasedOptimizerOptions.hpp"
#include "XMLGeneratorPlatoMainInputFileUtilities.hpp"
#include "XMLGeneratorPlatoAnalyzeInputFileUtilities.hpp"
#include "XMLGeneratorPlatoMainOperationFileUtilities.hpp"
#include "XMLGeneratorPlatoAnalyzeOperationsFileUtilities.hpp"

namespace PlatoTestXMLGenerator
{

TEST(PlatoTestXMLGenerator, AppendWriteOutputPlatoAnalyzeOperation)
{
    XMLGen::Output tOutput;
    tOutput.appendRandomQoI("dispx", "nodal field");
    tOutput.appendDeterminsiticQoI("vonmises", "element field");
}

TEST(PlatoTestXMLGenerator, AppendComputeQoiStatisticsOperation)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("0");
    tService.code("platomain");
    tMetaData.append(tService);
    XMLGen::Output tOutputMetadata;
    tOutputMetadata.appendRandomQoI("vonmises", "element field");
    tMetaData.mOutputMetaData.push_back(tOutputMetadata);

    pugi::xml_document tDocument;
    XMLGen::append_compute_qoi_statistics_operation(tMetaData, tDocument);

    //tDocument.save_file("testdoc.xml", "  ");
    // TEST OPERATION ARGUMENTS
    auto tOperation = tDocument.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    std::vector<std::string> tKeys = {"Name", "PerformerName", "For", "Output", "Output"};
    std::vector<std::string> tValues = {"compute vonmises statistics", "platomain_0", "", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    // TEST OUTPUTS
    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"vonmises mean", "vonmises mean"}, tOutput);
    tOutput = tOutput.next_sibling("Output");
    ASSERT_FALSE(tOutput.empty());
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"vonmises standard deviation", "vonmises standard deviation"}, tOutput);
    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    // TEST RANDOM QoIs
    auto tOuterFor = tOperation.child("For");
    ASSERT_FALSE(tOuterFor.empty());
    PlatoTestXMLGenerator::test_attributes({"var", "in"}, {"PerformerIndex", "Performers"}, tOuterFor);
    auto tInnerFor = tOuterFor.child("For");
    ASSERT_FALSE(tInnerFor.empty());
    PlatoTestXMLGenerator::test_attributes({"var", "in"}, {"PerformerSampleIndex", "PerformerSamples"}, tInnerFor);
    auto tInput = tInnerFor.child("Input");
    ASSERT_FALSE(tInput.empty());
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"vonmises {PerformerIndex*NumSamplesPerPerformer+PerformerSampleIndex}",
               "vonmises {PerformerIndex*NumSamplesPerPerformer+PerformerSampleIndex}"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
}

TEST(PlatoTestXMLGenerator, WritePlatoMainOperationsXmlFile)
{
    XMLGen::InputData tMetaData;
    XMLGen::Scenario tScenario;
    tScenario.physics("steady_state_incompressible_fluids");
    tMetaData.append(tScenario);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("max_iterations", "10");
    tOptimizationParameters.append("filter_radius_scale", "2.0");
    tOptimizationParameters.append("discretization", "density");
    tOptimizationParameters.append("optimization_algorithm", "oc");
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tOptimizationParameters.filterInEngine(true);
    tOptimizationParameters.normalizeInAggregator(true);
    tMetaData.set(tOptimizationParameters);
    XMLGen::Service tService;
    tService.id("1");
    tService.code("plato_analyze");
    tService.cacheState("false");
    tService.updateProblem("true");
    tMetaData.append(tService);
    XMLGen::Objective tObjective;
    tObjective.serviceIDs.push_back("1");
    tObjective.criteriaIDs.push_back("1");
    tMetaData.objective = tObjective;

    XMLGen::Output tOutputMetaData;
    tOutputMetaData.disableOutput();
    tMetaData.mOutputMetaData.push_back(tOutputMetaData);
    ASSERT_NO_THROW(XMLGen::write_plato_main_operations_xml_file(tMetaData));

    auto tReadData = XMLGen::read_data_from_file("plato_main_operations.xml");
    auto tGold = std::string("<?xmlversion=\"1.0\"?><Filter><Name>Kernel</Name><Scale>2.0</Scale></Filter><Operation><Function>InitializeField</Function><Name>InitializeField</Name><Method>Uniform</Method><Uniform><Value>0.5</Value></Uniform>")
    +"<Output><ArgumentName>InitializedField</ArgumentName></Output></Operation><Operation><Function>SetLowerBounds</Function><Name>ComputeLowerBounds</Name><UseCase>fluid</UseCase><Discretization>density</Discretization><Input><ArgumentName>LowerBoundValue</ArgumentName>"
    +"</Input><Output><ArgumentName>LowerBoundVector</ArgumentName></Output></Operation><Operation><Function>SetUpperBounds</Function><Name>ComputeUpperBounds</Name><UseCase>fluid</UseCase><Discretization>density</Discretization><Input><ArgumentName>UpperBoundValue</ArgumentName>"
    +"</Input><Output><ArgumentName>UpperBoundVector</ArgumentName></Output></Operation><Operation><Function>CopyField</Function><Name>CopyField</Name><Input><ArgumentName>InputField</ArgumentName></Input><Output><ArgumentName>OutputField</ArgumentName></Output></Operation><Operation><Function>CopyValue</Function><Name>CopyValue</Name><Input><ArgumentName>InputValue</ArgumentName></Input><Output><ArgumentName>OutputValue</ArgumentName></Output></Operation><Operation><Function>UpdateProblem</Function><Name>UpdateProblem</Name></Operation><Operation><Function>Filter</Function><Name>FilterControl</Name>"
    +"<Gradient>False</Gradient><Input><ArgumentName>Field</ArgumentName></Input><Output><ArgumentName>FilteredField</ArgumentName></Output></Operation><Operation><Function>Filter</Function><Name>FilterGradient</Name><Gradient>True</Gradient>"
    +"<Input><ArgumentName>Field</ArgumentName></Input><Input><ArgumentName>Gradient</ArgumentName></Input><Output><ArgumentName>FilteredGradient</ArgumentName></Output></Operation><Operation><Name>AggregateData</Name><Function>Aggregator</Function><Aggregate><Layout>Value</Layout><Input><ArgumentName>Value1</ArgumentName></Input><Output><ArgumentName>Value</ArgumentName></Output></Aggregate><Aggregate><Layout>NodalField</Layout><Input><ArgumentName>Field1</ArgumentName></Input><Output><ArgumentName>Field</ArgumentName></Output></Aggregate><Weighting><Weight><Value>1.0</Value></Weight><Normals><Input><ArgumentName>Normal1</ArgumentName></Input></Normals></Weighting></Operation>";
    ASSERT_STREQ(tGold.c_str(), tReadData.str().c_str());
    Plato::system("rm -f plato_main_operations.xml");
}

TEST(PlatoTestXMLGenerator, AppendObjectiveGradientStage)
{
    XMLGen::InputData tMetaData;

    // Create services
    XMLGen::Service tService;
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);

    // Create a criterion
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("mass");
    tMetaData.append(tCriterion);

    // Create a scenario
    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("steady_state_mechanics");
    tMetaData.append(tScenario);
    
    // Create an objective
    XMLGen::Objective tObjective;
    tObjective.type = "single_criterion";
    tObjective.serviceIDs.push_back("2");
    tObjective.criteriaIDs.push_back("3");
    tObjective.scenarioIDs.push_back("14");
    tObjective.weights.push_back("1");
    tMetaData.objective = tObjective;

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("discretization", "density");
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_objective_gradient_stage(tMetaData, tDocument));

    // STAGE INPUTS
    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    ASSERT_STREQ("Stage", tStage.name());
    auto tName = tStage.child("Name");
    ASSERT_STREQ("Compute Objective Gradient", tName.child_value());
    auto tInput = tStage.child("Input");
    ASSERT_STREQ("Input", tInput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Control"}, tInput);

    // FILTER CONTROL OPERATION
    auto tOperation = tStage.child("Operation");
    std::vector<std::string> tKeys = {"Name", "PerformerName", "Input", "Output"};
    std::vector<std::string> tValues = {"Filter Control", "platomain_1", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Field", "Control"}, tOpInputs);
    auto tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Filtered Field", "Topology"}, tOpOutputs);

    // OBJECTIVE VALUE OPERATION
    tOperation = tOperation.next_sibling("Operation");
    tValues = {"Compute Objective Gradient", "plato_analyze_2", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Topology"}, tOpInputs);
    tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Objective Gradient", "Criterion Gradient - criterion_3_service_2_scenario_14"}, tOpOutputs);

    // FILTER GRADIENT OPERATION
    tOperation = tOperation.next_sibling("Operation");
    tKeys = {"Name", "PerformerName", "Input", "Input", "Output"};
    tValues = {"Filter Gradient", "platomain_1", "", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Field", "Control"}, tOpInputs);
    tOpInputs = tOpInputs.next_sibling("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Gradient", "Criterion Gradient - criterion_3_service_2_scenario_14"}, tOpInputs);
    tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Filtered Gradient", "Objective Gradient"}, tOpOutputs);

    // STAGE OUTPUT
    auto tOutput = tStage.child("Output");
    ASSERT_STREQ("Output", tOutput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Objective Gradient"}, tOutput);
}

TEST(PlatoTestXMLGenerator, AppendObjectiveGradientStage_shape_multi_performer)
{
    XMLGen::InputData tMetaData;

    // Create services
    XMLGen::Service tService;
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("3");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("4");
    tService.code("plato_esp");
    tMetaData.append(tService);

    // Create a criterion
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("mechanical_compliance");
    tMetaData.append(tCriterion);

    // Create a scenario
    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("steady_state_mechanics");
    tMetaData.append(tScenario);
    tScenario.id("13");
    tScenario.physics("steady_state_mechanics");
    tMetaData.append(tScenario);
    
    // Create an objective
    XMLGen::Objective tObjective;
    tObjective.type = "weighted_sum";
    tObjective.serviceIDs.push_back("2");
    tObjective.serviceIDs.push_back("3");
    tObjective.criteriaIDs.push_back("3");
    tObjective.criteriaIDs.push_back("3");
    tObjective.scenarioIDs.push_back("14");
    tObjective.scenarioIDs.push_back("13");
    tObjective.weights.push_back("1");
    tObjective.weights.push_back("1");
    tObjective.shapeServiceIDs.push_back("4");
    tObjective.shapeServiceIDs.push_back("4");
    tMetaData.objective = tObjective;

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.optimizationType(XMLGen::OT_SHAPE);
    tOptimizationParameters.append("esp_workflow", "aflr4_aflr3");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_objective_gradient_stage(tMetaData, tDocument));
    //tDocument.save_file("xml.txt", " ");

    // STAGE INPUTS
    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    ASSERT_STREQ("Stage", tStage.name());
    auto tName = tStage.child("Name");
    ASSERT_STREQ("Compute Objective Gradient", tName.child_value());
    auto tInput = tStage.child("Input");
    ASSERT_STREQ("Input", tInput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Design Parameters"}, tInput);
    // Update Geometry on Change OPERATION
    auto tOperation = tStage.child("Operation");
    std::vector<std::string> tKeys = {"Name", "PerformerName", "Input"};
    std::vector<std::string> tValues = {"Update Geometry on Change", "platomain_1", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({ "SharedDataName","ArgumentName"}, { "Design Parameters","Parameters"}, tOpInputs);

    // Nested Reinitialize on Change OPERATION
    auto tOuterOperation = tOperation.next_sibling("Operation");
    tOperation = tOuterOperation.child("Operation");
    tKeys = {"Name", "PerformerName", "Input"};
    tValues = {"Reinitialize on Change", "plato_analyze_2", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({ "SharedDataName","ArgumentName"}, {"Design Parameters","Parameters"}, tOpInputs);
    tOperation = tOperation.next_sibling("Operation");
    tKeys = {"Name", "PerformerName", "Input"};
    tValues = {"Reinitialize on Change", "plato_analyze_3", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({ "SharedDataName","ArgumentName"}, { "Design Parameters","Parameters"}, tOpInputs);

    // Nested Compute Objective Gradient operations
    tOuterOperation = tOuterOperation.next_sibling("Operation");
    tOperation = tOuterOperation.child("Operation");
    tKeys = {"Name", "PerformerName", "Output"};
    tValues = {"Compute Objective Gradient", "plato_analyze_2", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tOpOutputs = tOperation.child("Output");
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"DFDX", "Criterion GradientX - criterion_3_service_2_scenario_14"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOpOutputs);
    tOperation = tOperation.next_sibling("Operation");
    tKeys = {"Name", "PerformerName", "Output"};
    tValues = {"Compute Objective Gradient", "plato_analyze_3", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpOutputs = tOperation.child("Output");
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"DFDX", "Criterion GradientX - criterion_3_service_3_scenario_13"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOpOutputs);
    tOperation = tOperation.next_sibling("For");
    PlatoTestXMLGenerator::test_attributes({"var", "in"}, {"I", "Parameters"}, tOperation);
    tOperation = tOperation.child("Operation");
    tKeys = {"Name", "PerformerName", "Parameter", "Input", "Output"};
    tValues = {"Compute Parameter Sensitivity on Change", "plato_esp_{I}", "", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOperation = tOperation.child("Parameter");
    tKeys = {"ArgumentName", "ArgumentValue"};
    tValues = {"Parameter Index", "{I-1}"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.next_sibling("Input");
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"Parameters", "Design Parameters"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOpInputs);
    tOpOutputs = tOpInputs.next_sibling("Output");
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"Parameter Sensitivity", "Parameter Sensitivity {I}"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOpOutputs);

    // Nested Compute Objective Sensitivity operations
    tOuterOperation = tOuterOperation.next_sibling("Operation");
    tOperation = tOuterOperation.child("Operation");
    tKeys = {"Name", "PerformerName", "For", "Input", "Output"};
    tValues = {"Chain Rule", "platomain_1", "", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tFor = tOperation.child("For");
    PlatoTestXMLGenerator::test_attributes({"var", "in"}, {"I", "Parameters"}, tFor);
    tOpInputs = tFor.child("Input");
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"Parameter Sensitivity {I}", "Parameter Sensitivity {I}"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOpInputs);
    tOpInputs = tFor.next_sibling("Input");
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"DFDX", "Criterion GradientX - criterion_3_service_2_scenario_14"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOpInputs);
    tOpOutputs = tOpInputs.next_sibling("Output");
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"Full Gradient", "Criterion Gradient - criterion_3_service_2_scenario_14"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOpOutputs);

    tOperation = tOperation.next_sibling("Operation");
    tKeys = {"Name", "PerformerName", "For", "Input", "Output"};
    tValues = {"Chain Rule", "platomain_1", "", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tFor = tOperation.child("For");
    PlatoTestXMLGenerator::test_attributes({"var", "in"}, {"I", "Parameters"}, tFor);
    tOpInputs = tFor.child("Input");
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"Parameter Sensitivity {I}", "Parameter Sensitivity {I}"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOpInputs);
    tOpInputs = tFor.next_sibling("Input");
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"DFDX", "Criterion GradientX - criterion_3_service_3_scenario_13"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOpInputs);
    tOpOutputs = tOpInputs.next_sibling("Output");
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"Full Gradient", "Criterion Gradient - criterion_3_service_3_scenario_13"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOpOutputs);

    // Aggregate Data operation
    tOperation = tOuterOperation.next_sibling("Operation");
    tKeys = {"Name", "PerformerName", "Input", "Input", "Output"};
    tValues = {"Aggregate Data", "platomain_1", "", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.child("Input");
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"Value 1", "Criterion Gradient - criterion_3_service_2_scenario_14"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOpInputs);
    tOpInputs = tOpInputs.next_sibling("Input");
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"Value 2", "Criterion Gradient - criterion_3_service_3_scenario_13"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOpInputs);
    tOpOutputs = tOpInputs.next_sibling("Output");
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"Value", "Objective Gradient"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOpOutputs);

    // STAGE OUTPUT
    auto tOutput = tStage.child("Output");
    ASSERT_STREQ("Output", tOutput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Objective Gradient"}, tOutput);
}

TEST(PlatoTestXMLGenerator, AppendObjectiveValueStage)
{
    XMLGen::InputData tMetaData;

    // Create a service
    XMLGen::Service tService;
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);

    // Create a criterion
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("mass");
    tMetaData.append(tCriterion);

    // Create a scenario
    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("steady_state_mechanics");
    tMetaData.append(tScenario);
    
    // Create an objective
    XMLGen::Objective tObjective;
    tObjective.type = "single_criterion";
    tObjective.serviceIDs.push_back("2");
    tObjective.criteriaIDs.push_back("3");
    tObjective.scenarioIDs.push_back("14");
    tObjective.weights.push_back("1");
    tMetaData.objective = tObjective;

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.filterInEngine(true);
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_objective_value_stage(tMetaData, tDocument));

    // STAGE INPUTS
    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    ASSERT_STREQ("Stage", tStage.name());
    auto tName = tStage.child("Name");
    ASSERT_STREQ("Compute Objective Value", tName.child_value());
    auto tInput = tStage.child("Input");
    ASSERT_STREQ("Input", tInput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Control"}, tInput);

    // FILTER CONTROL OPERATION
    auto tOperation = tStage.child("Operation");
    std::vector<std::string> tKeys = {"Name", "PerformerName", "Input", "Output"};
    std::vector<std::string> tValues = {"Filter Control", "platomain_1", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Field", "Control"}, tOpInputs);
    auto tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Filtered Field", "Topology"}, tOpOutputs);

    // OBJECTIVE VALUE OPERATION
    tOperation = tOperation.next_sibling("Operation");
    tValues = {"Compute Objective Value", "plato_analyze_2", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Topology"}, tOpInputs);
    tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Objective Value", "Criterion Value - criterion_3_service_2_scenario_14"}, tOpOutputs);

    // STAGE OUTPUT
    auto tOutput = tStage.child("Output");
    ASSERT_STREQ("Output", tOutput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Objective Value"}, tOutput);
}

TEST(PlatoTestXMLGenerator, AppendObjectiveValueStage_MultiObjective)
{
    XMLGen::InputData tMetaData;

    // Create 2 services
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("5");
    tService.code("plato_analyze");
    tMetaData.append(tService);

    // Create 2 criteria
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("mass");
    tMetaData.append(tCriterion);
    tCriterion.id("7");
    tCriterion.type("heat_conduction");
    tMetaData.append(tCriterion);

    // Create 2 scenarios
    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("mechanical");
    tMetaData.append(tScenario);
    tScenario.id("15");
    tScenario.physics("thermal");
    tMetaData.append(tScenario);
    
    // Create an objective
    XMLGen::Objective tObjective;
    tObjective.type = "weighted_sum";
    tObjective.serviceIDs.push_back("2");
    tObjective.serviceIDs.push_back("5");
    tObjective.criteriaIDs.push_back("3");
    tObjective.criteriaIDs.push_back("7");
    tObjective.scenarioIDs.push_back("14");
    tObjective.scenarioIDs.push_back("15");
    tObjective.weights.push_back("1");
    tObjective.weights.push_back("1");
    tMetaData.objective = tObjective;

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_objective_value_stage(tMetaData, tDocument));

    // STAGE INPUTS
    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    ASSERT_STREQ("Stage", tStage.name());
    auto tName = tStage.child("Name");
    ASSERT_STREQ("Compute Objective Value", tName.child_value());
    auto tInput = tStage.child("Input");
    ASSERT_STREQ("Input", tInput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Control"}, tInput);

    // FILTER CONTROL OPERATION
    auto tOperation = tStage.child("Operation");
    std::vector<std::string> tKeys = {"Name", "PerformerName", "Input", "Output"};
    std::vector<std::string> tValues = {"Filter Control", "platomain_1", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Field", "Control"}, tOpInputs);
    auto tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Filtered Field", "Topology"}, tOpOutputs);

    // OBJECTIVE VALUE OPERATION
    tOperation = tOperation.next_sibling("Operation");
    auto tSubOperation = tOperation.child("Operation");
    tValues = {"Compute Objective Value", "plato_analyze_2", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSubOperation);
    tOpInputs = tSubOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Topology"}, tOpInputs);
    tOpOutputs = tSubOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Objective Value", "Criterion Value - criterion_3_service_2_scenario_14"}, tOpOutputs);

    tSubOperation = tSubOperation.next_sibling("Operation");
    tValues = {"Compute Objective Value", "plato_analyze_5", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSubOperation);
    tOpInputs = tSubOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Topology"}, tOpInputs);
    tOpOutputs = tSubOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Objective Value", "Criterion Value - criterion_7_service_5_scenario_15"}, tOpOutputs);

    // STAGE OUTPUT
    auto tOutput = tStage.child("Output");
    ASSERT_STREQ("Output", tOutput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Objective Value"}, tOutput);
}

TEST(PlatoTestXMLGenerator, AppendObjectiveValueOperation_non_multi_load_case)
{
    XMLGen::InputData tMetaData;

    // Create a service
    XMLGen::Service tService;
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);

    // Create a criterion
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("mass");
    tMetaData.append(tCriterion);

    // Create a scenario
    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("steady_state_mechanics");
    tMetaData.append(tScenario);
    
    // Create an objective
    XMLGen::Objective tObjective;
    tObjective.type = "single_criterion";
    tObjective.serviceIDs.push_back("2");
    tObjective.criteriaIDs.push_back("3");
    tObjective.scenarioIDs.push_back("14");
    tObjective.weights.push_back("1");
    tMetaData.objective = tObjective;

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.filterInEngine(true);
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_objective_value_operation_for_non_multi_load_case(tMetaData, tDocument, false));
    //tDocument.save_file("xml.txt", " ");

    auto tOperation = tDocument.child("Operation");
    std::vector<std::string> tKeys = {"Name", "PerformerName", "Input", "Output"};
    std::vector<std::string> tValues = {"Compute Objective Value", "plato_analyze_2", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Topology"}, tOpInputs);
    auto tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Objective Value", "Criterion Value - criterion_3_service_2_scenario_14"}, tOpOutputs);
}

TEST(PlatoTestXMLGenerator, AppendObjectiveValueOperation_multi_load_case)
{
    XMLGen::InputData tMetaData;

    // Create a service
    XMLGen::Service tService;
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);

    // Create a criterion
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("mass");
    tMetaData.append(tCriterion);

    // Create a scenario
    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("steady_state_mechanics");
    tMetaData.append(tScenario);
    
    // Create a scenario
    tScenario.id("15");
    tScenario.physics("steady_state_mechanics");
    tMetaData.append(tScenario);

    // Create an objective
    XMLGen::Objective tObjective;
    tObjective.type = "single_criterion";
    tObjective.serviceIDs.push_back("2");
    tObjective.criteriaIDs.push_back("3");
    tObjective.scenarioIDs.push_back("14");
    tObjective.scenarioIDs.push_back("15");
    tObjective.weights.push_back("1");
    tObjective.multi_load_case = "true";
    tMetaData.objective = tObjective;

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.filterInEngine(true);
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_objective_value_operation_for_multi_load_case(tMetaData, tDocument, false));
    //tDocument.save_file("xml.txt", " ");

    auto tOperation = tDocument.child("Operation");
    std::vector<std::string> tKeys = {"Name", "PerformerName", "Input", "Output"};
    std::vector<std::string> tValues = {"Compute Objective Value", "plato_analyze_2", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Topology"}, tOpInputs);
    auto tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Objective Value", "Criterion Value - criterion_3_service_2_scenario_1415"}, tOpOutputs);
}

TEST(PlatoTestXMLGenerator, AppendAggregateObjectiveValueOperation_non_multi_load_case)
{
    XMLGen::InputData tMetaData;
    XMLGen::Objective tObjective;
    tObjective.serviceIDs.push_back("2");
    tObjective.serviceIDs.push_back("3");
    tObjective.criteriaIDs.push_back("3");
    tObjective.criteriaIDs.push_back("4");
    tObjective.scenarioIDs.push_back("14");
    tObjective.scenarioIDs.push_back("15");
    tObjective.multi_load_case = "false";
    tMetaData.objective = tObjective;

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_aggregate_objective_value_operation_for_non_multi_load_case(tMetaData, tDocument));
    //tDocument.save_file("xml.txt", " ");

    auto tInput = tDocument.child("Input");
    ASSERT_FALSE(tInput.empty());
    ASSERT_STREQ("Input", tInput.name());
    std::vector<std::string> tKeys = {"ArgumentName", "SharedDataName"};
    std::vector<std::string> tValues = {"Value 1", "Criterion Value - criterion_3_service_2_scenario_14"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
    tInput = tInput.next_sibling("Input");
    ASSERT_FALSE(tInput.empty());
    ASSERT_STREQ("Input", tInput.name());
    tValues = {"Value 2", "Criterion Value - criterion_4_service_3_scenario_15"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
}

TEST(PlatoTestXMLGenerator, AppendAggregateObjectiveValueOperation_multi_load_case)
{
    XMLGen::InputData tMetaData;
    XMLGen::Objective tObjective;
    tObjective.serviceIDs.push_back("2");
    tObjective.serviceIDs.push_back("3");
    tObjective.criteriaIDs.push_back("3");
    tObjective.criteriaIDs.push_back("4");
    tObjective.scenarioIDs.push_back("14");
    tObjective.scenarioIDs.push_back("15");
    tObjective.multi_load_case = "true";
    tMetaData.objective = tObjective;

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_aggregate_objective_value_operation_for_multi_load_case(tMetaData, tDocument));
    //tDocument.save_file("xml.txt", " ");

    auto tInput = tDocument.child("Input");
    ASSERT_FALSE(tInput.empty());
    ASSERT_STREQ("Input", tInput.name());
    std::vector<std::string> tKeys = {"ArgumentName", "SharedDataName"};
    std::vector<std::string> tValues = {"Value 1", "Criterion Value - criterion_3_service_2_scenario_1415"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
    tInput = tInput.next_sibling("Input");
    ASSERT_TRUE(tInput.empty());
}

TEST(PlatoTestXMLGenerator, AppendAggregateObjectiveGradientOperation_non_multi_load_case)
{
    XMLGen::InputData tMetaData;
    XMLGen::Objective tObjective;
    tObjective.serviceIDs.push_back("2");
    tObjective.serviceIDs.push_back("3");
    tObjective.criteriaIDs.push_back("3");
    tObjective.criteriaIDs.push_back("4");
    tObjective.scenarioIDs.push_back("14");
    tObjective.scenarioIDs.push_back("15");
    tObjective.multi_load_case = "false";
    tMetaData.objective = tObjective;

    pugi::xml_document tDocument;
    std::string tType = "Field";
    ASSERT_NO_THROW(XMLGen::append_aggregate_objective_gradient_operation_for_non_multi_load_case(tMetaData, tDocument, tType));
    //tDocument.save_file("xml.txt", " ");

    auto tInput = tDocument.child("Input");
    ASSERT_FALSE(tInput.empty());
    ASSERT_STREQ("Input", tInput.name());
    std::vector<std::string> tKeys = {"ArgumentName", "SharedDataName"};
    std::vector<std::string> tValues = {"Field 1", "Criterion Gradient - criterion_3_service_2_scenario_14"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
    tInput = tInput.next_sibling("Input");
    ASSERT_FALSE(tInput.empty());
    ASSERT_STREQ("Input", tInput.name());
    tValues = {"Field 2", "Criterion Gradient - criterion_4_service_3_scenario_15"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
    auto tOutput = tInput.next_sibling("Output");
    ASSERT_FALSE(tOutput.empty());
    ASSERT_STREQ("Output", tOutput.name());
    tValues = {"Field", "Aggregate Gradient"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);
}

TEST(PlatoTestXMLGenerator, AppendAggregateObjectiveGradientOperation_multi_load_case)
{
    XMLGen::InputData tMetaData;
    XMLGen::Objective tObjective;
    tObjective.serviceIDs.push_back("2");
    tObjective.serviceIDs.push_back("3");
    tObjective.criteriaIDs.push_back("3");
    tObjective.criteriaIDs.push_back("4");
    tObjective.scenarioIDs.push_back("14");
    tObjective.scenarioIDs.push_back("15");
    tObjective.multi_load_case = "true";
    tMetaData.objective = tObjective;

    pugi::xml_document tDocument;
    std::string tType = "Field";
    ASSERT_NO_THROW(XMLGen::append_aggregate_objective_gradient_operation_for_multi_load_case(tMetaData, tDocument, tType));
    //tDocument.save_file("xml.txt", " ");

    auto tInput = tDocument.child("Input");
    ASSERT_FALSE(tInput.empty());
    ASSERT_STREQ("Input", tInput.name());
    std::vector<std::string> tKeys = {"ArgumentName", "SharedDataName"};
    std::vector<std::string> tValues = {"Field 1", "Criterion Gradient - criterion_3_service_2_scenario_1415"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
    auto tOutput = tInput.next_sibling("Output");
    ASSERT_FALSE(tOutput.empty());
    ASSERT_STREQ("Output", tOutput.name());
    tValues = {"Field", "Aggregate Gradient"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);
}

TEST(PlatoTestXMLGenerator, AppendCacheStateStage)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tService.cacheState("true");
    tMetaData.append(tService);

    XMLGen::Objective tObjective;
    tObjective.serviceIDs.push_back("1");
    tObjective.criteriaIDs.push_back("1");
    tMetaData.objective = tObjective;

    XMLGen::Criterion tCriterion;
    tCriterion.id("1");
    tCriterion.type("mass");
    tMetaData.append(tCriterion);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_cache_state_stage(tMetaData, tDocument));

    //tDocument.save_file("xml.txt", " ");

    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    ASSERT_STREQ("Stage", tStage.name());
    PlatoTestXMLGenerator::test_children({"Name", "Operation"}, {"Cache State", ""}, tStage);
    auto tOperation = tStage.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    PlatoTestXMLGenerator::test_children({"Name", "PerformerName"}, {"Cache State", "plato_analyze_1"}, tOperation);
}

TEST(PlatoTestXMLGenerator, AppendCacheStateStage_multi_load_case)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tService.cacheState("true");
    tMetaData.append(tService);
    tMetaData.objective.multi_load_case = "true";
    tMetaData.objective.scenarioIDs.push_back("33");
    tMetaData.objective.serviceIDs.push_back("1");
    XMLGen::Output tOutputMetadata;
    tOutputMetadata.serviceID("1");
    tOutputMetadata.appendDeterminsiticQoI("dispx", "nodal field");
    tMetaData.mOutputMetaData.push_back(tOutputMetadata);

    XMLGen::Criterion tCriterion;
    tCriterion.id("1");
    tCriterion.type("mass");
    tMetaData.append(tCriterion);
    tMetaData.objective.criteriaIDs.push_back("1");

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_cache_state_stage(tMetaData, tDocument));

    //tDocument.save_file("xml.txt", " ");

    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    ASSERT_STREQ("Stage", tStage.name());
    PlatoTestXMLGenerator::test_children({"Name", "Operation"}, {"Cache State", ""}, tStage);
    auto tOperation = tStage.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    PlatoTestXMLGenerator::test_children({"Name", "PerformerName", "Output"}, {"Cache State", "plato_analyze_1", ""}, tOperation);
    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());
    ASSERT_STREQ("Output", tOutput.name());
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"dispx0", "dispx_plato_analyze_1_scenario_33"}, tOutput);
    //tDocument.save_file("xml.txt", " ");
}

TEST(PlatoTestXMLGenerator, AppendUpdateProblemStage)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tService.updateProblem("true");
    tMetaData.append(tService);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_update_problem_stage(tMetaData, tDocument));

    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    ASSERT_STREQ("Stage", tStage.name());
    PlatoTestXMLGenerator::test_children({"Name", "Operation"}, {"Update Problem", ""}, tStage);
    auto tOperation = tStage.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("Operation", tOperation.name());
    PlatoTestXMLGenerator::test_children({"Name", "PerformerName"}, {"Update Problem", "plato_analyze_1"}, tOperation);
}

TEST(PlatoTestXMLGenerator, AppendSharedData)
{
    XMLGen::InputData tMetaData;

    // Create a service
    XMLGen::Service tService;
    tService.id("1");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    XMLGen::Output tOutputMetadata;
    tOutputMetadata.serviceID("1");
    tOutputMetadata.appendDeterminsiticQoI("dispx", "nodal field");
    tMetaData.mOutputMetaData.push_back(tOutputMetadata);
    tService.id("2");
    tService.code("platomain");
    tMetaData.append(tService);

    // Create a criterion
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("mass");
    tMetaData.append(tCriterion);

    // Create a scenario
    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("mechanical");
    tMetaData.append(tScenario);
    
    // Create an objective
    XMLGen::Objective tObjective;
    tObjective.type = "single_criterion";
    tObjective.serviceIDs.push_back("1");
    tObjective.criteriaIDs.push_back("3");
    tObjective.scenarioIDs.push_back("14");
    tObjective.weights.push_back("1");
    tMetaData.objective = tObjective;

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tOptimizationParameters.append("discretization", "density");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    XMLGen::append_shared_data(tMetaData, tDocument);

    // TEST
    auto tSharedData = tDocument.child("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "OwnerName", "UserName"};
    std::vector<std::string> tValues = {"Control", "Scalar", "Nodal Field", "platomain_2", "platomain_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    tValues = {"Lower Bound Value", "Scalar", "Global", "1", "platomain_2", "platomain_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    tKeys = {"Name", "Type", "Layout", "OwnerName", "UserName"};
    tValues = {"Lower Bound Vector", "Scalar", "Nodal Field", "platomain_2", "platomain_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    tValues = {"Upper Bound Value", "Scalar", "Global", "1", "platomain_2", "platomain_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    tKeys = {"Name", "Type", "Layout", "OwnerName", "UserName"};
    tValues = {"Upper Bound Vector", "Scalar", "Nodal Field", "platomain_2", "platomain_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    tValues = {"Design Volume", "Scalar", "Global", "1", "platomain_2", "platomain_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    tValues = {"Criterion Value - criterion_3_service_1_scenario_14", "Scalar", "Global", "1", "plato_analyze_1", "platomain_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    tKeys = {"Name", "Type", "Layout", "OwnerName", "UserName"};
    tValues = {"Criterion Gradient - criterion_3_service_1_scenario_14", "Scalar", "Nodal Field", "plato_analyze_1", "platomain_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    tValues = {"Objective Value", "Scalar", "Global", "1", "platomain_2", "platomain_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    tKeys = {"Name", "Type", "Layout", "OwnerName", "UserName"};
    tValues = {"Objective Gradient", "Scalar", "Nodal Field", "platomain_2", "platomain_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    tKeys = {"Name", "Type", "Layout", "OwnerName", "UserName"};
    tValues = {"dispx_plato_analyze_1", "Scalar", "Nodal Field", "plato_analyze_1", "platomain_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    tKeys = {"Name", "Type", "Layout", "OwnerName", "UserName", "UserName"};
    tValues = {"Topology", "Scalar", "Nodal Field", "platomain_2", "plato_analyze_1","platomain_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);
}

TEST(PlatoTestXMLGenerator, AppendTopologySharedData_ErrorEmptyService)
{
    XMLGen::InputData tMetaData;
    pugi::xml_document tDocument;
    ASSERT_THROW(XMLGen::append_topology_shared_data(tMetaData, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendTopologySharedData)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);

    pugi::xml_document tDocument;

    ASSERT_NO_THROW(XMLGen::append_topology_shared_data(tMetaData, tDocument));

    auto tSharedData = tDocument.child("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "OwnerName", "UserName", "UserName"};
    std::vector<std::string> tValues = {"Topology", "Scalar", "Nodal Field", "platomain_1", "platomain_1", "plato_analyze_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);
    tSharedData = tSharedData.next_sibling("Performer");
    ASSERT_TRUE(tSharedData.empty());
}

TEST(PlatoTestXMLGenerator, AppendQoISharedData_ErrorEmptyService)
{
    XMLGen::InputData tMetaData;
    pugi::xml_document tDocument;
    ASSERT_THROW(XMLGen::append_qoi_shared_data(tMetaData, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendQoISharedData_EmptySharedData)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tMetaData.append(tService);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_qoi_shared_data(tMetaData, tDocument));

    auto tSharedData = tDocument.child("SharedData");
    ASSERT_TRUE(tSharedData.empty());
}

TEST(PlatoTestXMLGenerator, AppendQoISharedData_ErrorDidNotMatchOuputServiceID)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tMetaData.append(tService);
    XMLGen::Output tOutputMetadata;
    tOutputMetadata.serviceID("2");
    tOutputMetadata.appendDeterminsiticQoI("dispx", "nodal field");
    tMetaData.mOutputMetaData.push_back(tOutputMetadata);

    pugi::xml_document tDocument;
    ASSERT_THROW(XMLGen::append_qoi_shared_data(tMetaData, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendDeterministicQoIToPlatoMainOutput_non_multi_load_case)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("platomain");
    tMetaData.append(tService);
    XMLGen::Output tOutputMetadata;
    tOutputMetadata.serviceID("1");
    tOutputMetadata.appendDeterminsiticQoI("dispx", "nodal field");
    tMetaData.mOutputMetaData.push_back(tOutputMetadata);

    pugi::xml_document tDocument;
    XMLGen::append_deterministic_qoi_to_output_operation_in_interface_file_for_non_multi_load_case(tMetaData, tDocument);
    //tDocument.save_file("xml.txt", " ");

    auto tInput = tDocument.child("Input");
    ASSERT_FALSE(tInput.empty());
    ASSERT_STREQ("Input", tInput.name());
    std::vector<std::string> tKeys = {"ArgumentName", "SharedDataName"};
    std::vector<std::string> tValues = {"dispx_plato_analyze_1", "dispx_plato_analyze_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
}

TEST(PlatoTestXMLGenerator, AppendDeterministicQoIToPlatoMainOutput_multi_load_case)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("platomain");
    tMetaData.append(tService);
    XMLGen::Output tOutputMetadata;
    tOutputMetadata.serviceID("1");
    tOutputMetadata.appendDeterminsiticQoI("dispx", "nodal field");
    tMetaData.mOutputMetaData.push_back(tOutputMetadata);
    tMetaData.objective.multi_load_case = "true";
    tMetaData.objective.scenarioIDs.push_back("1");
    tMetaData.objective.scenarioIDs.push_back("2");

    pugi::xml_document tDocument;
    XMLGen::append_deterministic_qoi_to_output_operation_in_interface_file_for_multi_load_case(tMetaData, tDocument);
    //tDocument.save_file("xml.txt", " ");

    auto tInput = tDocument.child("Input");
    ASSERT_FALSE(tInput.empty());
    ASSERT_STREQ("Input", tInput.name());
    std::vector<std::string> tKeys = {"ArgumentName", "SharedDataName"};
    std::vector<std::string> tValues = {"dispx_plato_analyze_1_scenario_1", "dispx_plato_analyze_1_scenario_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
    tInput = tInput.next_sibling("Input");
    ASSERT_FALSE(tInput.empty());
    ASSERT_STREQ("Input", tInput.name());
    tValues = {"dispx_plato_analyze_1_scenario_2", "dispx_plato_analyze_1_scenario_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
}

TEST(PlatoTestXMLGenerator, AppendNormalizationSharedData_non_multi_load_case)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("platomain");
    tMetaData.append(tService);
    tMetaData.objective.multi_load_case = "false";
    tMetaData.objective.scenarioIDs.push_back("5");
    tMetaData.objective.scenarioIDs.push_back("6");
    tMetaData.objective.serviceIDs.push_back("1");
    tMetaData.objective.serviceIDs.push_back("2");
    tMetaData.objective.criteriaIDs.push_back("8");
    tMetaData.objective.criteriaIDs.push_back("9");
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.normalizeInAggregator(true);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    XMLGen::append_normalization_shared_data(tMetaData, tDocument);
    //tDocument.save_file("xml.txt", " ");

    auto tSharedData = tDocument.child("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    std::vector<std::string> tValues = {"Initial Criterion Value - criterion_8_service_1_scenario_5", 
               "Scalar", "Global", "1", "plato_analyze_1", "platomain_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);
    tSharedData = tSharedData.next_sibling("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    tValues = {"Initial Criterion Value - criterion_9_service_2_scenario_6", 
               "Scalar", "Global", "1", "platomain_2", "platomain_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);
}

TEST(PlatoTestXMLGenerator, AppendNormalizationSharedData_multi_load_case)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("platomain");
    tMetaData.append(tService);
    tMetaData.objective.multi_load_case = "true";
    tMetaData.objective.scenarioIDs.push_back("5");
    tMetaData.objective.scenarioIDs.push_back("6");
    tMetaData.objective.serviceIDs.push_back("1");
    tMetaData.objective.serviceIDs.push_back("2");
    tMetaData.objective.criteriaIDs.push_back("8");
    tMetaData.objective.criteriaIDs.push_back("9");
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.normalizeInAggregator(true);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    XMLGen::append_normalization_shared_data(tMetaData, tDocument);
    //tDocument.save_file("xml.txt", " ");

    auto tSharedData = tDocument.child("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    std::vector<std::string> tValues = {"Initial Criterion Value - criterion_8_service_1_scenario_56", 
               "Scalar", "Global", "1", "plato_analyze_1", "platomain_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);
    tSharedData = tSharedData.next_sibling("SharedData");
    ASSERT_TRUE(tSharedData.empty());
}

TEST(PlatoTestXMLGenerator, AppendQoISharedData)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("platomain");
    tMetaData.append(tService);
    XMLGen::Output tOutputMetadata;
    tOutputMetadata.serviceID("1");
    tOutputMetadata.appendDeterminsiticQoI("dispx", "nodal field");
    tMetaData.mOutputMetaData.push_back(tOutputMetadata);

    pugi::xml_document tDocument;
    XMLGen::append_qoi_shared_data(tMetaData, tDocument);

    auto tSharedData = tDocument.child("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "OwnerName", "UserName"};
    std::vector<std::string> tValues = {"dispx_plato_analyze_1", "Scalar", "Nodal Field", "plato_analyze_1", "platomain_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);
    tSharedData = tSharedData.next_sibling("Performer");
    ASSERT_TRUE(tSharedData.empty());
}

TEST(PlatoTestXMLGenerator, AppendQoISharedData_multi_load_case)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("platomain");
    tMetaData.append(tService);
    XMLGen::Output tOutputMetadata;
    tOutputMetadata.serviceID("1");
    tOutputMetadata.appendDeterminsiticQoI("dispx", "nodal field");
    tMetaData.mOutputMetaData.push_back(tOutputMetadata);
    tMetaData.objective.multi_load_case = "true";
    tMetaData.objective.scenarioIDs.push_back("22");

    pugi::xml_document tDocument;
    XMLGen::append_qoi_shared_data(tMetaData, tDocument);

    auto tSharedData = tDocument.child("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "OwnerName", "UserName"};
    std::vector<std::string> tValues = {"dispx_plato_analyze_1_scenario_22", "Scalar", "Nodal Field", "plato_analyze_1", "platomain_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);
    tSharedData = tSharedData.next_sibling("Performer");
    ASSERT_TRUE(tSharedData.empty());
}

TEST(PlatoTestXMLGenerator, AppendPhysicsPerformers_EmptyService)
{
    XMLGen::InputData tMetaData;
    pugi::xml_document tDocument;
    ASSERT_THROW(XMLGen::append_uniperformer_usecase(tMetaData, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendPhysicsPerformers)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tMetaData.mPerformerServices.push_back(tService);

    pugi::xml_document tDocument;
    ASSERT_EQ(0, XMLGen::append_plato_main_performer(tMetaData, tDocument));
    ASSERT_NO_THROW(XMLGen::append_uniperformer_usecase(tMetaData, tDocument));

    auto tPerformer = tDocument.child("Performer");
    ASSERT_FALSE(tPerformer.empty());
    tPerformer = tPerformer.next_sibling("Performer");
    ASSERT_FALSE(tPerformer.empty());
    ASSERT_STREQ("Performer", tPerformer.name());
    std::vector<std::string> tKeys = {"Name", "Code", "PerformerID"};
    std::vector<std::string> tValues = {"plato_analyze_2", "plato_analyze", "1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tPerformer);
    tPerformer = tPerformer.next_sibling("Performer");
    ASSERT_TRUE(tPerformer.empty());
}

TEST(PlatoTestXMLGenerator, AppendROLStepBlock_LinearConstraint)
{
    XMLGen::InputData tMetaData;
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("optimization_algorithm", "rol_linear_constraint");
    tOptimizationParameters.append("rol_subproblem_model", "lin_more");
    tOptimizationParameters.append("rol_lin_more_cauchy_initial_step_size", "4.5");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    XMLGen::append_rol_step_block(tMetaData, tDocument);

    auto tParameterList = tDocument.child("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Step"}, tParameterList);
    auto tParameter = tParameterList.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Type", "string", "Trust Region"}, tParameter);
    tParameterList = tParameter.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Line Search"}, tParameterList);
    tParameterList = tParameterList.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Trust Region"}, tParameterList);
    tParameter = tParameterList.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Subproblem Solver", "string", "Truncated CG"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Subproblem Model", "string", "Lin-More"}, tParameter);
    tParameterList = tParameter.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"SPG"}, tParameterList);
    tParameterList = tParameterList.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Kelley-Sachs"}, tParameterList);
    tParameterList = tParameterList.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Lin-More"}, tParameterList);
    tParameter = tParameterList.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Maximum Number of Minor Iterations", "int", "0"}, tParameter);
    tParameterList = tParameterList.child("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Cauchy Point"}, tParameterList);
    tParameter = tParameterList.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Maximum Number of Reduction Steps", "int", "10"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Maximum Number of Expansion Steps", "int", "10"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Initial Step Size", "double", "4.5"}, tParameter);
}

TEST(PlatoTestXMLGenerator, AppendROLStepBlock_BoundConstrained)
{
    XMLGen::InputData tMetaData;
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("optimization_algorithm", "rol_bound_constrained");
    tOptimizationParameters.append("rol_subproblem_model", "kelley_sachs");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    XMLGen::append_rol_step_block(tMetaData, tDocument);

    auto tParameterList = tDocument.child("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Step"}, tParameterList);
    auto tParameter = tParameterList.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Type", "string", "Trust Region"}, tParameter);
    tParameterList = tParameter.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Line Search"}, tParameterList);
    tParameterList = tParameterList.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Trust Region"}, tParameterList);
    tParameter = tParameterList.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Subproblem Solver", "string", "Truncated CG"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Subproblem Model", "string", "Kelley-Sachs"}, tParameter);
    tParameterList = tParameter.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"SPG"}, tParameterList);
    tParameterList = tParameterList.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Kelley-Sachs"}, tParameterList);
    tParameterList = tParameterList.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Lin-More"}, tParameterList);
    tParameter = tParameterList.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Maximum Number of Minor Iterations", "int", "0"}, tParameter);
}

TEST(PlatoTestXMLGenerator, AppendROLStepBlock_AugmentedLagrangian)
{
    XMLGen::InputData tMetaData;
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("optimization_algorithm", "rol_augmented_lagrangian");
    tOptimizationParameters.append("rol_subproblem_model", "kelley_sachs");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    XMLGen::append_rol_step_block(tMetaData, tDocument);

    auto tParameterList = tDocument.child("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Step"}, tParameterList);
    auto tParameter = tParameterList.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Type", "string", "Augmented Lagrangian"}, tParameter);
    tParameterList = tParameter.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Line Search"}, tParameterList);
    tParameterList = tParameterList.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Trust Region"}, tParameterList);
    tParameter = tParameterList.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Subproblem Solver", "string", "Truncated CG"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Subproblem Model", "string", "Kelley-Sachs"}, tParameter);
    tParameterList = tParameter.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"SPG"}, tParameterList);
    tParameterList = tParameterList.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Kelley-Sachs"}, tParameterList);
    tParameterList = tParameterList.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Lin-More"}, tParameterList);
    tParameter = tParameterList.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Maximum Number of Minor Iterations", "int", "0"}, tParameter);
}

TEST(PlatoTestXMLGenerator, AppendROLStepBlock_AugmentedLagrangian_Hessian)
{
    XMLGen::InputData tMetaData;
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("optimization_algorithm", "rol_augmented_lagrangian");
    tOptimizationParameters.append("rol_subproblem_model", "kelley_sachs");
    tOptimizationParameters.append("hessian_type", "finite_difference");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    XMLGen::append_rol_step_block(tMetaData, tDocument);

    auto tParameterList = tDocument.child("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Step"}, tParameterList);
    auto tParameter = tParameterList.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Type", "string", "Augmented Lagrangian"}, tParameter);
    tParameterList = tParameter.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Line Search"}, tParameterList);
    tParameterList = tParameterList.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Trust Region"}, tParameterList);
    tParameter = tParameterList.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Subproblem Solver", "string", "Truncated CG"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Subproblem Model", "string", "Kelley-Sachs"}, tParameter);
    tParameterList = tParameter.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"SPG"}, tParameterList);
    tParameterList = tParameterList.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Kelley-Sachs"}, tParameterList);
    tParameterList = tParameterList.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Lin-More"}, tParameterList);
    tParameter = tParameterList.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    PlatoTestXMLGenerator::test_attributes({"name","type","value"}, 
                                           {"Maximum Number of Minor Iterations", "int", "1"}, tParameter);
}

TEST(PlatoTestXMLGenerator, AppendPhysicsPerformersWithHelmholtz)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tMetaData.mPerformerServices.push_back(tService);
    tService.id("helmholtz");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tMetaData.mPerformerServices.push_back(tService);

    pugi::xml_document tDocument;
    ASSERT_EQ(0, XMLGen::append_plato_main_performer(tMetaData, tDocument));
    ASSERT_NO_THROW(XMLGen::append_uniperformer_usecase(tMetaData, tDocument));

    auto tPerformer = tDocument.child("Performer");
    ASSERT_FALSE(tPerformer.empty());

    tPerformer = tPerformer.next_sibling("Performer");
    ASSERT_FALSE(tPerformer.empty());
    ASSERT_STREQ("Performer", tPerformer.name());
    std::vector<std::string> tKeys1 = {"Name", "Code", "PerformerID"};
    std::vector<std::string> tValues1 = {"plato_analyze_2", "plato_analyze", "1"};
    PlatoTestXMLGenerator::test_children(tKeys1, tValues1, tPerformer);

    tPerformer = tPerformer.next_sibling("Performer");
    ASSERT_FALSE(tPerformer.empty());
    ASSERT_STREQ("Performer", tPerformer.name());
    std::vector<std::string> tKeys2 = {"Name", "Code", "PerformerID"};
    std::vector<std::string> tValues2 = {"plato_analyze_helmholtz", "plato_analyze", "2"};
    PlatoTestXMLGenerator::test_children(tKeys2, tValues2, tPerformer);

    tPerformer = tPerformer.next_sibling("Performer");
    ASSERT_TRUE(tPerformer.empty());
}

TEST(PlatoTestXMLGenerator, AppendTopologySharedDataWithHelmholtz)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("helmholtz");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tMetaData.mPerformerServices.push_back(tService);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("filter_type", "helmholtz");
    tOptimizationParameters.filterInEngine(false);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;

    ASSERT_NO_THROW(XMLGen::append_topology_shared_data(tMetaData, tDocument));

    auto tSharedData = tDocument.child("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "OwnerName", "UserName", "UserName", "UserName"};
    std::vector<std::string> tValues = {"Topology", "Scalar", "Nodal Field", "plato_analyze_helmholtz", "platomain_1", "plato_analyze_2", "plato_analyze_helmholtz"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);
    tSharedData = tSharedData.next_sibling("Performer");
    ASSERT_TRUE(tSharedData.empty());
}

TEST(PlatoTestXMLGenerator, AppendTopologySharedDataWithHelmholtzAndProjection)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("helmholtz");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tMetaData.mPerformerServices.push_back(tService);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("filter_type", "helmholtz");
    tOptimizationParameters.filterInEngine(false);
    tOptimizationParameters.append("projection_type", "heaviside");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;

    ASSERT_NO_THROW(XMLGen::append_topology_shared_data(tMetaData, tDocument));

    auto tSharedData = tDocument.child("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "OwnerName", "UserName", "UserName", "UserName"};
    std::vector<std::string> tValues = {"Topology", "Scalar", "Nodal Field", "platomain_1", "platomain_1", "plato_analyze_2", "plato_analyze_helmholtz"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    tKeys = {"Name", "Type", "Layout", "OwnerName", "UserName", "UserName"};
    tValues = {"Filtered Control", "Scalar", "Nodal Field", "plato_analyze_helmholtz", "plato_analyze_helmholtz", "platomain_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("Performer");
    ASSERT_TRUE(tSharedData.empty());
}

TEST(PlatoTestXMLGenerator, AppendObjectiveSharedDataWithHelmholtz)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("helmholtz");
    tService.code("plato_analyze");
    tMetaData.mPerformerServices.push_back(tService);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("filter_type", "helmholtz");
    tOptimizationParameters.filterInEngine(false);
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_gradient_based_objective_shared_data(tMetaData, tDocument));

    auto tSharedData = tDocument.child("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    std::vector<std::string> tValues = {"Objective Value", "Scalar", "Global", "1", "platomain_1", "platomain_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    tKeys = {"Name", "Type", "Layout", "OwnerName", "UserName", "UserName"};
    tValues = {"Objective Gradient", "Scalar", "Nodal Field", "plato_analyze_helmholtz", "plato_analyze_helmholtz", "platomain_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("Performer");
    ASSERT_TRUE(tSharedData.empty());
}

TEST(PlatoTestXMLGenerator, AppendConstraintSharedDataWithHelmholtz)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("helmholtz");
    tService.code("plato_analyze");
    tMetaData.mPerformerServices.push_back(tService);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("filter_type", "helmholtz");
    tOptimizationParameters.filterInEngine(false);
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tMetaData.set(tOptimizationParameters);

    XMLGen::Constraint tConstraint;
    tConstraint.id("1");
    tConstraint.criterion("3");
    tConstraint.service("1");
    tMetaData.constraints.push_back(tConstraint);

    pugi::xml_document tDocument;

    ASSERT_NO_THROW(XMLGen::append_gradient_based_constraint_shared_data(tMetaData, tDocument));

    auto tSharedData = tDocument.child("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    std::vector<std::string> tValues = {"Constraint Value 1", "Scalar", "Global", "1", "platomain_1", "platomain_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    tKeys = {"Name", "Type", "Layout", "OwnerName", "UserName", "UserName"};
    tValues = {"Constraint Gradient 1", "Scalar", "Nodal Field", "plato_analyze_helmholtz", "plato_analyze_helmholtz", "platomain_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("Performer");
    ASSERT_TRUE(tSharedData.empty());
}

TEST(PlatoTestXMLGenerator, AppendCriteriaSharedDataWithHelmholtz)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("helmholtz");
    tService.code("plato_analyze");
    tMetaData.mPerformerServices.push_back(tService);
    
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("mass");
    tMetaData.append(tCriterion);

    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("steady_state_mechanics");
    tMetaData.append(tScenario);
    
    XMLGen::Objective tObjective;
    tObjective.type = "single_criterion";
    tObjective.serviceIDs.push_back("2");
    tObjective.criteriaIDs.push_back("3");
    tObjective.scenarioIDs.push_back("14");
    tObjective.weights.push_back("1");
    tMetaData.objective = tObjective;

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("filter_type", "helmholtz");
    tOptimizationParameters.filterInEngine(false);
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tOptimizationParameters.append("discretization", "density");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_gradient_based_criterion_shared_data(tMetaData, tDocument));
    tDocument.save_file("append_gradient_based_criterion_shared_data.txt");

    auto tSharedData = tDocument.child("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    std::vector<std::string> tValues = {"Criterion Value - criterion_3_service_2_scenario_14", "Scalar", "Global", "1", "plato_analyze_2", "platomain_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    tKeys = {"Name", "Type", "Layout", "OwnerName", "UserName", "UserName"};
    tValues = {"Criterion Gradient - criterion_3_service_2_scenario_14", "Scalar", "Nodal Field", "plato_analyze_2", "platomain_1", "plato_analyze_helmholtz"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("Performer");
    ASSERT_TRUE(tSharedData.empty());
}

TEST(PlatoTestXMLGenerator, AppendCriteriaSharedDataWithHelmholtzAndProjection)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("helmholtz");
    tService.code("plato_analyze");
    tMetaData.mPerformerServices.push_back(tService);
    
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("mass");
    tMetaData.append(tCriterion);

    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("steady_state_mechanics");
    tMetaData.append(tScenario);
    
    XMLGen::Objective tObjective;
    tObjective.type = "single_criterion";
    tObjective.serviceIDs.push_back("2");
    tObjective.criteriaIDs.push_back("3");
    tObjective.scenarioIDs.push_back("14");
    tObjective.weights.push_back("1");
    tMetaData.objective = tObjective;

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("filter_type", "helmholtz");
    tOptimizationParameters.filterInEngine(false);
    tOptimizationParameters.append("projection_type", "tanh");
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tOptimizationParameters.append("discretization", "density");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;

    ASSERT_NO_THROW(XMLGen::append_gradient_based_criterion_shared_data(tMetaData, tDocument));

    auto tSharedData = tDocument.child("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    std::vector<std::string> tValues = {"Criterion Value - criterion_3_service_2_scenario_14", "Scalar", "Global", "1", "plato_analyze_2", "platomain_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    tKeys = {"Name", "Type", "Layout", "OwnerName", "UserName"};
    tValues = {"Criterion Gradient - criterion_3_service_2_scenario_14", "Scalar", "Nodal Field", "plato_analyze_2", "platomain_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    tKeys = {"Name", "Type", "Layout", "OwnerName", "UserName", "UserName"};
    tValues = {"Projected Gradient - criterion_3_service_2_scenario_14", "Scalar", "Nodal Field", "platomain_1", "platomain_1", "plato_analyze_helmholtz"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("Performer");
    ASSERT_TRUE(tSharedData.empty());
}

TEST(PlatoTestXMLGenerator, AppendObjectiveGradientStageWithHelmholtz)
{
    XMLGen::InputData tMetaData;

    // Create services
    XMLGen::Service tService;
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("helmholtz");
    tService.code("plato_analyze");
    tMetaData.mPerformerServices.push_back(tService);

    // Create a criterion
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("mass");
    tMetaData.append(tCriterion);

    // Create a scenario
    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("steady_state_mechanics");
    tMetaData.append(tScenario);
    
    // Create an objective
    XMLGen::Objective tObjective;
    tObjective.type = "single_criterion";
    tObjective.serviceIDs.push_back("2");
    tObjective.criteriaIDs.push_back("3");
    tObjective.scenarioIDs.push_back("14");
    tObjective.weights.push_back("1");
    tMetaData.objective = tObjective;

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("filter_type", "helmholtz");
    tOptimizationParameters.filterInEngine(false);
    tOptimizationParameters.append("discretization", "density");

    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_objective_gradient_stage(tMetaData, tDocument));

    // STAGE INPUTS
    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    ASSERT_STREQ("Stage", tStage.name());
    auto tName = tStage.child("Name");
    ASSERT_STREQ("Compute Objective Gradient", tName.child_value());
    auto tInput = tStage.child("Input");
    ASSERT_STREQ("Input", tInput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Control"}, tInput);

    // FILTER CONTROL OPERATION
    auto tOperation = tStage.child("Operation");
    std::vector<std::string> tKeys = {"Name", "PerformerName", "Input", "Output"};
    std::vector<std::string> tValues = {"Filter Control", "plato_analyze_helmholtz", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Control"}, tOpInputs);
    auto tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Topology"}, tOpOutputs);

    // OBJECTIVE GRADIENT OPERATION
    tOperation = tOperation.next_sibling("Operation");
    tValues = {"Compute Objective Gradient", "plato_analyze_2", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Topology"}, tOpInputs);
    tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Objective Gradient", "Criterion Gradient - criterion_3_service_2_scenario_14"}, tOpOutputs);

    // FILTER GRADIENT OPERATION
    tOperation = tOperation.next_sibling("Operation");
    tKeys = {"Name", "PerformerName", "Input", "Output"};//<<<removed 1 input
    tValues = {"Filter Gradient", "plato_analyze_helmholtz", "", ""};//<<<removed 1 ""
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Criterion Gradient - criterion_3_service_2_scenario_14"}, tOpInputs);
    tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Objective Gradient"}, tOpOutputs);

    // STAGE OUTPUT
    auto tOutput = tStage.child("Output");
    ASSERT_STREQ("Output", tOutput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Objective Gradient"}, tOutput);
}

TEST(PlatoTestXMLGenerator, AppendCriteriaSharedData_SO_egads_tetgen_workflow)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("mechanical_compliance");
    tMetaData.append(tCriterion);

    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("steady_state_mechanics");
    tMetaData.append(tScenario);
    
    XMLGen::Objective tObjective;
    tObjective.type = "single_criterion";
    tObjective.serviceIDs.push_back("2");
    tObjective.criteriaIDs.push_back("3");
    tObjective.scenarioIDs.push_back("14");
    tObjective.weights.push_back("1");
    tMetaData.objective = tObjective;

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.optimizationType(XMLGen::OT_SHAPE);
    tOptimizationParameters.append("esp_workflow", "egads_tetgen");
    tOptimizationParameters.append("num_shape_design_variables", "3");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;

    ASSERT_NO_THROW(XMLGen::append_gradient_based_criterion_shared_data(tMetaData, tDocument));

    auto tSharedData = tDocument.child("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    std::vector<std::string> tValues = {"Criterion Value - criterion_3_service_2_scenario_14", "Scalar", "Global", "1", "plato_analyze_2", "platomain_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    tKeys = {"Name", "Type", "Layout", "Dynamic", "OwnerName", "UserName"};
    tValues = {"Criterion GradientX - criterion_3_service_2_scenario_14", "Scalar", "Global", "true", "plato_analyze_2", "platomain_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    tValues = {"Criterion Gradient - criterion_3_service_2_scenario_14", "Scalar", "Global", "3", "platomain_1", "platomain_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);
}

TEST(PlatoTestXMLGenerator, AppendCriteriaSharedData_SO_aflr_workflow)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("mechanical_compliance");
    tMetaData.append(tCriterion);

    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("steady_state_mechanics");
    tMetaData.append(tScenario);
    
    XMLGen::Objective tObjective;
    tObjective.type = "single_criterion";
    tObjective.serviceIDs.push_back("2");
    tObjective.criteriaIDs.push_back("3");
    tObjective.scenarioIDs.push_back("14");
    tObjective.weights.push_back("1");
    tMetaData.objective = tObjective;

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.optimizationType(XMLGen::OT_SHAPE);
    tOptimizationParameters.append("esp_workflow", "aflr4_aflr3");
    tOptimizationParameters.append("num_shape_design_variables", "4");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;

    ASSERT_NO_THROW(XMLGen::append_gradient_based_criterion_shared_data(tMetaData, tDocument));

    auto tSharedData = tDocument.child("SharedData");
    ASSERT_FALSE(tSharedData.empty());
    ASSERT_STREQ("SharedData", tSharedData.name());
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    std::vector<std::string> tValues = {"Criterion Value - criterion_3_service_2_scenario_14", "Scalar", "Global", "1", "plato_analyze_2", "platomain_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);

    tSharedData = tSharedData.next_sibling("SharedData");
    tKeys = {"Name", "Type", "Layout", "Dynamic", "OwnerName", "UserName"};
    tValues = {"Criterion GradientX - criterion_3_service_2_scenario_14", "Scalar", "Global", "true", "plato_analyze_2", "platomain_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSharedData);
}

TEST(PlatoTestXMLGenerator, AppendConstraintGradientStageWithHelmholtz)
{
    XMLGen::InputData tMetaData;

    // Create services
    XMLGen::Service tService;
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("helmholtz");
    tService.code("plato_analyze");
    tMetaData.mPerformerServices.push_back(tService);

    // Create a criterion
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("volume");
    tMetaData.append(tCriterion);

    // Create a scenario
    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("steady_state_mechanics");
    tMetaData.append(tScenario);
    
    // Create a constraint
    XMLGen::Constraint tConstraint;
    tConstraint.id("18");
    tConstraint.criterion("3");
    tConstraint.service("1");
    tMetaData.constraints.push_back(tConstraint);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("filter_type", "helmholtz");
    tOptimizationParameters.filterInEngine(false);
    tOptimizationParameters.append("discretization", "density");
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_constraint_gradient_stage(tMetaData, tDocument));
    
    // STAGE INPUTS
    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    ASSERT_STREQ("Stage", tStage.name());
    auto tName = tStage.child("Name");
    ASSERT_STREQ("Compute Constraint Gradient 18", tName.child_value());
    auto tInput = tStage.child("Input");
    ASSERT_STREQ("Input", tInput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Control"}, tInput);

    // FILTER CONTROL OPERATION
    auto tOperation = tStage.child("Operation");
    std::vector<std::string> tKeys = {"Name", "PerformerName", "Input", "Output"};
    std::vector<std::string> tValues = {"Filter Control", "plato_analyze_helmholtz", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Control"}, tOpInputs);
    auto tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Topology"}, tOpOutputs);

    // CONSTRAINT GRADIENT OPERATION
    tOperation = tOperation.next_sibling("Operation");
    tKeys = {"Name", "PerformerName", "Input", "Output", "Output"};
    tValues = {"Compute Constraint Gradient 18", "platomain_1", "", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Topology"}, tOpInputs);
    tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Volume", "Criterion Value - criterion_3_service_1_scenario_"}, tOpOutputs);
    tOpOutputs = tOpOutputs.next_sibling("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Volume Gradient", "Criterion Gradient - criterion_3_service_1_scenario_"}, tOpOutputs);

    // FILTER GRADIENT OPERATION
    tOperation = tOperation.next_sibling("Operation");
    tKeys = {"Name", "PerformerName", "Input",  "Output"};
    tValues = {"Filter Gradient", "plato_analyze_helmholtz", "", ""};
   
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Criterion Gradient - criterion_3_service_1_scenario_"}, tOpInputs);
    tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Constraint Gradient 18"}, tOpOutputs);

    // STAGE OUTPUT
    auto tOutput = tStage.child("Output");
    ASSERT_STREQ("Output", tOutput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Constraint Gradient 18"}, tOutput);
}

TEST(PlatoTestXMLGenerator, AppendObjectiveValueStageWithHelmholtz)
{
    XMLGen::InputData tMetaData;

    // Create services
    XMLGen::Service tService;
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("helmholtz");
    tService.code("plato_analyze");
    tMetaData.mPerformerServices.push_back(tService);

    // Create a criterion
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("mass");
    tMetaData.append(tCriterion);

    // Create a scenario
    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("steady_state_mechanics");
    tMetaData.append(tScenario);
    
    // Create an objective
    XMLGen::Objective tObjective;
    tObjective.type = "single_criterion";
    tObjective.serviceIDs.push_back("2");
    tObjective.criteriaIDs.push_back("3");
    tObjective.scenarioIDs.push_back("14");
    tObjective.weights.push_back("1");
    tMetaData.objective = tObjective;

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("filter_type", "helmholtz");
    tOptimizationParameters.filterInEngine(false);
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tOptimizationParameters.append("discretization", "density");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_objective_value_stage(tMetaData, tDocument));

    // STAGE INPUTS
    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    ASSERT_STREQ("Stage", tStage.name());
    auto tName = tStage.child("Name");
    ASSERT_STREQ("Compute Objective Value", tName.child_value());
    auto tInput = tStage.child("Input");
    ASSERT_STREQ("Input", tInput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Control"}, tInput);

    // FILTER CONTROL OPERATION
    auto tOperation = tStage.child("Operation");
    std::vector<std::string> tKeys = {"Name", "PerformerName", "Input", "Output"};
    std::vector<std::string> tValues = {"Filter Control", "plato_analyze_helmholtz", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Control"}, tOpInputs);
    auto tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Topology"}, tOpOutputs);

    // OBJECTIVE VALUE OPERATION
    tOperation = tOperation.next_sibling("Operation");
    tValues = {"Compute Objective Value", "plato_analyze_2", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Topology"}, tOpInputs);
    tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Objective Value", "Criterion Value - criterion_3_service_2_scenario_14"}, tOpOutputs);

    // STAGE OUTPUT
    auto tOutput = tStage.child("Output");
    ASSERT_STREQ("Output", tOutput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Objective Value"}, tOutput);
}

TEST(PlatoTestXMLGenerator, AppendObjectiveValueStageWithHelmholtzAndProjection)
{
    XMLGen::InputData tMetaData;

    // Create services
    XMLGen::Service tService;
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("helmholtz");
    tService.code("plato_analyze");
    tMetaData.mPerformerServices.push_back(tService);

    // Create a criterion
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("mass");
    tMetaData.append(tCriterion);

    // Create a scenario
    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("steady_state_mechanics");
    tMetaData.append(tScenario);
    
    // Create an objective
    XMLGen::Objective tObjective;
    tObjective.type = "single_criterion";
    tObjective.serviceIDs.push_back("2");
    tObjective.criteriaIDs.push_back("3");
    tObjective.scenarioIDs.push_back("14");
    tObjective.weights.push_back("1");
    tMetaData.objective = tObjective;

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("filter_type", "helmholtz");
    tOptimizationParameters.filterInEngine(false);
    tOptimizationParameters.append("projection_type", "heaviside");
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tOptimizationParameters.append("discretization", "density");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_objective_value_stage(tMetaData, tDocument));

    // STAGE INPUTS
    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    ASSERT_STREQ("Stage", tStage.name());
    auto tName = tStage.child("Name");
    ASSERT_STREQ("Compute Objective Value", tName.child_value());
    auto tInput = tStage.child("Input");
    ASSERT_STREQ("Input", tInput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Control"}, tInput);

    // FILTER CONTROL OPERATION
    auto tOperation = tStage.child("Operation");
    std::vector<std::string> tKeys = {"Name", "PerformerName", "Input", "Output"};
    std::vector<std::string> tValues = {"Filter Control", "plato_analyze_helmholtz", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Control"}, tOpInputs);
    auto tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Filtered Control"}, tOpOutputs);
    
    // PROJECT CONTROL OPERATION
    tOperation = tOperation.next_sibling("Operation");
    tValues = {"Project Control", "platomain_1", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Field", "Filtered Control"}, tOpInputs);
    tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Filtered Field", "Topology"}, tOpOutputs);

    // OBJECTIVE VALUE OPERATION
    tOperation = tOperation.next_sibling("Operation");
    tValues = {"Compute Objective Value", "plato_analyze_2", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Topology"}, tOpInputs);
    tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Objective Value", "Criterion Value - criterion_3_service_2_scenario_14"}, tOpOutputs);

    // STAGE OUTPUT
    auto tOutput = tStage.child("Output");
    ASSERT_STREQ("Output", tOutput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Objective Value"}, tOutput);
}

TEST(PlatoTestXMLGenerator, AppendConstraintGradientStageWithHelmholtzAndProjection)
{
    XMLGen::InputData tMetaData;

    // Create services
    XMLGen::Service tService;
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("helmholtz");
    tService.code("plato_analyze");
    tMetaData.mPerformerServices.push_back(tService);

    // Create a criterion
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("volume");
    tMetaData.append(tCriterion);

    // Create a scenario
    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("steady_state_mechanics");
    tMetaData.append(tScenario);
    
    // Create a constraint
    XMLGen::Constraint tConstraint;
    tConstraint.id("18");
    tConstraint.criterion("3");
    tConstraint.service("1");
    tMetaData.constraints.push_back(tConstraint);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("filter_type", "helmholtz");
    tOptimizationParameters.filterInEngine(false);
    tOptimizationParameters.append("projection_type", "tanh");
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tOptimizationParameters.append("discretization", "density");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_constraint_gradient_stage(tMetaData, tDocument));

    // STAGE INPUTS
    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    ASSERT_STREQ("Stage", tStage.name());
    auto tName = tStage.child("Name");
    ASSERT_STREQ("Compute Constraint Gradient 18", tName.child_value());
    auto tInput = tStage.child("Input");
    ASSERT_STREQ("Input", tInput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Control"}, tInput);

    // FILTER CONTROL OPERATION
    auto tOperation = tStage.child("Operation");
    std::vector<std::string> tKeys = {"Name", "PerformerName", "Input", "Output"};
    std::vector<std::string> tValues = {"Filter Control", "plato_analyze_helmholtz", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Control"}, tOpInputs);
    auto tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Filtered Control"}, tOpOutputs);
    
    // PROJECT CONTROL OPERATION
    tOperation = tOperation.next_sibling("Operation");
    tValues = {"Project Control", "platomain_1", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Field", "Filtered Control"}, tOpInputs);
    tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Filtered Field", "Topology"}, tOpOutputs);

    // CONSTRAINT GRADIENT OPERATION
    tOperation = tOperation.next_sibling("Operation");
    tKeys = {"Name", "PerformerName", "Input", "Output", "Output"};
    tValues = {"Compute Constraint Gradient 18", "platomain_1", "", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Topology"}, tOpInputs);
    tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Volume", "Criterion Value - criterion_3_service_1_scenario_"}, tOpOutputs);
    tOpOutputs = tOpOutputs.next_sibling("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Volume Gradient", "Criterion Gradient - criterion_3_service_1_scenario_"}, tOpOutputs);
    
    // PROJECT GRADIENT OPERATION
    tOperation = tOperation.next_sibling("Operation");
    tKeys = {"Name", "PerformerName", "Input", "Input", "Output"};
    tValues = {"Project Gradient", "platomain_1", "", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Field", "Filtered Control"}, tOpInputs);
    tOpInputs = tOpInputs.next_sibling("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Gradient", "Criterion Gradient - criterion_3_service_1_scenario_"}, tOpInputs);
    tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Filtered Gradient", "Projected Constraint Gradient 18"}, tOpOutputs);

    // FILTER GRADIENT OPERATION
    tOperation = tOperation.next_sibling("Operation");
    tKeys = {"Name", "PerformerName", "Input", "Output"};
    tValues = {"Filter Gradient", "plato_analyze_helmholtz", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Projected Constraint Gradient 18"}, tOpInputs);
    tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Topology", "Constraint Gradient 18"}, tOpOutputs);

    // STAGE OUTPUT
    auto tOutput = tStage.child("Output");
    ASSERT_STREQ("Output", tOutput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Constraint Gradient 18"}, tOutput);
}

TEST(PlatoTestXMLGenerator, AppendConstraintOptions)
{
    XMLGen::InputData tMetaData;
    XMLGen::OptimizationParameters tOptParams;
    tOptParams.optimizationType(XMLGen::OT_SHAPE);
    tMetaData.set(tOptParams);

    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);

    XMLGen::Criterion tCriterion;
    tCriterion.id("1");
    tMetaData.append(tCriterion);

    XMLGen::Constraint tConstraint;
    tConstraint.service("1");
    tConstraint.criterion("1");
    tConstraint.id("1");
    tConstraint.append("absolute_target", ".008");
    tConstraint.append("divisor", "1.0");
    tMetaData.constraints.push_back(tConstraint);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_grad_based_optimizer_constraint_options(tMetaData, tDocument));
    
    auto tConstraintNode = tDocument.child("Constraint");
    ASSERT_FALSE(tConstraintNode.empty());

    std::vector<std::string> tKeys = {"AbsoluteTargetValue", "GradientName", "GradientStageName", "ReferenceValue",
                   "ValueName" , "ValueStageName"};
    std::vector<std::string> tValues = {".008", "Constraint Gradient 1", "Compute Constraint Gradient 1", "0.008000",
                   "Constraint Value 1", "Compute Constraint Value 1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tConstraintNode);
}

TEST(PlatoTestXMLGenerator, AppendMultipleConstraints)
{
    XMLGen::InputData tMetaData;
    XMLGen::OptimizationParameters tOptParams;
    tOptParams.optimizationType(XMLGen::OT_SHAPE);
    tMetaData.set(tOptParams);

    XMLGen::Service tService1;
    tService1.id("1");
    tService1.code("platomain");
    tMetaData.append(tService1);

    XMLGen::Service tService2;
    tService2.id("2");
    tService2.code("plato_analyze");
    tMetaData.append(tService2);

    XMLGen::Criterion tCriterion1;
    tCriterion1.id("1");
    tCriterion1.type("volume");
    tMetaData.append(tCriterion1);

    XMLGen::Criterion tCriterion2;
    tCriterion2.id("2");
    tCriterion2.type("mechanical_compliance");
    tMetaData.append(tCriterion2);

    XMLGen::Constraint tConstraint1;
    tConstraint1.service("1");
    tConstraint1.criterion("1");
    tConstraint1.id("1");
    tConstraint1.append("divisor", "1.0");
    tConstraint1.append("absolute_target", ".4");
    tMetaData.constraints.push_back(tConstraint1);

    XMLGen::Constraint tConstraint2;
    tConstraint2.service("2");
    tConstraint2.criterion("2");
    tConstraint2.id("2");
    tConstraint2.append("divisor", "1.0");
    tConstraint2.append("absolute_target", ".1");
    tMetaData.constraints.push_back(tConstraint2);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_grad_based_optimizer_constraint_options(tMetaData, tDocument));
    
    auto tConstraintNode = tDocument.child("Constraint");
    ASSERT_FALSE(tConstraintNode.empty());

    std::vector<std::string> tKeys = {"AbsoluteTargetValue", "GradientName", "GradientStageName", "ReferenceValue",
                    "ValueName", "ValueStageName"};
    std::vector<std::string> tValues = {".4", "Constraint Gradient 1", "Compute Constraint Gradient 1", "0.400000",
                    "Constraint Value 1", "Compute Constraint Value 1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tConstraintNode);

    tConstraintNode = tConstraintNode.next_sibling("Constraint");
    ASSERT_FALSE(tConstraintNode.empty());

    tKeys = {"AbsoluteTargetValue", "GradientName", "GradientStageName", "ReferenceValue",
                    "ValueName", "ValueStageName"};
    tValues = {".1", "Constraint Gradient 2", "Compute Constraint Gradient 2", "0.100000",
                    "Constraint Value 2", "Compute Constraint Value 2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tConstraintNode);
}

TEST(PlatoTestXMLGenerator, TOLSGenerateModelInput)
{
    XMLGen::InputData tMetaData;

    XMLGen::Service tService;
    tMetaData.append(tService);

    tService.id("3");
    tService.code("xtk");
    tMetaData.append(tService);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("discretization", "levelset");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    auto tStageNode = tDocument.append_child("Stage");
    XMLGen::append_generate_xtk_model_operation(tMetaData,true,false,tStageNode);
    // tDocument.save_file("xml_xtk.txt", " ");

    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());

    auto tOperation = tStage.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    std::vector<std::string> tKeys = {"Name", "PerformerName", "Input"};
    std::vector<std::string> tValues = {"Update Problem", "xtk_3", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    
    auto tInput = tOperation.child("Input");
    ASSERT_FALSE(tInput.empty());

    tKeys   = {"ArgumentName", "SharedDataName"};
    tValues = {"Topology", "Topology"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
    
}

TEST(PlatoTestXMLGenerator, TOLSGenerateModelOutput)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tMetaData.append(tService);

    tService.id("3");
    tService.code("xtk");
    tMetaData.append(tService);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("discretization", "levelset");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    auto tStageNode = tDocument.append_child("Stage");
    XMLGen::append_generate_xtk_model_operation(tMetaData,false,true,tStageNode);
    // tDocument.save_file("xml_xtk.txt", " ");

    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());

    auto tOperation = tStage.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    std::vector<std::string> tKeys = {"Name", "PerformerName", "Output"};
    std::vector<std::string> tValues = {"Update Problem", "xtk_3", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    
    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());

    tKeys   = { "SharedDataName","ArgumentName"};
    tValues = { "Initial Control","Initial Control"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);
    
}

TEST(PlatoTestXMLGenerator, TOLSInitialGuessData)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tMetaData.append(tService);

    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);

    tService.id("3");
    tService.code("xtk");
    tMetaData.append(tService);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("discretization", "levelset");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    XMLGen::append_initial_control_shared_data(tMetaData,tDocument);
    // tDocument.save_file("xml_xtk.txt", " ");

    auto tSD = tDocument.child("SharedData");
    ASSERT_FALSE(tSD.empty());
    std::vector<std::string> tKeys =   {"Name",            "Type",   "Layout",     "OwnerName","UserName",   "UserName"};
    std::vector<std::string> tValues = {"Initial Control", "Scalar", "Nodal Field","xtk_3",    "platomain_1","xtk_3"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSD);
}

TEST(PlatoTestXMLGenerator, TOLSInitialGuessStage)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tMetaData.append(tService);

    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);

    tService.id("3");
    tService.code("xtk");
    tMetaData.append(tService);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("discretization", "levelset");
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    XMLGen::append_initial_guess_stage(tMetaData, tDocument);
    ASSERT_FALSE(tDocument.empty());
    // tDocument.save_file("xml_xtk.txt", " ");

    auto tStage     = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    std::vector<std::string> tKeys =   {"Name", "Operation", "Output"};
    std::vector<std::string> tValues = {"Initial Guess", "", "" };
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tStage);

    auto tOperation = tStage.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    tKeys =   {"Name", "PerformerName", "Output"};
    tValues = {"Update Problem", "xtk_3", "" };
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tOutput = tOperation.child("Output");
    tKeys =   {"SharedDataName", "ArgumentName"};
    tValues = {"Initial Control", "Initial Control"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tOutput = tStage.child("Output");
    tKeys =   {"SharedDataName"};
    tValues = {"Initial Control"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

}

TEST(PlatoTestXMLGenerator, TOLSUpperBoundSD)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tMetaData.append(tService);

    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);

    tService.id("3");
    tService.code("xtk");
    tMetaData.append(tService);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("discretization", "levelset");
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    XMLGen::append_upper_bounds_shared_data(tMetaData, tDocument);
    //tDocument.save_file("xml_xtk.txt", " ");

    auto tSD = tDocument.child("SharedData");
    ASSERT_FALSE(tSD.empty());
    std::vector<std::string> tKeys =   {"Name","Type","Layout","Size","OwnerName","UserName"};
    std::vector<std::string> tValues = {"Upper Bound Value", "Scalar", "Global","1","platomain_1","platomain_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSD);

    tSD = tSD.next_sibling("SharedData");
    ASSERT_FALSE(tSD.empty());
    tKeys =   {"Name","Type","Layout","OwnerName","UserName","UserName"};
    tValues = {"Upper Bound Vector", "Scalar", "Nodal Field","xtk_3","platomain_1","xtk_3"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSD);
}

TEST(PlatoTestXMLGenerator, TOLSUpperBoundStage)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tMetaData.append(tService);

    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);

    tService.id("3");
    tService.code("xtk");
    tMetaData.append(tService);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("discretization", "levelset");
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    XMLGen::append_upper_bound_stage(tMetaData, tDocument);
    // tDocument.save_file("xml_xtk.txt", " ");

    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    std::vector<std::string> tKeys =   {"Name", "Operation", "Output"};
    std::vector<std::string> tValues = {"Set Upper Bounds", "", "" };
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tStage);

    auto tOperation = tStage.child("Operation");
    tKeys =   {"Name", "PerformerName", "Output"};
    tValues = {"Compute Upper Bounds", "xtk_3", "" };
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tOutput = tOperation.child("Output");
    tKeys   = {"SharedDataName", "ArgumentName"};
    tValues = {"Upper Bound Vector", "Upper Bound Vector"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tOutput = tStage.child("Output");
    tKeys   = {"SharedDataName"};
    tValues = {"Upper Bound Vector"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);
}

TEST(PlatoTestXMLGenerator, TOLSLowerBoundSD)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tMetaData.append(tService);

    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);

    tService.id("3");
    tService.code("xtk");
    tMetaData.append(tService);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("discretization", "levelset");
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    XMLGen::append_lower_bounds_shared_data(tMetaData, tDocument);
    //tDocument.save_file("xml_xtk.txt", " ");

    auto tSD = tDocument.child("SharedData");
    ASSERT_FALSE(tSD.empty());
    std::vector<std::string> tKeys =   {"Name","Type","Layout","Size","OwnerName","UserName"};
    std::vector<std::string> tValues = {"Lower Bound Value", "Scalar", "Global","1","platomain_1","platomain_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSD);

    tSD = tSD.next_sibling("SharedData");
    ASSERT_FALSE(tSD.empty());
    tKeys =   {"Name","Type","Layout","OwnerName","UserName","UserName"};
    tValues = {"Lower Bound Vector", "Scalar", "Nodal Field","xtk_3","platomain_1","xtk_3"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tSD);
}

TEST(PlatoTestXMLGenerator, TOLSLowerBoundStage)
{
    XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tMetaData.append(tService);

    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);

    tService.id("3");
    tService.code("xtk");
    tMetaData.append(tService);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("discretization", "levelset");
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    XMLGen::append_lower_bound_stage(tMetaData, tDocument);
    // tDocument.save_file("xml_xtk.txt", " ");

    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    std::vector<std::string> tKeys =   {"Name", "Operation", "Output"};
    std::vector<std::string> tValues = {"Set Lower Bounds", "", "" };
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tStage);

    auto tOperation = tStage.child("Operation");
    tKeys =   {"Name", "PerformerName", "Output"};
    tValues = {"Compute Lower Bounds", "xtk_3", "" };
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    auto tOutput = tOperation.child("Output");
    tKeys   = {"SharedDataName", "ArgumentName"};
    tValues = {"Lower Bound Vector", "Lower Bound Vector"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tOutput = tStage.child("Output");
    tKeys   = {"SharedDataName"};
    tValues = {"Lower Bound Vector"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);
}

TEST(PlatoTestXMLGenerator, TOLSUpdateProblem)
{
   XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tService.updateProblem("false");
    tMetaData.append(tService);

    tService.id("2");
    tService.code("plato_analyze");
    tService.updateProblem("false");
    tMetaData.append(tService);

    tService.id("3");
    tService.code("xtk");
    tService.updateProblem("true");
    tMetaData.append(tService);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("discretization", "levelset");
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    XMLGen::append_update_problem_stage(tMetaData, tDocument);
    
    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());

    auto tInput = tStage.child("Input");
    std::vector<std::string> tKeys =   {"SharedDataName"};
    std::vector<std::string> tValues = {"Control" };
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);

    auto tOperation = tStage.child("Operation");
    tKeys =   {"Name","PerformerName","Input","Output"};
    tValues = {"Filter Control","platomain_1","","" };
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    tInput = tOperation.child("Input");
    tKeys =   {"ArgumentName", "SharedDataName"};
    tValues = {"Field", "Control"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);
    
    auto tOutput = tOperation.child("Output");
    tKeys =   {"ArgumentName", "SharedDataName"};
    tValues = {"Filtered Field", "Topology"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOutput);

    tOperation = tOperation.next_sibling("Operation");
    tKeys =   {"Name","PerformerName","Input"};
    tValues = {"Update Problem","xtk_3","" };
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    tInput = tOperation.child("Input");
    tKeys =   {"ArgumentName", "SharedDataName"};
    tValues = {"Topology", "Topology"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tInput);

    tOperation = tOperation.next_sibling("Operation");
    tKeys =   {"Name","PerformerName"};
    tValues = {"Reload Mesh","plato_analyze_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

}

TEST(PlatoTestXMLGenerator, TOLSObjGradient)
{
       XMLGen::InputData tMetaData;
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tService.updateProblem("false");
    tMetaData.append(tService);

    tService.id("2");
    tService.code("plato_analyze");
    tService.updateProblem("false");
    tMetaData.append(tService);

    tService.id("3");
    tService.code("xtk");
    tService.updateProblem("true");
    tMetaData.append(tService);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("discretization", "levelset");
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tMetaData.set(tOptimizationParameters);

    // Create a criterion
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("mass");
    tMetaData.append(tCriterion);

    // Create a scenario
    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("steady_state_mechanics");
    

    // Create an objective
    XMLGen::Objective tObjective;
    tObjective.type = "single_criterion";
    tObjective.serviceIDs.push_back("2");
    tObjective.criteriaIDs.push_back("3");
    tObjective.scenarioIDs.push_back("14");
    tObjective.weights.push_back("1");
    tMetaData.objective = tObjective;

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_objective_gradient_stage_for_topology_levelset_problem(tMetaData, tDocument));
    //tDocument.save_file("xml_xtk.txt", " ");
}

TEST(PlatoTestXMLGenerator, LSTOAppendObjectiveGradientStage)
{
    XMLGen::InputData tMetaData;

    // Create services
    XMLGen::Service tService;
    tService.id("2");
    tService.code("plato_analyze");
    tMetaData.append(tService);
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("3");
    tService.code("xtk");
    tMetaData.append(tService);

    // Create a criterion
    XMLGen::Criterion tCriterion;
    tCriterion.id("3");
    tCriterion.type("mass");
    tMetaData.append(tCriterion);

    // Create a scenario
    XMLGen::Scenario tScenario;
    tScenario.id("14");
    tScenario.physics("steady_state_mechanics");
    tMetaData.append(tScenario);
    
    // Create an objective
    XMLGen::Objective tObjective;
    tObjective.type = "single_criterion";
    tObjective.serviceIDs.push_back("2");
    tObjective.criteriaIDs.push_back("3");
    tObjective.scenarioIDs.push_back("14");
    tObjective.weights.push_back("1");
    tMetaData.objective = tObjective;

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("discretization", "levelset");
    tOptimizationParameters.optimizationType(XMLGen::OT_TOPOLOGY);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_objective_gradient_stage(tMetaData, tDocument));

    // STAGE INPUTS
    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    ASSERT_STREQ("Stage", tStage.name());
    auto tName = tStage.child("Name");
    ASSERT_STREQ("Compute Objective Gradient", tName.child_value());
    auto tInput = tStage.child("Input");
    ASSERT_STREQ("Input", tInput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Control"}, tInput);

    // FILTER CONTROL OPERATION
    auto tOperation = tStage.child("Operation");
    std::vector<std::string> tKeys = {"Name", "PerformerName", "Input", "Output"};
    std::vector<std::string> tValues = {"Filter Control", "platomain_1", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Field", "Control"}, tOpInputs);
    auto tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Filtered Field", "Topology"}, tOpOutputs);

    // COMPUTE OBJECTIVE GRADIENT - There are no outputs (they are currently placed in hdf file)
    tOperation = tOperation.next_sibling("Operation");
    tKeys = {"Name", "PerformerName"};
    tValues = {"Compute Objective Gradient", "plato_analyze_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    // LOAD FROM HDF5
    tOperation  = tOperation.next_sibling("Operation");
    tKeys = {"Name", "PerformerName"};
    tValues = {"Load Objective GradientX From HDF5", "xtk_3"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    // XTK COMPUTE GRADIENT OPERATION
    tOperation = tOperation.next_sibling("Operation");
    tKeys = {"Name", "PerformerName", "Output"};
    tValues = {"Compute Objective Gradient XTK", "xtk_3", ""};
    tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Criterion Gradient - criterion_3_service_2_scenario_14", "Criterion Gradient - criterion_3_service_2_scenario_14"}, tOpOutputs);

    // FILTER GRADIENT OPERATION
    tOperation = tOperation.next_sibling("Operation");
    tKeys = {"Name", "PerformerName", "Input", "Input", "Output"};
    tValues = {"Filter Gradient", "platomain_1", "", "", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    tOpInputs = tOperation.child("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Field", "Control"}, tOpInputs);
    tOpInputs = tOpInputs.next_sibling("Input");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Gradient", "Criterion Gradient - criterion_3_service_2_scenario_14"}, tOpInputs);
    tOpOutputs = tOperation.child("Output");
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Filtered Gradient", "Objective Gradient"}, tOpOutputs);

    // STAGE OUTPUT
    auto tOutput = tStage.child("Output");
    ASSERT_STREQ("Output", tOutput.name());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, {"Objective Gradient"}, tOutput);
}

TEST(PlatoTestXMLGenerator, AppendInitializeGeometryOperation)
{
    XMLGen::InputData tMetaData;

    // Create a service
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_initialize_geometry_operation(tMetaData, tDocument));
    //tDocument.save_file("xml.txt", " ");

    auto tOperation = tDocument.child("Operation");
    std::vector<std::string> tKeys = {"Name", "PerformerName"};
    std::vector<std::string> tValues = {"Initialize Geometry", "platomain_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
}

TEST(PlatoTestXMLGenerator, AppendDefaultQOIToOutputOperation)
{
    XMLGen::InputData tMetaData;

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.enforceBounds(true);
    tMetaData.set(tOptimizationParameters);

    XMLGen::Constraint tConstraint;
    tConstraint.id("3");
    tConstraint.criterion("3");
    tConstraint.service("3");
    tMetaData.constraints.push_back(tConstraint);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_default_qoi_to_output_operation_in_interface_file(tMetaData, tDocument));
    //tDocument.save_file("xml.txt", " ");

    auto tNode = tDocument.child("Input");
    std::vector<std::string> tKeys = {"ArgumentName", "SharedDataName"};
    std::vector<std::string> tValues = {"topology", "Clamped Topology"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tNode);
    tNode = tNode.next_sibling("Input");
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"control", "Control"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tNode);
    tNode = tNode.next_sibling("Input");
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"objective gradient", "Objective Gradient"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tNode);
    tNode = tNode.next_sibling("Input");
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"constraint gradient 3", "Constraint Gradient 3"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tNode);
}

TEST(PlatoTestXMLGenerator, AppendInitialGuessStageForShapeOptimization)
{
    XMLGen::InputData tMetaData;

    // Create a service
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);

    // Create optimization parameters
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.optimizationType(XMLGen::OT_SHAPE);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_initial_guess_stage(tMetaData, tDocument));

    // STAGE NAME
    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    ASSERT_STREQ("Stage", tStage.name());
    auto tName = tStage.child("Name");
    ASSERT_STREQ("Initialize Design Parameters", tName.child_value());

    // INITIALIZE DESIGN PARAMETERS OPERATION
    auto tOperation = tStage.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    std::vector<std::string> tKeys = {"Name", "PerformerName", "Output"};
    std::vector<std::string> tValues = {"Initialize Values", "platomain_1", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Values", "Design Parameters"}, tOutput);

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    // INITIALIZE GEOMETRY
    tOperation = tOperation.next_sibling("Operation");
    ASSERT_FALSE(tOperation.empty());
    tKeys = {"Name", "PerformerName"};
    tValues = {"Initialize Geometry", "platomain_1"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());

    // STAGE OUTPUT
    tOutput = tStage.child("Output");
    ASSERT_FALSE(tOutput.empty());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, { "Design Parameters"}, tOutput);

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());
}

TEST(PlatoTestXMLGenerator, AppendLowerBoundStageForShapeOptimization)
{
    XMLGen::InputData tMetaData;

    // Create a service
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);

    // Create optimization parameters
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.optimizationType(XMLGen::OT_SHAPE);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_lower_bound_stage(tMetaData, tDocument));

    // STAGE NAME
    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    ASSERT_STREQ("Stage", tStage.name());
    auto tName = tStage.child("Name");
    ASSERT_STREQ("Set Lower Bounds", tName.child_value());

    // INITIALIZE DESIGN PARAMETERS OPERATION
    auto tOperation = tStage.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    std::vector<std::string> tKeys = {"Name", "PerformerName", "Output"};
    std::vector<std::string> tValues = {"Compute Lower Bounds", "platomain_1", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Lower Bounds", "Lower Bound Vector"}, tOutput);

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());

    // STAGE OUTPUT
    tOutput = tStage.child("Output");
    ASSERT_FALSE(tOutput.empty());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, { "Lower Bound Vector"}, tOutput);

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());
}

TEST(PlatoTestXMLGenerator, AppendUpperBoundStageForShapeOptimization)
{
    XMLGen::InputData tMetaData;

    // Create a service
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);

    // Create optimization parameters
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.optimizationType(XMLGen::OT_SHAPE);
    tMetaData.set(tOptimizationParameters);

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_upper_bound_stage(tMetaData, tDocument));

    // STAGE NAME
    auto tStage = tDocument.child("Stage");
    ASSERT_FALSE(tStage.empty());
    ASSERT_STREQ("Stage", tStage.name());
    auto tName = tStage.child("Name");
    ASSERT_STREQ("Set Upper Bounds", tName.child_value());

    // INITIALIZE DESIGN PARAMETERS OPERATION
    auto tOperation = tStage.child("Operation");
    ASSERT_FALSE(tOperation.empty());
    std::vector<std::string> tKeys = {"Name", "PerformerName", "Output"};
    std::vector<std::string> tValues = {"Compute Upper Bounds", "platomain_1", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);
    auto tOutput = tOperation.child("Output");
    ASSERT_FALSE(tOutput.empty());
    PlatoTestXMLGenerator::test_children({"ArgumentName", "SharedDataName"}, {"Upper Bounds", "Upper Bound Vector"}, tOutput);

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());

    tOperation = tOperation.next_sibling("Operation");
    ASSERT_TRUE(tOperation.empty());

    // STAGE OUTPUT
    tOutput = tStage.child("Output");
    ASSERT_FALSE(tOutput.empty());
    PlatoTestXMLGenerator::test_children({"SharedDataName"}, { "Upper Bound Vector"}, tOutput);

    tOutput = tOutput.next_sibling("Output");
    ASSERT_TRUE(tOutput.empty());
}

TEST(PlatoTestXMLGenerator, AppendOptimizationAlgorithmOptions_ErrorOptimizerNotSupported)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("optimization_algorithm", "stochastic gradient descent");
    tXMLMetaData.set(tOptimizationParameters);
    auto tOptimizerNode = tDocument.append_child("Optimizer");
    ASSERT_THROW(XMLGen::append_grad_based_optimizer_parameters(tXMLMetaData, tOptimizerNode), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendOptimizationAlgorithmOption_ErrorOptimizerNotSupported)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("optimization_algorithm", "stochastic gradient descent");
    tXMLMetaData.set(tOptimizationParameters);
    auto tOptimizerNode = tDocument.append_child("Optimizer");
    ASSERT_THROW(XMLGen::append_grad_based_optimizer_options(tXMLMetaData, tOptimizerNode), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendOptimizationOutputOptions)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    auto tOptimizerNode = tDocument.append_child("Optimizer");
    XMLGen::append_grad_based_optimizer_output_options(tXMLMetaData, tOptimizerNode);

    // ****** TEST RESULTS AGAINST GOLD VALUES ******
    std::vector<std::string> tGoldKeys = {"Output"};
    std::vector<std::string> tGoldValues = {""};
    PlatoTestXMLGenerator::test_children(tGoldKeys, tGoldValues, tOptimizerNode);
    auto tOutputNode = tOptimizerNode.child("Output");
    tGoldKeys = {"OutputStage"};
    tGoldValues = {"Output To File"};
    PlatoTestXMLGenerator::test_children(tGoldKeys, tGoldValues, tOutputNode);
}

TEST(PlatoTestXMLGenerator, AppendOptimizationCacheStateOptions)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::Service tService;
    tService.cacheState("true");
    tXMLMetaData.append(tService);
    auto tOptimizerNode = tDocument.append_child("Optimizer");
    XMLGen::append_optimizer_cache_stage_options(tXMLMetaData, tOptimizerNode);

    // ****** TEST RESULTS AGAINST GOLD VALUES ******
    std::vector<std::string> tGoldKeys = {"CacheStage"};
    std::vector<std::string> tGoldValues = {""};
    PlatoTestXMLGenerator::test_children(tGoldKeys, tGoldValues, tOptimizerNode);
    auto tOutputNode = tOptimizerNode.child("CacheStage");
    ASSERT_FALSE(tOutputNode.empty());
    tGoldKeys = {"Name"};
    tGoldValues = {"Cache State"};
    PlatoTestXMLGenerator::test_children(tGoldKeys, tGoldValues, tOutputNode);
}

TEST(PlatoTestXMLGenerator, AppendOptimizationUpdateProblemOptions)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::Service tService;
    tService.updateProblem("true");
    tXMLMetaData.append(tService);
    auto tOptimizerNode = tDocument.append_child("Optimizer");
    XMLGen::append_optimizer_update_problem_stage_options(tXMLMetaData, tOptimizerNode);

    // ****** TEST RESULTS AGAINST GOLD VALUES ******
    std::vector<std::string> tGoldKeys = {"UpdateProblemStage"};
    std::vector<std::string> tGoldValues = {""};
    PlatoTestXMLGenerator::test_children(tGoldKeys, tGoldValues, tOptimizerNode);
    auto tOutputNode = tOptimizerNode.child("UpdateProblemStage");
    ASSERT_FALSE(tOutputNode.empty());
    tGoldKeys = {"Name"};
    tGoldValues = {"Update Problem"};
    PlatoTestXMLGenerator::test_children(tGoldKeys, tGoldValues, tOutputNode);
}

TEST(PlatoTestXMLGenerator, AppendOptimizationVariablesOptions)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("optimization_type", "topology");
    tXMLMetaData.set(tOptimizationParameters);
    auto tOptimizerNode = tDocument.append_child("Optimizer");
    XMLGen::append_grad_based_optimizer_variables_options(tXMLMetaData, tOptimizerNode);

    // ****** TEST RESULTS AGAINST GOLD VALUES ******
    std::vector<std::string> tGoldKeys = {"OptimizationVariables"};
    std::vector<std::string> tGoldValues = {""};
    PlatoTestXMLGenerator::test_children(tGoldKeys, tGoldValues, tOptimizerNode);
    auto tNode = tOptimizerNode.child("OptimizationVariables");
    tGoldKeys = {"ValueName", "InitializationStage", "FilteredName", "LowerBoundValueName", "LowerBoundVectorName",
        "UpperBoundValueName", "UpperBoundVectorName", "SetLowerBoundsStage", "SetUpperBoundsStage"};
    tGoldValues = {"Control", "Initial Guess", "Topology", "Lower Bound Value", "Lower Bound Vector",
        "Upper Bound Value", "Upper Bound Vector", "Set Lower Bounds", "Set Upper Bounds"};
    PlatoTestXMLGenerator::test_children(tGoldKeys, tGoldValues, tNode);
}

TEST(PlatoTestXMLGenerator, AppendOptimizationObjectiveOptions)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;

    // Pose Criterion
    XMLGen::Criterion tCriterion;
    tCriterion.id("1");
    tXMLMetaData.append(tCriterion);

    // Pose Service	
    XMLGen::Service tService;
    tService.id("1");
    tService.code("plato_analyze");
    tXMLMetaData.append(tService);

    // Pose Service	
    XMLGen::Scenario tScenario;
    tScenario.id("1");
    tXMLMetaData.append(tScenario);

    XMLGen::Objective tObjective;
    tObjective.criteriaIDs.push_back("1");
    tObjective.serviceIDs.push_back("1");
    tObjective.scenarioIDs.push_back("1");

    tXMLMetaData.objective = tObjective;
    auto tOptimizerNode = tDocument.append_child("Optimizer");
    XMLGen::append_grad_based_optimizer_objective_options(tXMLMetaData, tOptimizerNode);
    
    // TEST RESULTS AGAINST GOLD VALUES
    std::vector<std::string> tGoldKeys = {"Objective"};
    std::vector<std::string> tGoldValues = {""};
    PlatoTestXMLGenerator::test_children(tGoldKeys, tGoldValues, tOptimizerNode);
    auto tObjectiveNode = tOptimizerNode.child("Objective");
    tGoldKeys = {"GradientName",
                 "GradientStageName",
                 "ValueName",
                 "ValueStageName"
                 };
    tGoldValues = {"Objective Gradient",
                   "Compute Objective Gradient",
                   "Objective Value",
                   "Compute Objective Value"
                   };
    PlatoTestXMLGenerator::test_children(tGoldKeys, tGoldValues, tObjectiveNode);
}

TEST(PlatoTestXMLGenerator, AppendOptimizationConstraintOptionsThrow)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion;
    tCriterion.id("1");
    tXMLMetaData.append(tCriterion);

    XMLGen::Service tService;
    tService.id("1");
    tXMLMetaData.append(tService);

    XMLGen::Constraint tConstraint;
    tConstraint.id("1");
    tConstraint.absoluteTarget("");  // EMPTY VALUE - IT WILL BE IGNORE
    tConstraint.relativeTarget("");
    tConstraint.divisor("1.0");
    tConstraint.criterion("1");
    tConstraint.service("1");

    tXMLMetaData.constraints.push_back(tConstraint);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("optimization_type", "topology");
    tOptimizationParameters.append("discretization","density");
    tXMLMetaData.set(tOptimizationParameters);

    auto tOptimizerNode = tDocument.append_child("Optimizer");
    ASSERT_THROW(XMLGen::append_grad_based_optimizer_constraint_options(tXMLMetaData, tOptimizerNode), std::runtime_error);

}

TEST(PlatoTestXMLGenerator, AppendOptimizationConstraintOptions)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion;
    tCriterion.id("1");
    tCriterion.type("volume");
    tXMLMetaData.append(tCriterion);

    XMLGen::Service tService;
    tService.id("1");
    tXMLMetaData.append(tService);

    XMLGen::Constraint tConstraint;
    tConstraint.id("1");
    tConstraint.absoluteTarget("");  // EMPTY VALUE - IT WILL BE IGNORE
    tConstraint.relativeTarget("");
    tConstraint.divisor("1.0");
    tConstraint.criterion("1");
    tConstraint.service("1");

    tXMLMetaData.constraints.push_back(tConstraint);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("optimization_type", "topology");
    tOptimizationParameters.append("discretization","density");
    tXMLMetaData.set(tOptimizationParameters);

    // TEST RESULTS AGAINST GOLD VALUES
    // CASE 1: ABSOLUTE TARGET VALUE IS DEFINED
    auto tOptimizerNode = tDocument.append_child("Optimizer");
    tXMLMetaData.constraints[0].absoluteTarget("1.0");
    XMLGen::append_grad_based_optimizer_constraint_options(tXMLMetaData, tOptimizerNode);
    
    std::vector<std::string> tGoldKeys = {"Constraint"};
    std::vector<std::string> tGoldValues = {""};
    PlatoTestXMLGenerator::test_children(tGoldKeys, tGoldValues, tOptimizerNode);
    auto tConstraintNode = tOptimizerNode.child("Constraint");
    tGoldKeys = {"AbsoluteTargetValue",
                 "GradientName",
                 "GradientStageName",
                 "ReferenceValue",
                 "ReferenceValueName", 
                 "ValueName", 
                 "ValueStageName"
                 };
    tGoldValues = {"1.0",
                   "Constraint Gradient 1", 
                   "Compute Constraint Gradient 1", 
                   "1.000000",
                   "Design Volume",
                   "Constraint Value 1", 
                   "Compute Constraint Value 1"
                   };
    PlatoTestXMLGenerator::test_children(tGoldKeys, tGoldValues, tConstraintNode);
}

TEST(PlatoTestXMLGenerator, AppendOptimizationBoundConstraintsOptions)
{
    pugi::xml_document tDocument;
    auto tOptimizerNode = tDocument.append_child("Optimizer");
    XMLGen::append_bound_constraints_options_topology_optimization({"1.0", "0.0"}, tOptimizerNode);

    // ****** TEST RESULTS AGAINST GOLD VALUES ******
    std::vector<std::string> tGoldKeys = {"BoundConstraint"};
    std::vector<std::string> tGoldValues = {""};
    PlatoTestXMLGenerator::test_children(tGoldKeys, tGoldValues, tOptimizerNode);
    auto tBoundConstraintNode = tOptimizerNode.child("BoundConstraint");
    tGoldKeys = {"Upper", "Lower"};
    tGoldValues = {"1.0", "0.0"};
    PlatoTestXMLGenerator::test_children(tGoldKeys, tGoldValues, tBoundConstraintNode);
}

TEST(PlatoTestXMLGenerator, WriteInterfaceXmlFile_ErrorEmptyObjective)
{
    XMLGen::InputData tXMLMetaData;
    ASSERT_THROW(XMLGen::write_interface_xml_file(tXMLMetaData), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, WriteInterfaceXmlFile_ErrorMultipleObjectives)
{
    XMLGen::InputData tXMLMetaData;
    XMLGen::Scenario tScenario;
    tScenario.id("1");
    tXMLMetaData.append(tScenario);
    tScenario.id("2");
    tXMLMetaData.append(tScenario);
    XMLGen::Objective tObjective1;
    tObjective1.scenarioIDs.push_back("1");
    tObjective1.scenarioIDs.push_back("2");
    tXMLMetaData.objective = tObjective1;
    ASSERT_THROW(XMLGen::write_interface_xml_file(tXMLMetaData), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendComputeConstraintGradientOperation)
{
    XMLGen::Constraint tConstraint;
    tConstraint.id("1");

    pugi::xml_document tDocument;

    ASSERT_NO_THROW(XMLGen::append_compute_constraint_gradient_operation(tConstraint, 
                        "plato_analyze_1", "criterion_1_service_2", tDocument));

    auto tNode = tDocument.child("Operation");
    ASSERT_FALSE(tNode.empty());
    ASSERT_STREQ("Operation", tNode.name());
    std::vector<std::string> tKeys = {"Name", "PerformerName", "Output"};
    std::vector<std::string> tValues = {"Compute Constraint Gradient 1", "plato_analyze_1", ""};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tNode);

    tNode = tNode.child("Output");
    ASSERT_FALSE(tNode.empty());
    ASSERT_STREQ("Output", tNode.name());
    tKeys = {"ArgumentName", "SharedDataName"};
    tValues = {"DGDX", "Criterion GradientX - criterion_1_service_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tNode);
}


}
// namespace PlatoTestXMLGenerator
