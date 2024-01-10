/*
//@HEADER
// *************************************************************************
//   Plato Engine v.1.0: Copyright 2018, National Technology & Engineering
//                    Solutions of Sandia, LLC (NTESS).
//
// Under the terms of Contract DE-NA0003525 with NTESS,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Sandia Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact the Plato team (plato3D-help@sandia.gov)
//
// *************************************************************************
//@HEADER
*/

#include <gtest/gtest.h>

#include "Plato_InputData.hpp"
#include "Plato_Parser.hpp"
#include "Plato_Exceptions.hpp"

namespace PlatoTestEngineDataParser
{
TEST(PlatoTestEngineDataParser, ParseOptimizerData)
{
  const std::string tInput =
  "<OptimizationVariables>\n"
  "  <ValueName>Control</ValueName>\n"
  "  <InitializationStage>Initial Guess</InitializationStage>\n"
  "  <FilteredName>Topology</FilteredName>\n"
  "  <LowerBoundValueName>Lower Bound Value</LowerBoundValueName>\n"
  "  <LowerBoundVectorName>Lower Bound Vector</LowerBoundVectorName>\n"
  "  <UpperBoundValueName>Upper Bound Value</UpperBoundValueName>\n"
  "  <UpperBoundVectorName>Upper Bound Vector</UpperBoundVectorName>\n"
  "  <SetLowerBoundsStage>Set Lower Bounds</SetLowerBoundsStage>\n"
  "  <SetUpperBoundsStage>Set Upper Bounds</SetUpperBoundsStage>\n"
  "  <StochasticSample>\n"
  "    <OutputValueSharedDataName>out_data_1</OutputValueSharedDataName>\n"
  "    <OutputGradientSharedDataName>out_grad_1</OutputGradientSharedDataName>\n"
  "    <StochasticParameter>\n"
  "      <SharedDataName>parameter_data_1_1</SharedDataName>\n"
  "      <DistributionName>uniform_1</DistributionName>\n"
  "    </StochasticParameter>\n"
  "    <StochasticParameter>\n"
  "      <SharedDataName>parameter_data_1_2</SharedDataName>\n"
  "      <DistributionName>uniform_2</DistributionName>\n"
  "    </StochasticParameter>\n"
  "  </StochasticSample>\n"
  "  <StochasticSample>\n"
  "    <OutputValueSharedDataName>out_data_2</OutputValueSharedDataName>\n"
  "    <OutputGradientSharedDataName>out_grad_2</OutputGradientSharedDataName>\n"
  "    <StochasticParameter>\n"
  "      <SharedDataName>parameter_data_2_1</SharedDataName>\n"
  "      <DistributionName>uniform_1</DistributionName>\n"
  "    </StochasticParameter>\n"
  "  </StochasticSample>\n"
  "</OptimizationVariables>\n";

  const Plato::PugiParser tParser;
  const Plato::InputData tInputData = tParser.parseString(tInput);

  Plato::OptimizerEngineStageData tEngineData;
  Plato::Parse::parseOptimizationVariablesNames(tInputData, tEngineData);

  EXPECT_EQ(tEngineData.getNumControlVectors(), 1); // ValueName
  EXPECT_EQ(tEngineData.getControlName(0), "Control"); // ValueName
  EXPECT_EQ(tEngineData.getInitializationStageName(), "Initial Guess"); // InitializationStage
  EXPECT_EQ(tEngineData.getFilteredControlName(0), "Topology"); // FilteredName
  EXPECT_EQ(tEngineData.getLowerBoundValueName(), "Lower Bound Value"); // LowerBoundValueName
  EXPECT_EQ(tEngineData.getLowerBoundVectorName(), "Lower Bound Vector"); // LowerBoundVectorName
  EXPECT_EQ(tEngineData.getUpperBoundValueName(), "Upper Bound Value"); // UpperBoundValueName
  EXPECT_EQ(tEngineData.getUpperBoundVectorName(), "Upper Bound Vector"); // UpperBoundVectorName
  EXPECT_EQ(tEngineData.getSetLowerBoundsStageName(), "Set Lower Bounds"); // SetLowerBoundsStage
  EXPECT_EQ(tEngineData.getSetUpperBoundsStageName(), "Set Upper Bounds"); // SetUpperBoundsStage

  ASSERT_EQ(tEngineData.getStochasticSampleSharedDataNames().size(), 2);
  EXPECT_EQ(tEngineData.getStochasticSampleSharedDataNames().front().mOutputValueSharedDataName, "out_data_1");
  EXPECT_EQ(tEngineData.getStochasticSampleSharedDataNames().front().mOutputGradientSharedDataName, "out_grad_1");
  ASSERT_EQ(tEngineData.getStochasticSampleSharedDataNames().front().mParameters.size(), 2);
  EXPECT_EQ(tEngineData.getStochasticSampleSharedDataNames().front().mParameters.at(0).mParameterName, "parameter_data_1_1");
  EXPECT_EQ(tEngineData.getStochasticSampleSharedDataNames().front().mParameters.at(0).mDistributionName, "uniform_1");
  EXPECT_EQ(tEngineData.getStochasticSampleSharedDataNames().front().mParameters.at(1).mParameterName, "parameter_data_1_2");
  EXPECT_EQ(tEngineData.getStochasticSampleSharedDataNames().front().mParameters.at(1).mDistributionName, "uniform_2");
}

TEST(PlatoTestEngineDataParser, MalformedStochasticSamplesOutputValue)
{
  const std::string tInput =
  "<OptimizationVariables>\n"
  "  <StochasticSample>\n"
  "    <OutputGradientSharedDataName>out_grad_1</OutputGradientSharedDataName>\n"
  "    <StochasticParameter>\n"
  "      <SharedDataName>parameter_data_1_1</SharedDataName>\n"
  "      <DistributionName>uniform_1</DistributionName>\n"
  "    </StochasticParameter>\n"
  "  </StochasticSample>\n"
  "</OptimizationVariables>\n";

  const Plato::InputData tInputData = Plato::PugiParser{}.parseString(tInput);
  Plato::OptimizerEngineStageData tEngineData;
  EXPECT_THROW(Plato::Parse::parseOptimizationVariablesNames(tInputData, tEngineData), Plato::ParsingException);
}

TEST(PlatoTestEngineDataParser, MalformedStochasticSamplesOutputGradient)
{
  const std::string tInput =
  "<OptimizationVariables>\n"
  "  <StochasticSample>\n"
  "    <OutputValueSharedDataName>out_data_1</OutputValueSharedDataName>\n"
  "    <StochasticParameter>\n"
  "      <SharedDataName>parameter_data_1_1</SharedDataName>\n"
  "      <DistributionName>uniform_1</DistributionName>\n"
  "    </StochasticParameter>\n"
  "  </StochasticSample>\n"
  "</OptimizationVariables>\n";

  const Plato::InputData tInputData = Plato::PugiParser{}.parseString(tInput);
  Plato::OptimizerEngineStageData tEngineData;
  EXPECT_THROW(Plato::Parse::parseOptimizationVariablesNames(tInputData, tEngineData), Plato::ParsingException);
}

TEST(PlatoTestEngineDataParser, MalformedStochasticSamplesParameter)
{
  const std::string tInput =
  "<OptimizationVariables>\n"
  "  <StochasticSample>\n"
  "    <OutputSharedDataName>out_data_1</OutputSharedDataName>\n"
  "  </StochasticSample>\n"
  "</OptimizationVariables>\n";

  const Plato::InputData tInputData = Plato::PugiParser{}.parseString(tInput);
  Plato::OptimizerEngineStageData tEngineData;
  EXPECT_THROW(Plato::Parse::parseOptimizationVariablesNames(tInputData, tEngineData), Plato::ParsingException);
}

TEST(PlatoTestEngineDataParser, MalformedStochasticSamplesSharedData)
{
  const std::string tInput =
  "<OptimizationVariables>\n"
  "  <StochasticSample>\n"
  "    <OutputSharedDataName>out_data_1</OutputSharedDataName>\n"
  "    <StochasticParameter>\n"
  "      <DistributionName>uniform_1</DistributionName>\n"
  "    </StochasticParameter>\n"
  "  </StochasticSample>\n"
  "</OptimizationVariables>\n";

  const Plato::InputData tInputData = Plato::PugiParser{}.parseString(tInput);
  Plato::OptimizerEngineStageData tEngineData;
  EXPECT_THROW(Plato::Parse::parseOptimizationVariablesNames(tInputData, tEngineData), Plato::ParsingException);
}

TEST(PlatoTestEngineDataParser, MalformedStochasticSamplesDistribution)
{
  const std::string tInput =
  "<OptimizationVariables>\n"
  "  <StochasticSample>\n"
  "    <OutputSharedDataName>out_data_1</OutputSharedDataName>\n"
  "    <StochasticParameter>\n"
  "      <SharedDataName>parameter_data_1_1</SharedDataName>\n"
  "    </StochasticParameter>\n"
  "  </StochasticSample>\n"
  "</OptimizationVariables>\n";

  const Plato::InputData tInputData = Plato::PugiParser{}.parseString(tInput);
  Plato::OptimizerEngineStageData tEngineData;
  EXPECT_THROW(Plato::Parse::parseOptimizationVariablesNames(tInputData, tEngineData), Plato::ParsingException);
}

TEST(PlatoTestEngineDataParser, ParseObjectiveData)
{
  const std::string tInput =
  "<Objective>\n"
  "  <GradientName>one fish</GradientName>\n"
  "  <GradientStageName>two fish</GradientStageName>\n"
  
  "  <ValueName>red fish</ValueName>\n"
  "  <ValueStageName>blue fish</ValueStageName>\n"

  "  <HessianName>Some are fast</HessianName>\n"
  "  <HessianStageName>Some are slow</HessianStageName>\n"
  "</Objective>\n";

  const Plato::PugiParser tParser;
  const Plato::InputData tInputData = tParser.parseString(tInput);
  auto tObjectiveNode = tInputData.get<Plato::InputData>("Objective");

  Plato::OptimizerEngineStageData tEngineData;
  Plato::Parse::parseObjectiveStagesData(tObjectiveNode, tEngineData);

  EXPECT_EQ(tEngineData.getObjectiveGradientOutputName(), "one fish");
  EXPECT_EQ(tEngineData.getObjectiveGradientStageName(), "two fish");

  EXPECT_EQ(tEngineData.getObjectiveValueOutputName(), "red fish");
  EXPECT_EQ(tEngineData.getObjectiveValueStageName(), "blue fish");

  EXPECT_EQ(tEngineData.getObjectiveHessianOutputName(), "Some are fast");
  EXPECT_EQ(tEngineData.getObjectiveHessianStageName(), "Some are slow");
}

TEST(PlatoTestEngineDataParser, ParseOptimizerOptions)
{
  const std::string tInput =
  "<Options>\n"
  "  <InputFileName>Max</InputFileName>\n"
  "  <ROLStochasticDistributionsFile>Carol</ROLStochasticDistributionsFile>\n"
  "  <ROLStochasticNumberOfSamples>42</ROLStochasticNumberOfSamples>\n"
  "  <ROLStochasticSamplerSeed>17</ROLStochasticSamplerSeed>\n"
  "<Options>\n";

  const Plato::PugiParser tParser;
  const Plato::InputData tInputData = tParser.parseString(tInput);

  Plato::OptimizerEngineStageData tEngineData;
  Plato::Parse::parseOptimizerOptions(tInputData, tEngineData);

  EXPECT_EQ(tEngineData.getInputFileName(), "Max");
  EXPECT_EQ(tEngineData.getROLStochasticDistributionsFile(), "Carol");
  EXPECT_EQ(tEngineData.getROLStochasticNumberOfSamples(), 42);
  EXPECT_EQ(tEngineData.getROLStochasticSamplerSeed(), 17);
}

} // end PlatoTestInputData namespace
