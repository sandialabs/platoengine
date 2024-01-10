/*
 * Plato_Test_PlatoDakotaDriver.cpp
 *
 *  Created on: Jul 27, 2021
 */

#include <gtest/gtest.h>

#include <memory>
#include <iterator>
#include <algorithm>
#include <unordered_map>

// Dakota includes
#include "PRPMultiIndex.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaResponse.hpp"
#include "DakotaVariables.hpp"
#include "ParamResponsePair.hpp"
#include "LibraryEnvironment.hpp"
#include "DirectApplicInterface.hpp"
#include "PluginSerialDirectApplicInterface.hpp"

// trilinos includes
#include <Teuchos_ParameterList.hpp>
#include <Teuchos_SerialDenseVector.hpp>

// Plato includes
#include "Plato_DakotaDriver.hpp"
#include "Plato_DakotaDataMap.hpp"
#include "lightmp.hpp"
#include "Plato_Macros.hpp"
#include "Plato_Parser.hpp"
#include "Plato_DriverInterface.hpp"
#include "Plato_DakotaAppInterface.hpp"
#include "Plato_DakotaAppInterfaceUtilities.hpp"
#include "Plato_OptimizerUtilities.hpp"

namespace TestPlatoDakotaInterface
{

Plato::InputData makeDakotaInputData()
{
    // create criterion value stage
    Plato::InputData tCriterionValueStage("Stage");
    tCriterionValueStage.add<std::string>("StageTag", "criterion_value_0");
    tCriterionValueStage.add<std::string>("StageName", "CriteriaValueEvaluation");

    // add inputs for criterion value stage
    Plato::InputData tInput1("Input");
    tInput1.add<std::string>("Tag", "continuous");
    tInput1.add<std::string>("SharedDataName", "design_parameters_0");
    tCriterionValueStage.add<Plato::InputData>("Input", tInput1);
    Plato::InputData tInput2("Input");
    tInput2.add<std::string>("Tag", "continuous");
    tInput2.add<std::string>("SharedDataName", "design_parameters_1");
    tCriterionValueStage.add<Plato::InputData>("Input", tInput2);

    // add outputs for criterion value stage
    Plato::InputData tOutput1("Output");
    tOutput1.add<std::string>("SharedDataName", "compliance_value_0");
    tCriterionValueStage.add<Plato::InputData>("Output", tOutput1);
    Plato::InputData tOutput2("Output");
    tOutput2.add<std::string>("SharedDataName", "compliance_value_1");
    tCriterionValueStage.add<Plato::InputData>("Output", tOutput2);

    // append criterion value stage
    Plato::InputData tDriver("DakotaDriver");
    tDriver.add<Plato::InputData>("Stage", tCriterionValueStage);

    // create criterion gradient stage
    Plato::InputData tCriterionGradientStage("Stage");
    tCriterionGradientStage.add<std::string>("StageTag", "criterion_gradient_0");
    tCriterionGradientStage.add<std::string>("StageName", "CriteriaGradientEvaluation");

    // add inputs for criterion gradient stage
    tCriterionGradientStage.add<Plato::InputData>("Input", tInput1);
    tCriterionGradientStage.add<Plato::InputData>("Input", tInput2);

    // add outputs for criterion gradient stage
    Plato::InputData tOutputG1("Output");
    tOutputG1.add<std::string>("SharedDataName", "compliance_gradient_0");
    tCriterionGradientStage.add<Plato::InputData>("Output", tOutputG1);
    Plato::InputData tOutputG2("Output");
    tOutputG2.add<std::string>("SharedDataName", "compliance_gradient_1");
    tCriterionGradientStage.add<Plato::InputData>("Output", tOutputG2);

    // append criterion gradient stage
    tDriver.add<Plato::InputData>("Stage", tCriterionGradientStage);

    // append dakota driver stage
    Plato::InputData Inputs("Inputs");
    Inputs.add<Plato::InputData>("DakotaDriver", tDriver);

    return Inputs;
}

// move to free_functions unit tests
// move tokenize (and other free function tests) into their own file

TEST(PlatoTest, DakotaAppInterface_resetEvaluationFlags)
{
    size_t tNumCriteria = 3;
    std::vector<Plato::DakotaEvaluationType> tEvaluationFlags;
    Plato::resetEvaluationFlags(tNumCriteria, tEvaluationFlags);

    EXPECT_EQ(tEvaluationFlags.size(), tNumCriteria);

    for(auto flag : tEvaluationFlags)
        EXPECT_EQ(flag, Plato::DakotaEvaluationType::NO_DATA);
}

TEST(PlatoTest, DakotaAppInterface_setCriterionEvaluationFlagsForPRP)
{
    size_t tNumCriteria = 3;
    std::vector<Plato::DakotaEvaluationType> tEvaluationFlags;
    Plato::resetEvaluationFlags(tNumCriteria, tEvaluationFlags);

    // invalid tASV
    short tASV = 10;
    size_t tCriterionIndex = 0;
    EXPECT_THROW(Plato::setCriterionEvaluationFlagsForPRP(tASV,tCriterionIndex,tEvaluationFlags),std::out_of_range);

    // tASV requesting hessian
    tASV = 4;
    EXPECT_THROW(Plato::setCriterionEvaluationFlagsForPRP(tASV,tCriterionIndex,tEvaluationFlags),std::runtime_error);

    // tCriterionIndex out of range
    tASV = 1;
    tCriterionIndex = 3;
    EXPECT_THROW(Plato::setCriterionEvaluationFlagsForPRP(tASV,tCriterionIndex,tEvaluationFlags),std::out_of_range);

    // set Evaluation Flags
    // existing no data should get set to input value
    Plato::setCriterionEvaluationFlagsForPRP(1,0,tEvaluationFlags);
    EXPECT_EQ(tEvaluationFlags[0], Plato::DakotaEvaluationType::GET_VALUE);
    EXPECT_EQ(tEvaluationFlags[1], Plato::DakotaEvaluationType::NO_DATA);
    EXPECT_EQ(tEvaluationFlags[2], Plato::DakotaEvaluationType::NO_DATA);

    Plato::setCriterionEvaluationFlagsForPRP(2,1,tEvaluationFlags);
    EXPECT_EQ(tEvaluationFlags[0], Plato::DakotaEvaluationType::GET_VALUE);
    EXPECT_EQ(tEvaluationFlags[1], Plato::DakotaEvaluationType::GET_GRADIENT);
    EXPECT_EQ(tEvaluationFlags[2], Plato::DakotaEvaluationType::NO_DATA);

    Plato::setCriterionEvaluationFlagsForPRP(3,2,tEvaluationFlags);
    EXPECT_EQ(tEvaluationFlags[0], Plato::DakotaEvaluationType::GET_VALUE);
    EXPECT_EQ(tEvaluationFlags[1], Plato::DakotaEvaluationType::GET_GRADIENT);
    EXPECT_EQ(tEvaluationFlags[2], Plato::DakotaEvaluationType::GET_GRADIENT_AND_VALUE);

    // redundant evaluation types have no effect
    Plato::setCriterionEvaluationFlagsForPRP(1,0,tEvaluationFlags);
    EXPECT_EQ(tEvaluationFlags[0], Plato::DakotaEvaluationType::GET_VALUE);
    EXPECT_EQ(tEvaluationFlags[1], Plato::DakotaEvaluationType::GET_GRADIENT);
    EXPECT_EQ(tEvaluationFlags[2], Plato::DakotaEvaluationType::GET_GRADIENT_AND_VALUE);

    Plato::setCriterionEvaluationFlagsForPRP(2,1,tEvaluationFlags);
    EXPECT_EQ(tEvaluationFlags[0], Plato::DakotaEvaluationType::GET_VALUE);
    EXPECT_EQ(tEvaluationFlags[1], Plato::DakotaEvaluationType::GET_GRADIENT);
    EXPECT_EQ(tEvaluationFlags[2], Plato::DakotaEvaluationType::GET_GRADIENT_AND_VALUE);

    Plato::setCriterionEvaluationFlagsForPRP(3,2,tEvaluationFlags);
    EXPECT_EQ(tEvaluationFlags[0], Plato::DakotaEvaluationType::GET_VALUE);
    EXPECT_EQ(tEvaluationFlags[1], Plato::DakotaEvaluationType::GET_GRADIENT);
    EXPECT_EQ(tEvaluationFlags[2], Plato::DakotaEvaluationType::GET_GRADIENT_AND_VALUE);

    Plato::setCriterionEvaluationFlagsForPRP(1,2,tEvaluationFlags);
    EXPECT_EQ(tEvaluationFlags[0], Plato::DakotaEvaluationType::GET_VALUE);
    EXPECT_EQ(tEvaluationFlags[1], Plato::DakotaEvaluationType::GET_GRADIENT);
    EXPECT_EQ(tEvaluationFlags[2], Plato::DakotaEvaluationType::GET_GRADIENT_AND_VALUE);

    Plato::setCriterionEvaluationFlagsForPRP(2,2,tEvaluationFlags);
    EXPECT_EQ(tEvaluationFlags[0], Plato::DakotaEvaluationType::GET_VALUE);
    EXPECT_EQ(tEvaluationFlags[1], Plato::DakotaEvaluationType::GET_GRADIENT);
    EXPECT_EQ(tEvaluationFlags[2], Plato::DakotaEvaluationType::GET_GRADIENT_AND_VALUE);

    // evaluation types sum
    Plato::setCriterionEvaluationFlagsForPRP(2,0,tEvaluationFlags);
    EXPECT_EQ(tEvaluationFlags[0], Plato::DakotaEvaluationType::GET_GRADIENT_AND_VALUE);
    EXPECT_EQ(tEvaluationFlags[1], Plato::DakotaEvaluationType::GET_GRADIENT);
    EXPECT_EQ(tEvaluationFlags[2], Plato::DakotaEvaluationType::GET_GRADIENT_AND_VALUE);

    Plato::setCriterionEvaluationFlagsForPRP(1,1,tEvaluationFlags);
    EXPECT_EQ(tEvaluationFlags[0], Plato::DakotaEvaluationType::GET_GRADIENT_AND_VALUE);
    EXPECT_EQ(tEvaluationFlags[1], Plato::DakotaEvaluationType::GET_GRADIENT_AND_VALUE);
    EXPECT_EQ(tEvaluationFlags[2], Plato::DakotaEvaluationType::GET_GRADIENT_AND_VALUE);


    // GET_GRADIENT_AND_VALUE takes precedence
    Plato::resetEvaluationFlags(tNumCriteria, tEvaluationFlags);
    Plato::setCriterionEvaluationFlagsForPRP(1,0,tEvaluationFlags);
    Plato::setCriterionEvaluationFlagsForPRP(2,1,tEvaluationFlags);
    Plato::setCriterionEvaluationFlagsForPRP(3,2,tEvaluationFlags);
    EXPECT_EQ(tEvaluationFlags[0], Plato::DakotaEvaluationType::GET_VALUE);
    EXPECT_EQ(tEvaluationFlags[1], Plato::DakotaEvaluationType::GET_GRADIENT);
    EXPECT_EQ(tEvaluationFlags[2], Plato::DakotaEvaluationType::GET_GRADIENT_AND_VALUE);

    Plato::setCriterionEvaluationFlagsForPRP(3,0,tEvaluationFlags);
    EXPECT_EQ(tEvaluationFlags[0], Plato::DakotaEvaluationType::GET_GRADIENT_AND_VALUE);
    EXPECT_EQ(tEvaluationFlags[1], Plato::DakotaEvaluationType::GET_GRADIENT);
    EXPECT_EQ(tEvaluationFlags[2], Plato::DakotaEvaluationType::GET_GRADIENT_AND_VALUE);
    
    Plato::setCriterionEvaluationFlagsForPRP(3,1,tEvaluationFlags);
    EXPECT_EQ(tEvaluationFlags[0], Plato::DakotaEvaluationType::GET_GRADIENT_AND_VALUE);
    EXPECT_EQ(tEvaluationFlags[1], Plato::DakotaEvaluationType::GET_GRADIENT_AND_VALUE);
    EXPECT_EQ(tEvaluationFlags[2], Plato::DakotaEvaluationType::GET_GRADIENT_AND_VALUE);
}

TEST(PlatoTest, DakotaAppInterface_makeStageTags)
{
    // invalid tASV
    short tASV = 10;
    size_t tCriterionIndex = 0;
    EXPECT_THROW(Plato::makeStageTags(tASV,tCriterionIndex),std::out_of_range);

    // tASV requesting hessian
    tASV = 4;
    EXPECT_THROW(Plato::makeStageTags(tASV,tCriterionIndex),std::runtime_error);

    tASV = 1;
    tCriterionIndex = 100;
    std::vector<std::string> tStageTags = Plato::makeStageTags(tASV,tCriterionIndex);
    EXPECT_EQ(tStageTags.size(),1);
    EXPECT_EQ(tStageTags[0],"criterion_value_100");

    tASV = 2;
    tCriterionIndex = 100;
    tStageTags = Plato::makeStageTags(tASV,tCriterionIndex);
    EXPECT_EQ(tStageTags.size(),1);
    EXPECT_EQ(tStageTags[0],"criterion_gradient_100");

    tASV = 3;
    tCriterionIndex = 100;
    tStageTags = Plato::makeStageTags(tASV,tCriterionIndex);
    EXPECT_EQ(tStageTags.size(),2);
    EXPECT_EQ(tStageTags[0],"criterion_value_100");
    EXPECT_EQ(tStageTags[1],"criterion_gradient_100");
}

TEST(PlatoTest, DakotaAppInterface_allocateStageOutputSharedData)
{
    Plato::InputData tInputData = makeDakotaInputData();

    // construct dakota data map
    Plato::DakotaDataMap tDakotaDataMap(tInputData);

    std::string tStageTag = "fake_stage_tag";
    size_t tPRPIndex = 0;
    Teuchos::ParameterList tParameterList;

    EXPECT_THROW(Plato::allocateStageOutputSharedData(tStageTag, tPRPIndex, tDakotaDataMap, tParameterList),std::runtime_error);

    // attempt to set stage output data for out of range prpindex
    tStageTag = "criterion_value_0";
    tPRPIndex = 3;
    EXPECT_THROW(Plato::allocateStageOutputSharedData(tStageTag, tPRPIndex, tDakotaDataMap, tParameterList),std::out_of_range);

    // set values as intended
    tStageTag = "criterion_value_0";
    tPRPIndex = 0;
    Plato::allocateStageOutputSharedData(tStageTag, tPRPIndex, tDakotaDataMap, tParameterList);

    tPRPIndex = 1;
    Plato::allocateStageOutputSharedData(tStageTag, tPRPIndex, tDakotaDataMap, tParameterList);

    auto tSharedDataNames = tDakotaDataMap.getOutputSharedDataNames(tStageTag);
    auto tMetaData0 = tDakotaDataMap.getOutputVarsSharedData(tStageTag,0);
    auto tMetaData1 = tDakotaDataMap.getOutputVarsSharedData(tStageTag,1);

    EXPECT_EQ(tMetaData0.mName, tSharedDataNames[0]);
    EXPECT_EQ(tMetaData0.mValues.size(), 1);
    EXPECT_EQ(tMetaData0.mValues[0], 0.0);

    EXPECT_EQ(tMetaData1.mName, tSharedDataNames[1]);
    EXPECT_EQ(tMetaData1.mValues.size(), 1);
    EXPECT_EQ(tMetaData1.mValues[0], 0.0);
}

TEST(PlatoTest, DakotaAppInterface_getStageNamesForEvaluation)
{
    auto tInputData = makeDakotaInputData();

    Plato::InputData tInitializeStage("Stage");
    tInitializeStage.add<std::string>("StageTag", "initialize");
    tInitializeStage.add<std::string>("StageName", "initialize_stage_name");
    auto tDakotaDriver = tInputData.get<Plato::InputData>("DakotaDriver");
    tDakotaDriver.add<Plato::InputData>("Stage",tInitializeStage);

    Plato::InputData tFinalizeStage("Stage");
    tFinalizeStage.add<std::string>("StageTag", "finalize");
    tFinalizeStage.add<std::string>("StageName", "finalize_stage_name");
    tDakotaDriver.add<Plato::InputData>("Stage",tFinalizeStage);

    Plato::DakotaDataMap tDataMap(tInputData);
    const std::vector<std::string> tStageNames = setStageNamesForEvaluation(tDataMap);

    EXPECT_EQ(tStageNames.size(), 4);
    EXPECT_NE(std::find(tStageNames.cbegin(), tStageNames.cend(), "initialize_stage_name"), tStageNames.cend());
    EXPECT_NE(std::find(tStageNames.cbegin(), tStageNames.cend(), "CriteriaValueEvaluation"), tStageNames.cend());
    EXPECT_NE(std::find(tStageNames.cbegin(), tStageNames.cend(), "CriteriaGradientEvaluation"), tStageNames.cend());
    EXPECT_NE(std::find(tStageNames.cbegin(), tStageNames.cend(), "finalize_stage_name"), tStageNames.cend());
}

TEST(PlatoTest, DakotaAppInterface_setValueOutputs)
{
    Plato::InputData tInputData = makeDakotaInputData();

    // construct dakota data map
    Plato::DakotaDataMap tDakotaDataMap(tInputData);

    // set metadata values
    Plato::dakota::SharedDataMetaData tMetaData;
    tMetaData.mName = "compliance_value_0";
    std::vector<double> tValues = {1000.01};
    tMetaData.mValues = tValues;
    size_t tOutputIndex = 0;
    tDakotaDataMap.setOutputVarsSharedData("criterion_value_0",tMetaData,tOutputIndex);

    tValues = {-234.5};
    tMetaData.mValues = tValues;
    tOutputIndex = 1;
    tDakotaDataMap.setOutputVarsSharedData("criterion_value_0",tMetaData,tOutputIndex);

    // create vector for outputs
    size_t tNumCriteria = 1;
    Teuchos::SerialDenseVector<int, double> tDakotaValues(tNumCriteria);
    Teuchos::SerialDenseVector<int, double> tDakotaValuesLong(tNumCriteria+1);

    // criterion index doesn't have output data allocated
    size_t tCriterionIndex = 1;
    size_t tPrpIndex = 0;
    EXPECT_THROW(setValueOutputs(tCriterionIndex,tPrpIndex,tDakotaValuesLong,tDakotaDataMap),std::runtime_error);

    // prp index out of range of output data
    tCriterionIndex = 0;
    tPrpIndex = 2;
    EXPECT_THROW(setValueOutputs(tCriterionIndex,tPrpIndex,tDakotaValues,tDakotaDataMap),std::out_of_range);

    // criterion index out of range of dakota values
    tCriterionIndex = 1;
    tPrpIndex = 1;
    EXPECT_THROW(setValueOutputs(tCriterionIndex,tPrpIndex,tDakotaValues,tDakotaDataMap),std::out_of_range);

    // output data correctly assigned
    tCriterionIndex = 0;
    tPrpIndex = 0;
    setValueOutputs(tCriterionIndex,tPrpIndex,tDakotaValues,tDakotaDataMap);
    EXPECT_EQ(tDakotaValues[0], 1000.01);

    tPrpIndex = 1;
    setValueOutputs(tCriterionIndex,tPrpIndex,tDakotaValues,tDakotaDataMap);
    EXPECT_EQ(tDakotaValues[0], -234.5);
}

TEST(PlatoTest, DakotaAppInterface_setGradientOutputs)
{
    Plato::InputData tInputData = makeDakotaInputData();

    // construct dakota data map
    Plato::DakotaDataMap tDakotaDataMap(tInputData);

    // set metadata values
    Plato::dakota::SharedDataMetaData tMetaData;
    tMetaData.mName = "compliance_gradient_0";
    std::vector<double> tValues = {1000.01, 23.4, -99};
    tMetaData.mValues = tValues;
    size_t tOutputIndex = 0;
    tDakotaDataMap.setOutputVarsSharedData("criterion_gradient_0",tMetaData,tOutputIndex);

    tValues = {-234.5, 0.001, 65.7};
    tMetaData.mValues = tValues;
    tOutputIndex = 1;
    tDakotaDataMap.setOutputVarsSharedData("criterion_gradient_0",tMetaData,tOutputIndex);

    // create matrix for outputs
    size_t tNumCriteria = 1;
    size_t tGradientLength = 3;
    Teuchos::SerialDenseMatrix<int, double> tDakotaValues(tNumCriteria,tGradientLength);
    Teuchos::SerialDenseMatrix<int, double> tDakotaValuesLongRows(tNumCriteria+1,tGradientLength);
    Teuchos::SerialDenseMatrix<int, double> tDakotaValuesLongCols(tNumCriteria,tGradientLength+1);

    // criterion index doesn't have output data allocated
    size_t tCriterionIndex = 1;
    size_t tPrpIndex = 0;
    EXPECT_THROW(setGradientOutputs(tCriterionIndex,tPrpIndex,tDakotaValuesLongRows,tGradientLength,tDakotaDataMap),std::runtime_error);

    // prp index out of range of output data
    tCriterionIndex = 0;
    tPrpIndex = 2;
    EXPECT_THROW(setGradientOutputs(tCriterionIndex,tPrpIndex,tDakotaValues,tGradientLength,tDakotaDataMap),std::out_of_range);

    // criterion index out of range of dakota values
    tCriterionIndex = 2;
    tPrpIndex = 1;
    EXPECT_THROW(setGradientOutputs(tCriterionIndex,tPrpIndex,tDakotaValues,tGradientLength,tDakotaDataMap),std::out_of_range);

    // gradient length larger than dakota values gradient length
    tCriterionIndex = 0;
    tPrpIndex = 0;
    tGradientLength = 4;
    EXPECT_THROW(setGradientOutputs(tCriterionIndex,tPrpIndex,tDakotaValues,tGradientLength,tDakotaDataMap),std::out_of_range);

    // gradient length larger than stored output gradient length
    tCriterionIndex = 0;
    tPrpIndex = 0;
    tGradientLength = 4;
    EXPECT_THROW(setGradientOutputs(tCriterionIndex,tPrpIndex,tDakotaValuesLongCols,tGradientLength,tDakotaDataMap),std::out_of_range);

    // output data correctly assigned
    tCriterionIndex = 0;
    tPrpIndex = 0;
    tGradientLength = 3;
    setGradientOutputs(tCriterionIndex,tPrpIndex,tDakotaValues,tGradientLength,tDakotaDataMap);
    EXPECT_EQ(tDakotaValues[0][0], 1000.01);
    EXPECT_EQ(tDakotaValues[0][1], 23.4);
    EXPECT_EQ(tDakotaValues[0][2], -99);

    tPrpIndex = 1;
    setGradientOutputs(tCriterionIndex,tPrpIndex,tDakotaValues,tGradientLength,tDakotaDataMap);
    EXPECT_EQ(tDakotaValues[0][0], -234.5);
    EXPECT_EQ(tDakotaValues[0][1], 0.001);
    EXPECT_EQ(tDakotaValues[0][2], 65.7);
}

TEST(PlatoTest, DakotaDataMap_parseInputs)
{
    // create criterion value stage
    Plato::InputData tCriterionValueStage("Stage");
    tCriterionValueStage.add<std::string>("StageTag", "criterion_value_0");
    tCriterionValueStage.add<std::string>("StageName", "CriteriaValueEvaluation");

    // add inputs for criterion value stage
    Plato::InputData tInput1("Input");
    tInput1.add<std::string>("Tag", "continuous");
    tInput1.add<std::string>("SharedDataName", "design_parameters_0");
    tCriterionValueStage.add<Plato::InputData>("Input", tInput1);
    Plato::InputData tInput2("Input");
    tInput2.add<std::string>("Tag", "continuous");
    tInput2.add<std::string>("SharedDataName", "design_parameters_1");
    tCriterionValueStage.add<Plato::InputData>("Input", tInput2);

    // add outputs for criterion value stage
    Plato::InputData tOutput1("Output");
    tOutput1.add<std::string>("SharedDataName", "compliance_value_0");
    tCriterionValueStage.add<Plato::InputData>("Output", tOutput1);
    Plato::InputData tOutput2("Output");
    tOutput2.add<std::string>("SharedDataName", "compliance_value_1");
    tCriterionValueStage.add<Plato::InputData>("Output", tOutput2);

    // append criterion value stage
    Plato::InputData tDriver("DakotaDriver");
    tDriver.add<Plato::InputData>("Stage", tCriterionValueStage);

    // create criterion gradient stage
    Plato::InputData tCriterionGradientStage("Stage");
    tCriterionGradientStage.add<std::string>("StageTag", "criterion_gradient_0");
    tCriterionGradientStage.add<std::string>("StageName", "CriteriaGradientEvaluation");

    // add inputs for criterion gradient stage
    tCriterionGradientStage.add<Plato::InputData>("Input", tInput1);
    tCriterionGradientStage.add<Plato::InputData>("Input", tInput2);

    // add outputs for criterion gradient stage
    Plato::InputData tOutputG1("Output");
    tOutputG1.add<std::string>("SharedDataName", "compliance_gradient_0");
    tCriterionGradientStage.add<Plato::InputData>("Output", tOutputG1);
    Plato::InputData tOutputG2("Output");
    tOutputG2.add<std::string>("SharedDataName", "compliance_gradient_1");
    tCriterionGradientStage.add<Plato::InputData>("Output", tOutputG2);

    // append criterion gradient stage
    tDriver.add<Plato::InputData>("Stage", tCriterionGradientStage);

    // append dakota driver stage
    Plato::InputData Inputs("Inputs");
    Inputs.add<Plato::InputData>("DakotaDriver", tDriver);

    // construct dakota data map
    Plato::DakotaDataMap tDakotaDataMap(Inputs);

    // test parsing of stage tags
    auto tStageTags = tDakotaDataMap.getStageTags();
    EXPECT_EQ(2u, tStageTags.size());
    std::vector<std::string> tGoldStageTags = {"criterion_value_0", "criterion_gradient_0"};
    for(auto& tStageTag : tStageTags)
    {
        auto tItr = std::find(tGoldStageTags.begin(), tGoldStageTags.end(), tStageTag);
        if(tItr == tGoldStageTags.end())
        {
            ASSERT_STREQ(tStageTag.c_str(), "FAIL - DID NOT FIND VALUE IN GOLD SET");
        }
        EXPECT_STREQ(tItr.operator*().c_str(), tStageTag.c_str());
    }

    // test parsing of stage names
    auto tStageName = tDakotaDataMap.getStageName("made_up_stage_name");
    EXPECT_STREQ("", tStageName.c_str());

    tStageName = tDakotaDataMap.getStageName("criterion_value_0");
    EXPECT_STREQ("CriteriaValueEvaluation", tStageName.c_str());

    tStageName = tDakotaDataMap.getStageName("criterion_gradient_0");
    EXPECT_STREQ("CriteriaGradientEvaluation", tStageName.c_str());

    // test parsing of inputs
    bool tHasInputs = tDakotaDataMap.stageHasInputSharedData("criterion_value_0");
    EXPECT_EQ(tHasInputs, true);
    auto tInputSharedDataNames = tDakotaDataMap.getInputSharedDataNames("criterion_value_0", "continuous"); 
    std::vector<std::string> tGoldInputNames = {"design_parameters_0", "design_parameters_1"};
    for(auto& tName : tInputSharedDataNames)
    {
        auto tIndex = &tName - &tInputSharedDataNames[0];
        EXPECT_STREQ(tGoldInputNames[tIndex].c_str(), tName.c_str());
    }

    tHasInputs = tDakotaDataMap.stageHasInputSharedData("criterion_gradient_0");
    EXPECT_EQ(tHasInputs, true);
    tInputSharedDataNames = tDakotaDataMap.getInputSharedDataNames("criterion_gradient_0", "continuous"); 
    tGoldInputNames = {"design_parameters_0", "design_parameters_1"};
    for(auto& tName : tInputSharedDataNames)
    {
        auto tIndex = &tName - &tInputSharedDataNames[0];
        EXPECT_STREQ(tGoldInputNames[tIndex].c_str(), tName.c_str());
    }

    // test parsing of outputs
    bool tHasOutputs = tDakotaDataMap.stageHasOutputSharedData("criterion_value_0");
    EXPECT_EQ(tHasOutputs, true);
    auto tOutputSharedDataNames = tDakotaDataMap.getOutputSharedDataNames("criterion_value_0");
    std::vector<std::string> tGoldOutputNames = {"compliance_value_0", "compliance_value_1"};
    for(auto& tName : tOutputSharedDataNames)
    {
        auto tIndex = &tName - &tOutputSharedDataNames[0];
        EXPECT_STREQ(tGoldOutputNames[tIndex].c_str(), tName.c_str());
    }

    tHasOutputs = tDakotaDataMap.stageHasOutputSharedData("criterion_gradient_0");
    EXPECT_EQ(tHasOutputs, true);
    tOutputSharedDataNames = tDakotaDataMap.getOutputSharedDataNames("criterion_gradient_0");
    tGoldOutputNames = {"compliance_gradient_0", "compliance_gradient_1"};
    for(auto& tName : tOutputSharedDataNames)
    {
        auto tIndex = &tName - &tOutputSharedDataNames[0];
        EXPECT_STREQ(tGoldOutputNames[tIndex].c_str(), tName.c_str());
    }
}

TEST(PlatoTest, DakotaDataMap_accessContinuousVars)
{
    // create initialize stage
    Plato::InputData tInitializeStage("Stage");
    tInitializeStage.add<std::string>("StageTag", "initialize");
    tInitializeStage.add<std::string>("StageName", "GenerateMeshes");
    
    // add inputs for initialize stage
    Plato::InputData tInput1("Input");
    tInput1.add<std::string>("Tag", "continuous");
    tInput1.add<std::string>("SharedDataName", "design_parameters_0");
    tInitializeStage.add<Plato::InputData>("Input", tInput1);

    // append initialize stage
    Plato::InputData tDriver("DakotaDriver");
    tDriver.add<Plato::InputData>("Stage", tInitializeStage);

    // create criterion value stage
    Plato::InputData tCriterionValueStage("Stage");
    tCriterionValueStage.add<std::string>("StageTag", "criterion_value_0");
    tCriterionValueStage.add<std::string>("StageName", "CriteriaValueEvaluation");

    // add outputs for criterion value stage
    Plato::InputData tOutput1("Output");
    tOutput1.add<std::string>("SharedDataName", "compliance_value_0");
    tCriterionValueStage.add<Plato::InputData>("Output", tOutput1);

    // append criterion value stage
    tDriver.add<Plato::InputData>("Stage", tCriterionValueStage);

    // append dakota driver stage
    Plato::InputData Inputs("Inputs");
    Inputs.add<Plato::InputData>("DakotaDriver", tDriver);

    // construct dakota data map
    Plato::DakotaDataMap tDakotaDataMap(Inputs);

    // create vector of inputs
    Teuchos::SerialDenseVector<int, double> tVariableData(4);
    tVariableData[0] = 1.2;
    tVariableData[1] = 0.8;
    tVariableData[2] = 47.3;
    tVariableData[3] = -5.1;
    
    // check for shared data
    bool tHasInputs = tDakotaDataMap.stageHasInputSharedData("criterion_value_0");
    EXPECT_EQ(tHasInputs, false);
    tHasInputs = tDakotaDataMap.stageHasInputSharedData("initialize");
    EXPECT_EQ(tHasInputs, true);

    // set continuous variables 
    auto tInputSharedDataNames = tDakotaDataMap.getInputSharedDataNames("initialize", "continuous"); 
    tDakotaDataMap.setContinuousVarsSharedData(tInputSharedDataNames[0],4,tVariableData);

    // numACV larger allocated vector size in map 
    EXPECT_THROW(tDakotaDataMap.setContinuousVarsSharedData(tInputSharedDataNames[0],5,tVariableData),std::out_of_range);

    // numACV larger than input vector
    Teuchos::SerialDenseVector<int, double> tBogusVariableData(3);
    tBogusVariableData[0] = 1.2;
    tBogusVariableData[1] = 0.8;
    tBogusVariableData[2] = 47.3;
    EXPECT_THROW(tDakotaDataMap.setContinuousVarsSharedData(tInputSharedDataNames[0],4,tBogusVariableData),std::out_of_range);

    // test parsing of continuous variables
    auto tStoredVariables = tDakotaDataMap.getContinuousVarsSharedData(tInputSharedDataNames[0]);
    std::vector<double> tGoldVals = {1.2, 0.8, 47.3, -5.1};

    for(auto& tVal : tStoredVariables)
    {
        auto tIndex = &tVal - &tStoredVariables[0];
        EXPECT_EQ(tGoldVals[tIndex], tVal);
    }

    // get continuous variables for fake shared data name
    tStoredVariables = tDakotaDataMap.getContinuousVarsSharedData("fake_shared_data_name");
    EXPECT_TRUE(tStoredVariables.empty());
}

TEST(PlatoTest, DakotaDataMap_accessDiscreteRealVars)
{
    // create criterion value stage
    Plato::InputData tCriterionValueStage("Stage");
    tCriterionValueStage.add<std::string>("StageTag", "criterion_value_0");
    tCriterionValueStage.add<std::string>("StageName", "CriteriaValueEvaluation");

    // add inputs for criterion value stage
    Plato::InputData tInput1("Input");
    tInput1.add<std::string>("Tag", "discrete real");
    tInput1.add<std::string>("SharedDataName", "design_parameters_0");
    tCriterionValueStage.add<Plato::InputData>("Input", tInput1);

    // add outputs for criterion value stage
    Plato::InputData tOutput1("Output");
    tOutput1.add<std::string>("SharedDataName", "compliance_value_0");
    tCriterionValueStage.add<Plato::InputData>("Output", tOutput1);

    // append criterion value stage
    Plato::InputData tDriver("DakotaDriver");
    tDriver.add<Plato::InputData>("Stage", tCriterionValueStage);

    // append dakota driver stage
    Plato::InputData Inputs("Inputs");
    Inputs.add<Plato::InputData>("DakotaDriver", tDriver);

    // construct dakota data map
    Plato::DakotaDataMap tDakotaDataMap(Inputs);

    // create vector of inputs
    Teuchos::SerialDenseVector<int, double> tVariableData(4);
    tVariableData[0] = 1.2;
    tVariableData[1] = 2.4;
    tVariableData[2] = 9.6;
    tVariableData[3] = 1.2;

    // set discrete real variables
    bool tHasInputs = tDakotaDataMap.stageHasInputSharedData("criterion_value_0");
    EXPECT_EQ(tHasInputs, true);
    auto tInputSharedDataNames = tDakotaDataMap.getInputSharedDataNames("criterion_value_0", "discrete real"); 
    tDakotaDataMap.setDiscreteRealVarsSharedData(tInputSharedDataNames[0],4,tVariableData);

    // numADR larger allocated vector size in map 
    EXPECT_THROW(tDakotaDataMap.setDiscreteRealVarsSharedData(tInputSharedDataNames[0],5,tVariableData),std::out_of_range);

    // numADR larger than input vector
    Teuchos::SerialDenseVector<int, double> tBogusVariableData(3);
    tBogusVariableData[0] = 1.2;
    tBogusVariableData[1] = 2.4;
    tBogusVariableData[2] = 9.6;
    EXPECT_THROW(tDakotaDataMap.setDiscreteRealVarsSharedData(tInputSharedDataNames[0],4,tBogusVariableData),std::out_of_range);

    // test parsing of discrete real variables
    auto tStoredVariables = tDakotaDataMap.getDiscreteRealVarsSharedData(tInputSharedDataNames[0]);
    std::vector<double> tGoldVals = {1.2, 2.4, 9.6, 1.2};

    for(auto& tVal : tStoredVariables)
    {
        auto tIndex = &tVal - &tStoredVariables[0];
        EXPECT_EQ(tGoldVals[tIndex], tVal);
    }

    // get discrete real variables for fake shared data name
    tStoredVariables = tDakotaDataMap.getDiscreteRealVarsSharedData("fake_shared_data_name");
    EXPECT_TRUE(tStoredVariables.empty());
}

TEST(PlatoTest, DakotaDataMap_accessDiscreteIntegerVars)
{
    // create criterion value stage
    Plato::InputData tCriterionValueStage("Stage");
    tCriterionValueStage.add<std::string>("StageTag", "criterion_value_0");
    tCriterionValueStage.add<std::string>("StageName", "CriteriaValueEvaluation");

    // add inputs for criterion value stage
    Plato::InputData tInput1("Input");
    tInput1.add<std::string>("Tag", "discrete integer");
    tInput1.add<std::string>("SharedDataName", "design_parameters_0");
    tCriterionValueStage.add<Plato::InputData>("Input", tInput1);

    // add outputs for criterion value stage
    Plato::InputData tOutput1("Output");
    tOutput1.add<std::string>("SharedDataName", "compliance_value_0");
    tCriterionValueStage.add<Plato::InputData>("Output", tOutput1);

    // append criterion value stage
    Plato::InputData tDriver("DakotaDriver");
    tDriver.add<Plato::InputData>("Stage", tCriterionValueStage);

    // append dakota driver stage
    Plato::InputData Inputs("Inputs");
    Inputs.add<Plato::InputData>("DakotaDriver", tDriver);

    // construct dakota data map
    Plato::DakotaDataMap tDakotaDataMap(Inputs);

    // create vector of inputs
    Teuchos::SerialDenseVector<int, int> tVariableData(4);
    tVariableData[0] = 1;
    tVariableData[1] = 2;
    tVariableData[2] = 9;
    tVariableData[3] = 3;

    // set discrete integer variables
    bool tHasInputs = tDakotaDataMap.stageHasInputSharedData("criterion_value_0");
    EXPECT_EQ(tHasInputs, true);
    auto tInputSharedDataNames = tDakotaDataMap.getInputSharedDataNames("criterion_value_0", "discrete integer"); 
    tDakotaDataMap.setDiscreteIntegerVarsSharedData(tInputSharedDataNames[0],4,tVariableData);

    // numADI larger allocated vector size in map 
    EXPECT_THROW(tDakotaDataMap.setDiscreteIntegerVarsSharedData(tInputSharedDataNames[0],5,tVariableData),std::out_of_range);

    // numADI larger than input vector
    Teuchos::SerialDenseVector<int, int> tBogusVariableData(3);
    tBogusVariableData[0] = 1;
    tBogusVariableData[1] = 2;
    tBogusVariableData[2] = 9;
    EXPECT_THROW(tDakotaDataMap.setDiscreteIntegerVarsSharedData(tInputSharedDataNames[0],4,tBogusVariableData),std::out_of_range);

    // test parsing of discrete integer variables
    auto tStoredVariables = tDakotaDataMap.getDiscreteIntegerVarsSharedData(tInputSharedDataNames[0]);
    std::vector<double> tGoldVals = {1, 2, 9, 3};

    for(auto& tVal : tStoredVariables)
    {
        auto tIndex = &tVal - &tStoredVariables[0];
        EXPECT_EQ(tGoldVals[tIndex], tVal);
    }

    // get discrete integer variables for fake shared data name
    tStoredVariables = tDakotaDataMap.getDiscreteIntegerVarsSharedData("fake_shared_data_name");
    EXPECT_TRUE(tStoredVariables.empty());
}

TEST(PlatoTest, DakotaDataMap_accessOutputData)
{
    // create criterion gradient stage
    Plato::InputData tCriterionGradientStage("Stage");
    tCriterionGradientStage.add<std::string>("StageTag", "criterion_gradient_0");
    tCriterionGradientStage.add<std::string>("StageName", "CriteriaGradientEvaluation");

    // add inputs for criterion gradient stage
    Plato::InputData tInput1("Input");
    tInput1.add<std::string>("Tag", "discrete integer");
    tInput1.add<std::string>("SharedDataName", "design_parameters_0");
    tCriterionGradientStage.add<Plato::InputData>("Input", tInput1);

    // append criterion gradient stage
    Plato::InputData tDriver("DakotaDriver");
    tDriver.add<Plato::InputData>("Stage", tCriterionGradientStage);
    
    // create finalize stage
    Plato::InputData tFinalizeStage("Stage");
    tFinalizeStage.add<std::string>("StageTag", "finalize");
    tFinalizeStage.add<std::string>("StageName", "OutputGradients");
    
    // add outputs for finalize stage
    Plato::InputData tOutput1("Output");
    tOutput1.add<std::string>("SharedDataName", "compliance_gradient_0");
    tFinalizeStage.add<Plato::InputData>("Output", tOutput1);

    // append finalize stage
    tDriver.add<Plato::InputData>("Stage", tFinalizeStage);

    // append dakota driver stage
    Plato::InputData Inputs("Inputs");
    Inputs.add<Plato::InputData>("DakotaDriver", tDriver);

    // construct dakota data map
    Plato::DakotaDataMap tDakotaDataMap(Inputs);

    // check for shared data
    bool tHasOutputs = tDakotaDataMap.stageHasOutputSharedData("criterion_gradient_0");
    EXPECT_EQ(tHasOutputs, false);
    tHasOutputs = tDakotaDataMap.stageHasOutputSharedData("finalize");
    EXPECT_EQ(tHasOutputs, true);

    // create output metadata
    auto tOutputSharedDataNames = tDakotaDataMap.getOutputSharedDataNames("finalize");
    Plato::dakota::SharedDataMetaData tMetaData;
    tMetaData.mName = tOutputSharedDataNames[0];
    tMetaData.mValues.resize(1u,0.0);

    // set continuous variables
    auto tStageTags = tDakotaDataMap.getStageTags();
    tDakotaDataMap.setOutputVarsSharedData(tStageTags[0],tMetaData,0);
    
    // index out of range for existing stage tag
    EXPECT_THROW(tDakotaDataMap.setOutputVarsSharedData(tStageTags[0],tMetaData,1000),std::out_of_range);

    // attempt to set data for non-existent stage tag 
    EXPECT_THROW(tDakotaDataMap.setOutputVarsSharedData("fake_stage_tag",tMetaData,1000),std::runtime_error);

    // test parsing of output
    auto tOutMetadata = tDakotaDataMap.getOutputVarsSharedData(tStageTags[0],0);
    EXPECT_STREQ("compliance_gradient_0", tOutMetadata.mName.c_str());
    EXPECT_EQ(1u, tOutMetadata.mValues.size());
    EXPECT_EQ(0.0, tOutMetadata.mValues[0]);

    // index out of range
    EXPECT_THROW(tOutMetadata = tDakotaDataMap.getOutputVarsSharedData(tStageTags[0],1000),std::out_of_range);
    EXPECT_THROW(tOutMetadata = tDakotaDataMap.getOutputVarsSharedData("fake_stage_tag",0),std::runtime_error);
}

TEST(PlatoTest, DakotaDataMap_noSharedDataNames)
{
    // create criterion value stage
    Plato::InputData tCriterionValueStage("Stage");
    tCriterionValueStage.add<std::string>("StageTag", "criterion_value_0");
    tCriterionValueStage.add<std::string>("StageName", "CriteriaValueEvaluation");

    // add inputs for criterion value stage
    Plato::InputData tInput1("Input");
    tInput1.add<std::string>("Tag", "discrete integer");
    tInput1.add<std::string>("SharedDataName", "design_parameters_0");
    tCriterionValueStage.add<Plato::InputData>("Input", tInput1);

    // add outputs for criterion value stage
    Plato::InputData tOutput1("Output");
    tOutput1.add<std::string>("SharedDataName", "compliance_value_0");
    tCriterionValueStage.add<Plato::InputData>("Output", tOutput1);

    // append criterion value stage
    Plato::InputData tDriver("DakotaDriver");
    tDriver.add<Plato::InputData>("Stage", tCriterionValueStage);

    // append dakota driver stage
    Plato::InputData Inputs("Inputs");
    Inputs.add<Plato::InputData>("DakotaDriver", tDriver);

    // construct dakota data map
    Plato::DakotaDataMap tDakotaDataMap(Inputs);

    // check cases with no stage tag found
    auto tInputSharedDataNames = tDakotaDataMap.getInputSharedDataNames("objective_value", "discrete integer"); 
    EXPECT_EQ(0u, tInputSharedDataNames.size());

    auto tOutputSharedDataNames = tDakotaDataMap.getOutputSharedDataNames("finalize");
    EXPECT_EQ(0u, tOutputSharedDataNames.size());

    // check cases with no input data tag found
    tInputSharedDataNames = tDakotaDataMap.getInputSharedDataNames("criterion_value_0", "continuous"); 
    EXPECT_EQ(0u, tInputSharedDataNames.size());
    tInputSharedDataNames = tDakotaDataMap.getInputSharedDataNames("criterion_value_0", "discrete real"); 
    EXPECT_EQ(0u, tInputSharedDataNames.size());
}

TEST(PlatoTest, DakotaDataMap_constructor_error_checking)
{
    Plato::InputData tInputData("Inputs");
    
    // no DakotaDriver block in input data
    EXPECT_THROW(Plato::DakotaDataMap tDataMap(tInputData),std::runtime_error);

    // no stages in DakotaDriver block
    Plato::InputData tDakotaDriver("DakotaDriver");
    tInputData.add<Plato::InputData>("DakotaDriver", tDakotaDriver);
    EXPECT_THROW(Plato::DakotaDataMap tDataMap(tInputData),std::runtime_error);

    // stage has no StageName or StageTag
    Plato::InputData tStageNode("Stage");
    tDakotaDriver.add<Plato::InputData>("Stage", tStageNode);
    EXPECT_THROW(Plato::DakotaDataMap tDataMap(tInputData),std::runtime_error);

    // stage has no StageName but does have a StageTag
    tStageNode.add<std::string>("StageTag", "criterion_value_0");
    EXPECT_THROW(Plato::DakotaDataMap tDataMap(tInputData),std::runtime_error);
    tStageNode.add<std::string>("StageName", "my_stage_name_1");

    // stage has no StageTag but does have a StageName 
    Plato::InputData tStageNode2("Stage");
    tDakotaDriver.add<Plato::InputData>("Stage", tStageNode2);
    tStageNode2.add<std::string>("StageName", "my_stage_name_2");
    EXPECT_THROW(Plato::DakotaDataMap tDataMap(tInputData),std::runtime_error);
    tStageNode2.add<std::string>("StageTag", "criterion_gradient_100");

    // stage input has no SharedDataName
    Plato::InputData tInputNode("Input");
    tStageNode.add<Plato::InputData>("Input",tInputNode);
    EXPECT_THROW(Plato::DakotaDataMap tDataMap(tInputData),std::runtime_error);

    // stage input has no Tag
    tInputNode.add<std::string>("SharedDataName", "my_shared_data_name");
    EXPECT_THROW(Plato::DakotaDataMap tDataMap(tInputData),std::runtime_error);

    // stage input tag is not valid
    tInputNode.add<std::string>("Tag", "my_tag");
    EXPECT_THROW(Plato::DakotaDataMap tDataMap(tInputData),std::runtime_error);
    tInputNode.set<std::string>("Tag", "continuous");

    // stage output has no SharedDataName
    Plato::InputData tOutputNode("Output");
    tStageNode.add<Plato::InputData>("Output",tOutputNode);
    EXPECT_THROW(Plato::DakotaDataMap tDataMap(tInputData),std::runtime_error);
    
    // valid input
    tOutputNode.add<std::string>("SharedDataName", "my_shared_data_name");
    EXPECT_NO_THROW(Plato::DakotaDataMap tDataMap(tInputData));

    // StageTag is 3 tokens but first is not criterion
    tStageNode2.set<std::string>("StageTag", "criterio_value_0");
    EXPECT_THROW(Plato::DakotaDataMap tDataMap(tInputData),std::runtime_error);
    
    // StageTag has 3 tokens but second is not value or gradient
    tStageNode2.set<std::string>("StageTag", "criterion_valu_0");
    EXPECT_THROW(Plato::DakotaDataMap tDataMap(tInputData),std::runtime_error);

    // // StageTag has 3 tokens but third is not a number
    tStageNode2.set<std::string>("StageTag", "criterion_value_0a");
    EXPECT_THROW(Plato::DakotaDataMap tDataMap(tInputData),std::runtime_error);

    // Plato::InputData tStageNode6("Stage");
    tStageNode2.set<std::string>("StageTag", "criterion_value_a");
    EXPECT_THROW(Plato::DakotaDataMap tDataMap(tInputData),std::runtime_error);
    
    // // Stage tag is not 3 tokens and is not exactly 'initialize' or 'finalize'
    tStageNode2.set<std::string>("StageTag", "criterion_value_10_something_incorrect");
    EXPECT_THROW(Plato::DakotaDataMap tDataMap(tInputData),std::runtime_error);

    tStageNode2.set<std::string>("StageTag", "criterionvalue10");
    EXPECT_THROW(Plato::DakotaDataMap tDataMap(tInputData),std::runtime_error);
    
    // StageTag set to "initialize"
    tStageNode2.set<std::string>("StageTag", "initialize");
    EXPECT_NO_THROW(Plato::DakotaDataMap tDataMap(tInputData));

    // StageTag set to "finalize"
    tStageNode2.set<std::string>("StageTag", "finalize");
    EXPECT_NO_THROW(Plato::DakotaDataMap tDataMap(tInputData));

    // Two stages with the same StageTag
    tStageNode.set<std::string>("StageTag", "finalize");
    EXPECT_THROW(Plato::DakotaDataMap tDataMap(tInputData),std::runtime_error);

    // Two stages with the same StageName
    tStageNode.set<std::string>("StageTag", "criterion_value_0");
    tStageNode.set<std::string>("StageName", "my_stage_name_1");
    tStageNode2.set<std::string>("StageName", "my_stage_name_1");
    EXPECT_THROW(Plato::DakotaDataMap tDataMap(tInputData),std::runtime_error);;
}

}
// TestPlatoDakotaInterfack
