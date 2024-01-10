/*
 * XMLGeneratorPlatoAnalyzeInputFile_UnitTester.cpp
 *
 *  Created on: Jun 8, 2020
 */

#include <algorithm>
#include <gtest/gtest.h>

#include "XMLGenerator_UnitTester_Tools.hpp"

#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorValidInputKeys.hpp"
#include "XMLGeneratorPlatoAnalyzeUtilities.hpp"
#include "XMLGeneratorPlatoAnalyzeInputFileUtilities.hpp"
#include "XMLGeneratorAnalyzePhysicsFunctionInterface.hpp"
#include "XMLGeneratorAnalyzeLoadFunctionInterface.hpp"
#include "XMLGeneratorAnalyzeEssentialBCFunctionInterface.hpp"
#include "XMLGeneratorAnalyzeAssemblyFunctionInterface.hpp"
#include "XMLGeneratorAnalyzeLoadTagFunctionInterface.hpp"
#include "XMLGeneratorAnalyzeMaterialModelFunctionInterface.hpp"
#include "XMLGeneratorAnalyzeEssentialBCTagFunctionInterface.hpp"
#include "XMLGeneratorAnalyzeCriterionUtilities.hpp"

namespace PlatoTestXMLGenerator
{

TEST(PlatoTestXMLGenerator, AppendPhysics_IncompressibleFluids)
{
    std::vector<XMLGen::Output> tOutput;
    XMLGen::Scenario tScenario;
    tScenario.physics("steady_state_incompressible_fluids");
    tScenario.append("output_frequency","1");
    tScenario.append("discretization","density");
    tScenario.append("steady_state_iterations","500");
    tScenario.append("steady_state_tolerance","1e-5");
    tScenario.append("time_step_safety_factor","0.7");
    tScenario.append("heat_transfer","none");
    tScenario.append("momentum_damping","0.31");
    tScenario.append("linear_solver_tolerance","1e-12");
    tScenario.append("linear_solver_type","epetra");
    tScenario.append("linear_solver_max_iterations","1000");
    
    pugi::xml_document tDocument;
    XMLGen::AnalyzePhysicsFunctionInterface tPhysics;
    tPhysics.call(tScenario, tOutput, tDocument);
    //tDocument.save_file("dummy.xml");

    // TEST HYPERBOLIC BLOCK
    auto tHyperbolic = tDocument.child("ParameterList");
    ASSERT_FALSE(tHyperbolic.empty());
    ASSERT_STREQ("ParameterList", tHyperbolic.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Hyperbolic"}, tHyperbolic);

    auto tParameter = tHyperbolic.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    ASSERT_STREQ("Parameter", tParameter.name());
    std::vector<std::string> tKeys = {"name", "type", "value"};
    std::vector<std::string> tValues = {"Scenario", "string", "Density-Based Topology Optimization"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    ASSERT_STREQ("Parameter", tParameter.name());
    tValues = {"Heat Transfer", "string", "none"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());

    // TEST HYPERBOLIC -> MASS CONSERVATION BLOCK
    auto tConservationEqn = tHyperbolic.child("ParameterList");
    ASSERT_FALSE(tConservationEqn.empty());
    ASSERT_STREQ("ParameterList", tConservationEqn.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Mass Conservation"}, tConservationEqn);
    tParameter = tConservationEqn.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    ASSERT_STREQ("Parameter", tParameter.name());
    tValues = {"Surface Momentum Damping", "double", "0.31"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());
    tConservationEqn = tConservationEqn.next_sibling("ParameterList");
    ASSERT_TRUE(tConservationEqn.empty());

    // TEST TIME INTEGRATION BLOCK
    auto tTimeIntegration = tHyperbolic.next_sibling("ParameterList");
    ASSERT_FALSE(tTimeIntegration.empty());
    ASSERT_STREQ("ParameterList", tTimeIntegration.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Time Integration"}, tTimeIntegration);
    tParameter = tTimeIntegration.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    ASSERT_STREQ("Parameter", tParameter.name());
    tValues = {"Safety Factor", "double", "0.7"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());

    // TEST CONVERGENCE BLOCK
    auto tConvergence = tTimeIntegration.next_sibling("ParameterList");
    ASSERT_FALSE(tConvergence.empty());
    ASSERT_STREQ("ParameterList", tConvergence.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Convergence"}, tConvergence);
    tParameter = tConvergence.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    ASSERT_STREQ("Parameter", tParameter.name());
    tValues = {"Output Frequency", "int", "1"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    ASSERT_STREQ("Parameter", tParameter.name());
    tValues = {"Steady State Iterations", "int", "500"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    ASSERT_STREQ("Parameter", tParameter.name());
    tValues = {"Steady State Tolerance", "double", "1e-5"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());

    // TEST LINEAR SOLVER BLOCK
    auto tLinearSolver = tConvergence.next_sibling("ParameterList");
    ASSERT_FALSE(tLinearSolver.empty());
    ASSERT_STREQ("ParameterList", tLinearSolver.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Linear Solver"}, tLinearSolver);
    tParameter = tLinearSolver.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    ASSERT_STREQ("Parameter", tParameter.name());
    tValues = {"Iterations", "int", "1000"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    ASSERT_STREQ("Parameter", tParameter.name());
    tValues = {"Tolerance", "double", "1e-12"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    ASSERT_STREQ("Parameter", tParameter.name());
    tValues = {"Solver Stack", "string", "epetra"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());

    tLinearSolver = tLinearSolver.next_sibling("ParameterList");
    ASSERT_TRUE(tLinearSolver.empty());
}

TEST(PlatoTestXMLGenerator, AppendPhysicsMechanical)
{
    std::vector<XMLGen::Output> tOutputs;
    XMLGen::Output tOutput;
    tOutput.appendDeterminsiticQoI("stress", "element field");
    tOutput.appendDeterminsiticQoI("vonmises", "element field");
    tOutputs.push_back(tOutput);
    XMLGen::Scenario tScenario;
    tScenario.physics("steady_state_mechanics");
    tScenario.additiveContinuation("");
    tScenario.materialPenaltyExponent("3.0");
    tScenario.minErsatzMaterialConstant("1e-9");
    pugi::xml_document tDocument;
    XMLGen::AnalyzePhysicsFunctionInterface tPhysics;
    tPhysics.call(tScenario, tOutputs, tDocument);

    // TEST RESULTS
    auto tElliptic = tDocument.child("ParameterList");
    ASSERT_FALSE(tElliptic.empty());
    ASSERT_STREQ("ParameterList", tElliptic.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Elliptic"}, tElliptic);

    auto tParameter = tElliptic.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    ASSERT_STREQ("Parameter", tParameter.name());
    std::vector<std::string> tKeys = {"name", "type", "value"};
    std::vector<std::string> tValues = {"Plottable", "Array(string)", "{stress, Vonmises}"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());

    auto tPenaltyFunction = tElliptic.child("ParameterList");
    ASSERT_FALSE(tPenaltyFunction.empty());
    ASSERT_STREQ("ParameterList", tPenaltyFunction.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Penalty Function"}, tPenaltyFunction);
    tParameter = tPenaltyFunction.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Type", "string", "SIMP"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Exponent", "double", "3.0"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Minimum Value", "double", "1e-9"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());
}

TEST(PlatoTestXMLGenerator, AppendPhysicsThermal)
{
    std::vector<XMLGen::Output> tOutputs;
    XMLGen::Output tOutput;
    tOutput.appendDeterminsiticQoI("vonmises", "element field");
    tOutput.appendDeterminsiticQoI("temperature", "nodal field");
    tOutputs.push_back(tOutput);
    XMLGen::Scenario tScenario;
    tScenario.physics("steady_state_thermal");
    tScenario.additiveContinuation("");
    tScenario.materialPenaltyExponent("3.0");
    tScenario.minErsatzMaterialConstant("1e-9");
    pugi::xml_document tDocument;
    XMLGen::AnalyzePhysicsFunctionInterface tPhysics;
    tPhysics.call(tScenario, tOutputs, tDocument);

    // TEST RESULTS
    auto tElliptic = tDocument.child("ParameterList");
    ASSERT_FALSE(tElliptic.empty());
    ASSERT_STREQ("ParameterList", tElliptic.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Elliptic"}, tElliptic);

    auto tParameter = tElliptic.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    ASSERT_STREQ("Parameter", tParameter.name());
    std::vector<std::string> tKeys = {"name", "type", "value"};
    std::vector<std::string> tValues = {"Plottable", "Array(string)", "{Vonmises}"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());

    auto tPenaltyFunction = tElliptic.child("ParameterList");
    ASSERT_FALSE(tPenaltyFunction.empty());
    ASSERT_STREQ("ParameterList", tPenaltyFunction.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Penalty Function"}, tPenaltyFunction);
    tParameter = tPenaltyFunction.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Type", "string", "SIMP"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Exponent", "double", "3.0"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Minimum Value", "double", "1e-9"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());
}

TEST(PlatoTestXMLGenerator, AppendPhysicsThermoMechanical)
{
    std::vector<XMLGen::Output> tOutput;
    XMLGen::Scenario tScenario;
    tScenario.physics("steady_state_thermomechanics");
    tScenario.additiveContinuation("");
    tScenario.materialPenaltyExponent("3.0");
    tScenario.minErsatzMaterialConstant("1e-9");
    pugi::xml_document tDocument;
    XMLGen::AnalyzePhysicsFunctionInterface tPhysics;
    tPhysics.call(tScenario, tOutput, tDocument);

    // TEST RESULTS
    auto tElliptic = tDocument.child("ParameterList");
    ASSERT_FALSE(tElliptic.empty());
    ASSERT_STREQ("ParameterList", tElliptic.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Elliptic"}, tElliptic);
    auto tParameter = tElliptic.child("Parameter");
    ASSERT_TRUE(tParameter.empty());

    auto tPenaltyFunction = tElliptic.child("ParameterList");
    ASSERT_FALSE(tPenaltyFunction.empty());
    ASSERT_STREQ("ParameterList", tPenaltyFunction.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Penalty Function"}, tPenaltyFunction);
    tParameter = tPenaltyFunction.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    std::vector<std::string> tKeys = {"name", "type", "value"};
    std::vector<std::string> tValues = {"Type", "string", "SIMP"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Exponent", "double", "3.0"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Minimum Value", "double", "1e-9"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());
}

TEST(PlatoTestXMLGenerator, AppendPhysicsElectroMechanical)
{
    std::vector<XMLGen::Output> tOutput;
    XMLGen::Scenario tScenario;
    tScenario.physics("steady_state_electromechanics");
    tScenario.additiveContinuation("");
    tScenario.materialPenaltyExponent("3.0");
    tScenario.minErsatzMaterialConstant("1e-9");
    pugi::xml_document tDocument;
    XMLGen::AnalyzePhysicsFunctionInterface tPhysics;
    tPhysics.call(tScenario, tOutput, tDocument);

    // TEST RESULTS
    auto tElliptic = tDocument.child("ParameterList");
    ASSERT_FALSE(tElliptic.empty());
    ASSERT_STREQ("ParameterList", tElliptic.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Elliptic"}, tElliptic);
    auto tParameter = tElliptic.child("Parameter");
    ASSERT_TRUE(tParameter.empty());

    auto tPenaltyFunction = tElliptic.child("ParameterList");
    ASSERT_FALSE(tPenaltyFunction.empty());
    ASSERT_STREQ("ParameterList", tPenaltyFunction.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Penalty Function"}, tPenaltyFunction);
    tParameter = tPenaltyFunction.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    std::vector<std::string> tKeys = {"name", "type", "value"};
    std::vector<std::string> tValues = {"Type", "string", "SIMP"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Exponent", "double", "3.0"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Minimum Value", "double", "1e-9"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());
}

TEST(PlatoTestXMLGenerator, AppendPhysicsHeatConduction)
{
    std::vector<XMLGen::Output> tOutput;
    XMLGen::Scenario tScenario;
    tScenario.physics("transient_thermal");
    tScenario.timeStep("1.0");
    tScenario.numTimeSteps("10");
    tScenario.materialPenaltyExponent("3.0");
    tScenario.minErsatzMaterialConstant("1e-9");
    pugi::xml_document tDocument;
    XMLGen::AnalyzePhysicsFunctionInterface tPhysics;
    tPhysics.call(tScenario, tOutput, tDocument);

    // TEST RESULTS
    auto tParabolic = tDocument.child("ParameterList");
    ASSERT_FALSE(tParabolic.empty());
    ASSERT_STREQ("ParameterList", tParabolic.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Parabolic"}, tParabolic);
    auto tParameter = tParabolic.child("Parameter");
    ASSERT_TRUE(tParameter.empty());

    auto tPenaltyFunction = tParabolic.child("ParameterList");
    ASSERT_FALSE(tPenaltyFunction.empty());
    ASSERT_STREQ("ParameterList", tPenaltyFunction.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Penalty Function"}, tPenaltyFunction);
    tParameter = tPenaltyFunction.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    std::vector<std::string> tKeys = {"name", "type", "value"};
    std::vector<std::string> tValues = {"Type", "string", "SIMP"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Exponent", "double", "3.0"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Minimum Value", "double", "1e-9"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());

    auto tTimeStep = tParabolic.next_sibling("ParameterList");
    ASSERT_FALSE(tTimeStep.empty());
    ASSERT_STREQ("ParameterList", tTimeStep.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Time Integration"}, tTimeStep);
    tParameter = tTimeStep.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Number Time Steps", "int", "10"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Time Step", "double", "1.0"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());
}

TEST(PlatoTestXMLGenerator, AppendPhysicsTransientThermoMechanics)
{
    std::vector<XMLGen::Output> tOutput;
    XMLGen::Scenario tScenario;
    tScenario.physics("transient_thermomechanics");
    tScenario.timeStep("2.0");
    tScenario.numTimeSteps("20");
    tScenario.materialPenaltyExponent("3.0");
    tScenario.minErsatzMaterialConstant("1e-9");
    pugi::xml_document tDocument;
    XMLGen::AnalyzePhysicsFunctionInterface tPhysics;
    tPhysics.call(tScenario, tOutput, tDocument);

    // TEST RESULTS
    auto tParabolic = tDocument.child("ParameterList");
    ASSERT_FALSE(tParabolic.empty());
    ASSERT_STREQ("ParameterList", tParabolic.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Parabolic"}, tParabolic);
    auto tParameter = tParabolic.child("Parameter");
    ASSERT_TRUE(tParameter.empty());

    auto tPenaltyFunction = tParabolic.child("ParameterList");
    ASSERT_FALSE(tPenaltyFunction.empty());
    ASSERT_STREQ("ParameterList", tPenaltyFunction.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Penalty Function"}, tPenaltyFunction);
    tParameter = tPenaltyFunction.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    std::vector<std::string> tKeys = {"name", "type", "value"};
    std::vector<std::string> tValues = {"Type", "string", "SIMP"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Exponent", "double", "3.0"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Minimum Value", "double", "1e-9"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());

    auto tTimeStep = tParabolic.next_sibling("ParameterList");
    ASSERT_FALSE(tTimeStep.empty());
    ASSERT_STREQ("ParameterList", tTimeStep.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Time Integration"}, tTimeStep);
    tParameter = tTimeStep.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Number Time Steps", "int", "20"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Time Step", "double", "2.0"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());
}

TEST(PlatoTestXMLGenerator, AppendPhysicsTransientMechanics)
{
    std::vector<XMLGen::Output> tOutput;
    XMLGen::Scenario tScenario;
    tScenario.physics("transient_mechanics");
    tScenario.timeStep("2.0");
    tScenario.numTimeSteps("20");
    tScenario.newmarkBeta("0.25");
    tScenario.newmarkGamma("0.5");
    tScenario.materialPenaltyExponent("3.0");
    tScenario.minErsatzMaterialConstant("1e-9");
    pugi::xml_document tDocument;
    XMLGen::AnalyzePhysicsFunctionInterface tPhysics;
    tPhysics.call(tScenario, tOutput, tDocument);

    // TEST RESULTS
    auto tParabolic = tDocument.child("ParameterList");
    ASSERT_FALSE(tParabolic.empty());
    ASSERT_STREQ("ParameterList", tParabolic.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Hyperbolic"}, tParabolic);
    auto tParameter = tParabolic.child("Parameter");
    ASSERT_TRUE(tParameter.empty());

    auto tPenaltyFunction = tParabolic.child("ParameterList");
    ASSERT_FALSE(tPenaltyFunction.empty());
    ASSERT_STREQ("ParameterList", tPenaltyFunction.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Penalty Function"}, tPenaltyFunction);
    tParameter = tPenaltyFunction.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    std::vector<std::string> tKeys = {"name", "type", "value"};
    std::vector<std::string> tValues = {"Type", "string", "SIMP"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Exponent", "double", "3.0"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Minimum Value", "double", "1e-9"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());

    auto tTimeStep = tParabolic.next_sibling("ParameterList");
    ASSERT_FALSE(tTimeStep.empty());
    ASSERT_STREQ("ParameterList", tTimeStep.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Time Integration"}, tTimeStep);
    tParameter = tTimeStep.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Number Time Steps", "int", "20"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Time Step", "double", "2.0"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Newmark Beta", "double", "0.25"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Newmark Gamma", "double", "0.5"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());
}

TEST(PlatoTestXMLGenerator, AppendPhysicsStabilizedMechanics)
{
    std::vector<XMLGen::Output> tOutput;
    XMLGen::Scenario tScenario;
    tScenario.physics("stabilized_mechanics");
    tScenario.timeStep("2.0");
    tScenario.numTimeSteps("20");
    tScenario.materialPenaltyExponent("3.0");
    tScenario.minErsatzMaterialConstant("1e-9");
    pugi::xml_document tDocument;
    XMLGen::AnalyzePhysicsFunctionInterface tPhysics;
    tPhysics.call(tScenario, tOutput, tDocument);

    // TEST RESULTS
    auto tElliptic = tDocument.child("ParameterList");
    ASSERT_FALSE(tElliptic.empty());
    ASSERT_STREQ("ParameterList", tElliptic.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Elliptic"}, tElliptic);
    auto tParameter = tElliptic.child("Parameter");
    ASSERT_TRUE(tParameter.empty());

    auto tPenaltyFunction = tElliptic.child("ParameterList");
    ASSERT_FALSE(tPenaltyFunction.empty());
    ASSERT_STREQ("ParameterList", tPenaltyFunction.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Penalty Function"}, tPenaltyFunction);
    tParameter = tPenaltyFunction.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    std::vector<std::string> tKeys = {"name", "type", "value"};
    std::vector<std::string> tValues = {"Type", "string", "SIMP"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Exponent", "double", "3.0"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Minimum Value", "double", "1e-9"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());

    auto tTimeStep = tElliptic.next_sibling("ParameterList");
    ASSERT_FALSE(tTimeStep.empty());
    ASSERT_STREQ("ParameterList", tTimeStep.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Time Stepping"}, tTimeStep);
    tParameter = tTimeStep.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Number Time Steps", "int", "20"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Time Step", "double", "2.0"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());
}

TEST(PlatoTestXMLGenerator, AppendPhysicsPlasticity)
{
    std::vector<XMLGen::Output> tOutput;
    XMLGen::Scenario tScenario;
    tScenario.physics("plasticity");
    tScenario.timeStep("2.0");
    tScenario.numTimeSteps("20");
    tScenario.maxNumTimeSteps("40");
    tScenario.timeStepExpansion("1.1");
    tScenario.solverTolerance("1e-5");
    tScenario.solverMaxNumIterations("10");
    tScenario.materialPenaltyExponent("3.0");
    tScenario.minErsatzMaterialConstant("1e-9");
    tScenario.solverConvergenceCriterion("residual");
    tScenario.newtonSolverTolerance("1e-5");
    pugi::xml_document tDocument;
    XMLGen::AnalyzePhysicsFunctionInterface tPhysics;
    tPhysics.call(tScenario, tOutput, tDocument);
    //tDocument.save_file("dummy.txt");

    // TEST RESULTS
    // global residual
    auto tElliptic = tDocument.child("ParameterList");
    ASSERT_FALSE(tElliptic.empty());
    ASSERT_STREQ("ParameterList", tElliptic.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Elliptic"}, tElliptic);
    auto tParameter = tElliptic.child("Parameter");
    ASSERT_TRUE(tParameter.empty());

    auto tPenaltyFunction = tElliptic.child("ParameterList");
    ASSERT_FALSE(tPenaltyFunction.empty());
    ASSERT_STREQ("ParameterList", tPenaltyFunction.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Penalty Function"}, tPenaltyFunction);
    tParameter = tPenaltyFunction.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    std::vector<std::string> tKeys = {"name", "type", "value"};
    std::vector<std::string> tValues = {"Type", "string", "SIMP"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Exponent", "double", "3.0"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Minimum Value", "double", "1e-9"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());

    // projected gradient residual
    auto tStateGradProjection = tElliptic.next_sibling("ParameterList");
    ASSERT_FALSE(tStateGradProjection.empty());
    ASSERT_STREQ("ParameterList", tStateGradProjection.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"State Gradient Projection"}, tStateGradProjection);
    tPenaltyFunction = tStateGradProjection.child("ParameterList");
    ASSERT_FALSE(tPenaltyFunction.empty());
    ASSERT_STREQ("ParameterList", tPenaltyFunction.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Penalty Function"}, tPenaltyFunction);
    tParameter = tPenaltyFunction.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Type", "string", "SIMP"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Exponent", "double", "3.0"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Minimum Value", "double", "1e-9"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());

    auto tTimeStep = tStateGradProjection.next_sibling("ParameterList");
    ASSERT_FALSE(tTimeStep.empty());
    ASSERT_STREQ("ParameterList", tTimeStep.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Time Stepping"}, tTimeStep);
    tParameter = tTimeStep.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Expansion Multiplier", "double", "1.1"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Initial Num. Pseudo Time Steps", "int", "20"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Maximum Num. Pseudo Time Steps", "int", "40"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());

    auto tSolver = tTimeStep.next_sibling("ParameterList");
    ASSERT_FALSE(tSolver.empty());
    ASSERT_STREQ("ParameterList", tSolver.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Newton-Raphson"}, tSolver);
    tParameter = tSolver.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Stop Measure", "string", "residual"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Stopping Tolerance", "double", "1e-5"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_FALSE(tParameter.empty());
    tValues = {"Maximum Number Iterations", "int", "10"};
    PlatoTestXMLGenerator::test_attributes(tKeys, tValues, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());
}

TEST(PlatoTestXMLGenerator, ReturnObjectivesComputedByPlatoAnalyze)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion;
    tCriterion.type("internal elastic energy");
    tCriterion.id("1");
    tXMLMetaData.append(tCriterion);
    tCriterion.type("internal thermal energy");
    tCriterion.id("2");
    tXMLMetaData.append(tCriterion);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);
    tService.code("aria");
    tService.id("2");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("steady_state_mechanics");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);
    tScenario.physics("steady_state_thermal");
    tScenario.id("2");
    tXMLMetaData.append(tScenario);

    tXMLMetaData.objective.scenarioIDs.push_back("1");
    tXMLMetaData.objective.scenarioIDs.push_back("2");
    tXMLMetaData.objective.criteriaIDs.push_back("1");
    tXMLMetaData.objective.criteriaIDs.push_back("2");
    tXMLMetaData.objective.serviceIDs.push_back("1");
    tXMLMetaData.objective.serviceIDs.push_back("2");

    auto tCategories = XMLGen::return_objectives_computed_by_plato_analyze(tXMLMetaData);
    ASSERT_EQ(1u, tCategories.size());
    ASSERT_STREQ("internal elastic energy", tCategories[0].c_str());
}

TEST(PlatoTestXMLGenerator, ReturnConstraintsComputedByPlatoAnalyze)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion;
    tCriterion.type("internal elastic energy");
    tCriterion.id("1");
    tXMLMetaData.append(tCriterion);
    tCriterion.type("internal thermal energy");
    tCriterion.id("2");
    tXMLMetaData.append(tCriterion);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);
    tService.code("aria");
    tService.id("2");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("steady_state_mechanics");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);
    tScenario.physics("steady_state_thermal");
    tScenario.id("2");
    tXMLMetaData.append(tScenario);

    XMLGen::Constraint tConstraint;
    tConstraint.scenario("1"); 
    tConstraint.service("1"); 
    tConstraint.criterion("1"); 
    tXMLMetaData.constraints.push_back(tConstraint);
    tConstraint.scenario("2"); 
    tConstraint.service("2"); 
    tConstraint.criterion("2"); 
    tXMLMetaData.constraints.push_back(tConstraint);

    auto tCategories = XMLGen::return_constraints_computed_by_plato_analyze(tXMLMetaData);
    ASSERT_EQ(1u, tCategories.size());
    ASSERT_STREQ("internal elastic energy", tCategories[0].c_str());
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_ErrorEmptyAppName)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_THROW(tInterface.call("Displacement Boundary Condition with ID 1", "steady_state_mechanics", tBC, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_ErrorInvalidPhysics)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_THROW(tInterface.call("Displacement Boundary Condition with ID 1", "cfd", tBC, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_ErrorInvalidCategory)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("location_name", "ss_1");
    tBC.property("type", "pin");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_THROW(tInterface.call("Displacement Boundary Condition with ID 1", "steady_state_mechanics", tBC, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_CategoryZeroValue_ErrorInvalidPhysics)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("location_name", "ss_1");
    tBC.property("type", "zero_value");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_THROW(tInterface.call("Thermal Boundary Condition with ID 1", "cfd", tBC, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_CategoryZeroValue_ErrorEmptyDof)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("location_name", "ss_1");
    tBC.property("type", "zero_value");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_THROW(tInterface.call("Thermal Boundary Condition with ID 1", "steady_state_thermal", tBC, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_CategoryZeroValue_ErrorInvalidDof)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("location_name", "ss_1");
    tBC.property("degree_of_freedom", "dispx");
    tBC.property("type", "zero_value");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_THROW(tInterface.call("Thermal Boundary Condition with ID 1", "steady_state_thermal", tBC, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_CategoryRigid)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("location_name", "ss_1");
    tBC.property("type", "rigid");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_NO_THROW(tInterface.call("Displacement Boundary Condition with ID 1", "steady_state_mechanics", tBC, tDocument));

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Zero Value"}, {"Index", "int", "2"}, {"Sides", "string", "ss_1"},
          {"Type", "string", "Zero Value"}, {"Index", "int", "1"}, {"Sides", "string", "ss_1"},
          {"Type", "string", "Zero Value"}, {"Index", "int", "0"}, {"Sides", "string", "ss_1"} };
    std::vector<std::string> tGoldParameterListNames =
        {"Displacement Boundary Condition with ID 1 applied to Dof with tag DISPZ",
         "Displacement Boundary Condition with ID 1 applied to Dof with tag DISPY",
         "Displacement Boundary Condition with ID 1 applied to Dof with tag DISPX"};

    auto tParamList = tDocument.child("ParameterList");
    auto tGoldValuesItr = tGoldValues.begin();
    auto tGoldParameterListNamesItr = tGoldParameterListNames.begin();
    while(!tParamList.empty())
    {
        ASSERT_FALSE(tParamList.empty());
        ASSERT_STREQ("ParameterList", tParamList.name());
        PlatoTestXMLGenerator::test_attributes({"name"}, {tGoldParameterListNamesItr->c_str()}, tParamList);

        auto tParameter = tParamList.child("Parameter");
        while(!tParameter.empty())
        {
            ASSERT_FALSE(tParameter.empty());
            ASSERT_STREQ("Parameter", tParameter.name());
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
            tParameter = tParameter.next_sibling();
            std::advance(tGoldValuesItr, 1);
        }
        tParamList = tParamList.next_sibling();
        std::advance(tGoldParameterListNamesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_CategoryFixed)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("type", "fixed");
    tBC.property("location_name", "ns_1");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_NO_THROW(tInterface.call("Displacement Boundary Condition with ID 1", "steady_state_mechanics", tBC, tDocument));

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Zero Value"}, {"Index", "int", "2"}, {"Sides", "string", "ns_1"},
          {"Type", "string", "Zero Value"}, {"Index", "int", "1"}, {"Sides", "string", "ns_1"},
          {"Type", "string", "Zero Value"}, {"Index", "int", "0"}, {"Sides", "string", "ns_1"} };
    std::vector<std::string> tGoldParameterListNames =
        {"Displacement Boundary Condition with ID 1 applied to Dof with tag DISPZ",
         "Displacement Boundary Condition with ID 1 applied to Dof with tag DISPY",
         "Displacement Boundary Condition with ID 1 applied to Dof with tag DISPX"};

    auto tParamList = tDocument.child("ParameterList");
    auto tGoldValuesItr = tGoldValues.begin();
    auto tGoldParameterListNamesItr = tGoldParameterListNames.begin();
    while(!tParamList.empty())
    {
        ASSERT_FALSE(tParamList.empty());
        ASSERT_STREQ("ParameterList", tParamList.name());
        PlatoTestXMLGenerator::test_attributes({"name"}, {tGoldParameterListNamesItr->c_str()}, tParamList);

        auto tParameter = tParamList.child("Parameter");
        while(!tParameter.empty())
        {
            ASSERT_FALSE(tParameter.empty());
            ASSERT_STREQ("Parameter", tParameter.name());
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
            tParameter = tParameter.next_sibling();
            std::advance(tGoldValuesItr, 1);
        }
        tParamList = tParamList.next_sibling();
        std::advance(tGoldParameterListNamesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_CategoryZeroValue)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("degree_of_freedom", "temp");
    tBC.property("location_name", "ss_2");
    tBC.property("type", "zero_value");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_NO_THROW(tInterface.call("Thermal Boundary Condition with ID 1", "steady_state_thermal", tBC, tDocument));

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Zero Value"}, {"Index", "int", "0"}, {"Sides", "string", "ss_2"} };

    auto tParamList = tDocument.child("ParameterList");
    ASSERT_FALSE(tParamList.empty());
    ASSERT_STREQ("ParameterList", tParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Thermal Boundary Condition with ID 1 applied to Dof with tag TEMP"}, tParamList);

    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tParamList.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_CategoryTimeFunction)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("degree_of_freedom", "temp");
    tBC.property("location_name", "ss_2");
    tBC.property("type", "time_function");
    tBC.property("value", "100*t+300");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_NO_THROW(tInterface.call("Thermal Boundary Condition with ID 1", "thermoplasticity", tBC, tDocument));

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Time Dependent"}, {"Index", "int", "3"}, {"Sides", "string", "ss_2"}, {"Function", "string", "100*t+300"}};

    auto tParamList = tDocument.child("ParameterList");
    ASSERT_FALSE(tParamList.empty());
    ASSERT_STREQ("ParameterList", tParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Thermal Boundary Condition with ID 1 applied to Dof with tag TEMP"}, tParamList);

    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tParamList.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_CategoryFixedValue_ErrorInvalidPhysics)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("location_name", "ss_1");
    tBC.property("type", "fixed_value");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_THROW(tInterface.call("Thermal Boundary Condition with ID 1", "cfd", tBC, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_CategoryFixedValue_ErrorEmptyDof)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("location_name", "ss_1");
    tBC.property("type", "fixed_value");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_THROW(tInterface.call("Thermal Boundary Condition with ID 1", "steady_state_thermal", tBC, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_CategoryFixedValue_ErrorInvalidDof)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("degree_of_freedom", "dispx");
    tBC.property("location_name", "ss_1");
    tBC.property("type", "fixed_value");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_THROW(tInterface.call("Thermal Boundary Condition with ID 1", "steady_state_thermal", tBC, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_CategoryFixedValue_ErrorEmptyValue)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("degree_of_freedom", "temp");
    tBC.property("location_name", "ss_1");
    tBC.property("type", "fixed_value");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_THROW(tInterface.call("Thermal Boundary Condition with ID 1", "steady_state_thermal", tBC, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_CategoryFixedValue)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("degree_of_freedom", "temp");
    tBC.property("value", "10.0");
    tBC.property("location_name", "ss_2");
    tBC.property("type", "fixed_value");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_NO_THROW(tInterface.call("Thermal Boundary Condition with ID 1", "steady_state_thermal", tBC, tDocument));

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Fixed Value"}, {"Index", "int", "0"}, {"Sides", "string", "ss_2"}, {"Value", "double", "10.0"} };

    auto tParamList = tDocument.child("ParameterList");
    ASSERT_FALSE(tParamList.empty());
    ASSERT_STREQ("ParameterList", tParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Thermal Boundary Condition with ID 1 applied to Dof with tag TEMP"}, tParamList);

    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tParamList.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_CategoryInsulated_ErrorInvalidPhysics)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("degree_of_freedom", "temp");
    tBC.property("location_name", "ss_1");
    tBC.property("type", "insulated");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_THROW(tInterface.call("Thermal Boundary Condition with ID 1", "cfd", tBC, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_CategoryInsulated_ErrorEmptyDof)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("location_name", "ss_1");
    tBC.property("type", "insulated");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_THROW(tInterface.call("Thermal Boundary Condition with ID 1", "steady_state_thermal", tBC, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_CategoryInsulated_ErrorInvalidDof)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("degree_of_freedom", "dispx");
    tBC.property("location_name", "ss_1");
    tBC.property("type", "insulated");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_THROW(tInterface.call("Thermal Boundary Condition with ID 1", "steady_state_thermal", tBC, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryCondition_CategoryInsulated)
{
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("degree_of_freedom", "temp");
    tBC.property("location_name", "ss_11");
    tBC.property("type", "insulated");
    pugi::xml_document tDocument;

    XMLGen::AppendEssentialBoundaryCondition tInterface;
    ASSERT_NO_THROW(tInterface.call("Thermal Boundary Condition with ID 1", "steady_state_thermal", tBC, tDocument));

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Zero Value"}, {"Index", "int", "0"}, {"Sides", "string", "ss_11"} };

    auto tParamList = tDocument.child("ParameterList");
    ASSERT_FALSE(tParamList.empty());
    ASSERT_STREQ("ParameterList", tParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Thermal Boundary Condition with ID 1 applied to Dof with tag TEMP"}, tParamList);

    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tParamList.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryConditionsToPlatoAnalyzeInputDeck_CFD)
{
    // POSE PROBLEM
    XMLGen::InputData tXMLMetaData;
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("location_name", "no_slip");
    tBC.property("type", "zero_value");
    tBC.property("degree_of_freedom", "velx");
    tBC.property("value", "0");
    tXMLMetaData.ebcs.push_back(tBC);
    tBC.property("id", "2");
    tBC.property("location_name", "no_slip");
    tBC.property("type", "zero_value");
    tBC.property("degree_of_freedom", "vely");
    tBC.property("value", "0");
    tXMLMetaData.ebcs.push_back(tBC);
    tBC.property("id", "3");
    tBC.property("location_name", "no_slip");
    tBC.property("type", "zero_value");
    tBC.property("degree_of_freedom", "velz");
    tBC.property("value", "0");
    tXMLMetaData.ebcs.push_back(tBC);
    tBC.property("id", "4");
    tBC.property("location_name", "inlet");
    tBC.property("type", "fixed_value");
    tBC.property("degree_of_freedom", "velx");
    tBC.property("value", "1.5");
    tXMLMetaData.ebcs.push_back(tBC);
    tBC.property("id", "5");
    tBC.property("location_name", "inlet");
    tBC.property("type", "fixed_value");
    tBC.property("degree_of_freedom", "vely");
    tBC.property("value", "0");
    tXMLMetaData.ebcs.push_back(tBC);
    tBC.property("id", "6");
    tBC.property("location_name", "inlet");
    tBC.property("type", "fixed_value");
    tBC.property("degree_of_freedom", "velz");
    tBC.property("value", "0");
    tXMLMetaData.ebcs.push_back(tBC);
    tBC.property("id", "7");
    tBC.property("location_name", "outlet");
    tBC.property("type", "fixed_value");
    tBC.property("degree_of_freedom", "press");
    tBC.property("value", "1");
    tXMLMetaData.ebcs.push_back(tBC);
    XMLGen::Scenario tScenario;
    tScenario.id("1");
    tScenario.physics("steady_state_incompressible_fluids");
    std::vector<std::string> bcIDs = {{"1"},{"2"},{"3"},{"4"},{"5"},{"6"},{"7"}};
    tScenario.setBCIDs(bcIDs);
    tXMLMetaData.append(tScenario);
    std::vector<XMLGen::Scenario> tScenarioList;
    tScenarioList.push_back(tScenario);
    tXMLMetaData.objective.scenarioIDs.push_back("1");

    pugi::xml_document tDocument;
    XMLGen::append_essential_boundary_conditions_to_plato_analyze_input_deck(tXMLMetaData, tDocument);
    //tDocument.save_file("dummy.xml");

    // TEST PRESSURE BCS
    auto tEssentialBC = tDocument.child("ParameterList");
    ASSERT_FALSE(tEssentialBC.empty());
    ASSERT_STREQ("ParameterList", tEssentialBC.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Pressure Essential Boundary Conditions"}, tEssentialBC);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Fixed Value"}, {"Index", "int", "0"}, {"Sides", "string", "outlet"}, {"Value", "double", "1"} };
    std::vector<std::string> tGoldParameterListNames = 
        {"Pressure Boundary Condition with ID 7 applied to Dof with tag PRESS"};

    auto tParamList = tEssentialBC.child("ParameterList");
    auto tGoldValuesItr = tGoldValues.begin();
    auto tGoldParameterListNamesItr = tGoldParameterListNames.begin();
    while(!tParamList.empty())
    {
        ASSERT_FALSE(tParamList.empty());
        ASSERT_STREQ("ParameterList", tParamList.name());
        PlatoTestXMLGenerator::test_attributes({"name"}, {tGoldParameterListNamesItr->c_str()}, tParamList);

        auto tParameter = tParamList.child("Parameter");
        while(!tParameter.empty())
        {
            ASSERT_FALSE(tParameter.empty());
            ASSERT_STREQ("Parameter", tParameter.name());
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
            tParameter = tParameter.next_sibling();
            std::advance(tGoldValuesItr, 1);
        }
        tParamList = tParamList.next_sibling();
        std::advance(tGoldParameterListNamesItr, 1);
    }

    // TEST VELOCITY BCS
    tEssentialBC = tEssentialBC.next_sibling("ParameterList");
    ASSERT_FALSE(tEssentialBC.empty());
    ASSERT_STREQ("ParameterList", tEssentialBC.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Velocity Essential Boundary Conditions"}, tEssentialBC);

    tGoldValues =
        { {"Type", "string", "Zero Value"}, {"Index", "int", "0"}, {"Sides", "string", "no_slip"}, 
          {"Type", "string", "Zero Value"}, {"Index", "int", "1"}, {"Sides", "string", "no_slip"}, 
          {"Type", "string", "Zero Value"}, {"Index", "int", "2"}, {"Sides", "string", "no_slip"}, 
          {"Type", "string", "Fixed Value"}, {"Index", "int", "0"}, {"Sides", "string", "inlet"}, {"Value", "double", "1.5"},
          {"Type", "string", "Fixed Value"}, {"Index", "int", "1"}, {"Sides", "string", "inlet"}, {"Value", "double", "0"},
          {"Type", "string", "Fixed Value"}, {"Index", "int", "2"}, {"Sides", "string", "inlet"}, {"Value", "double", "0"} };
    tGoldParameterListNames = 
        {"Velocity Boundary Condition with ID 1 applied to Dof with tag VELX",
         "Velocity Boundary Condition with ID 2 applied to Dof with tag VELY",
         "Velocity Boundary Condition with ID 3 applied to Dof with tag VELZ",
         "Velocity Boundary Condition with ID 4 applied to Dof with tag VELX",
         "Velocity Boundary Condition with ID 5 applied to Dof with tag VELY",
         "Velocity Boundary Condition with ID 6 applied to Dof with tag VELZ"};

    tParamList = tEssentialBC.child("ParameterList");
    tGoldValuesItr = tGoldValues.begin();
    tGoldParameterListNamesItr = tGoldParameterListNames.begin();
    while(!tParamList.empty())
    {
        ASSERT_FALSE(tParamList.empty());
        ASSERT_STREQ("ParameterList", tParamList.name());
        PlatoTestXMLGenerator::test_attributes({"name"}, {tGoldParameterListNamesItr->c_str()}, tParamList);

        auto tParameter = tParamList.child("Parameter");
        while(!tParameter.empty())
        {
            ASSERT_FALSE(tParameter.empty());
            ASSERT_STREQ("Parameter", tParameter.name());
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
            tParameter = tParameter.next_sibling();
            std::advance(tGoldValuesItr, 1);
        }
        tParamList = tParamList.next_sibling();
        std::advance(tGoldParameterListNamesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryConditionsToPlatoAnalyzeInputDeck_Error_UnsuportedDOF)
{
    // POSE PROBLEM
    XMLGen::InputData tXMLMetaData;
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("location_name", "ss_1");
    tBC.property("type", "zero_value");
    tBC.property("degree_of_freedom", "dispz");
    tBC.property("value", "0");
    tXMLMetaData.ebcs.push_back(tBC);
    tBC.property("id", "2");
    tBC.property("location_name", "ss_1");
    tBC.property("type", "zero_value");
    tBC.property("degree_of_freedom", "dispy");
    tBC.property("value", "0");
    tXMLMetaData.ebcs.push_back(tBC);
    tBC.property("id", "3");
    tBC.property("location_name", "ss_1");
    tBC.property("type", "zero_value");
    tBC.property("degree_of_freedom", "velx");
    tBC.property("value", "0");
    tXMLMetaData.ebcs.push_back(tBC);
    XMLGen::Scenario tScenario;
    tScenario.id("1");
    tScenario.physics("steady_state_mechanics");
    std::vector<std::string> bcIDs = {{"1"},{"2"},{"3"}};
    tScenario.setBCIDs(bcIDs);
    tXMLMetaData.append(tScenario);
    tXMLMetaData.objective.scenarioIDs.push_back("1");

    // CALL FUNCTION
    pugi::xml_document tDocument;
    ASSERT_THROW(XMLGen::append_essential_boundary_conditions_to_plato_analyze_input_deck(tXMLMetaData, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendEssentialBoundaryConditionsToPlatoAnalyzeInputDeck_Mechanics)
{
    // POSE PROBLEM
    XMLGen::InputData tXMLMetaData;
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("id", "1");
    tBC.property("location_name", "ss_1");
    tBC.property("type", "zero_value");
    tBC.property("degree_of_freedom", "dispz");
    tBC.property("value", "0");
    tXMLMetaData.ebcs.push_back(tBC);
    tBC.property("id", "2");
    tBC.property("location_name", "ss_1");
    tBC.property("type", "zero_value");
    tBC.property("degree_of_freedom", "dispy");
    tBC.property("value", "0");
    tXMLMetaData.ebcs.push_back(tBC);
    tBC.property("id", "3");
    tBC.property("location_name", "ss_1");
    tBC.property("type", "zero_value");
    tBC.property("degree_of_freedom", "dispx");
    tBC.property("value", "0");
    tXMLMetaData.ebcs.push_back(tBC);
    XMLGen::Scenario tScenario;
    tScenario.id("1");
    tScenario.physics("steady_state_mechanics");
    std::vector<std::string> bcIDs = {{"1"},{"2"},{"3"}};
    tScenario.setBCIDs(bcIDs);
    tXMLMetaData.append(tScenario);
    tXMLMetaData.objective.scenarioIDs.push_back("1");

    // CALL FUNCTION
    pugi::xml_document tDocument;
    XMLGen::append_essential_boundary_conditions_to_plato_analyze_input_deck(tXMLMetaData, tDocument);

    // TEST
    auto tEssentialBC = tDocument.child("ParameterList");
    ASSERT_FALSE(tEssentialBC.empty());
    ASSERT_STREQ("ParameterList", tEssentialBC.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Essential Boundary Conditions"}, tEssentialBC);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Zero Value"}, {"Index", "int", "2"}, {"Sides", "string", "ss_1"},
          {"Type", "string", "Zero Value"}, {"Index", "int", "1"}, {"Sides", "string", "ss_1"},
          {"Type", "string", "Zero Value"}, {"Index", "int", "0"}, {"Sides", "string", "ss_1"} };
    std::vector<std::string> tGoldParameterListNames =
        {"Displacement Boundary Condition with ID 1 applied to Dof with tag DISPZ",
         "Displacement Boundary Condition with ID 2 applied to Dof with tag DISPY",
         "Displacement Boundary Condition with ID 3 applied to Dof with tag DISPX"};

    auto tParamList = tEssentialBC.child("ParameterList");
    auto tGoldValuesItr = tGoldValues.begin();
    auto tGoldParameterListNamesItr = tGoldParameterListNames.begin();
    while(!tParamList.empty())
    {
        ASSERT_FALSE(tParamList.empty());
        ASSERT_STREQ("ParameterList", tParamList.name());
        PlatoTestXMLGenerator::test_attributes({"name"}, {tGoldParameterListNamesItr->c_str()}, tParamList);

        auto tParameter = tParamList.child("Parameter");
        while(!tParameter.empty())
        {
            ASSERT_FALSE(tParameter.empty());
            ASSERT_STREQ("Parameter", tParameter.name());
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
            tParameter = tParameter.next_sibling();
            std::advance(tGoldValuesItr, 1);
        }
        tParamList = tParamList.next_sibling();
        std::advance(tGoldParameterListNamesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, EssentialBoundaryConditionTag_InvalidTag)
{
    XMLGen::EssentialBoundaryConditionTag tInterface;
    XMLGen::EssentialBoundaryCondition tBC;
    tBC.property("type", "fluid velocity");
    ASSERT_THROW(tInterface.call(tBC), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, EssentialBoundaryConditionTag)
{
    XMLGen::EssentialBoundaryConditionTag tInterface;
    XMLGen::EssentialBoundaryCondition tBC;

    // TEST 1
    tBC.property("type", "fixed_value");
    tBC.property("id", "1");
    tBC.property("degree_of_freedom", "dispx");
    auto tName = tInterface.call(tBC);
    ASSERT_STREQ("Displacement Boundary Condition with ID 1", tName.c_str());

    // TEST 2
    tBC.property("type", "fixed_value");
    tBC.property("id", "1");
    tBC.property("degree_of_freedom", "temp");
    tName = tInterface.call(tBC);
    ASSERT_STREQ("Temperature Boundary Condition with ID 1", tName.c_str());

    // TEST 3
    tBC.property("type", "fixed_value");
    tBC.property("id", "1");
    tBC.property("degree_of_freedom", "potential");
    tName = tInterface.call(tBC);
    tName = tInterface.call(tBC);
    ASSERT_STREQ("Potential Boundary Condition with ID 1", tName.c_str());

    // TEST 4
    tBC.property("type", "fixed_value");
    tBC.property("id", "1");
    tBC.property("degree_of_freedom", "velocity");
    tName = tInterface.call(tBC);
    ASSERT_STREQ("Velocity Boundary Condition with ID 1", tName.c_str());
}

TEST(PlatoTestXMLGenerator, AppendThermalSourceToPlatoAnalyzeInputDeck)
{
    // set input data
    XMLGen::InputData tXMLMetaData;
    tXMLMetaData.objective.scenarioIDs.push_back("1");

    XMLGen::Load tLoad1;
    tLoad1.type("uniform_thermal_source");
    tLoad1.id("1");
    tLoad1.location_name("block_1");
    std::vector<std::string> tValues = {"2.0"};
    tLoad1.load_values(tValues);
    tXMLMetaData.loads.push_back(tLoad1);

    XMLGen::Load tLoad2;
    tLoad2.type("uniform_thermal_source");
    tLoad2.id("2");
    tLoad2.location_name("block_2");
    std::vector<std::string> tValues2 = {"4.0"};
    tLoad2.load_values(tValues2);
    tXMLMetaData.loads.push_back(tLoad2);

    XMLGen::Scenario tScenario;
    tScenario.id("1");
    tScenario.physics("steady_state_incompressible_fluids");
    std::vector<std::string> tLoadIDs = {"1","2"};
    tScenario.setLoadIDs(tLoadIDs);
    tXMLMetaData.append(tScenario);

    // run test
    pugi::xml_document tDocument;
    XMLGen::append_loads_to_plato_analyze_input_deck(tXMLMetaData, tDocument);
    //tDocument.save_file("dummy.xml");

    // test results
    auto tParameterList = tDocument.child("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    ASSERT_STREQ("ParameterList", tParameterList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Momentum Natural Boundary Conditions"}, tParameterList);
    
    tParameterList = tParameterList.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    ASSERT_STREQ("ParameterList", tParameterList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Thermal Natural Boundary Conditions"}, tParameterList);
    
    tParameterList = tParameterList.next_sibling("ParameterList");
    ASSERT_FALSE(tParameterList.empty());
    ASSERT_STREQ("ParameterList", tParameterList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Thermal Sources"}, tParameterList);

    // load 1
    auto tSourceParamList = tParameterList.child("ParameterList");
    ASSERT_FALSE(tSourceParamList.empty());
    ASSERT_STREQ("ParameterList", tParameterList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Uniform Thermal Source with ID 1"}, tSourceParamList);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Uniform"}, {"Value", "double", "2.0"}, {"Domains", "Array(string)", "{block_1}"} };
    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tSourceParamList.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }

    // load 2
    tSourceParamList = tSourceParamList.next_sibling("ParameterList");
    ASSERT_FALSE(tSourceParamList.empty());
    ASSERT_STREQ("ParameterList", tParameterList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Uniform Thermal Source with ID 2"}, tSourceParamList);

    tGoldValues = { {"Type", "string", "Uniform"}, {"Value", "double", "4.0"}, {"Domains", "Array(string)", "{block_2}"} };
    tGoldValuesItr = tGoldValues.begin();
    tParameter = tSourceParamList.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }

    tSourceParamList = tSourceParamList.next_sibling("ParameterList");
    ASSERT_TRUE(tSourceParamList.empty());
}

TEST(PlatoTestXMLGenerator, AppendNaturalBoundaryConditionsToPlatoAnalyzeInputDeck)
{
    XMLGen::InputData tXMLMetaData;

    tXMLMetaData.objective.scenarioIDs.push_back("1");

    XMLGen::Load tLoad;
    tLoad.type("traction");
    tLoad.id("1");
    tLoad.location_name("ss_1");
    std::vector<std::string> tValues = {"1.0", "2.0", "3.0"};
    tLoad.load_values(tValues);
    tXMLMetaData.loads.push_back(tLoad);

    XMLGen::Scenario tScenario;
    tScenario.id("1");
    tScenario.physics("steady_state_mechanics");
    std::vector<std::string> tLoadIDs = {"1"};
    tScenario.setLoadIDs(tLoadIDs);
    tXMLMetaData.append(tScenario);

    pugi::xml_document tDocument;
    XMLGen::append_loads_to_plato_analyze_input_deck(tXMLMetaData, tDocument);

    auto tLoadParamList = tDocument.child("ParameterList");
    ASSERT_FALSE(tLoadParamList.empty());
    ASSERT_STREQ("ParameterList", tLoadParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Natural Boundary Conditions"}, tLoadParamList);

    auto tTraction = tLoadParamList.child("ParameterList");
    ASSERT_FALSE(tTraction.empty());
    ASSERT_STREQ("ParameterList", tTraction.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Traction Vector Boundary Condition with ID 1"}, tTraction);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Uniform"}, {"Values", "Array(string)", "{1.0, 2.0, 3.0}"}, {"Sides", "string", "ss_1"} };
    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tLoadParamList.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendPlasticityNaturalBoundaryConditionsToPlatoAnalyzeInputDeck)
{
    XMLGen::InputData tXMLMetaData;

    tXMLMetaData.objective.scenarioIDs.push_back("1");

    XMLGen::Load tLoad;
    tLoad.type("traction");
    tLoad.id("1");
    tLoad.location_name("ss_1");
    std::vector<std::string> tValues = {"1.0", "2.0", "3.0"};
    tLoad.load_values(tValues);
    tXMLMetaData.loads.push_back(tLoad);

    XMLGen::Scenario tScenario;
    tScenario.id("1");
    tScenario.physics("plasticity");
    std::vector<std::string> tLoadIDs = {"1"};
    tScenario.setLoadIDs(tLoadIDs);
    tXMLMetaData.append(tScenario);

    pugi::xml_document tDocument;
    XMLGen::append_loads_to_plato_analyze_input_deck(tXMLMetaData, tDocument);

    auto tLoadParamList = tDocument.child("ParameterList");
    ASSERT_FALSE(tLoadParamList.empty());
    ASSERT_STREQ("ParameterList", tLoadParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Natural Boundary Conditions"}, tLoadParamList);

    auto tMechanicalLoadParamList = tLoadParamList.child("ParameterList");
    ASSERT_FALSE(tMechanicalLoadParamList.empty());
    ASSERT_STREQ("ParameterList", tMechanicalLoadParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Mechanical Natural Boundary Conditions"}, tMechanicalLoadParamList);

    auto tTraction = tMechanicalLoadParamList.child("ParameterList");
    ASSERT_FALSE(tTraction.empty());
    ASSERT_STREQ("ParameterList", tTraction.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Traction Vector Boundary Condition with ID 1"}, tTraction);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Uniform"}, {"Values", "Array(string)", "{1.0, 2.0, 3.0}"}, {"Sides", "string", "ss_1"} };
    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tLoadParamList.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendThermoplasticityNaturalBoundaryConditionsToPlatoAnalyzeInputDeck)
{
    XMLGen::InputData tXMLMetaData;

    tXMLMetaData.objective.scenarioIDs.push_back("1");

    XMLGen::Load tLoad;
    tLoad.type("traction");
    tLoad.id("1");
    tLoad.location_name("ss_1");
    std::vector<std::string> tValues = {"1.0*t", "2.0*t", "3.0*t"};
    tLoad.load_values(tValues);
    tXMLMetaData.loads.push_back(tLoad);

    XMLGen::Load tLoad2;
    tLoad2.type("uniform_surface_flux");
    tLoad2.id("2");
    tLoad2.location_name("ss_2");
    std::vector<std::string> tValues2 = {"10.0*t"};
    tLoad2.load_values(tValues2);
    tXMLMetaData.loads.push_back(tLoad2);

    XMLGen::Scenario tScenario;
    tScenario.id("1");
    tScenario.physics("thermoplasticity");
    std::vector<std::string> tLoadIDs = {"1","2"};
    tScenario.setLoadIDs(tLoadIDs);
    tXMLMetaData.append(tScenario);

    pugi::xml_document tDocument;
    XMLGen::append_loads_to_plato_analyze_input_deck(tXMLMetaData, tDocument);

    auto tLoadParamList = tDocument.child("ParameterList");
    ASSERT_FALSE(tLoadParamList.empty());
    ASSERT_STREQ("ParameterList", tLoadParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Natural Boundary Conditions"}, tLoadParamList);

    auto tMechanicalLoadParamList = tLoadParamList.child("ParameterList");
    ASSERT_FALSE(tMechanicalLoadParamList.empty());
    ASSERT_STREQ("ParameterList", tMechanicalLoadParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Mechanical Natural Boundary Conditions"}, tMechanicalLoadParamList);

    auto tThermalLoadParamList = tMechanicalLoadParamList.next_sibling("ParameterList");
    ASSERT_FALSE(tThermalLoadParamList.empty());
    ASSERT_STREQ("ParameterList", tThermalLoadParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Thermal Natural Boundary Conditions"}, tThermalLoadParamList);

    auto tTraction = tMechanicalLoadParamList.child("ParameterList");
    ASSERT_FALSE(tTraction.empty());
    ASSERT_STREQ("ParameterList", tTraction.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Traction Vector Boundary Condition with ID 1"}, tTraction);

    auto tHeatFlux = tThermalLoadParamList.child("ParameterList");
    ASSERT_FALSE(tHeatFlux.empty());
    ASSERT_STREQ("ParameterList", tHeatFlux.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Uniform Surface Flux Boundary Condition with ID 2"}, tHeatFlux);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Uniform"}, {"Values", "Array(string)", "{1.0*t,2.0*t,3.0*t}"}, {"Sides", "string", "ss_1"} };
    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tLoadParamList.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }

    tGoldKeys = {"name", "type", "value"};
    tGoldValues =
        { {"Type", "string", "Uniform"}, {"Value", "string", "10.0*t"}, {"Sides", "string", "ss_2"} };
    tGoldValuesItr = tGoldValues.begin();
    tParameter = tHeatFlux.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendNaturalBoundaryConditionsToPlatoAnalyzeInputDeck_pressure)
{
    XMLGen::InputData tXMLMetaData;

    tXMLMetaData.objective.scenarioIDs.push_back("1");

    XMLGen::Load tLoad;
    tLoad.type("pressure");
    tLoad.id("1");
    tLoad.location_name("ss_1");
    std::vector<std::string> tValues = {"1.0"};
    tLoad.load_values(tValues);
    tXMLMetaData.loads.push_back(tLoad);

    XMLGen::Scenario tScenario;
    tScenario.id("1");
    tScenario.physics("steady_state_mechanics");
    std::vector<std::string> tLoadIDs = {"1"};
    tScenario.setLoadIDs(tLoadIDs);
    tXMLMetaData.append(tScenario);

    pugi::xml_document tDocument;
    XMLGen::append_loads_to_plato_analyze_input_deck(tXMLMetaData, tDocument);

    auto tLoadParamList = tDocument.child("ParameterList");
    ASSERT_FALSE(tLoadParamList.empty());
    ASSERT_STREQ("ParameterList", tLoadParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Natural Boundary Conditions"}, tLoadParamList);

    auto tTraction = tLoadParamList.child("ParameterList");
    ASSERT_FALSE(tTraction.empty());
    ASSERT_STREQ("ParameterList", tTraction.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Uniform Pressure Boundary Condition with ID 1"}, tTraction);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Uniform"}, {"Values", "Array(double)", "{1.0}"}, {"Sides", "string", "ss_1"} };
    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tLoadParamList.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendNaturalBoundaryConditionsToPlatoAnalyzeInputDeck_RandomUseCase)
{
    XMLGen::InputData tXMLMetaData;

    // POSE LOAD SET 1
    XMLGen::LoadCase tLoadCase1;
    tLoadCase1.id = "1";
    XMLGen::Load tLoad1;
    tLoad1.is_random("true");
    tLoad1.type("traction");
    tLoad1.id("1");
    tLoad1.location_name("sideset");
    std::vector<std::string> tValues = {"1.0", "2.0", "3.0"};
    tLoad1.load_values(tValues);
    tLoadCase1.loads.push_back(tLoad1);
    auto tLoadSet1 = std::make_pair(0.5, tLoadCase1);

    // POSE LOAD SET 2
    XMLGen::LoadCase tLoadCase2;
    tLoadCase2.id = "2";
    XMLGen::Load tLoad2;
    tLoad2.is_random("true");
    tLoad2.type("traction");
    tLoad1.id("1");
    tLoad2.location_name("sideset");
    tValues = {"11", "12", "13"};
    tLoad2.load_values(tValues);
    tLoadCase2.loads.push_back(tLoad2);
    auto tLoadSet2 = std::make_pair(0.5, tLoadCase2);

    // Scenario
    XMLGen::Scenario tScenario;
    tScenario.id("1");
    tScenario.physics("steady_state_mechanics");
    tXMLMetaData.append(tScenario);
    tXMLMetaData.objective.scenarioIDs.push_back("1");

    // CONSTRUCT SAMPLES SET
    ASSERT_NO_THROW(tXMLMetaData.mRandomMetaData.append(tLoadSet1));
    ASSERT_NO_THROW(tXMLMetaData.mRandomMetaData.append(tLoadSet2));
    ASSERT_NO_THROW(tXMLMetaData.mRandomMetaData.finalize());

    // CALL FUNCTION
    pugi::xml_document tDocument;
    XMLGen::append_loads_to_plato_analyze_input_deck(tXMLMetaData, tDocument);

    // TEST
    auto tLoadParamList = tDocument.child("ParameterList");
    ASSERT_FALSE(tLoadParamList.empty());
    ASSERT_STREQ("ParameterList", tLoadParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Natural Boundary Conditions"}, tLoadParamList);

    auto tTraction = tLoadParamList.child("ParameterList");
    ASSERT_FALSE(tTraction.empty());
    ASSERT_STREQ("ParameterList", tTraction.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Random Traction Vector Boundary Condition with ID 1"}, tTraction);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Uniform"}, {"Values", "Array(string)", "{1.0, 2.0, 3.0}"}, {"Sides", "string", "ss_1"} };
    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tLoadParamList.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}
TEST(PlatoTestXMLGenerator, AppendNaturalBoundaryCondition_ErrorInvalidType)
{
    XMLGen::Load tLoad;
    tLoad.type("nonsense");
    pugi::xml_document tDocument;
    XMLGen::AppendLoad tInterface;
    ASSERT_THROW(tInterface.call("Traction Vector Boundary Condition 1", tLoad, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendNaturalBoundaryCondition_Traction)
{
    XMLGen::Load tLoad;
    tLoad.type("traction");
    tLoad.id("1");
    tLoad.location_name("ss_1");
    std::vector<std::string> tValues = {"1.0", "2.0", "3.0"};
    tLoad.load_values(tValues);
    pugi::xml_document tDocument;
    XMLGen::AppendLoad tInterface;
    ASSERT_NO_THROW(tInterface.call("Traction Vector Boundary Condition 1", tLoad, tDocument));

    auto tLoadParamList = tDocument.child("ParameterList");
    ASSERT_FALSE(tLoadParamList.empty());
    ASSERT_STREQ("ParameterList", tLoadParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Traction Vector Boundary Condition 1"}, tLoadParamList);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Uniform"}, {"Values", "Array(string)", "{1.0,2.0,3.0}"}, {"Sides", "string", "ss_1"} };
    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tLoadParamList.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendNaturalBoundaryCondition_UniformPressure)
{
    XMLGen::Load tLoad;
    tLoad.type("pressure");
    tLoad.id("1");
    tLoad.location_name("ss_1");
    std::vector<std::string> tValues = {"1.0"};
    tLoad.load_values(tValues);
    pugi::xml_document tDocument;
    XMLGen::AppendLoad tInterface;
    ASSERT_NO_THROW(tInterface.call("Uniform Pressure Boundary Condition 1", tLoad, tDocument));

    auto tLoadParamList = tDocument.child("ParameterList");
    ASSERT_FALSE(tLoadParamList.empty());
    ASSERT_STREQ("ParameterList", tLoadParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Uniform Pressure Boundary Condition 1"}, tLoadParamList);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Uniform"}, {"Value", "string", "1.0"}, {"Sides", "string", "ss_1"} };
    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tLoadParamList.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendNaturalBoundaryCondition_UniformSurfacePotential)
{
    XMLGen::Load tLoad;
    tLoad.type("uniform_surface_potential");
    tLoad.id("1");
    tLoad.location_name("ss_1");
    std::vector<std::string> tValues = {"1.0"};
    tLoad.load_values(tValues);
    pugi::xml_document tDocument;
    XMLGen::AppendLoad tInterface;
    ASSERT_NO_THROW(tInterface.call("Uniform Surface Potential Boundary Condition 1", tLoad, tDocument));

    auto tLoadParamList = tDocument.child("ParameterList");
    ASSERT_FALSE(tLoadParamList.empty());
    ASSERT_STREQ("ParameterList", tLoadParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Uniform Surface Potential Boundary Condition 1"}, tLoadParamList);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Uniform"}, {"Value", "string", "1.0"}, {"Sides", "string", "ss_1"} };
    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tLoadParamList.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendNaturalBoundaryCondition_UniformSurfaceFlux)
{
    XMLGen::Load tLoad;
    tLoad.type("uniform_surface_flux");
    tLoad.id("1");
    tLoad.location_name("ss_1");
    std::vector<std::string> tValues = {"1.0"};
    tLoad.load_values(tValues);
    pugi::xml_document tDocument;
    XMLGen::AppendLoad tInterface;
    ASSERT_NO_THROW(tInterface.call("Uniform Surface Flux Boundary Condition 1", tLoad, tDocument));

    auto tLoadParamList = tDocument.child("ParameterList");
    ASSERT_FALSE(tLoadParamList.empty());
    ASSERT_STREQ("ParameterList", tLoadParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Uniform Surface Flux Boundary Condition 1"}, tLoadParamList);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Uniform"}, {"Value", "string", "1.0"}, {"Sides", "string", "ss_1"} };
    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tLoadParamList.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, LoadTag_ErrorInvalidType)
{
    XMLGen::LoadTag tInterface;
    XMLGen::Load tLoad;
    tLoad.type("nonsense");
    ASSERT_THROW(tInterface.call(tLoad), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, LoadTag)
{
    XMLGen::LoadTag tInterface;
    XMLGen::Load tLoad;

    // TRACTION TEST
    tLoad.type("traction");
    tLoad.id("1");
    auto tName = tInterface.call(tLoad);
    ASSERT_STREQ("Traction Vector Boundary Condition with ID 1", tName.c_str());

    tLoad.is_random("true");
    tName = tInterface.call(tLoad);
    ASSERT_STREQ("Random Traction Vector Boundary Condition with ID 1", tName.c_str());

    // PRESSURE TEST
    tLoad.is_random("false");
    tLoad.type("pressure");
    tName = tInterface.call(tLoad);
    ASSERT_STREQ("Uniform Pressure Boundary Condition with ID 1", tName.c_str());

    tLoad.is_random("true");
    tName = tInterface.call(tLoad);
    ASSERT_STREQ("Random Uniform Pressure Boundary Condition with ID 1", tName.c_str());

    // SURFACE POTENTIAL TEST
    tLoad.is_random("false");
    tLoad.type("uniform_surface_potential");
    tName = tInterface.call(tLoad);
    ASSERT_STREQ("Uniform Surface Potential Boundary Condition with ID 1", tName.c_str());

    tLoad.is_random("true");
    tName = tInterface.call(tLoad);
    ASSERT_STREQ("Random Uniform Surface Potential Boundary Condition with ID 1", tName.c_str());

    // SURFACE POTENTIAL TEST
    tLoad.is_random("false");
    tLoad.type("uniform_surface_flux");
    tName = tInterface.call(tLoad);
    ASSERT_STREQ("Uniform Surface Flux Boundary Condition with ID 1", tName.c_str());

    tLoad.is_random("true");
    tName = tInterface.call(tLoad);
    ASSERT_STREQ("Random Uniform Surface Flux Boundary Condition with ID 1", tName.c_str());
}

TEST(PlatoTestXMLGenerator, AppendAssembly_ErrorInvalidParentBlock)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Assembly tAssembly;
    tAssembly.property("id", "1");
    tAssembly.property("type", "tied");
    tAssembly.property("child_nodeset", "ns_1");
    tAssembly.property("parent_block", "2");
    std::vector<std::string> tDefaultOffset = {"0.0", "0.0", "0.0"};
    tAssembly.offset(tDefaultOffset);
    tAssembly.property("rhs_value", "0.0");
    tXMLMetaData.assemblies.push_back(tAssembly);

    XMLGen::Block tBlock;
    tBlock.block_id = "1";
    tXMLMetaData.blocks.push_back(tBlock);

    pugi::xml_document tDocument;

    ASSERT_THROW(XMLGen::append_assemblies_to_plato_analyze_input_deck(tXMLMetaData, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendAssembly_CategoryFixed)
{
    XMLGen::Assembly tAssembly;
    tAssembly.property("id", "1");
    tAssembly.property("type", "tied");
    tAssembly.property("child_nodeset", "ns_1");
    tAssembly.property("parent_block", "2");
    std::vector<std::string> tDefaultOffset = {"0.0", "0.0", "0.0"};
    tAssembly.offset(tDefaultOffset);
    tAssembly.property("rhs_value", "0.0");
    pugi::xml_document tDocument;

    XMLGen::AppendAssembly tInterface;
    ASSERT_NO_THROW(tInterface.call(tAssembly, tDocument));

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "PBC"}, {"Child", "string", "ns_1"}, {"Parent", "string", "Block 2"}, {"Vector", "Array(double)", "{0.0,0.0,0.0}"}, {"Value", "double", "0.0"} };
    std::vector<std::string> tGoldParameterListNames =
        {"Tied Assembly 1 Using PBC Multipoint Constraint"};

    auto tParamList = tDocument.child("ParameterList");
    auto tGoldValuesItr = tGoldValues.begin();
    auto tGoldParameterListNamesItr = tGoldParameterListNames.begin();
    while(!tParamList.empty())
    {
        ASSERT_FALSE(tParamList.empty());
        ASSERT_STREQ("ParameterList", tParamList.name());
        PlatoTestXMLGenerator::test_attributes({"name"}, {tGoldParameterListNamesItr->c_str()}, tParamList);

        auto tParameter = tParamList.child("Parameter");
        while(!tParameter.empty())
        {
            ASSERT_FALSE(tParameter.empty());
            ASSERT_STREQ("Parameter", tParameter.name());
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
            tParameter = tParameter.next_sibling();
            std::advance(tGoldValuesItr, 1);
        }
        tParamList = tParamList.next_sibling();
        std::advance(tGoldParameterListNamesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendAssembliesToPlatoAnalyzeInputDeck)
{
    // POSE PROBLEM
    std::vector<std::string> tDefaultOffset = {"0.0", "0.0", "0.0"};
    std::vector<std::string> tPrescribedOffset = {"1.0", "-0.01", "80.0"};

    XMLGen::InputData tXMLMetaData;
    XMLGen::Assembly tAssembly;
    tAssembly.property("id", "1");
    tAssembly.property("type", "tied");
    tAssembly.property("child_nodeset", "ns_1");
    tAssembly.property("parent_block", "2");
    tAssembly.offset(tDefaultOffset);
    tAssembly.property("rhs_value", "0.0");
    tXMLMetaData.assemblies.push_back(tAssembly);
    tAssembly.property("id", "2");
    tAssembly.property("type", "tied");
    tAssembly.property("child_nodeset", "ns_2");
    tAssembly.property("parent_block", "2");
    tAssembly.offset(tDefaultOffset);
    tAssembly.property("rhs_value", "2.4");
    tXMLMetaData.assemblies.push_back(tAssembly);
    tAssembly.property("id", "3");
    tAssembly.property("type", "tied");
    tAssembly.property("child_nodeset", "ns_5");
    tAssembly.property("parent_block", "3");
    tAssembly.offset(tPrescribedOffset);
    tAssembly.property("rhs_value", "0.0");
    tXMLMetaData.assemblies.push_back(tAssembly);

    XMLGen::Block tBlock;
    tBlock.block_id = "2";
    tXMLMetaData.blocks.push_back(tBlock);
    tBlock.block_id = "3";
    tXMLMetaData.blocks.push_back(tBlock);

    XMLGen::Scenario tScenario;
    tScenario.id("1");

    tScenario.physics("steady_state_mechanics");

    std::vector<std::string> assemblyIDs = {{"1"},{"2"},{"3"}};
    tScenario.setAssemblyIDs(assemblyIDs);
    tXMLMetaData.append(tScenario);
    tXMLMetaData.objective.scenarioIDs.push_back("1");

    // CALL FUNCTION
    pugi::xml_document tDocument;
    XMLGen::append_assemblies_to_plato_analyze_input_deck(tXMLMetaData, tDocument);

    // TEST
    auto tAssemblyDoc = tDocument.child("ParameterList");
    ASSERT_FALSE(tAssemblyDoc.empty());
    ASSERT_STREQ("ParameterList", tAssemblyDoc.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Multipoint Constraints"}, tAssemblyDoc);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "PBC"}, {"Child", "string", "ns_1"}, {"Parent", "string", "Block 2"}, {"Vector", "Array(double)", "{0.0,0.0,0.0}"}, {"Value", "double", "0.0"}, 
          {"Type", "string", "PBC"}, {"Child", "string", "ns_2"}, {"Parent", "string", "Block 2"}, {"Vector", "Array(double)", "{0.0,0.0,0.0}"}, {"Value", "double", "2.4"},
          {"Type", "string", "PBC"}, {"Child", "string", "ns_5"}, {"Parent", "string", "Block 3"}, {"Vector", "Array(double)", "{1.0,-0.01,80.0}"}, {"Value", "double", "0.0"} };
    std::vector<std::string> tGoldParameterListNames =
        {"Tied Assembly 1 Using PBC Multipoint Constraint",
         "Tied Assembly 2 Using PBC Multipoint Constraint",
         "Tied Assembly 3 Using PBC Multipoint Constraint"};

    auto tParamList = tAssemblyDoc.child("ParameterList");
    auto tGoldValuesItr = tGoldValues.begin();
    auto tGoldParameterListNamesItr = tGoldParameterListNames.begin();
    while(!tParamList.empty())
    {
        ASSERT_FALSE(tParamList.empty());
        ASSERT_STREQ("ParameterList", tParamList.name());
        PlatoTestXMLGenerator::test_attributes({"name"}, {tGoldParameterListNamesItr->c_str()}, tParamList);

        auto tParameter = tParamList.child("Parameter");
        while(!tParameter.empty())
        {
            ASSERT_FALSE(tParameter.empty());
            ASSERT_STREQ("Parameter", tParameter.name());
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
            tParameter = tParameter.next_sibling();
            std::advance(tGoldValuesItr, 1);
        }
        tParamList = tParamList.next_sibling();
        std::advance(tGoldParameterListNamesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendSpatialModelToPlatoAnalyzeInputDeck_ErrorEmptyBlockContainer)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    ASSERT_THROW(XMLGen::append_spatial_model_to_plato_analyze_input_deck(tXMLMetaData, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendMaterialModelToPlatoAnalyzeInputDeck_ErrorEmptyMaterialContainer)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    ASSERT_THROW(XMLGen::append_material_models_to_plato_analyze_input_deck(tXMLMetaData, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendSpatialModelToPlatoAnalyzeInputDeck_ErrorNoMaterialWithMatchingID)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::Block tBlock;
    tBlock.block_id = "1";
    tBlock.element_type = "tet4";
    tBlock.material_id = "1";
    tBlock.name = "design volume";
    tXMLMetaData.blocks.push_back(tBlock);

    ASSERT_THROW(XMLGen::append_spatial_model_to_plato_analyze_input_deck(tXMLMetaData, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendSpatialModelToPlatoAnalyzeInputDeck_OneBlockOneMaterial)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::Block tBlock;
    tBlock.block_id = "1";
    tBlock.element_type = "tet4";
    tBlock.material_id = "1";
    tBlock.name = "block_1";
    tXMLMetaData.blocks.push_back(tBlock);

    XMLGen::Material tMaterial;
    tMaterial.id("1");
    tMaterial.code("plato_analyze");
    tMaterial.name("adamantium");
    tMaterial.materialModel("isotropic linear elastic");
    tMaterial.property("youngs_modulus", "1e9");
    tMaterial.property("poissons_ratio", "0.3");
    tXMLMetaData.materials.push_back(tMaterial);

    ASSERT_NO_THROW(XMLGen::append_spatial_model_to_plato_analyze_input_deck(tXMLMetaData, tDocument));
    //tDocument.save_file("dummy.xml", "    >     ");

    auto tSpatialModelList = tDocument.child("ParameterList");
    ASSERT_FALSE(tSpatialModelList.empty());
    ASSERT_STREQ("ParameterList", tSpatialModelList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Spatial Model"}, tSpatialModelList);

    auto tDomainList = tSpatialModelList.child("ParameterList");
    ASSERT_FALSE(tDomainList.empty());
    ASSERT_STREQ("ParameterList", tDomainList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Domains"}, tDomainList);

    auto tSpatialModelParams = tDomainList.child("ParameterList");
    ASSERT_FALSE(tSpatialModelParams.empty());
    ASSERT_STREQ("ParameterList", tSpatialModelParams.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Block 1"}, tSpatialModelParams);

    auto tElementBlock = tSpatialModelParams.child("Parameter");
    PlatoTestXMLGenerator::test_attributes({"name", "type", "value"}, {"Element Block", "string", "block_1"}, tElementBlock);

    auto tMaterialModel = tElementBlock.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes({"name", "type", "value"}, {"Material Model", "string", "adamantium"}, tMaterialModel);
}

TEST(PlatoTestXMLGenerator, AppendSpatialModelToPlatoAnalyzeInputDeck_OneBlockTwoMaterials)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::Block tBlock;
    tBlock.block_id = "1";
    tBlock.element_type = "tet4";
    tBlock.material_id = "2";
    tBlock.name = "block_1";
    tXMLMetaData.blocks.push_back(tBlock);

    XMLGen::Material tMaterial1;
    tMaterial1.id("1");
    tMaterial1.code("plato_analyze");
    tMaterial1.name("adamantium");
    tMaterial1.materialModel("isotropic linear elastic");
    tMaterial1.property("youngs_modulus", "1e9");
    tMaterial1.property("poissons_ratio", "0.3");

    XMLGen::Material tMaterial2;
    tMaterial2.id("2");
    tMaterial2.code("plato_analyze");
    tMaterial2.name("carbonadium");
    tMaterial2.materialModel("isotropic linear elastic");
    tMaterial2.property("youngs_modulus", "1e91");
    tMaterial2.property("poissons_ratio", "0.29");

    tXMLMetaData.materials.push_back(tMaterial1);
    tXMLMetaData.materials.push_back(tMaterial2);

    ASSERT_NO_THROW(XMLGen::append_spatial_model_to_plato_analyze_input_deck(tXMLMetaData, tDocument));
    //tDocument.save_file("dummy.xml", "    >     ");

    auto tSpatialModelList = tDocument.child("ParameterList");
    ASSERT_FALSE(tSpatialModelList.empty());
    ASSERT_STREQ("ParameterList", tSpatialModelList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Spatial Model"}, tSpatialModelList);

    auto tDomainList = tSpatialModelList.child("ParameterList");
    ASSERT_FALSE(tDomainList.empty());
    ASSERT_STREQ("ParameterList", tDomainList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Domains"}, tDomainList);

    auto tSpatialModelParams = tDomainList.child("ParameterList");
    ASSERT_FALSE(tSpatialModelParams.empty());
    ASSERT_STREQ("ParameterList", tSpatialModelParams.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Block 1"}, tSpatialModelParams);

    auto tElementBlock = tSpatialModelParams.child("Parameter");
    PlatoTestXMLGenerator::test_attributes({"name", "type", "value"}, {"Element Block", "string", "block_1"}, tElementBlock);

    auto tMaterialModel = tElementBlock.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes({"name", "type", "value"}, {"Material Model", "string", "carbonadium"}, tMaterialModel);
}

TEST(PlatoTestXMLGenerator, AppendSpatialModelToPlatoAnalyzeInputDeck_TwoBlocksOneMaterial)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;

    XMLGen::Block tBlock1;
    tBlock1.block_id = "1";
    tBlock1.element_type = "tet4";
    tBlock1.material_id = "1";
    tBlock1.name = "block_1";

    XMLGen::Block tBlock2;
    tBlock2.block_id = "2";
    tBlock2.element_type = "tet4";
    tBlock2.material_id = "1";
    tBlock2.name = "block_2";

    tXMLMetaData.blocks.push_back(tBlock1);
    tXMLMetaData.blocks.push_back(tBlock2);

    XMLGen::Material tMaterial;
    tMaterial.id("1");
    tMaterial.code("plato_analyze");
    tMaterial.name("adamantium");
    tMaterial.materialModel("isotropic linear elastic");
    tMaterial.property("youngs_modulus", "1e9");
    tMaterial.property("poissons_ratio", "0.3");
    tXMLMetaData.materials.push_back(tMaterial);

    ASSERT_NO_THROW(XMLGen::append_spatial_model_to_plato_analyze_input_deck(tXMLMetaData, tDocument));
    //tDocument.save_file("dummy.xml", "    >     ");

    auto tSpatialModelList = tDocument.child("ParameterList");
    ASSERT_FALSE(tSpatialModelList.empty());
    ASSERT_STREQ("ParameterList", tSpatialModelList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Spatial Model"}, tSpatialModelList);

    auto tDomainList = tSpatialModelList.child("ParameterList");
    ASSERT_FALSE(tDomainList.empty());
    ASSERT_STREQ("ParameterList", tDomainList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Domains"}, tDomainList);

    auto tSpatialModelParams1 = tDomainList.child("ParameterList");
    ASSERT_FALSE(tSpatialModelParams1.empty());
    ASSERT_STREQ("ParameterList", tSpatialModelParams1.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Block 1"}, tSpatialModelParams1);

    auto tElementBlock = tSpatialModelParams1.child("Parameter");
    PlatoTestXMLGenerator::test_attributes({"name", "type", "value"}, {"Element Block", "string", "block_1"}, tElementBlock);

    auto tMaterialModel = tElementBlock.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes({"name", "type", "value"}, {"Material Model", "string", "adamantium"}, tMaterialModel);

    auto tSpatialModelParams2 = tSpatialModelParams1.next_sibling();
    ASSERT_FALSE(tSpatialModelParams2.empty());
    ASSERT_STREQ("ParameterList", tSpatialModelParams2.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Block 2"}, tSpatialModelParams2);

    tElementBlock = tSpatialModelParams2.child("Parameter");
    PlatoTestXMLGenerator::test_attributes({"name", "type", "value"}, {"Element Block", "string", "block_2"}, tElementBlock);

    tMaterialModel = tElementBlock.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes({"name", "type", "value"}, {"Material Model", "string", "adamantium"}, tMaterialModel);
}

TEST(PlatoTestXMLGenerator, AppendSpatialModelToPlatoAnalyzeInputDeck_TwoBlocksTwoMaterials)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;

    XMLGen::Block tBlock1;
    tBlock1.block_id = "1";
    tBlock1.element_type = "tet4";
    tBlock1.material_id = "2";
    tBlock1.name = "block_1";

    XMLGen::Block tBlock2;
    tBlock2.block_id = "2";
    tBlock2.element_type = "tet4";
    tBlock2.material_id = "1";
    tBlock2.name = "block_2";

    tXMLMetaData.blocks.push_back(tBlock1);
    tXMLMetaData.blocks.push_back(tBlock2);

    XMLGen::Material tMaterial1;
    tMaterial1.id("1");
    tMaterial1.code("plato_analyze");
    tMaterial1.name("adamantium");
    tMaterial1.materialModel("isotropic_linear_elastic");
    tMaterial1.property("youngs_modulus", "1e9");
    tMaterial1.property("poissons_ratio", "0.3");

    XMLGen::Material tMaterial2;
    tMaterial2.id("2");
    tMaterial2.code("plato_analyze");
    tMaterial2.name("carbonadium");
    tMaterial2.materialModel("isotropic_linear_elastic");
    tMaterial2.property("youngs_modulus", "1e91");
    tMaterial2.property("poissons_ratio", "0.29");

    tXMLMetaData.materials.push_back(tMaterial1);
    tXMLMetaData.materials.push_back(tMaterial2);

    ASSERT_NO_THROW(XMLGen::append_spatial_model_to_plato_analyze_input_deck(tXMLMetaData, tDocument));

    auto tSpatialModelList = tDocument.child("ParameterList");
    ASSERT_FALSE(tSpatialModelList.empty());
    ASSERT_STREQ("ParameterList", tSpatialModelList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Spatial Model"}, tSpatialModelList);

    auto tDomainList = tSpatialModelList.child("ParameterList");
    ASSERT_FALSE(tDomainList.empty());
    ASSERT_STREQ("ParameterList", tDomainList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Domains"}, tDomainList);

    auto tSpatialModelParams1 = tDomainList.child("ParameterList");
    ASSERT_FALSE(tSpatialModelParams1.empty());
    ASSERT_STREQ("ParameterList", tSpatialModelParams1.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Block 1"}, tSpatialModelParams1);

    auto tElementBlock = tSpatialModelParams1.child("Parameter");
    PlatoTestXMLGenerator::test_attributes({"name", "type", "value"}, {"Element Block", "string", "block_1"}, tElementBlock);

    auto tMaterialModel = tElementBlock.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes({"name", "type", "value"}, {"Material Model", "string", "carbonadium"}, tMaterialModel);

    auto tSpatialModelParams2 = tSpatialModelParams1.next_sibling();
    ASSERT_FALSE(tSpatialModelParams2.empty());
    ASSERT_STREQ("ParameterList", tSpatialModelParams2.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Block 2"}, tSpatialModelParams2);

    tElementBlock = tSpatialModelParams2.child("Parameter");
    PlatoTestXMLGenerator::test_attributes({"name", "type", "value"}, {"Element Block", "string", "block_2"}, tElementBlock);

    tMaterialModel = tElementBlock.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes({"name", "type", "value"}, {"Material Model", "string", "adamantium"}, tMaterialModel);
}

TEST(PlatoTestXMLGenerator, AppendSpatialModelToPlatoAnalyzeInputDeck_TwoBlocksOneFixed)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;

    XMLGen::Block tBlock1;
    tBlock1.block_id = "1";
    tBlock1.element_type = "tet4";
    tBlock1.material_id = "1";
    tBlock1.name = "block_1";

    XMLGen::Block tBlock2;
    tBlock2.block_id = "2";
    tBlock2.element_type = "tet4";
    tBlock2.material_id = "2";
    tBlock2.name = "block_2";

    tXMLMetaData.blocks.push_back(tBlock1);
    tXMLMetaData.blocks.push_back(tBlock2);

    XMLGen::Material tMaterial1;
    tMaterial1.id("1");
    tMaterial1.code("plato_analyze");
    tMaterial1.name("adamantium");
    tMaterial1.materialModel("isotropic_linear_elastic");
    tMaterial1.property("youngs_modulus", "1e9");
    tMaterial1.property("poissons_ratio", "0.3");

    XMLGen::Material tMaterial2;
    tMaterial2.id("2");
    tMaterial2.code("plato_analyze");
    tMaterial2.name("carbonadium");
    tMaterial2.materialModel("isotropic_linear_elastic");
    tMaterial2.property("youngs_modulus", "1e91");
    tMaterial2.property("poissons_ratio", "0.29");

    tXMLMetaData.materials.push_back(tMaterial1);
    tXMLMetaData.materials.push_back(tMaterial2);

    XMLGen::OptimizationParameters tOptimizationParameters;
    std::vector<std::string> tFixedBlockIds;
    tFixedBlockIds.push_back("2");
    tOptimizationParameters.setFixedBlockIDs(tFixedBlockIds);
    tXMLMetaData.set(tOptimizationParameters);

    ASSERT_NO_THROW(XMLGen::append_spatial_model_to_plato_analyze_input_deck(tXMLMetaData, tDocument));
    //tDocument.save_file("dummy.xml", "    >     ");

    auto tSpatialModelList = tDocument.child("ParameterList");
    ASSERT_FALSE(tSpatialModelList.empty());
    ASSERT_STREQ("ParameterList", tSpatialModelList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Spatial Model"}, tSpatialModelList);

    auto tDomainList = tSpatialModelList.child("ParameterList");
    ASSERT_FALSE(tDomainList.empty());
    ASSERT_STREQ("ParameterList", tDomainList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Domains"}, tDomainList);

    auto tSpatialModelParams1 = tDomainList.child("ParameterList");
    ASSERT_FALSE(tSpatialModelParams1.empty());
    ASSERT_STREQ("ParameterList", tSpatialModelParams1.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Block 1"}, tSpatialModelParams1);

    auto tElementBlock = tSpatialModelParams1.child("Parameter");
    PlatoTestXMLGenerator::test_attributes({"name", "type", "value"}, {"Element Block", "string", "block_1"}, tElementBlock);

    auto tMaterialModel = tElementBlock.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes({"name", "type", "value"}, {"Material Model", "string", "adamantium"}, tMaterialModel);

    auto tSpatialModelParams2 = tSpatialModelParams1.next_sibling();
    ASSERT_FALSE(tSpatialModelParams2.empty());
    ASSERT_STREQ("ParameterList", tSpatialModelParams2.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Block 2"}, tSpatialModelParams2);

    tElementBlock = tSpatialModelParams2.child("Parameter");
    PlatoTestXMLGenerator::test_attributes({"name", "type", "value"}, {"Element Block", "string", "block_2"}, tElementBlock);

    tMaterialModel = tElementBlock.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes({"name", "type", "value"}, {"Material Model", "string", "carbonadium"}, tMaterialModel);

    // Uncomment when 'Fixed Blocks' parameter is fixed in PA
    // auto tFixedControl = tMaterialModel.next_sibling("Parameter");
    // PlatoTestXMLGenerator::test_attributes({"name", "type", "value"}, {"Fixed Control", "bool", "true"}, tFixedControl);
}

TEST(PlatoTestXMLGenerator, AppendMaterialModelToPlatoAnalyzeInputDeck_Empty_MaterialIsNotFromAnalyzePerformer)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::Material tMaterial;
    tMaterial.code("sierra");
    tXMLMetaData.materials.push_back(tMaterial);
    ASSERT_NO_THROW(XMLGen::append_material_models_to_plato_analyze_input_deck(tXMLMetaData, tDocument));
    //tDocument.save_file("dummy.xml", "    >     ");
    auto tParamList = tDocument.child("ParameterList");
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Material Models"}, tParamList);
    auto tMaterials = tParamList.child("ParameterList");
    ASSERT_TRUE(tMaterials.empty());
}

TEST(PlatoTestXMLGenerator, AppendMaterialModelToPlatoAnalyzeInputDeck_ErrorInvalidMatModel)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::Material tMaterial;
    tMaterial.code("plato_analyze");
    tMaterial.materialModel("isotropic_linear_viscoelastic");
    tXMLMetaData.materials.push_back(tMaterial);
    ASSERT_THROW(XMLGen::append_material_models_to_plato_analyze_input_deck(tXMLMetaData, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendMaterialModelToPlatoAnalyzeInputDeck_ErrorMatPropAreNotDefined)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::Material tMaterial;
    tMaterial.id("1");
    tMaterial.code("plato_analyze");
    tXMLMetaData.materials.push_back(tMaterial);
    ASSERT_THROW(XMLGen::append_material_models_to_plato_analyze_input_deck(tXMLMetaData, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendMaterialModelToPlatoAnalyzeInputDeck_IsotropicLinearElasticMatModel)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::Material tMaterial;
    tMaterial.code("plato_analyze");
    tMaterial.name("unobtainium");
    tMaterial.materialModel("isotropic_linear_elastic");
    tMaterial.property("youngs_modulus", "1e9");
    tMaterial.property("poissons_ratio", "0.3");
    tXMLMetaData.materials.push_back(tMaterial);
    ASSERT_NO_THROW(XMLGen::append_material_models_to_plato_analyze_input_deck(tXMLMetaData, tDocument));
    //tDocument.save_file("dummy.xml", "    >     ");

    auto tMaterialModelsList = tDocument.child("ParameterList");
    ASSERT_FALSE(tMaterialModelsList.empty());
    ASSERT_STREQ("ParameterList", tMaterialModelsList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Material Models"}, tMaterialModelsList);

    auto tFirstMaterial = tMaterialModelsList.child("ParameterList");
    PlatoTestXMLGenerator::test_attributes({"name"}, {"unobtainium"}, tFirstMaterial);
    auto tMyMaterialModel = tFirstMaterial.child("ParameterList");
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Isotropic Linear Elastic"}, tMyMaterialModel);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Poissons Ratio", "double", "0.3"}, {"Youngs Modulus", "double", "1e9"} };
    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tMyMaterialModel.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendMaterialModelToPlatoAnalyzeInputDeck_IsotropicLinearElasticMatModelWithDensity)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::Material tMaterial;
    tMaterial.code("plato_analyze");
    tMaterial.name("unobtainium");
    tMaterial.materialModel("isotropic_linear_elastic");
    tMaterial.property("youngs_modulus", "1e9");
    tMaterial.property("poissons_ratio", "0.3");
    tMaterial.property("mass_density", "1000.0");
    tXMLMetaData.materials.push_back(tMaterial);
    ASSERT_NO_THROW(XMLGen::append_material_models_to_plato_analyze_input_deck(tXMLMetaData, tDocument));

    auto tMaterialModelsList = tDocument.child("ParameterList");
    ASSERT_FALSE(tMaterialModelsList.empty());
    ASSERT_STREQ("ParameterList", tMaterialModelsList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Material Models"}, tMaterialModelsList);

    auto tFirstMaterial = tMaterialModelsList.child("ParameterList");
    ASSERT_FALSE(tFirstMaterial.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"unobtainium"}, tFirstMaterial);

    auto tMyDensity = tFirstMaterial.child("Parameter");
    ASSERT_FALSE(tMyDensity.empty());
    ASSERT_STREQ("Parameter", tMyDensity.name());
    PlatoTestXMLGenerator::test_attributes({"name", "type", "value"}, {"Density", "double", "1000.0"}, tMyDensity);

    auto tMyMaterialModel = tFirstMaterial.child("ParameterList");
    ASSERT_FALSE(tMyMaterialModel.empty());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Isotropic Linear Elastic"}, tMyMaterialModel);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Poissons Ratio", "double", "0.3"}, {"Youngs Modulus", "double", "1e9"}, {"Mass Density", "double", "1000.0"}};
    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tMyMaterialModel.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendMaterialModelToPlatoAnalyzeInputDeck_IsotropicLinearThermalMatModel)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::Material tMaterial;
    tMaterial.code("plato_analyze");
    tMaterial.name("adamantium");
    tMaterial.materialModel("isotropic_linear_thermal");
    tMaterial.property("thermal_conductivity", "10");
    tMaterial.property("mass_density", "200");
    tMaterial.property("specific_heat", "20");
    tXMLMetaData.materials.push_back(tMaterial);
    ASSERT_NO_THROW(XMLGen::append_material_models_to_plato_analyze_input_deck(tXMLMetaData, tDocument));
    
    auto tMaterialModelsList = tDocument.child("ParameterList");
    ASSERT_FALSE(tMaterialModelsList.empty());
    ASSERT_STREQ("ParameterList", tMaterialModelsList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Material Models"}, tMaterialModelsList);

    auto tFirstMaterial = tMaterialModelsList.child("ParameterList");
    PlatoTestXMLGenerator::test_attributes({"name"}, {"adamantium"}, tFirstMaterial);
    auto tThermalConduction = tFirstMaterial.child("ParameterList");
    ASSERT_FALSE(tThermalConduction.empty());
    ASSERT_STREQ("ParameterList", tThermalConduction.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Thermal Conduction"}, tThermalConduction);
    auto tParameter = tThermalConduction.child("Parameter");
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Thermal Conductivity", "double", "10"}, tParameter);

    auto tThermalMass = tThermalConduction.next_sibling("ParameterList");
    ASSERT_FALSE(tThermalMass.empty());
    ASSERT_STREQ("ParameterList", tThermalMass.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Thermal Mass"}, tThermalMass);
    tParameter = tThermalMass.child("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Mass Density", "double", "200"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Specific Heat", "double", "20"}, tParameter);
}

TEST(PlatoTestXMLGenerator, AppendMaterialModelToPlatoAnalyzeInputDeck_OrthotropicLinearElasticMatModel)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::Material tMaterial;
    tMaterial.code("plato_analyze");
    tMaterial.name("vibranium");
    tMaterial.materialModel("orthotropic_linear_elastic");
    tMaterial.property("youngs_modulus_x", "1.0");
    tMaterial.property("youngs_modulus_y", "2.0");
    tMaterial.property("youngs_modulus_z", "3.0");
    tMaterial.property("poissons_ratio_xy", "0.3");
    tMaterial.property("poissons_ratio_xz", "0.4");
    tMaterial.property("poissons_ratio_yz", "0.25");
    tMaterial.property("shear_modulus_xy", "1.3");
    tMaterial.property("shear_modulus_xz", "1.4");
    tMaterial.property("shear_modulus_yz", "1.25");
    tXMLMetaData.materials.push_back(tMaterial);
    ASSERT_NO_THROW(XMLGen::append_material_models_to_plato_analyze_input_deck(tXMLMetaData, tDocument));
    //tDocument.save_file("dummy.xml", "    >     ");
    auto tMaterialModelsList = tDocument.child("ParameterList");
    ASSERT_FALSE(tMaterialModelsList.empty());
    ASSERT_STREQ("ParameterList", tMaterialModelsList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Material Models"}, tMaterialModelsList);

    auto tFirstMaterial = tMaterialModelsList.child("ParameterList");
    PlatoTestXMLGenerator::test_attributes({"name"}, {"vibranium"}, tFirstMaterial);
    auto tMyMaterialModel = tFirstMaterial.child("ParameterList");
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Orthotropic Linear Elastic"}, tMyMaterialModel);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Poissons Ratio XY", "double", "0.3"}, {"Poissons Ratio XZ", "double", "0.4"}, {"Poissons Ratio YZ", "double", "0.25"},
          {"Shear Modulus XY", "double", "1.3"}, {"Shear Modulus XZ", "double", "1.4"}, {"Shear Modulus YZ", "double", "1.25"},
          {"Youngs Modulus X", "double", "1.0"}, {"Youngs Modulus Y", "double", "2.0"}, {"Youngs Modulus Z", "double", "3.0"} };
    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tMyMaterialModel.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendMaterialModelToPlatoAnalyzeInputDeck_IsotropicLinearThermoelasticMatModel)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::Material tMaterial;
    tMaterial.code("plato_analyze");
    tMaterial.name("kryptonite");
    tMaterial.materialModel("isotropic_linear_thermoelastic");
    tMaterial.property("thermal_conductivity", "1.0");
    tMaterial.property("youngs_modulus", "2.3");
    tMaterial.property("poissons_ratio", "0.3");
    tMaterial.property("thermal_expansivity", "0.4");
    tMaterial.property("reference_temperature", "1.25");
    tXMLMetaData.materials.push_back(tMaterial);
    ASSERT_NO_THROW(XMLGen::append_material_models_to_plato_analyze_input_deck(tXMLMetaData, tDocument));

    auto tMaterialModelsList = tDocument.child("ParameterList");
    ASSERT_FALSE(tMaterialModelsList.empty());
    ASSERT_STREQ("ParameterList", tMaterialModelsList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Material Models"}, tMaterialModelsList);

    auto tFirstMaterial = tMaterialModelsList.child("ParameterList");
    PlatoTestXMLGenerator::test_attributes({"name"}, {"kryptonite"}, tFirstMaterial);
    auto tThermalModel = tFirstMaterial.child("ParameterList");
    ASSERT_FALSE(tThermalModel.empty());
    ASSERT_STREQ("ParameterList", tThermalModel.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Thermoelastic"}, tThermalModel);
    auto tParameter = tThermalModel.child("Parameter");
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Thermal Expansivity", "double", "0.4"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Thermal Conductivity", "double", "1.0"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Reference Temperature", "double", "1.25"}, tParameter);

    auto tElasticModel = tThermalModel.child("ParameterList");
    ASSERT_FALSE(tElasticModel.empty());
    ASSERT_STREQ("ParameterList", tElasticModel.name());
    tParameter = tElasticModel.child("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Youngs Modulus", "double", "2.3"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Poissons Ratio", "double", "0.3"}, tParameter);
}

TEST(PlatoTestXMLGenerator, AppendMaterialModelToPlatoAnalyzeInputDeck_J2PlasticityMatModel)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::Material tMaterial;
    tMaterial.code("plato_analyze");
    tMaterial.name("mithril");
    tMaterial.materialModel("j2_plasticity");
    tMaterial.property("youngs_modulus", "2.3");
    tMaterial.property("poissons_ratio", "0.3");
    tMaterial.property("initial_yield_stress", "2.0");
    tMaterial.property("hardening_modulus_isotropic", "0.4");
    tMaterial.property("hardening_modulus_kinematic", "1.25");
    tXMLMetaData.materials.push_back(tMaterial);
    XMLGen::append_material_models_to_plato_analyze_input_deck(tXMLMetaData, tDocument);

    // Material Models list
    auto tMaterialModelsList = tDocument.child("ParameterList");
    ASSERT_FALSE(tMaterialModelsList.empty());
    ASSERT_STREQ("ParameterList", tMaterialModelsList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Material Models"}, tMaterialModelsList);

    auto tCurMaterialModel = tMaterialModelsList.child("ParameterList");
    PlatoTestXMLGenerator::test_attributes({"name"}, {"mithril"}, tCurMaterialModel);
    auto tElasticModel = tCurMaterialModel.child("ParameterList");
    ASSERT_FALSE(tElasticModel.empty());
    ASSERT_STREQ("ParameterList", tElasticModel.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Isotropic Linear Elastic"}, tElasticModel);
    auto tParameter = tElasticModel.child("Parameter");
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Poissons Ratio", "double", "0.3"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Youngs Modulus", "double", "2.3"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());

    // plasticity model
    auto tPlasticityParamList = tElasticModel.next_sibling("ParameterList");
    ASSERT_FALSE(tPlasticityParamList.empty());
    ASSERT_STREQ("ParameterList", tPlasticityParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Plasticity Model"}, tPlasticityParamList);

    auto tPlasticModel = tPlasticityParamList.child("ParameterList");
    ASSERT_FALSE(tPlasticModel.empty());
    ASSERT_STREQ("ParameterList", tPlasticModel.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"J2 Plasticity"}, tPlasticModel);
    tParameter = tPlasticModel.child("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Initial Yield Stress", "double", "2.0"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Hardening Modulus Isotropic", "double", "0.4"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Hardening Modulus Kinematic", "double", "1.25"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());
}

TEST(PlatoTestXMLGenerator, AppendMaterialModelToPlatoAnalyzeInputDeck_ForcedConvection)
{
    XMLGen::InputData tXMLMetaData;
    XMLGen::Material tMaterial;
    tMaterial.code("plato_analyze");
    tMaterial.name("Water");
    tMaterial.materialModel("forced_convection");
    tMaterial.property("darcy_number", "1e-5");
    tMaterial.property("prandtl_number", "0.7");
    tMaterial.property("reynolds_number", "100");
    tMaterial.property("temperature_difference", "10");
    tMaterial.property("characteristic_length", "0.1");
    tMaterial.property("thermal_conductivity", "1.0e-6");
    tMaterial.property("thermal_diffusivity", "2.1117e-5");
    tMaterial.property("kinematic_viscocity", "1.5111e-5");
    tXMLMetaData.materials.push_back(tMaterial);

    pugi::xml_document tDocument;
    XMLGen::append_material_models_to_plato_analyze_input_deck(tXMLMetaData, tDocument);
    //tDocument.save_file("dummy.xml");

    // TEST MATERIAL MODEL BLOCK
    auto tMaterialModelsList = tDocument.child("ParameterList");
    ASSERT_FALSE(tMaterialModelsList.empty());
    ASSERT_STREQ("ParameterList", tMaterialModelsList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Material Models"}, tMaterialModelsList);

    // TEST WATER BLOCK
    auto tWater = tMaterialModelsList.child("ParameterList");
    ASSERT_FALSE(tWater.empty());
    ASSERT_STREQ("ParameterList", tWater.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Water"}, tWater);
    // TEST WATER BLOCK PARAMETERS
    auto tParameter = tWater.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    ASSERT_STREQ("Parameter", tParameter.name());
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Darcy Number", "double", "1e-5"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Prandtl Number", "double", "0.7"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Reynolds Number", "double", "100"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Thermal Diffusivity", "double", "2.1117e-5"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Kinematic Viscocity", "double", "1.5111e-5"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Thermal Conductivity", "double", "1.0e-6"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Characteristic Length", "double", "0.1"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Temperature Difference", "double", "10"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());
    // TEST WATER BLOCK END
    tWater = tWater.next_sibling("ParameterList");
    ASSERT_TRUE(tWater.empty());
    //TEST MATERIAL MODEL BLOCK END
    tMaterialModelsList = tMaterialModelsList.next_sibling("ParameterList");
    ASSERT_TRUE(tMaterialModelsList.empty());
}

TEST(PlatoTestXMLGenerator, AppendMaterialModelToPlatoAnalyzeInputDeck_NaturalConvection)
{
    XMLGen::InputData tXMLMetaData;
    XMLGen::Material tMaterial;
    tMaterial.code("plato_analyze");
    tMaterial.name("Water");
    tMaterial.materialModel("natural_convection");
    tMaterial.property("darcy_number", "110");
    tMaterial.property("prandtl_number", "17");
    tMaterial.property("temperature_difference", "10");
    tMaterial.property("characteristic_length", "1");
    tMaterial.property("grashof_number", std::vector<std::string>{"10", "10"} );
    tMaterial.property("richardson_number", std::vector<std::string>{"20", "20"} );
    tMaterial.property("rayleigh_number", std::vector<std::string>{"30", "30"} );
    tMaterial.property("thermal_conductivity", "1.0e-6");
    tMaterial.property("thermal_diffusivity", "2.1117e-5");
    tMaterial.property("kinematic_viscocity", "1.5111e-5");
    tXMLMetaData.materials.push_back(tMaterial);

    pugi::xml_document tDocument;
    XMLGen::append_material_models_to_plato_analyze_input_deck(tXMLMetaData, tDocument);
    //tDocument.save_file("dummy.xml");

    // TEST MATERIAL MODEL BLOCK
    auto tMaterialModelsList = tDocument.child("ParameterList");
    ASSERT_FALSE(tMaterialModelsList.empty());
    ASSERT_STREQ("ParameterList", tMaterialModelsList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Material Models"}, tMaterialModelsList);

    // TEST WATER BLOCK
    auto tWater = tMaterialModelsList.child("ParameterList");
    ASSERT_FALSE(tWater.empty());
    ASSERT_STREQ("ParameterList", tWater.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Water"}, tWater);
    // TEST WATER BLOCK PARAMETERS
    auto tParameter = tWater.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    ASSERT_STREQ("Parameter", tParameter.name());
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Darcy Number", "double", "110"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Prandtl Number", "double", "17"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Thermal Diffusivity", "double", "2.1117e-5"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Kinematic Viscocity", "double", "1.5111e-5"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Thermal Conductivity", "double", "1.0e-6"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Characteristic Length", "double", "1"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Temperature Difference", "double", "10"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Grashof Number", "Array(double)", "{10,10}"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Rayleigh Number", "Array(double)", "{30,30}"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Richardson Number", "Array(double)", "{20,20}"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());
    // TEST WATER BLOCK END
    tWater = tWater.next_sibling("ParameterList");
    ASSERT_TRUE(tWater.empty());
    //TEST MATERIAL MODEL BLOCK END
    tMaterialModelsList = tMaterialModelsList.next_sibling("ParameterList");
    ASSERT_TRUE(tMaterialModelsList.empty());
}

TEST(PlatoTestXMLGenerator, AppendMaterialModelToPlatoAnalyzeInputDeck_IncompressibleFlow)
{
    XMLGen::InputData tXMLMetaData;
    XMLGen::Material tMaterial;
    tMaterial.code("plato_analyze");
    tMaterial.name("Water");
    tMaterial.materialModel("laminar_flow");
    tMaterial.property("reynolds_number", "400");
    tMaterial.property("impermeability_number", "100");
    tXMLMetaData.materials.push_back(tMaterial);

    pugi::xml_document tDocument;
    XMLGen::append_material_models_to_plato_analyze_input_deck(tXMLMetaData, tDocument);
    //tDocument.save_file("dummy.xml");

    // TEST MATERIAL MODEL BLOCK
    auto tMaterialModelsList = tDocument.child("ParameterList");
    ASSERT_FALSE(tMaterialModelsList.empty());
    ASSERT_STREQ("ParameterList", tMaterialModelsList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Material Models"}, tMaterialModelsList);

    // TEST WATER BLOCK
    auto tWater = tMaterialModelsList.child("ParameterList");
    ASSERT_FALSE(tWater.empty());
    ASSERT_STREQ("ParameterList", tWater.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Water"}, tWater);
    // TEST WATER BLOCK PARAMETERS
    auto tParameter = tWater.child("Parameter");
    ASSERT_FALSE(tParameter.empty());
    ASSERT_STREQ("Parameter", tParameter.name());
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Reynolds Number", "double", "400"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Impermeability Number", "double", "100"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    ASSERT_TRUE(tParameter.empty());
    // TEST WATER BLOCK END
    tWater = tWater.next_sibling("ParameterList");
    ASSERT_TRUE(tWater.empty());
    //TEST MATERIAL MODEL BLOCK END
    tMaterialModelsList = tMaterialModelsList.next_sibling("ParameterList");
    ASSERT_TRUE(tMaterialModelsList.empty());
}

TEST(PlatoTestXMLGenerator, AppendMaterialModelToPlatoAnalyzeInputDeck_ThermoplasticityMatModel)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::Material tMaterial;
    tMaterial.code("plato_analyze");
    tMaterial.name("my_thermoplasticity_model");
    tMaterial.materialModel("thermoplasticity");
    tMaterial.property("youngs_modulus", "2.3");
    tMaterial.property("poissons_ratio", "0.3");
    tMaterial.property("thermal_expansivity", "16");
    tMaterial.property("thermal_conductivity", "17");
    tMaterial.property("reference_temperature", "18");
    tMaterial.property("initial_yield_stress", "19");
    tMaterial.property("hardening_modulus_isotropic", "20");
    tMaterial.property("hardening_modulus_kinematic", "21");
    tMaterial.property("elastic_properties_penalty_exponent", "3.0");
    tMaterial.property("elastic_properties_minimum_ersatz", "1.0e-6");
    tMaterial.property("plastic_properties_penalty_exponent", "2.5");
    tMaterial.property("plastic_properties_minimum_ersatz", "1.0e-3");
    tXMLMetaData.materials.push_back(tMaterial);
    XMLGen::append_material_models_to_plato_analyze_input_deck(tXMLMetaData, tDocument);

    // Material Models list
    auto tMaterialModelsList = tDocument.child("ParameterList");
    ASSERT_FALSE(tMaterialModelsList.empty());
    ASSERT_STREQ("ParameterList", tMaterialModelsList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Material Models"}, tMaterialModelsList);

    auto tCurMaterialModel = tMaterialModelsList.child("ParameterList");
    PlatoTestXMLGenerator::test_attributes({"name"}, {"my_thermoplasticity_model"}, tCurMaterialModel);
    auto tElasticModel = tCurMaterialModel.child("ParameterList");
    ASSERT_FALSE(tElasticModel.empty());
    ASSERT_STREQ("ParameterList", tElasticModel.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Isotropic Linear Thermoelastic"}, tElasticModel);
    auto tParameter = tElasticModel.child("Parameter");
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Poissons Ratio", "double", "0.3"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Youngs Modulus", "double", "2.3"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Thermal Expansivity", "double", "16"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Thermal Conductivity", "double", "17"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Reference Temperature", "double", "18"}, tParameter);

    // plasticity model
    auto tPlasticityParamList = tElasticModel.next_sibling("ParameterList");
    ASSERT_FALSE(tPlasticityParamList.empty());
    ASSERT_STREQ("ParameterList", tPlasticityParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Plasticity Model"}, tPlasticityParamList);

    auto tPlasticModel = tPlasticityParamList.child("ParameterList");
    ASSERT_FALSE(tPlasticModel.empty());
    ASSERT_STREQ("ParameterList", tPlasticModel.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"J2 Plasticity"}, tPlasticModel);
    tParameter = tPlasticModel.child("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Initial Yield Stress", "double", "19"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Hardening Modulus Isotropic", "double", "20"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Hardening Modulus Kinematic", "double", "21"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Elastic Properties Minimum Ersatz", "double", "1.0e-6"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Elastic Properties Penalty Exponent", "double", "3.0"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Plastic Properties Minimum Ersatz", "double", "1.0e-3"}, tParameter);
    tParameter = tParameter.next_sibling("Parameter");
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, {"Plastic Properties Penalty Exponent", "double", "2.5"}, tParameter);
    
}

TEST(PlatoTestXMLGenerator, AppendMaterialModelToPlatoAnalyzeInputDeck_IsotropicLinearElectroelasticMatModel)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::Material tMaterial;
    tMaterial.code("plato_analyze");
    tMaterial.name("bavarium");
    tMaterial.materialModel("isotropic_linear_electroelastic");
    tMaterial.property("youngs_modulus", "2.3");
    tMaterial.property("poissons_ratio", "0.3");
    tMaterial.property("dielectric_permittivity_11", "1.0");
    tMaterial.property("dielectric_permittivity_33", "0.4");
    tMaterial.property("piezoelectric_coupling_15", "1.25");
    tMaterial.property("piezoelectric_coupling_33", "2.25");
    tMaterial.property("piezoelectric_coupling_31", "3.25");
    tMaterial.property("thermal_expansivity", "0.25");
    tXMLMetaData.materials.push_back(tMaterial);
    ASSERT_NO_THROW(XMLGen::append_material_models_to_plato_analyze_input_deck(tXMLMetaData, tDocument));
    //tDocument.save_file("dummy.xml", "    >     ");

    auto tMaterialModelsList = tDocument.child("ParameterList");
    ASSERT_FALSE(tMaterialModelsList.empty());
    ASSERT_STREQ("ParameterList", tMaterialModelsList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Material Models"}, tMaterialModelsList);

    auto tMaterialNode = tMaterialModelsList.child("ParameterList");
    PlatoTestXMLGenerator::test_attributes({"name"}, {"bavarium"}, tMaterialNode);
    auto tMyMaterialModel = tMaterialNode.child("ParameterList");
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Isotropic Linear Electroelastic"}, tMyMaterialModel);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"p11", "double", "1.0"}, {"p33", "double", "0.4"}, {"e15", "double", "1.25"},
          {"e31", "double", "3.25"}, {"e33", "double", "2.25"}, {"Poissons Ratio", "double", "0.3"},
          {"Alpha", "double", "0.25"}, {"Youngs Modulus", "double", "2.3"} };
    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tMyMaterialModel.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendMaterialModelToPlatoAnalyzeInputDeck_RandomIsotropicLinearElasticMatModel)
{
    // POSE INPUTS
    XMLGen::InputData tXMLMetaData;
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("optimization_type", "topology");
    tXMLMetaData.set(tOptimizationParameters);
    XMLGen::Objective tObjective;
    tXMLMetaData.objective = tObjective;

    // CREATE MATERIAL
    XMLGen::Material tMaterial;
    tMaterial.id("1");
    tMaterial.name("carbonadium");
    tMaterial.materialModel("isotropic_linear_elastic");
    tMaterial.property("youngs_modulus", "1");
    tMaterial.property("poissons_ratio", "0.3");
    tXMLMetaData.materials.push_back(tMaterial);

    // POSE MATERIAL SET 1
    XMLGen::Material tMaterial1;
    tMaterial1.id("1");
    tMaterial1.materialModel("isotropic_linear_elastic");
    tMaterial1.property("youngs_modulus", "1");
    tMaterial1.property("poissons_ratio", "0.3");
    XMLGen::MaterialSet tMaterialSetOne;
    tMaterialSetOne.insert({"1", tMaterial1});
    auto tRandomMaterialCaseOne = std::make_pair(0.5, tMaterialSetOne);

    // POSE MATERIAL SET 2
    XMLGen::Material tMaterial2;
    tMaterial2.id("1");
    tMaterial2.materialModel("isotropic_linear_elastic");
    tMaterial2.property("youngs_modulus", "1.1");
    tMaterial2.property("poissons_ratio", "0.33");
    XMLGen::MaterialSet tMaterialSetTwo;
    tMaterialSetTwo.insert({"1", tMaterial2});
    auto tRandomMaterialCaseTwo = std::make_pair(0.5, tMaterialSetTwo);

    // CONSTRUCT SAMPLES SET
    ASSERT_NO_THROW(tXMLMetaData.mRandomMetaData.append(tRandomMaterialCaseOne));
    ASSERT_NO_THROW(tXMLMetaData.mRandomMetaData.append(tRandomMaterialCaseTwo));
    ASSERT_NO_THROW(tXMLMetaData.mRandomMetaData.finalize());

    // CALL FUNCTION WITH RANDOM MATERIAL
    pugi::xml_document tDocument;
    ASSERT_NO_THROW(XMLGen::append_material_model_to_plato_problem(tXMLMetaData.materials, tXMLMetaData, tDocument));

    auto tMaterialModelsList = tDocument.child("ParameterList");
    ASSERT_FALSE(tMaterialModelsList.empty());
    ASSERT_STREQ("ParameterList", tMaterialModelsList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Material Models"}, tMaterialModelsList);

    auto tMaterialModelParamList = tMaterialModelsList.child("ParameterList");
    ASSERT_FALSE(tMaterialModelParamList.empty());
    ASSERT_STREQ("ParameterList", tMaterialModelParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"carbonadium"}, tMaterialModelParamList);
    auto tMyMaterialModel = tMaterialModelParamList.child("ParameterList");
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Isotropic Linear Elastic"}, tMyMaterialModel);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Poissons Ratio", "double", "0.3"}, {"Youngs Modulus", "double", "1"} };
    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tMyMaterialModel.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendPressureScalingToPlatoAnalyzeMaterialModels_Plasticity)
{
    // POSE INPUTS
    XMLGen::InputData tXMLMetaData;

    // CREATE MATERIAL
    XMLGen::Material tMaterial;
    tMaterial.id("1");
    tMaterial.name("carbonadium");
    tMaterial.materialModel("j2_plasticity");
    tMaterial.property("youngs_modulus", "2.7");
    tMaterial.property("poissons_ratio", "0.3");
    tMaterial.property("initial_yield_stress", "1.5");
    tMaterial.property("hardening_modulus_isotropic", "2.0");
    tMaterial.property("hardening_modulus_kinematic", "2.0");
    tMaterial.property("elastic_properties_penalty_exponent", "2.0");
    tMaterial.property("elastic_properties_minimum_ersatz", "2.0");
    tMaterial.property("plastic_properties_penalty_exponent", "2.0");
    tMaterial.property("plastic_properties_minimum_ersatz", "2.0");
    tXMLMetaData.materials.push_back(tMaterial);

    XMLGen::Scenario tScenario;
    tScenario.id("1");
    tScenario.physics("plasticity");
    tScenario.pressureScaling("2.5");
    tXMLMetaData.append(tScenario);

    pugi::xml_document tDocument;
    XMLGen::append_material_model_to_plato_problem(tXMLMetaData.materials, tXMLMetaData, tDocument);
    //tDocument.save_file("dummy.xml", "    >     ");
    // TEST STARTS
    auto tMaterialModelsList = tDocument.child("ParameterList");
    ASSERT_FALSE(tMaterialModelsList.empty());
    ASSERT_STREQ("ParameterList", tMaterialModelsList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Material Models"}, tMaterialModelsList);

    // CHECK PRESSURE SCALING
    auto tParameter = tMaterialModelsList.child("Parameter");
    std::vector<std::string> tGoldKeys   = {"name", "type", "value"};
    std::vector<std::string> tGoldValues = {"Pressure Scaling", "double", "2.5"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    // CHECK MATERIAL SPECIFIC PROPERTIES
    auto tMaterialModelParamList = tMaterialModelsList.child("ParameterList");
    ASSERT_FALSE(tMaterialModelParamList.empty());
    ASSERT_STREQ("ParameterList", tMaterialModelParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"carbonadium"}, tMaterialModelParamList);
    auto tMyMaterialModel1 = tMaterialModelParamList.child("ParameterList");
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Isotropic Linear Elastic"}, tMyMaterialModel1);
    auto tMyMaterialModel2 = tMyMaterialModel1.next_sibling("ParameterList");
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Plasticity Model"}, tMyMaterialModel2);

    auto tMyJ2Model = tMyMaterialModel2.child("ParameterList");
    PlatoTestXMLGenerator::test_attributes({"name"}, {"J2 Plasticity"}, tMyJ2Model);


    tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValuesVector =
        { {"Poissons Ratio", "double", "0.3"}, {"Youngs Modulus", "double", "2.7"} };
    auto tGoldValuesItr = tGoldValuesVector.begin();
    tParameter = tMyMaterialModel1.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }

    tGoldKeys = {"name", "type", "value"};
    tGoldValuesVector =
        { {"Initial Yield Stress", "double", "1.5"}, {"Hardening Modulus Isotropic", "double", "2.0"},
          {"Hardening Modulus Kinematic", "double", "2.1"}, {"Elastic Properties Penalty Exponent", "double", "2.2"},
          {"Elastic Properties Minimum Ersatz", "double", "2.3"}, {"Plastic Properties Penalty Exponent", "double", "2.4"},
          {"Plastic Properties Minimum Ersatz", "double", "2.5"} };
    tGoldValuesItr = tGoldValuesVector.begin();
    tParameter = tMyMaterialModel2.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendPressureAndTemperatureScalingToPlatoAnalyzeMaterialModels_Thermoplasticity)
{
    // POSE INPUTS
    XMLGen::InputData tXMLMetaData;

    // CREATE MATERIAL
    XMLGen::Material tMaterial;
    tMaterial.id("1");
    tMaterial.name("carbonadium");
    tMaterial.materialModel("thermoplasticity");
    tMaterial.property("youngs_modulus", "2.7");
    tMaterial.property("poissons_ratio", "0.3");
    tMaterial.property("thermal_conductivity", "2.1");
    tMaterial.property("thermal_expansivity", "2.2");
    tMaterial.property("reference_temperature", "2.3");
    tMaterial.property("initial_yield_stress", "1.5");
    tMaterial.property("hardening_modulus_isotropic", "2.0");
    tMaterial.property("hardening_modulus_kinematic", "2.0");
    tMaterial.property("elastic_properties_penalty_exponent", "2.0");
    tMaterial.property("elastic_properties_minimum_ersatz", "2.0");
    tMaterial.property("plastic_properties_penalty_exponent", "2.0");
    tMaterial.property("plastic_properties_minimum_ersatz", "2.0");
    tXMLMetaData.materials.push_back(tMaterial);

    XMLGen::Scenario tScenario;
    tScenario.id("1");
    tScenario.physics("thermoplasticity");
    tScenario.pressureScaling("2.5");
    tScenario.temperatureScaling("20.5");
    tXMLMetaData.append(tScenario);

    pugi::xml_document tDocument;
    XMLGen::append_material_model_to_plato_problem(tXMLMetaData.materials, tXMLMetaData, tDocument);
    //tDocument.save_file("dummy.xml", "    >     ");
    // TEST STARTS
    auto tMaterialModelsList = tDocument.child("ParameterList");
    ASSERT_FALSE(tMaterialModelsList.empty());
    ASSERT_STREQ("ParameterList", tMaterialModelsList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Material Models"}, tMaterialModelsList);

    // CHECK PRESSURE SCALING
    auto tParameter = tMaterialModelsList.child("Parameter");
    std::vector<std::string> tGoldKeys   = {"name", "type", "value"};
    std::vector<std::string> tGoldValues = {"Pressure Scaling", "double", "2.5"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    // CHECK TEMPERATURE SCALING
    tParameter = tParameter.next_sibling("Parameter");
    tGoldKeys   = {"name", "type", "value"};
    tGoldValues = {"Temperature Scaling", "double", "20.5"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    // CHECK MATERIAL SPECIFIC PROPERTIES
    auto tMaterialModelParamList = tParameter.next_sibling("ParameterList");
    ASSERT_FALSE(tMaterialModelParamList.empty());
    ASSERT_STREQ("ParameterList", tMaterialModelParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"carbonadium"}, tMaterialModelParamList);
    auto tMyMaterialModel1 = tMaterialModelParamList.child("ParameterList");
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Isotropic Linear Thermoelastic"}, tMyMaterialModel1);

    auto tMyMaterialModel2 = tMyMaterialModel1.next_sibling("ParameterList");
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Plasticity Model"}, tMyMaterialModel2);

    auto tMyJ2Model = tMyMaterialModel2.child("ParameterList");
    PlatoTestXMLGenerator::test_attributes({"name"}, {"J2 Plasticity"}, tMyJ2Model);

    tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValuesVector =
        { {"Poissons Ratio", "double", "0.3"}, {"Youngs Modulus", "double", "2.7"},
          {"Thermal Expansivity", "double", "2.2"}, {"Thermal Conductivity", "double", "2.1"},
          {"Reference Temperature", "double", "2.3"} };
    auto tGoldValuesItr = tGoldValuesVector.begin();
    tParameter = tMyMaterialModel1.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }

    tGoldKeys = {"name", "type", "value"};
    tGoldValuesVector =
        { {"Initial Yield Stress", "double", "1.5"}, {"Hardening Modulus Isotropic", "double", "2.0"},
          {"Hardening Modulus Kinematic", "double", "2.1"}, {"Elastic Properties Penalty Exponent", "double", "2.2"},
          {"Elastic Properties Minimum Ersatz", "double", "2.3"}, {"Plastic Properties Penalty Exponent", "double", "2.4"},
          {"Plastic Properties Minimum Ersatz", "double", "2.5"} };
    tGoldValuesItr = tGoldValuesVector.begin();
    tParameter = tMyMaterialModel2.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendPhysicsToPlatoAnalyzeInputDeck)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::Scenario tScenario;
    tScenario.physics("steady_state_mechanics");
    tXMLMetaData.append(tScenario);
    XMLGen::Output tOutputMetadata;
    tXMLMetaData.mOutputMetaData.push_back(tOutputMetadata);
    ASSERT_NO_THROW(XMLGen::append_physics_to_plato_analyze_input_deck(tXMLMetaData, tDocument));

    auto tPDE = tDocument.child("ParameterList");
    ASSERT_FALSE(tPDE.empty());
    ASSERT_STREQ("ParameterList", tPDE.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Elliptic"}, tPDE);

    auto tPenaltyFunc = tPDE.child("Penalty Function");
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "SIMP"}, {"Exponent", "double", "3.0"}, {"Minimum Value", "double", "1e-9"} };
    auto tGoldValuesItr = tGoldValues.begin();
    auto tParameter = tPenaltyFunc.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendPhysicsToPlatoAnalyzeInputDeck_ErrorInvalidPhysics)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    XMLGen::Scenario tScenario;
    tScenario.physics("computational fluid dynamics");
    tXMLMetaData.append(tScenario);
    ASSERT_THROW(XMLGen::append_physics_to_plato_analyze_input_deck(tXMLMetaData, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendSelfAdjointParameterToPlatoProblem_ErrorInvalidCriterion)
{
    XMLGen::InputData tXMLMetaData;
    XMLGen::Criterion tCriterion;
    tCriterion.type("maximize thrust");
    tCriterion.id("1");
    tXMLMetaData.append(tCriterion);
    tXMLMetaData.objective.criteriaIDs.push_back("1");
    pugi::xml_document tDocument;
    ASSERT_THROW(XMLGen::append_self_adjoint_parameter_to_plato_problem(tXMLMetaData, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendPDEConstraintParameterToPlatoProblem_ErrorInvalidPhysics)
{
    XMLGen::InputData tXMLMetaData;
    XMLGen::Scenario tScenario;
    tScenario.physics("computational fluid dynamics");
    tXMLMetaData.append(tScenario);
    pugi::xml_document tDocument;
    ASSERT_THROW(XMLGen::append_pde_constraint_parameter_to_plato_problem(tXMLMetaData, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendPhysicsParameterToPlatoProblem_ErrorInvalidPhysics)
{
    XMLGen::InputData tXMLMetaData;
    XMLGen::Scenario tScenario;
    tScenario.physics("computational fluid dynamics");
    tXMLMetaData.append(tScenario);
    pugi::xml_document tDocument;
    ASSERT_THROW(XMLGen::append_physics_parameter_to_plato_problem(tXMLMetaData, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendProblemDescriptionToPlatoAnalyzeInputDeck_ErrorInvalidSpatialDim)
{
    XMLGen::InputData tXMLMetaData;
    XMLGen::Scenario tScenario;
    tScenario.dimensions("1");
    tXMLMetaData.append(tScenario);
    pugi::xml_document tDocument;
    ASSERT_THROW(XMLGen::append_problem_description_to_plato_analyze_input_deck(tXMLMetaData, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendProblemDescriptionToPlatoAnalyzeInputDeck_ErrorEmptyMeshFile)
{
    XMLGen::InputData tXMLMetaData;
    XMLGen::Scenario tScenario;
    tScenario.dimensions("2");
    tXMLMetaData.append(tScenario);
    pugi::xml_document tDocument;
    ASSERT_THROW(XMLGen::append_problem_description_to_plato_analyze_input_deck(tXMLMetaData, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendObjectiveCriteriaToPlatoProblem_StressConstraintGeneral)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion;
    tCriterion.id("1");
    tCriterion.type("stress_constraint_general");
    tCriterion.append("stress_limit", "2.0");
    tXMLMetaData.append(tCriterion);
    
    XMLGen::Service tService;
    tService.id("1");
    tService.code("plato_analyze");
    tXMLMetaData.append(tService);

    XMLGen::Objective tObjective;
    tObjective.criteriaIDs.push_back("1");
    tObjective.serviceIDs.push_back("1");
    tXMLMetaData.objective = tObjective;

    pugi::xml_document tDocument;
    XMLGen::append_objective_criteria_to_plato_problem(tXMLMetaData, tDocument);

    // TEST MY OBJECTIVE
    auto tParamList = tDocument.child("ParameterList");
    ASSERT_FALSE(tParamList.empty());
    ASSERT_STREQ("ParameterList", tParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"my_stress_constraint_general_criterion_id_1"}, tParamList);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        {
          {"Type", "string", "Scalar Function"},
          {"Scalar Function Type", "string", "Stress Constraint General"},
          {"Stress Limit", "double", "2.0"},
          {"Mass Criterion Weight", "double", "1.0"},
          {"Stress Criterion Weight", "double", "1.0"}
        };

    auto tParam = tParamList.child("Parameter");
    auto tValuesItr = tGoldValues.begin();
    while(!tParam.empty())
    {
        ASSERT_FALSE(tParam.empty());
        ASSERT_STREQ("Parameter", tParam.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tValuesItr.operator*(), tParam);
        tParam = tParam.next_sibling();
        std::advance(tValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendObjectiveCriteriaToPlatoProblem_StressConstraintQuadratic)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion;
    tCriterion.id("1");
    tCriterion.type("stress_constraint_quadratic");
    tCriterion.append("stress_limit", "3.0");
    tCriterion.append("local_measure", "vonmises");
    tCriterion.append("material_penalty_exponent", "2.0");
    tCriterion.append("minimum_ersatz_material_value", "1.0e-8");
    tXMLMetaData.append(tCriterion);
    
    XMLGen::Service tService;
    tService.id("1");
    tService.code("plato_analyze");
    tXMLMetaData.append(tService);

    XMLGen::Objective tObjective;
    tObjective.criteriaIDs.push_back("1");
    tObjective.serviceIDs.push_back("1");
    tXMLMetaData.objective = tObjective;

    pugi::xml_document tDocument;
    XMLGen::append_objective_criteria_to_plato_problem(tXMLMetaData, tDocument);

    // TEST MY OBJECTIVE
    auto tParamList = tDocument.child("ParameterList");
    ASSERT_FALSE(tParamList.empty());
    ASSERT_STREQ("ParameterList", tParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"my_stress_constraint_quadratic_criterion_id_1"}, tParamList);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        {
          {"Type", "string", "Scalar Function"},
          {"Scalar Function Type", "string", "Stress Constraint Quadratic"},
          {"Local Measure", "string", "vonmises"},
          {"Local Measure Limit", "double", "3.0"},
          {"SIMP Penalty", "double", "2.0"},
          {"Min. Ersatz Material", "double", "1.0e-8"}
        };

    auto tParam = tParamList.child("Parameter");
    auto tValuesItr = tGoldValues.begin();
    while(!tParam.empty())
    {
        ASSERT_FALSE(tParam.empty());
        ASSERT_STREQ("Parameter", tParam.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tValuesItr.operator*(), tParam);
        tParam = tParam.next_sibling();
        std::advance(tValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendObjectiveCriteriaToCriteriaList)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion;
    tCriterion.type("mechanical_compliance");
    tCriterion.id("1");
    tCriterion.materialPenaltyExponent("1.0");
    tCriterion.minErsatzMaterialConstant("0.0");
    tXMLMetaData.append(tCriterion);
    tCriterion.type("thermal_compliance");
    tCriterion.id("2");
    tCriterion.materialPenaltyExponent("1.0");
    tCriterion.minErsatzMaterialConstant("0.0");
    tXMLMetaData.append(tCriterion);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);
    tService.code("plato_analyze");
    tService.id("2");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("steady_state_mechanics");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);
    tScenario.physics("steady_state_thermal");
    tScenario.id("2");
    tXMLMetaData.append(tScenario);

    tXMLMetaData.objective.scenarioIDs.push_back("1");
    tXMLMetaData.objective.scenarioIDs.push_back("2");
    tXMLMetaData.objective.criteriaIDs.push_back("1");
    tXMLMetaData.objective.criteriaIDs.push_back("2");
    tXMLMetaData.objective.serviceIDs.push_back("1");
    tXMLMetaData.objective.serviceIDs.push_back("2");
    tXMLMetaData.objective.weights.push_back("1.0");
    tXMLMetaData.objective.weights.push_back("1.0");

    pugi::xml_document tDocument;
    auto tCriteriaList = tDocument.append_child("ParameterList");
    XMLGen::append_objective_criteria_to_criteria_list(tXMLMetaData, tCriteriaList);
    //tDocument.save_file("dummy.xml");

    // TEST MY OBJECTIVE
    auto tParamList = tCriteriaList.child("ParameterList");
    ASSERT_FALSE(tParamList.empty());
    ASSERT_STREQ("ParameterList", tParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"My Objective"}, tParamList);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Type", "string", "Weighted Sum"}, {"Functions", "Array(string)", "{my_mechanical_compliance_criterion_id_1,my_thermal_compliance_criterion_id_2}"}, {"Weights", "Array(double)", "{1.0,1.0}"} };
    auto tGoldValuesItr = tGoldValues.begin();

    auto tChild = tParamList.child("Parameter");
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ("Parameter", tChild.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }

    // TEST MY OBJECTIVE 1 - 'my maximize stiffness'
    tParamList = tParamList.next_sibling("ParameterList");
    ASSERT_FALSE(tParamList.empty());
    ASSERT_STREQ("ParameterList", tParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"my_mechanical_compliance_criterion_id_1"}, tParamList);
    tGoldValues = { {"Type", "string", "Scalar Function"}, {"Scalar Function Type", "string", "Internal Elastic Energy"}, {} };
    tGoldValuesItr = tGoldValues.begin();

    tChild = tParamList.child("Parameter");
    std::vector<std::string> tGoldChildName = {"Parameter", "Parameter", "ParameterList"};
    auto tGoldChildItr = tGoldChildName.begin();
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ(tGoldChildItr->c_str(), tChild.name());
        if (tGoldChildItr->compare("Parameter") == 0)
        {
            // TEST PARAMETER CHILDREN, SKIP PENALTY FUNCTION CHILDREN (TEST BELOW)
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        }
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
        std::advance(tGoldChildItr, 1);
    }

    auto tPenaltyModel = tParamList.child("Penalty Function");
    tGoldValues = { {"Type", "string", "SIMP"}, {"Exponent", "double", "3.0"}, {"Minimum Value", "double", "1e-9"} };
    tGoldValuesItr = tGoldValues.begin();
    tChild = tPenaltyModel.child("Parameter");
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ("Parameter", tChild.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }

    // TEST MY OBJECTIVE 2 - 'my thermal_compliance'
    tParamList = tParamList.next_sibling("ParameterList");
    ASSERT_FALSE(tParamList.empty());
    ASSERT_STREQ("ParameterList", tParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"my_thermal_compliance_criterion_id_2"}, tParamList);
    tGoldValues = { {"Type", "string", "Scalar Function"}, {"Scalar Function Type", "string", "Internal Thermal Energy"}, {} };
    tGoldValuesItr = tGoldValues.begin();

    tChild = tParamList.child("Parameter");
    tGoldChildItr = tGoldChildName.begin();
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ(tGoldChildItr->c_str(), tChild.name());
        if (tGoldChildItr->compare("Parameter") == 0)
        {
            // TEST PARAMETER CHILDREN, SKIP PENALTY FUNCTION CHILDREN (TEST BELOW)
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        }
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
        std::advance(tGoldChildItr, 1);
    }

    tPenaltyModel = tParamList.child("Penalty Function");
    tGoldValues = { {"Type", "string", "SIMP"}, {"Exponent", "double", "1.0"}, {"Minimum Value", "double", "0.0"} };
    tGoldValuesItr = tGoldValues.begin();
    tChild = tPenaltyModel.child("Parameter");
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ("Parameter", tChild.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendObjectivePressureMisfitCriteriaToCriteriaList)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tWeightedSum;
    tWeightedSum.id("1");
    tWeightedSum.type("composite");
    tWeightedSum.criterionIDs({"2","3"});
    tWeightedSum.criterionWeights({"1.0","-1.0"});
    tXMLMetaData.append(tWeightedSum);

    XMLGen::Criterion tCriterion1;
    tCriterion1.type("mean_surface_pressure");
    tCriterion1.id("2");
    tCriterion1.append("location_name", "inlet");
    tXMLMetaData.append(tCriterion1);
    XMLGen::Criterion tCriterion2;
    tCriterion2.type("mean_surface_pressure");
    tCriterion2.id("3");
    tCriterion2.append("location_name", "outlet");
    tXMLMetaData.append(tCriterion2);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("incompressible_fluids");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);

    tXMLMetaData.objective.scenarioIDs.push_back("1");
    tXMLMetaData.objective.criteriaIDs.push_back("1");
    tXMLMetaData.objective.serviceIDs.push_back("1");

    pugi::xml_document tDocument;
    XMLGen::append_criteria_list_to_plato_analyze_input_deck(tXMLMetaData, tDocument);
    //tDocument.save_file("dummy.xml");

    // TEST CRITERIA
    auto tCriteria = tDocument.child("ParameterList");
    ASSERT_FALSE(tCriteria.empty());
    ASSERT_STREQ("ParameterList", tCriteria.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Criteria"}, tCriteria);

    // TEST MY OBJECTIVE CRITERION
    auto tCriterion = tCriteria.child("ParameterList");
    ASSERT_FALSE(tCriterion.empty());
    ASSERT_STREQ("ParameterList", tCriterion.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"My Objective"}, tCriterion);

    // TEST PARAMETERS
    auto tParameter = tCriterion.child("Parameter");
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::string> tGoldValues = {"Type", "string", "Weighted Sum"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Functions", "Array(string)", "{my_mean_surface_pressure_criterion_id_2,my_mean_surface_pressure_criterion_id_3}"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Weights", "Array(double)", "{1.0,-1.0}"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    // 2. TEST MY INLET PRESSURE
    tCriterion = tCriterion.next_sibling();
    tParameter = tCriterion.child("Parameter");
    tGoldValues = {"Type", "string", "Scalar Function"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Scalar Function Type", "string", "Mean Surface Pressure"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Sides", "Array(string)", "{inlet}"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    // 2. TEST MY OUTLET PRESSURE
    tCriterion = tCriterion.next_sibling();
    tParameter = tCriterion.child("Parameter");
    tGoldValues = {"Type", "string", "Scalar Function"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Scalar Function Type", "string", "Mean Surface Pressure"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Sides", "Array(string)", "{outlet}"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);
}

TEST(PlatoTestXMLGenerator, AppendObjectiveTemperatureMisfitCriteriaToCriteriaList)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tWeightedSum;
    tWeightedSum.id("1");
    tWeightedSum.type("composite");
    tWeightedSum.criterionIDs({"2","3"});
    tWeightedSum.criterionWeights({"1.0","-1.0"});
    tXMLMetaData.append(tWeightedSum);

    XMLGen::Criterion tCriterion1;
    tCriterion1.type("mean_surface_temperature");
    tCriterion1.id("2");
    tCriterion1.append("location_name", "inlet");
    tXMLMetaData.append(tCriterion1);
    XMLGen::Criterion tCriterion2;
    tCriterion2.type("mean_surface_temperature");
    tCriterion2.id("3");
    tCriterion2.append("location_name", "outlet");
    tXMLMetaData.append(tCriterion2);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("incompressible_fluids");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);

    tXMLMetaData.objective.scenarioIDs.push_back("1");
    tXMLMetaData.objective.criteriaIDs.push_back("1");
    tXMLMetaData.objective.serviceIDs.push_back("1");

    pugi::xml_document tDocument;
    XMLGen::append_criteria_list_to_plato_analyze_input_deck(tXMLMetaData, tDocument);
    //tDocument.save_file("dummy.xml");

    // TEST CRITERIA
    auto tCriteria = tDocument.child("ParameterList");
    ASSERT_FALSE(tCriteria.empty());
    ASSERT_STREQ("ParameterList", tCriteria.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Criteria"}, tCriteria);

    // TEST MY OBJECTIVE CRITERION
    auto tCriterion = tCriteria.child("ParameterList");
    ASSERT_FALSE(tCriterion.empty());
    ASSERT_STREQ("ParameterList", tCriterion.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"My Objective"}, tCriterion);

    // TEST PARAMETERS
    auto tParameter = tCriterion.child("Parameter");
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::string> tGoldValues = {"Type", "string", "Weighted Sum"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Functions", "Array(string)", "{my_mean_surface_temperature_criterion_id_2,my_mean_surface_temperature_criterion_id_3}"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Weights", "Array(double)", "{1.0,-1.0}"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    // 2. TEST MY INLET PRESSURE
    tCriterion = tCriterion.next_sibling();
    tParameter = tCriterion.child("Parameter");
    tGoldValues = {"Type", "string", "Scalar Function"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Scalar Function Type", "string", "Mean Surface Temperature"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Sides", "Array(string)", "{inlet}"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    // 2. TEST MY OUTLET PRESSURE
    tCriterion = tCriterion.next_sibling();
    tParameter = tCriterion.child("Parameter");
    tGoldValues = {"Type", "string", "Scalar Function"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Scalar Function Type", "string", "Mean Surface Temperature"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Sides", "Array(string)", "{outlet}"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);
}

TEST(PlatoTestXMLGenerator, AppendObjectiveThermalFluxCriteriaToCriteriaList_Error)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion1;
    tCriterion1.type("maximize_fluid_thermal_flux");
    tCriterion1.id("1");
    tCriterion1.append("location_name", "flux");
    tXMLMetaData.append(tCriterion1);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("incompressible_fluids");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);

    tXMLMetaData.objective.scenarioIDs.push_back("1");
    tXMLMetaData.objective.criteriaIDs.push_back("1");
    tXMLMetaData.objective.serviceIDs.push_back("1");

    // CONDUCTIVITY RATIOS KEYWORD IS NOT DEFINED
    pugi::xml_document tDocument;
    ASSERT_THROW(XMLGen::append_criteria_list_to_plato_analyze_input_deck(tXMLMetaData, tDocument), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, AppendObjectiveThermalFluxCriteriaToCriteriaList_DefaultConductivityValues)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion1;
    tCriterion1.type("maximize_fluid_thermal_flux");
    tCriterion1.id("1");
    tCriterion1.append("location_name", "flux");
    tCriterion1.append("conductivity_ratios", ""); // use default value of 1.0
    tXMLMetaData.append(tCriterion1);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("incompressible_fluids");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);

    tXMLMetaData.objective.scenarioIDs.push_back("1");
    tXMLMetaData.objective.criteriaIDs.push_back("1");
    tXMLMetaData.objective.serviceIDs.push_back("1");

    pugi::xml_document tDocument;
    XMLGen::append_criteria_list_to_plato_analyze_input_deck(tXMLMetaData, tDocument);
    //tDocument.save_file("dummy.xml");

    // TEST CRITERIA
    auto tCriteria = tDocument.child("ParameterList");
    ASSERT_FALSE(tCriteria.empty());
    ASSERT_STREQ("ParameterList", tCriteria.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Criteria"}, tCriteria);

    // TEST MY OBJECTIVE CRITERION
    auto tCriterion = tCriteria.child("ParameterList");
    ASSERT_FALSE(tCriterion.empty());
    ASSERT_STREQ("ParameterList", tCriterion.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"My Objective"}, tCriterion);

    // TEST PARAMETERS
    auto tParameter = tCriterion.child("Parameter");
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::string> tGoldValues = {"Type", "string", "Scalar Function"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Scalar Function Type", "string", "Thermal Flux"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Sides", "Array(string)", "{flux}"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Conductivity Ratios", "Array(double)", "{-1.0}"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);
    tParameter = tParameter.next_sibling();
    ASSERT_TRUE(tParameter.empty());
}

TEST(PlatoTestXMLGenerator, AppendObjectiveThermalFluxCriteriaToCriteriaList)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion1;
    tCriterion1.type("maximize_fluid_thermal_flux");
    tCriterion1.id("1");
    tCriterion1.append("location_name", "flux");
    tCriterion1.append("conductivity_ratios", "40.0");
    tXMLMetaData.append(tCriterion1);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("incompressible_fluids");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);

    tXMLMetaData.objective.scenarioIDs.push_back("1");
    tXMLMetaData.objective.criteriaIDs.push_back("1");
    tXMLMetaData.objective.serviceIDs.push_back("1");

    pugi::xml_document tDocument;
    XMLGen::append_criteria_list_to_plato_analyze_input_deck(tXMLMetaData, tDocument);
    //tDocument.save_file("dummy.xml");

    // TEST CRITERIA
    auto tCriteria = tDocument.child("ParameterList");
    ASSERT_FALSE(tCriteria.empty());
    ASSERT_STREQ("ParameterList", tCriteria.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Criteria"}, tCriteria);

    // TEST MY OBJECTIVE CRITERION
    auto tCriterion = tCriteria.child("ParameterList");
    ASSERT_FALSE(tCriterion.empty());
    ASSERT_STREQ("ParameterList", tCriterion.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"My Objective"}, tCriterion);

    // TEST PARAMETERS
    auto tParameter = tCriterion.child("Parameter");
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::string> tGoldValues = {"Type", "string", "Scalar Function"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Scalar Function Type", "string", "Thermal Flux"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Sides", "Array(string)", "{flux}"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Conductivity Ratios", "Array(double)", "{-40.0}"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);
    tParameter = tParameter.next_sibling();
    ASSERT_TRUE(tParameter.empty());
}

TEST(PlatoTestXMLGenerator, AppendObjectiveMeanTemperatureToCriteriaList)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion1;
    tCriterion1.type("mean_temperature");
    tCriterion1.id("1");
    tCriterion1.materialPenaltyExponent("3");
    tXMLMetaData.append(tCriterion1);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("incompressible_fluids");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);

    tXMLMetaData.objective.scenarioIDs.push_back("1");
    tXMLMetaData.objective.criteriaIDs.push_back("1");
    tXMLMetaData.objective.serviceIDs.push_back("1");

    pugi::xml_document tDocument;
    XMLGen::append_criteria_list_to_plato_analyze_input_deck(tXMLMetaData, tDocument);
    //tDocument.save_file("dummy.xml");

    // TEST CRITERIA
    auto tCriteria = tDocument.child("ParameterList");
    ASSERT_FALSE(tCriteria.empty());
    ASSERT_STREQ("ParameterList", tCriteria.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Criteria"}, tCriteria);

    // TEST MY OBJECTIVE CRITERION
    auto tCriterion = tCriteria.child("ParameterList");
    ASSERT_FALSE(tCriterion.empty());
    ASSERT_STREQ("ParameterList", tCriterion.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"My Objective"}, tCriterion);

    // TEST PARAMETERS
    auto tParameter = tCriterion.child("Parameter");
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::string> tGoldValues = {"Type", "string", "Scalar Function"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Scalar Function Type", "string", "Mean Temperature"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    // TEST CRITERION
    auto tPenalty = tCriterion.child("ParameterList");
    ASSERT_FALSE(tPenalty.empty());
    ASSERT_STREQ("ParameterList", tPenalty.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Penalty Function"}, tPenalty);

    tParameter = tPenalty.child("Parameter");
    tGoldKeys = {"name", "type", "value"};
    tGoldValues = {"Type", "string", "SIMP"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);
    tParameter = tParameter.next_sibling();
    tGoldValues = {"Exponent", "double", "3"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);
    tParameter = tParameter.next_sibling();
    ASSERT_TRUE(tParameter.empty());
}

TEST(PlatoTestXMLGenerator, AppendObjectiveVolumeCriteriaToCriteriaList_DefaultConductivityValues)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion1;
    tCriterion1.type("volume");
    tCriterion1.id("1");
    tCriterion1.append("location_type", "element_block");
    tCriterion1.append("location_name", "block_1");
    tCriterion1.append("material_penalty_exponent", "3.0");
    tXMLMetaData.append(tCriterion1);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("incompressible_fluids");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);

    tXMLMetaData.objective.scenarioIDs.push_back("1");
    tXMLMetaData.objective.criteriaIDs.push_back("1");
    tXMLMetaData.objective.serviceIDs.push_back("1");

    pugi::xml_document tDocument;
    XMLGen::append_criteria_list_to_plato_analyze_input_deck(tXMLMetaData, tDocument);
    //tDocument.save_file("dummy.xml");

    // TEST CRITERIA
    auto tCriteria = tDocument.child("ParameterList");
    ASSERT_FALSE(tCriteria.empty());
    ASSERT_STREQ("ParameterList", tCriteria.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Criteria"}, tCriteria);

    // TEST MY OBJECTIVE CRITERION
    auto tCriterion = tCriteria.child("ParameterList");
    ASSERT_FALSE(tCriterion.empty());
    ASSERT_STREQ("ParameterList", tCriterion.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"My Objective"}, tCriterion);

    // TEST PARAMETERS
    auto tParameter = tCriterion.child("Parameter");
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::string> tGoldValues = {"Type", "string", "Scalar Function"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Linear", "bool", "true"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Domains", "Array(string)", "{block_1}"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Scalar Function Type", "string", "Volume"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);
    
    auto tPenaltyModel = tCriterion.child("ParameterList");
    ASSERT_FALSE(tPenaltyModel.empty());
    ASSERT_STREQ("ParameterList", tPenaltyModel.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Penalty Function"}, tPenaltyModel);
    
    tParameter = tPenaltyModel.child("Parameter");
    tGoldValues = {"Type", "string", "SIMP"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Exponent", "double", "3.0"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);
    tParameter = tParameter.next_sibling();
    ASSERT_TRUE(tParameter.empty());
}

TEST(PlatoTestXMLGenerator, AppendConstraintCriteriaToCriteriaList)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion;
    tCriterion.type("stress_p-norm");
    tCriterion.id("1");
    tCriterion.pnormExponent("6");
    tCriterion.materialPenaltyExponent("3.0");
    tCriterion.minErsatzMaterialConstant("1e-9");
    tXMLMetaData.append(tCriterion);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("steady_state_mechanics");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);

    XMLGen::Constraint tConstraint;
    tConstraint.scenario("1"); 
    tConstraint.service("1"); 
    tConstraint.id("3"); 
    tConstraint.criterion("1"); 
    tConstraint.weight("0.5"); 
    tXMLMetaData.constraints.push_back(tConstraint);

    pugi::xml_document tDocument;
    auto tCriteriaList = tDocument.append_child("ParameterList");
    XMLGen::append_constraint_criteria_to_criteria_list(tXMLMetaData, tCriteriaList);

    // TEST MY CONSTRAINT
    auto tParamList = tCriteriaList.child("ParameterList");
    ASSERT_FALSE(tParamList.empty());
    ASSERT_STREQ("ParameterList", tParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"my_stress_p-norm_criterion_id_1"}, tParamList);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues = { {"Type", "string", "Scalar Function"}, {"Scalar Function Type", "string", "Stress P-Norm"}, {}, {"Exponent", "double", "6"} };
    auto tGoldValuesItr = tGoldValues.begin();

    auto tChild = tParamList.child("Parameter");
    std::vector<std::string> tGoldChildName = {"Parameter", "Parameter", "ParameterList", "Parameter"};
    auto tGoldChildItr = tGoldChildName.begin();
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ(tGoldChildItr->c_str(), tChild.name());
        if (tGoldChildItr->compare("Parameter") == 0)
        {
            // TEST PARAMETER CHILDREN, SKIP PENALTY FUNCTION CHILDREN (TEST BELOW)
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        }
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
        std::advance(tGoldChildItr, 1);
    }

    auto tPenaltyModel = tParamList.child("Penalty Function");
    tGoldValues = { {"Type", "string", "SIMP"}, {"Exponent", "double", "3.0"}, {"Minimum Value", "double", "1e-9"} };
    tGoldValuesItr = tGoldValues.begin();
    tChild = tPenaltyModel.child("Parameter");
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ("Parameter", tChild.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendPNormVonMisesCriteriaToCriteriaList)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion;
    tCriterion.type("stress_p-norm");
    tCriterion.id("1");
    tCriterion.pnormExponent("20");
    tCriterion.pnormMeasure("vonmises");
    tCriterion.materialPenaltyExponent("3.0");
    tCriterion.minErsatzMaterialConstant("1e-9");
    tXMLMetaData.append(tCriterion);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("steady_state_mechanics");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);

    XMLGen::Constraint tConstraint;
    tConstraint.scenario("1"); 
    tConstraint.service("1"); 
    tConstraint.id("3"); 
    tConstraint.criterion("1"); 
    tConstraint.weight("0.5"); 
    tXMLMetaData.constraints.push_back(tConstraint);

    pugi::xml_document tDocument;
    auto tCriteriaList = tDocument.append_child("ParameterList");
    XMLGen::append_constraint_criteria_to_criteria_list(tXMLMetaData, tCriteriaList);

    // TEST MY CONSTRAINT
    auto tParamList = tCriteriaList.child("ParameterList");
    ASSERT_FALSE(tParamList.empty());
    ASSERT_STREQ("ParameterList", tParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"my_stress_p-norm_criterion_id_1"}, tParamList);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues = { {"Type", "string", "Scalar Function"}, {"Scalar Function Type", "string", "Stress P-Norm"}, {}, {"Exponent", "double", "20"}, {} };
    auto tGoldValuesItr = tGoldValues.begin();

    auto tChild = tParamList.child("Parameter");
    std::vector<std::string> tGoldChildName = {"Parameter", "Parameter", "ParameterList", "Parameter", "ParameterList"};
    auto tGoldChildItr = tGoldChildName.begin();
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ(tGoldChildItr->c_str(), tChild.name());
        if (tGoldChildItr->compare("Parameter") == 0)
        {
            // TEST PARAMETER CHILDREN, SKIP PENALTY FUNCTION CHILDREN (TEST BELOW)
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        }
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
        std::advance(tGoldChildItr, 1);
    }

    auto tPenaltyModel = tParamList.child("ParameterList");
    ASSERT_FALSE(tPenaltyModel.empty());
    ASSERT_STREQ("ParameterList", tPenaltyModel.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Penalty Function"}, tPenaltyModel);

    tGoldValues = { {"Type", "string", "SIMP"}, {"Exponent", "double", "3.0"}, {"Minimum Value", "double", "1e-9"} };
    tGoldValuesItr = tGoldValues.begin();

    tChild = tPenaltyModel.child("Parameter");
    tGoldChildName = {"Parameter", "Parameter", "Parameter"};
    tGoldChildItr = tGoldChildName.begin();
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ(tGoldChildItr->c_str(), tChild.name());
        if (tGoldChildItr->compare("Parameter") == 0)
        {
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        }
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
        std::advance(tGoldChildItr, 1);
    }

    auto tNormalize = tPenaltyModel.next_sibling("ParameterList");
    ASSERT_FALSE(tNormalize.empty());
    ASSERT_STREQ("ParameterList", tNormalize.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Normalize"}, tNormalize);

    tGoldValues = { {"Type", "string", "Von Mises"} };
    tGoldValuesItr = tGoldValues.begin();

    tChild = tNormalize.child("Parameter");
    tGoldChildName = {"Parameter"};
    tGoldChildItr = tGoldChildName.begin();
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ(tGoldChildItr->c_str(), tChild.name());
        if (tGoldChildItr->compare("Parameter") == 0)
        {
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        }
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
        std::advance(tGoldChildItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendPNormVonMisesNoVolumeScalingCriteriaToCriteriaList)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion;
    tCriterion.type("stress_p-norm");
    tCriterion.id("1");
    tCriterion.pnormExponent("20");
    tCriterion.pnormMeasure("vonmises");
    tCriterion.pnormVolumeScaling("false");
    tCriterion.materialPenaltyExponent("3.0");
    tCriterion.minErsatzMaterialConstant("1e-9");
    tXMLMetaData.append(tCriterion);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("steady_state_mechanics");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);

    XMLGen::Constraint tConstraint;
    tConstraint.scenario("1"); 
    tConstraint.service("1"); 
    tConstraint.id("3"); 
    tConstraint.criterion("1"); 
    tConstraint.weight("0.5"); 
    tXMLMetaData.constraints.push_back(tConstraint);

    pugi::xml_document tDocument;
    auto tCriteriaList = tDocument.append_child("ParameterList");
    XMLGen::append_constraint_criteria_to_criteria_list(tXMLMetaData, tCriteriaList);

    // TEST MY CONSTRAINT
    auto tParamList = tCriteriaList.child("ParameterList");
    ASSERT_FALSE(tParamList.empty());
    ASSERT_STREQ("ParameterList", tParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"my_stress_p-norm_criterion_id_1"}, tParamList);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues = { {"Type", "string", "Scalar Function"}, {"Scalar Function Type", "string", "Stress P-Norm"}, {}, {"Exponent", "double", "20"}, {} };
    auto tGoldValuesItr = tGoldValues.begin();

    auto tChild = tParamList.child("Parameter");
    std::vector<std::string> tGoldChildName = {"Parameter", "Parameter", "ParameterList", "Parameter", "ParameterList"};
    auto tGoldChildItr = tGoldChildName.begin();
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ(tGoldChildItr->c_str(), tChild.name());
        if (tGoldChildItr->compare("Parameter") == 0)
        {
            // TEST PARAMETER CHILDREN, SKIP PENALTY FUNCTION CHILDREN (TEST BELOW)
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        }
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
        std::advance(tGoldChildItr, 1);
    }

    auto tPenaltyModel = tParamList.child("ParameterList");
    ASSERT_FALSE(tPenaltyModel.empty());
    ASSERT_STREQ("ParameterList", tPenaltyModel.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Penalty Function"}, tPenaltyModel);

    tGoldValues = { {"Type", "string", "SIMP"}, {"Exponent", "double", "3.0"}, {"Minimum Value", "double", "1e-9"} };
    tGoldValuesItr = tGoldValues.begin();

    tChild = tPenaltyModel.child("Parameter");
    tGoldChildName = {"Parameter", "Parameter", "Parameter"};
    tGoldChildItr = tGoldChildName.begin();
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ(tGoldChildItr->c_str(), tChild.name());
        if (tGoldChildItr->compare("Parameter") == 0)
        {
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        }
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
        std::advance(tGoldChildItr, 1);
    }

    auto tNormalize = tPenaltyModel.next_sibling("ParameterList");
    ASSERT_FALSE(tNormalize.empty());
    ASSERT_STREQ("ParameterList", tNormalize.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Normalize"}, tNormalize);

    tGoldValues = { {"Type", "string", "Von Mises"}, {"Volume Scaling", "bool", "false"} };
    tGoldValuesItr = tGoldValues.begin();

    tChild = tNormalize.child("Parameter");
    tGoldChildName = {"Parameter"};
    tGoldChildItr = tGoldChildName.begin();
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ(tGoldChildItr->c_str(), tChild.name());
        if (tGoldChildItr->compare("Parameter") == 0)
        {
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        }
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
        std::advance(tGoldChildItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendThermomechanicalCriteriaToCriteriaList)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion;
    tCriterion.type("thermomechanical_compliance");
    tCriterion.id("1");
    tCriterion.mechanicalWeightingFactor("0.5");
    tCriterion.thermalWeightingFactor("0.0");
    tCriterion.materialPenaltyExponent("3.0");
    tCriterion.minErsatzMaterialConstant("1e-8");
    tXMLMetaData.append(tCriterion);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("steady_state_thermomechanics");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);

    XMLGen::Constraint tConstraint;
    tConstraint.scenario("1"); 
    tConstraint.service("1"); 
    tConstraint.id("3"); 
    tConstraint.criterion("1"); 
    tConstraint.weight("0.5"); 
    tXMLMetaData.constraints.push_back(tConstraint);

    pugi::xml_document tDocument;
    auto tCriteriaList = tDocument.append_child("ParameterList");
    XMLGen::append_constraint_criteria_to_criteria_list(tXMLMetaData, tCriteriaList);

    // TEST MY CONSTRAINT
    auto tParamList = tCriteriaList.child("ParameterList");
    ASSERT_FALSE(tParamList.empty());
    ASSERT_STREQ("ParameterList", tParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"my_thermomechanical_compliance_criterion_id_1"}, tParamList);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues = { {"Type", "string", "Scalar Function"}, {"Scalar Function Type", "string", "Internal Thermoelastic Energy"}, {}, {"Mechanical Weighting Factor", "double", "0.5"}, {"Thermal Weighting Factor", "double", "0.0"} };
    auto tGoldValuesItr = tGoldValues.begin();

    auto tChild = tParamList.child("Parameter");
    std::vector<std::string> tGoldChildName = {"Parameter", "Parameter", "ParameterList", "Parameter", "Parameter"};
    auto tGoldChildItr = tGoldChildName.begin();
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ(tGoldChildItr->c_str(), tChild.name());
        if (tGoldChildItr->compare("Parameter") == 0)
        {
            // TEST PARAMETER CHILDREN, SKIP PENALTY FUNCTION CHILDREN (TEST BELOW)
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        }
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
        std::advance(tGoldChildItr, 1);
    }

    auto tPenaltyModel = tParamList.child("Penalty Function");
    tGoldValues = { {"Type", "string", "SIMP"}, {"Exponent", "double", "3.0"}, {"Minimum Value", "double", "1e-9"} };
    tGoldValuesItr = tGoldValues.begin();
    tChild = tPenaltyModel.child("Parameter");
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ("Parameter", tChild.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendThermoplasticityElasticWorkCriteriaToCriteriaList)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion;
    tCriterion.type("elastic_work");
    tCriterion.id("1");
    tCriterion.materialPenaltyExponent("1.0");
    tCriterion.minErsatzMaterialConstant("0.0");
    tXMLMetaData.append(tCriterion);
    tCriterion.type("plastic_work");
    tCriterion.id("2");
    tCriterion.materialPenaltyExponent("2.0");
    tCriterion.minErsatzMaterialConstant("0.0");
    tXMLMetaData.append(tCriterion);
    tCriterion.type("total_work");
    tCriterion.id("3");
    tCriterion.materialPenaltyExponent("3.0");
    tCriterion.minErsatzMaterialConstant("0.0");
    tXMLMetaData.append(tCriterion);
    tCriterion.type("thermoplasticity_thermal_energy");
    tCriterion.id("4");
    tCriterion.materialPenaltyExponent("4.0");
    tCriterion.minErsatzMaterialConstant("0.0");
    tXMLMetaData.append(tCriterion);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("thermoplasticity");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);

    tXMLMetaData.objective.scenarioIDs.push_back("1");
    tXMLMetaData.objective.criteriaIDs.push_back("1");
    tXMLMetaData.objective.serviceIDs.push_back("1");
    tXMLMetaData.objective.weights.push_back("1.0");
       

    pugi::xml_document tDocument;
    auto tCriteriaList = tDocument.append_child("ParameterList");
    XMLGen::append_objective_criteria_to_criteria_list(tXMLMetaData, tCriteriaList);

    // TEST MY OBJECTIVE
    auto tParamList = tCriteriaList.child("ParameterList");

    // TEST MY OBJECTIVE 1 - 'my maximize stiffness'
    ASSERT_FALSE(tParamList.empty());
    ASSERT_STREQ("ParameterList", tParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"My Objective"}, tParamList);
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues = { {"Type", "string", "Scalar Function"}, {"Scalar Function Type", "string", "Elastic Work"}, {} };
    auto tGoldValuesItr = tGoldValues.begin();

    auto tChild = tParamList.child("Parameter");
    std::vector<std::string> tGoldChildName = {"Parameter", "Parameter", "ParameterList"};
    auto tGoldChildItr = tGoldChildName.begin();
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ(tGoldChildItr->c_str(), tChild.name());
        if (tGoldChildItr->compare("Parameter") == 0)
        {
            // TEST PARAMETER CHILDREN, SKIP PENALTY FUNCTION CHILDREN (TEST BELOW)
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        }
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
        std::advance(tGoldChildItr, 1);
    }

    auto tPenaltyModel = tParamList.child("Penalty Function");
    tGoldValues = { {"Type", "string", "SIMP"}, {"Exponent", "double", "1.0"}, {"Minimum Value", "double", "0.0"} };
    tGoldValuesItr = tGoldValues.begin();
    tChild = tPenaltyModel.child("Parameter");
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ("Parameter", tChild.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendThermoplasticityPlasticWorkCriteriaToCriteriaList)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion;
    tCriterion.type("plastic_work");
    tCriterion.id("2");
    tCriterion.materialPenaltyExponent("2.0");
    tCriterion.minErsatzMaterialConstant("0.0");
    tXMLMetaData.append(tCriterion);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("thermoplasticity");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);

    tXMLMetaData.objective.scenarioIDs.push_back("1");
    tXMLMetaData.objective.criteriaIDs.push_back("2");
    tXMLMetaData.objective.serviceIDs.push_back("1");
    tXMLMetaData.objective.weights.push_back("1.0");
       

    pugi::xml_document tDocument;
    auto tCriteriaList = tDocument.append_child("ParameterList");
    XMLGen::append_objective_criteria_to_criteria_list(tXMLMetaData, tCriteriaList);

    // TEST MY OBJECTIVE
    auto tParamList = tCriteriaList.child("ParameterList");

    // TEST MY OBJECTIVE 1 - 'my maximize stiffness'
    ASSERT_FALSE(tParamList.empty());
    ASSERT_STREQ("ParameterList", tParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"My Objective"}, tParamList);
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues = { {"Type", "string", "Scalar Function"}, {"Scalar Function Type", "string", "Plastic Work"}, {} };
    auto tGoldValuesItr = tGoldValues.begin();

    auto tChild = tParamList.child("Parameter");
    std::vector<std::string> tGoldChildName = {"Parameter", "Parameter", "ParameterList"};
    auto tGoldChildItr = tGoldChildName.begin();
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ(tGoldChildItr->c_str(), tChild.name());
        if (tGoldChildItr->compare("Parameter") == 0)
        {
            // TEST PARAMETER CHILDREN, SKIP PENALTY FUNCTION CHILDREN (TEST BELOW)
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        }
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
        std::advance(tGoldChildItr, 1);
    }

    auto tPenaltyModel = tParamList.child("Penalty Function");
    tGoldValues = { {"Type", "string", "SIMP"}, {"Exponent", "double", "2.0"}, {"Minimum Value", "double", "0.0"} };
    tGoldValuesItr = tGoldValues.begin();
    tChild = tPenaltyModel.child("Parameter");
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ("Parameter", tChild.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendThermoplasticityTotalWorkCriteriaToCriteriaList)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion;
    tCriterion.type("total_work");
    tCriterion.id("3");
    tCriterion.materialPenaltyExponent("3.0");
    tCriterion.minErsatzMaterialConstant("0.0");
    tXMLMetaData.append(tCriterion);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("thermoplasticity");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);

    tXMLMetaData.objective.scenarioIDs.push_back("1");
    tXMLMetaData.objective.criteriaIDs.push_back("3");
    tXMLMetaData.objective.serviceIDs.push_back("1");
    tXMLMetaData.objective.weights.push_back("1.0");
       

    pugi::xml_document tDocument;
    auto tCriteriaList = tDocument.append_child("ParameterList");
    XMLGen::append_objective_criteria_to_criteria_list(tXMLMetaData, tCriteriaList);

    // TEST MY OBJECTIVE
    auto tParamList = tCriteriaList.child("ParameterList");

    // TEST MY OBJECTIVE 1 - 'my maximize stiffness'
    ASSERT_FALSE(tParamList.empty());
    ASSERT_STREQ("ParameterList", tParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"My Objective"}, tParamList);
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues = { {"Type", "string", "Scalar Function"}, {"Scalar Function Type", "string", "Total Work"}, {} };
    auto tGoldValuesItr = tGoldValues.begin();

    auto tChild = tParamList.child("Parameter");
    std::vector<std::string> tGoldChildName = {"Parameter", "Parameter", "ParameterList"};
    auto tGoldChildItr = tGoldChildName.begin();
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ(tGoldChildItr->c_str(), tChild.name());
        if (tGoldChildItr->compare("Parameter") == 0)
        {
            // TEST PARAMETER CHILDREN, SKIP PENALTY FUNCTION CHILDREN (TEST BELOW)
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        }
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
        std::advance(tGoldChildItr, 1);
    }

    auto tPenaltyModel = tParamList.child("Penalty Function");
    tGoldValues = { {"Type", "string", "SIMP"}, {"Exponent", "double", "3.0"}, {"Minimum Value", "double", "0.0"} };
    tGoldValuesItr = tGoldValues.begin();
    tChild = tPenaltyModel.child("Parameter");
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ("Parameter", tChild.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendThermoplasticityThermalEnergyCriteriaToCriteriaList)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tCriterion;
    tCriterion.type("thermoplasticity_thermal_energy");
    tCriterion.id("4");
    tCriterion.materialPenaltyExponent("4.0");
    tCriterion.minErsatzMaterialConstant("0.0");
    tXMLMetaData.append(tCriterion);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("thermoplasticity");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);

    tXMLMetaData.objective.scenarioIDs.push_back("1");
    tXMLMetaData.objective.criteriaIDs.push_back("4");
    tXMLMetaData.objective.serviceIDs.push_back("1");
    tXMLMetaData.objective.weights.push_back("1.0");
       

    pugi::xml_document tDocument;
    auto tCriteriaList = tDocument.append_child("ParameterList");
    XMLGen::append_objective_criteria_to_criteria_list(tXMLMetaData, tCriteriaList);

    // TEST MY OBJECTIVE
    auto tParamList = tCriteriaList.child("ParameterList");

    // TEST MY OBJECTIVE 1 - 'my maximize stiffness'
    ASSERT_FALSE(tParamList.empty());
    ASSERT_STREQ("ParameterList", tParamList.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"My Objective"}, tParamList);
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues = { {"Type", "string", "Scalar Function"}, {"Scalar Function Type", "string", "Thermal Energy"}, {} };
    auto tGoldValuesItr = tGoldValues.begin();

    auto tChild = tParamList.child("Parameter");
    std::vector<std::string> tGoldChildName = {"Parameter", "Parameter", "ParameterList"};
    auto tGoldChildItr = tGoldChildName.begin();
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ(tGoldChildItr->c_str(), tChild.name());
        if (tGoldChildItr->compare("Parameter") == 0)
        {
            // TEST PARAMETER CHILDREN, SKIP PENALTY FUNCTION CHILDREN (TEST BELOW)
            PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        }
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
        std::advance(tGoldChildItr, 1);
    }

    auto tPenaltyModel = tParamList.child("Penalty Function");
    tGoldValues = { {"Type", "string", "SIMP"}, {"Exponent", "double", "4.0"}, {"Minimum Value", "double", "0.0"} };
    tGoldValuesItr = tGoldValues.begin();
    tChild = tPenaltyModel.child("Parameter");
    while(!tChild.empty())
    {
        ASSERT_FALSE(tChild.empty());
        ASSERT_STREQ("Parameter", tChild.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tChild);
        tChild = tChild.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendProblemDescriptionToPlatoAnalyzeInputDeck)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tXMLMetaData;
    tXMLMetaData.mesh.run_name = "lbracket.exo";
    XMLGen::Scenario tScenario;
    tScenario.dimensions("2");
    tXMLMetaData.append(tScenario);
    ASSERT_NO_THROW(XMLGen::append_problem_description_to_plato_analyze_input_deck(tXMLMetaData, tDocument));

    auto tProblem = tDocument.child("ParameterList");
    ASSERT_FALSE(tProblem.empty());
    ASSERT_STREQ("ParameterList", tProblem.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Problem"}, tProblem);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Physics", "string", "Plato Driver"}, {"Spatial Dimension", "int", "2"}, {"Input Mesh", "string", "lbracket.exo"} };
    auto tGoldValuesItr = tGoldValues.begin();

    auto tParameter = tProblem.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, AppendPlatoProblemToPlatoAnalyzeInputDeck)
{
    XMLGen::InputData tXMLMetaData;
    tXMLMetaData.mesh.run_name = "lbracket.exo";
    XMLGen::Scenario tScenario;
    tScenario.physics("steady_state_mechanics");
    tScenario.dimensions("2");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);
    XMLGen::Criterion tCriterion;
    tCriterion.type("mechanical_compliance");
    tCriterion.id("1");
    tXMLMetaData.append(tCriterion);
    tCriterion.type("volume");
    tCriterion.id("2");
    tXMLMetaData.append(tCriterion);
    XMLGen::Service tService;
    tService.id("1");
    tService.code("plato_analyze");
    tXMLMetaData.append(tService);
    tXMLMetaData.objective.serviceIDs.push_back("1");
    tXMLMetaData.objective.criteriaIDs.push_back("1");
    tXMLMetaData.objective.scenarioIDs.push_back("1");
    XMLGen::Constraint tConstraint;
    tConstraint.service("1");
    tConstraint.criterion("2");
    tXMLMetaData.constraints.push_back(tConstraint);

    pugi::xml_document tDocument;
    auto tProblem = tDocument.append_child("ParameterList");
    XMLGen::append_attributes({"name"}, {"Problem"}, tProblem);
    XMLGen::append_plato_problem_description_to_plato_analyze_input_deck(tXMLMetaData, tDocument);

    tProblem = tDocument.child("ParameterList");
    ASSERT_FALSE(tProblem.empty());
    ASSERT_STREQ("ParameterList", tProblem.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Problem"}, tProblem);

    auto tPlatoProblem = tProblem.child("ParameterList");
    ASSERT_FALSE(tPlatoProblem.empty());
    ASSERT_STREQ("ParameterList", tPlatoProblem.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Plato Problem"}, tPlatoProblem);

    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::vector<std::string>> tGoldValues =
        { {"Physics", "string", "Mechanical"}, {"PDE Constraint", "string", "Elliptic"},
          {"Self-Adjoint", "bool", "true"} };
    auto tGoldValuesItr = tGoldValues.begin();

    auto tParameter = tPlatoProblem.child("Parameter");
    while(!tParameter.empty())
    {
        ASSERT_FALSE(tParameter.empty());
        ASSERT_STREQ("Parameter", tParameter.name());
        PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValuesItr.operator*(), tParameter);
        tParameter = tParameter.next_sibling();
        std::advance(tGoldValuesItr, 1);
    }
}

TEST(PlatoTestXMLGenerator, ErrorHelmholtzFilterLengthScale)
{
    // POSE INPUTS
    XMLGen::InputData tXMLMetaData;

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("filter_radius_scale", "3.0");
    tXMLMetaData.set(tOptimizationParameters);

    ASSERT_THROW(write_plato_analyze_helmholtz_input_deck_file(tXMLMetaData), std::runtime_error);
}

TEST(PlatoTestXMLGenerator, WritePlatoAnalyzeInputXmlFileForHelmholtzFilter)
{
    // POSE INPUTS
    XMLGen::InputData tXMLMetaData;

    tXMLMetaData.mesh.run_name = "lbracket.exo";
    XMLGen::Scenario tScenario;
    tScenario.dimensions("3");
    tXMLMetaData.append(tScenario);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("filter_type", "helmholtz");
    tOptimizationParameters.append("filter_in_engine", "false");
    tOptimizationParameters.append("optimization_type", "topology");
    tOptimizationParameters.append("filter_radius_absolute", "3.2");
    tXMLMetaData.set(tOptimizationParameters);

    XMLGen::Block tBlock;
    tBlock.block_id = "1";
    tBlock.element_type = "tet4";
    tBlock.material_id = "1";
    tBlock.name = "block_1";
    tXMLMetaData.blocks.push_back(tBlock);

    XMLGen::Material tMaterial;
    tMaterial.id("1");
    tMaterial.code("plato_analyze");
    tMaterial.name("adamantium");
    tMaterial.materialModel("isotropic linear elastic");
    tMaterial.property("youngs_modulus", "1e9");
    tMaterial.property("poissons_ratio", "0.3");
    tXMLMetaData.materials.push_back(tMaterial);

    XMLGen::Service tService;
    tService.id("helmholtz");
    tService.code("plato_analyze");
    tXMLMetaData.append(tService);

    // CALL FUNCTION
    ASSERT_NO_THROW(XMLGen::write_plato_analyze_helmholtz_input_deck_file(tXMLMetaData));
    auto tData = XMLGen::read_data_from_file("plato_analyze_helmholtz_input_deck.xml");
    auto tGold = std::string("<?xmlversion=\"1.0\"?><ParameterListname=\"Problem\"><Parametername=\"Physics\"type=\"string\"value=\"PlatoDriver\"/><Parametername=\"SpatialDimension\"type=\"int\"value=\"3\"/><Parametername=\"InputMesh\"type=\"string\"value=\"lbracket.exo\"/><ParameterListname=\"PlatoProblem\"><Parametername=\"Physics\"type=\"string\"value=\"HelmholtzFilter\"/><Parametername=\"PDEConstraint\"type=\"string\"value=\"HelmholtzFilter\"/><ParameterListname=\"SpatialModel\"><ParameterListname=\"Domains\"><ParameterListname=\"Block1\"><Parametername=\"ElementBlock\"type=\"string\"value=\"block_1\"/><Parametername=\"MaterialModel\"type=\"string\"value=\"adamantium\"/></ParameterList></ParameterList></ParameterList><ParameterListname=\"Parameters\"><Parametername=\"LengthScale\"type=\"double\"value=\"0.923760\"/><Parametername=\"SurfaceLengthScale\"type=\"double\"value=\"1.0\"/></ParameterList></ParameterList></ParameterList>");
    ASSERT_STREQ(tGold.c_str(), tData.str().c_str());
    Plato::system("rm -f plato_analyze_helmholtz_input_deck.xml");
}

TEST(PlatoTestXMLGenerator, WritePlatoAnalyzeInputXmlFileForHelmholtzFilterWithFixedBlock)
{
    // POSE INPUTS
    XMLGen::InputData tXMLMetaData;

    tXMLMetaData.mesh.run_name = "lbracket.exo";
    XMLGen::Scenario tScenario;
    tScenario.dimensions("3");
    tXMLMetaData.append(tScenario);

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("filter_type", "helmholtz");
    tOptimizationParameters.append("filter_in_engine", "false");
    tOptimizationParameters.append("optimization_type", "topology");
    tOptimizationParameters.append("filter_radius_absolute", "3.2");
    std::vector<std::string> tFixedBlockIds;
    tFixedBlockIds.push_back("2");
    tOptimizationParameters.setFixedBlockIDs(tFixedBlockIds);
    tXMLMetaData.set(tOptimizationParameters);

    XMLGen::Block tBlock1;
    tBlock1.block_id = "1";
    tBlock1.element_type = "tet4";
    tBlock1.material_id = "1";
    tBlock1.name = "block_1";

    XMLGen::Block tBlock2;
    tBlock2.block_id = "2";
    tBlock2.element_type = "tet4";
    tBlock2.material_id = "1";
    tBlock2.name = "block_2";

    tXMLMetaData.blocks.push_back(tBlock1);
    tXMLMetaData.blocks.push_back(tBlock2);

    XMLGen::Material tMaterial;
    tMaterial.id("1");
    tMaterial.code("plato_analyze");
    tMaterial.name("adamantium");
    tMaterial.materialModel("isotropic linear elastic");
    tMaterial.property("youngs_modulus", "1e9");
    tMaterial.property("poissons_ratio", "0.3");
    tXMLMetaData.materials.push_back(tMaterial);

    XMLGen::Service tService;
    tService.id("helmholtz");
    tService.code("plato_analyze");
    tXMLMetaData.append(tService);

    // CALL FUNCTION
    ASSERT_NO_THROW(XMLGen::write_plato_analyze_helmholtz_input_deck_file(tXMLMetaData));
    auto tData = XMLGen::read_data_from_file("plato_analyze_helmholtz_input_deck.xml");
    auto tGold = std::string("<?xmlversion=\"1.0\"?><ParameterListname=\"Problem\"><Parametername=\"Physics\"type=\"string\"value=\"PlatoDriver\"/><Parametername=\"SpatialDimension\"type=\"int\"value=\"3\"/>")
      + "<Parametername=\"InputMesh\"type=\"string\"value=\"lbracket.exo\"/><ParameterListname=\"PlatoProblem\"><Parametername=\"Physics\"type=\"string\"value=\"HelmholtzFilter\"/>"
      + "<Parametername=\"PDEConstraint\"type=\"string\"value=\"HelmholtzFilter\"/><ParameterListname=\"SpatialModel\"><ParameterListname=\"Domains\"><ParameterListname=\"Block1\">"
      + "<Parametername=\"ElementBlock\"type=\"string\"value=\"block_1\"/><Parametername=\"MaterialModel\"type=\"string\"value=\"adamantium\"/></ParameterList><ParameterListname=\"Block2\">"
      + "<Parametername=\"ElementBlock\"type=\"string\"value=\"block_2\"/><Parametername=\"MaterialModel\"type=\"string\"value=\"adamantium\"/></ParameterList></ParameterList></ParameterList>"
      + "<ParameterListname=\"Parameters\"><Parametername=\"LengthScale\"type=\"double\"value=\"0.923760\"/><Parametername=\"SurfaceLengthScale\"type=\"double\"value=\"1.0\"/></ParameterList></ParameterList></ParameterList>";
    ASSERT_STREQ(tGold.c_str(), tData.str().c_str());
    Plato::system("rm -f plato_analyze_helmholtz_input_deck.xml");
}

TEST(PlatoTestXMLGenerator, WritePlatoAnalyzeInputXmlFileForMassProperties_All)
{
    pugi::xml_document tDocument;

    XMLGen::Criterion tCriterion;
    tCriterion.type("mass_properties");
    tCriterion.id("1");
    tCriterion.setMassProperty("Mass", 0.0664246, 1.0);
    tCriterion.setMassProperty("Ixx", 3.7079, 1.0);
    tCriterion.setMassProperty("Iyy", 2.7113, 1.0);
    tCriterion.setMassProperty("Izz", 4.2975, 1.0);
    tCriterion.setMassProperty("CGx", 4.1376, 1.0);
    tCriterion.setMassProperty("CGy", 5.6817, 1.0);
    tCriterion.setMassProperty("CGz", 2.9269, 1.0);

    ASSERT_NO_THROW(XMLGen::Private::append_mass_properties_criterion(tCriterion, tDocument));
    tDocument.save_file("plato_analyze_input_deck.xml");

    auto tData = XMLGen::read_data_from_file("plato_analyze_input_deck.xml");
    auto tGold = std::string("<?xmlversion=\"1.0\"?>"
    "<ParameterListname=\"my_mass_properties_criterion_id_1\">"
    "<Parametername=\"Linear\"type=\"bool\"value=\"true\"/>"
    "<Parametername=\"Type\"type=\"string\"value=\"MassProperties\"/>"
    "<Parametername=\"Properties\"type=\"Array(string)\"value=\"{CGx,CGy,CGz,Ixx,Iyy,Izz,Mass}\"/>"
    "<Parametername=\"GoldValues\"type=\"Array(double)\"value=\"{4.1376,5.6817,2.9269,3.7079,2.7113,4.2975,0.0664246}\"/>"
    "<Parametername=\"Weights\"type=\"Array(double)\"value=\"{1,1,1,1,1,1,1}\"/>"
    "</ParameterList>");
    ASSERT_STREQ(tGold.c_str(), tData.str().c_str());
    Plato::system("rm -f plato_analyze_input_deck.xml");
}

TEST(PlatoTestXMLGenerator, WritePlatoAnalyzeInputXmlFileForDisplacementCriterion)
{
    pugi::xml_document tDocument;

    XMLGen::Criterion tCriterion;
    tCriterion.type("displacement");
    tCriterion.id("3");
    tCriterion.displacementDirection({"-1.0", "0.0", "0.0"});
    tCriterion.append("measure_magnitude", "false");
    tCriterion.append("location_type", "sideset");
    tCriterion.append("location_name", "ss4");

    ASSERT_NO_THROW(XMLGen::Private::append_displacement_criterion(tCriterion, tDocument));
    tDocument.save_file("plato_analyze_input_deck.xml");

    auto tData = XMLGen::read_data_from_file("plato_analyze_input_deck.xml");
    auto tGold = std::string("<?xmlversion=\"1.0\"?>"
    "<ParameterListname=\"my_displacement_criterion_id_3\">"
    "<Parametername=\"Type\"type=\"string\"value=\"Solution\"/>"
    "<Parametername=\"Normal\"type=\"Array(double)\"value=\"{-1.0,0.0,0.0}\"/>"
    "<Parametername=\"Domain\"type=\"string\"value=\"ss4\"/>"
    "<Parametername=\"Magnitude\"type=\"bool\"value=\"false\"/>"
    "</ParameterList>");
    ASSERT_STREQ(tGold.c_str(), tData.str().c_str());
    Plato::system("rm -f plato_analyze_input_deck.xml");
}

TEST(PlatoTestXMLGenerator, WritePlatoAnalyzeInputXmlFileForMassProperties_Subset)
{
    pugi::xml_document tDocument;

    XMLGen::Criterion tCriterion;
    tCriterion.type("mass_properties");
    tCriterion.id("1");
    tCriterion.setMassProperty("Mass", 0.0664246, 1.0);
    tCriterion.setMassProperty("Ixx", 3.7079, 1.0);
    tCriterion.setMassProperty("CGz", 2.9269, 1.0);

    ASSERT_NO_THROW(XMLGen::Private::append_mass_properties_criterion(tCriterion, tDocument));
    tDocument.save_file("plato_analyze_input_deck.xml");

    auto tData = XMLGen::read_data_from_file("plato_analyze_input_deck.xml");
    auto tGold = std::string("<?xmlversion=\"1.0\"?>"
    "<ParameterListname=\"my_mass_properties_criterion_id_1\">"
    "<Parametername=\"Linear\"type=\"bool\"value=\"true\"/>"
    "<Parametername=\"Type\"type=\"string\"value=\"MassProperties\"/>"
    "<Parametername=\"Properties\"type=\"Array(string)\"value=\"{CGz,Ixx,Mass}\"/>"
    "<Parametername=\"GoldValues\"type=\"Array(double)\"value=\"{2.9269,3.7079,0.0664246}\"/>"
    "<Parametername=\"Weights\"type=\"Array(double)\"value=\"{1,1,1}\"/>"
    "</ParameterList>");
    ASSERT_STREQ(tGold.c_str(), tData.str().c_str());
    Plato::system("rm -f plato_analyze_input_deck.xml");
}

TEST(PlatoTestXMLGenerator, AppendObjectiveVolumeAverageCriteriaToCriteriaList)
{
    XMLGen::InputData tXMLMetaData;

    XMLGen::Criterion tWeightedSum;
    tWeightedSum.id("1");
    tWeightedSum.type("composite");
    tWeightedSum.criterionIDs({"2","3"});
    tWeightedSum.criterionWeights({"1.0","1.0"});
    tXMLMetaData.append(tWeightedSum);

    XMLGen::Criterion tCriterion1;
    tCriterion1.type("volume_average");
    tCriterion1.id("2");
    tCriterion1.append("local_measure", "vonmises");
    tCriterion1.append("spatial_weighting_function", "2.0*x+y-z");
    tXMLMetaData.append(tCriterion1);
    XMLGen::Criterion tCriterion2;
    tCriterion2.type("volume_average");
    tCriterion2.id("3");
    tCriterion2.append("local_measure", "tensileenergydensity");
    tCriterion2.append("spatial_weighting_function", "3.0*x*y*z");
    tXMLMetaData.append(tCriterion2);

    XMLGen::Service tService;
    tService.code("plato_analyze");
    tService.id("1");
    tXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.physics("steady_state_mechanical");
    tScenario.id("1");
    tXMLMetaData.append(tScenario);

    tXMLMetaData.objective.scenarioIDs.push_back("1");
    tXMLMetaData.objective.criteriaIDs.push_back("1");
    tXMLMetaData.objective.serviceIDs.push_back("1");

    pugi::xml_document tDocument;
    XMLGen::append_criteria_list_to_plato_analyze_input_deck(tXMLMetaData, tDocument);
    //tDocument.save_file("dummy.xml");

    // TEST CRITERIA
    auto tCriteria = tDocument.child("ParameterList");
    ASSERT_FALSE(tCriteria.empty());
    ASSERT_STREQ("ParameterList", tCriteria.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"Criteria"}, tCriteria);

    // TEST MY OBJECTIVE CRITERION
    auto tCriterion = tCriteria.child("ParameterList");
    ASSERT_FALSE(tCriterion.empty());
    ASSERT_STREQ("ParameterList", tCriterion.name());
    PlatoTestXMLGenerator::test_attributes({"name"}, {"My Objective"}, tCriterion);

    // TEST PARAMETERS
    auto tParameter = tCriterion.child("Parameter");
    std::vector<std::string> tGoldKeys = {"name", "type", "value"};
    std::vector<std::string> tGoldValues = {"Type", "string", "Weighted Sum"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Functions", "Array(string)", "{my_volume_average_criterion_id_2,my_volume_average_criterion_id_3}"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Weights", "Array(double)", "{1.0,1.0}"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    // 2. TEST MY INLET PRESSURE
    tCriterion = tCriterion.next_sibling();
    tParameter = tCriterion.child("Parameter");
    tGoldValues = {"Type", "string", "Volume Average Criterion"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Local Measure", "string", "vonmises"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Function", "string", "2.0*x+y-z"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    // 2. TEST MY OUTLET PRESSURE
    tCriterion = tCriterion.next_sibling();
    tParameter = tCriterion.child("Parameter");
    tGoldValues = {"Type", "string", "Volume Average Criterion"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Local Measure", "string", "tensileenergydensity"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);

    tParameter = tParameter.next_sibling();
    tGoldValues = {"Function", "string", "3.0*x*y*z"};
    PlatoTestXMLGenerator::test_attributes(tGoldKeys, tGoldValues, tParameter);
}

}
// namespace PlatoTestXMLGenerator
