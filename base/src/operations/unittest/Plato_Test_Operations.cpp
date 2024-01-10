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
#include "Plato_Utils.hpp"
#include "Plato_InputData.hpp"
#include "Plato_EnforceBounds.hpp"
#include "Plato_SystemCallOperation.hpp"
#include "Plato_HarvestDataFromFile.hpp"
#include "Plato_OperationsUtilities.hpp"

#include <fstream>
#include <gtest/gtest.h>

namespace PlatoTestOperations
{

void createMatchedYAMLTemplateFile()
{
    std::ofstream tOutFile;
    tOutFile.open("matched.yaml.template", std::ofstream::out | std::ofstream::trunc);
    tOutFile << "%YAML 1.1\n";
    tOutFile << "---\n\n";

    tOutFile << "Gemma-dynamic:\n\n";

    tOutFile << "  Global:\n";
    tOutFile << "    Description: Higgins cylinder\n";
    tOutFile << "    Solution type: power balance\n\n";

    tOutFile << "  Power balance: \n";
    tOutFile << "    Algorithm: matched bound\n";
    tOutFile << "    Radius: {r} # original value r=0.1016\n";
    tOutFile << "    Height: {h} # original value h=0.1016\n";
    tOutFile << "    Conductivity: 2.6e7\n";
    tOutFile << "    Slot length: 0.0508\n";
    tOutFile << "    Slot width: 2.54e-3\n";
    tOutFile << "    Slot depth: 0.006350\n";
    tOutFile << "    Start frequency range: 1e9\n";
    tOutFile << "    End frequency range: 2.2e9\n";
    tOutFile << "    Frequency interval size: 1e7\n\n";

    tOutFile << "...\n";
    tOutFile.close();
}

void createMatchedYAMLFile()
{
    std::ofstream tOutFile;
    tOutFile.open("matched.yaml", std::ofstream::out | std::ofstream::trunc);
    tOutFile << "%YAML 1.1\n";
    tOutFile << "---\n\n";

    tOutFile << "Gemma-dynamic:\n\n";

    tOutFile << "  Global:\n";
    tOutFile << "    Description: Higgins cylinder\n";
    tOutFile << "    Solution type: power balance\n\n";

    tOutFile << "  Power balance: \n";
    tOutFile << "    Algorithm: matched bound\n";
    tOutFile << "    Radius: 0.1016 # original value r=0.1016\n";
    tOutFile << "    Height: 0.1016 # original value h=0.1016\n";
    tOutFile << "    Conductivity: 2.6e7\n";
    tOutFile << "    Slot length: 0.0508\n";
    tOutFile << "    Slot width: 2.54e-3\n";
    tOutFile << "    Slot depth: 0.006350\n";
    tOutFile << "    Start frequency range: 1e9\n";
    tOutFile << "    End frequency range: 2.2e9\n";
    tOutFile << "    Frequency interval size: 1e7\n\n";

    tOutFile << "...\n";
    tOutFile.close();
}

TEST(LocalOperation, SystemCall_Error_InputArraySizeMismatch)
{
    Plato::InputData tInputNode("Operation");
    tInputNode.add<std::string>("Command", "mkdir evaluation0; mv matched.yaml.template evaluation0; cd evaluation0; aprepro");
    tInputNode.add<std::string>("Name", "aprepro_0");
    tInputNode.add<std::string>("OnChange", "true");
    tInputNode.add<std::string>("AppendInput", "true");
    tInputNode.add<std::string>("Argument", "matched.yaml.template matched.yaml");
    tInputNode.add<std::string>("Argument", "-q");
    tInputNode.add<std::string>("Option", "r=");
    tInputNode.add<std::string>("Option", "h=");

    Plato::InputData tInputOne("Input");
    tInputOne.add<std::string>("ArgumentName", "Parameters_0");
    tInputOne.add<std::string>("Layout", "Scalar");
    tInputOne.add<std::string>("Size", "2");
    tInputNode.add<Plato::InputData>("Input", tInputOne);
    Plato::InputData tInputTwo("Input");
    tInputTwo.add<std::string>("ArgumentName", "Parameters_1");
    tInputTwo.add<std::string>("Layout", "Scalar");
    tInputTwo.add<std::string>("Size", "2");
    tInputNode.add<Plato::InputData>("Input", tInputTwo);

    Plato::SystemCall tSystemCall(tInputNode);
    Plato::SystemCallMetadata tMetaData;
    std::vector<double> tParameters= {1,2};
    tMetaData.mInputArgumentMap["Parameters_0"] = &tParameters;
    EXPECT_THROW(tSystemCall(tMetaData),std::runtime_error);
}

TEST(LocalOperation, SystemCall_Error_OptionsGreaterThanNumParam)
{
    Plato::InputData tInputNode("Operation");
    tInputNode.add<std::string>("Command", "mkdir evaluation0; mv matched.yaml.template evaluation0; cd evaluation0; aprepro");
    tInputNode.add<std::string>("Name", "aprepro_0");
    tInputNode.add<std::string>("OnChange", "true");
    tInputNode.add<std::string>("AppendInput", "true");
    tInputNode.add<std::string>("Argument", "matched.yaml.template matched.yaml");
    tInputNode.add<std::string>("Argument", "-q");
    tInputNode.add<std::string>("Option", "r=");
    tInputNode.add<std::string>("Option", "h=");
    tInputNode.add<std::string>("Option", "l=");

    Plato::InputData tInput("Input");
    tInput.add<std::string>("ArgumentName", "Parameters_0");
    tInput.add<std::string>("Layout", "Scalar");
    tInput.add<std::string>("Size", "2");
    tInputNode.add<Plato::InputData>("Input", tInput);
    
    Plato::SystemCall tSystemCall(tInputNode);
    Plato::SystemCallMetadata tMetaData;
    std::vector<double> tParameters= {1,2};
    tMetaData.mInputArgumentMap["Parameters_0"] = &tParameters;
    EXPECT_THROW(tSystemCall(tMetaData),std::runtime_error);
}

TEST(LocalOperation, SystemCall_InputArgumentSizeNotDefinedError)
{
    Plato::InputData tInputNode("Operation");
    tInputNode.add<std::string>("Command", "mkdir evaluation0; mv matched.yaml.template evaluation0; cd evaluation0; aprepro");
    tInputNode.add<std::string>("Name", "aprepro_0");
    tInputNode.add<std::string>("OnChange", "true");
    tInputNode.add<std::string>("AppendInput", "true");
    tInputNode.add<std::string>("Argument", "matched.yaml.template matched.yaml");
    tInputNode.add<std::string>("Argument", "-q");
    tInputNode.add<std::string>("Option", "r=");
    tInputNode.add<std::string>("Option", "h=");

    Plato::InputData tInput("Input");
    tInput.add<std::string>("ArgumentName", "Parameters_0");
    tInput.add<std::string>("Layout", "Scalar");
    tInputNode.add<Plato::InputData>("Input", tInput);
    EXPECT_THROW(Plato::SystemCall tSystemCall(tInputNode),std::runtime_error);
}

TEST(LocalOperation, SystemCall_InputArgumentLayoutNotDefinedError)
{
    Plato::InputData tInputNode("Operation");
    tInputNode.add<std::string>("Command", "mkdir evaluation0; mv matched.yaml.template evaluation0; cd evaluation0; aprepro");
    tInputNode.add<std::string>("Name", "aprepro_0");
    tInputNode.add<std::string>("OnChange", "true");
    tInputNode.add<std::string>("AppendInput", "true");
    tInputNode.add<std::string>("Argument", "matched.yaml.template matched.yaml");
    tInputNode.add<std::string>("Argument", "-q");
    tInputNode.add<std::string>("Option", "r=");
    tInputNode.add<std::string>("Option", "h=");

    Plato::InputData tInput("Input");
    tInput.add<std::string>("ArgumentName", "Parameters_0");
    tInput.add<std::string>("Size", "2");
    tInputNode.add<Plato::InputData>("Input", tInput);
    EXPECT_THROW(Plato::SystemCall tSystemCall(tInputNode),std::runtime_error);
}

TEST(LocalOperation, SystemCall_InputArgumentLayourNotSupported_Error)
{
    Plato::InputData tInputNode("Operation");
    tInputNode.add<std::string>("Command", "mkdir evaluation0; mv matched.yaml.template evaluation0; cd evaluation0; aprepro");
    tInputNode.add<std::string>("Name", "aprepro_0");
    tInputNode.add<std::string>("OnChange", "true");
    tInputNode.add<std::string>("AppendInput", "true");
    tInputNode.add<std::string>("Argument", "matched.yaml.template matched.yaml");
    tInputNode.add<std::string>("Argument", "-q");
    tInputNode.add<std::string>("Option", "r=");
    tInputNode.add<std::string>("Option", "h=");

    Plato::InputData tInput("Input");
    tInput.add<std::string>("ArgumentName", "Parameters_0");
    tInput.add<std::string>("Layout", "Nodal");
    tInput.add<std::string>("Size", "2");
    tInputNode.add<Plato::InputData>("Input", tInput);
    EXPECT_THROW(Plato::SystemCall tSystemCall(tInputNode),std::runtime_error);
}

TEST(LocalOperation, SystemCall_constructor)
{
    Plato::InputData tInputNode("Operation");
    tInputNode.add<std::string>("Command", "mkdir evaluation0; mv matched.yaml.template evaluation0; cd evaluation0; aprepro");
    tInputNode.add<std::string>("Name", "aprepro_0");
    tInputNode.add<std::string>("OnChange", "true");
    tInputNode.add<std::string>("AppendInput", "true");
    tInputNode.add<std::string>("Argument", "matched.yaml.template matched.yaml");
    tInputNode.add<std::string>("Argument", "-q");
    tInputNode.add<std::string>("Option", "r=");
    tInputNode.add<std::string>("Option", "h=");

    Plato::InputData tInput("Input");
    tInput.add<std::string>("ArgumentName", "Parameters_0");
    tInput.add<std::string>("Layout", "Scalar");
    tInput.add<std::string>("Size", "2");
    tInputNode.add<Plato::InputData>("Input", tInput);

    Plato::SystemCall tSystemCall(tInputNode);

    EXPECT_STREQ("aprepro_0", tSystemCall.name().c_str());
    EXPECT_STREQ("mkdir evaluation0; mv matched.yaml.template evaluation0; cd evaluation0; aprepro", tSystemCall.command().c_str());
    EXPECT_TRUE(tSystemCall.onChange());
    EXPECT_TRUE(tSystemCall.appendInput());

    auto tOptions = tSystemCall.options();
    std::vector<std::string> tGoldOptions = {"r=", "h="};
    EXPECT_EQ(tOptions[0], tGoldOptions[0]);
    EXPECT_EQ(tOptions[1], tGoldOptions[1]);

    auto tArguments = tSystemCall.arguments();
    std::vector<std::string> tGoldArguments = {"matched.yaml.template matched.yaml", "-q"};
    for(auto& tArgument : tArguments)
    {
        auto tItr = std::find(tGoldArguments.begin(), tGoldArguments.end(), tArgument);
        EXPECT_TRUE(tItr != tGoldArguments.end());
    }

    auto tInputs = tSystemCall.inputNames();
    std::vector<std::string> tGoldInputs = {"Parameters_0"};
    for(auto& tCurInput : tInputs)
    {
        auto tItr = std::find(tGoldInputs.begin(), tGoldInputs.end(), tCurInput);
        EXPECT_TRUE(tItr != tGoldInputs.end());
    }
}

TEST(LocalOperation, SystemCallOperation_CallSerialGemma_Delete)
{
    std::cout << "Current working directory: " << Plato::Utils::current_working_directory() << std::endl;
    auto tTrash = std::system("rm -rf matched.yaml evaluation_0/");
    Plato::Utils::ignore_unused(tTrash);
}

TEST(LocalOperation, SystemCallOperation_Constructor)
{
    Plato::InputData tInputNode("Operation");
    tInputNode.add<std::string>("Command", "mkdir evaluation0; mv matched.yaml.template evaluation0; cd evaluation0; aprepro");
    tInputNode.add<std::string>("Name", "aprepro_0");
    tInputNode.add<std::string>("OnChange", "true");
    tInputNode.add<std::string>("AppendInput", "true");
    tInputNode.add<std::string>("Argument", "matched.yaml.template matched.yaml");
    tInputNode.add<std::string>("Argument", "-q");
    tInputNode.add<std::string>("Option", "r=");
    tInputNode.add<std::string>("Option", "h=");

    Plato::InputData tInput("Input");
    tInput.add<std::string>("ArgumentName", "Parameters_0");
    tInput.add<std::string>("Layout", "Scalar");
    tInput.add<std::string>("Size", "2");
    tInputNode.add<Plato::InputData>("Input", tInput);
    Plato::SystemCallOperation tSystemCallOperation(nullptr, tInputNode);

    std::vector<Plato::LocalArg> tArguments;
    tSystemCallOperation.getArguments(tArguments);
    EXPECT_EQ(1u, tArguments.size());
    EXPECT_FALSE(tArguments.front().mWrite);
    EXPECT_EQ(2u, tArguments.front().mLength);
    EXPECT_EQ(Plato::data::SCALAR, tArguments.front().mLayout);
    EXPECT_STREQ("Parameters_0", tArguments.front().mName.c_str());
}

TEST(LocalOperation, read_table_1)
{
    std::ofstream tOutFile;
    tOutFile.open("test.txt");
    std::stringstream ss;
    ss << "#f[Hz]  SE[dB]\n";
    tOutFile << ss.str();
    tOutFile.close();

    // no numerical values in table - outcome: empty table
    std::vector<std::vector<double>> tTable;
    Plato::read_table("test.txt", tTable);
    EXPECT_EQ(0, tTable.size());

    auto tTrash = std::system("rm -f test.txt");
    Plato::Utils::ignore_unused(tTrash);
}

TEST(LocalOperation, read_table_2)
{
    std::ofstream tOutFile;
    tOutFile.open("test.txt");
    std::stringstream ss;
    ss << "#f[Hz]  SE[dB]\n    1000000000    39.00415657\n    1010000000    38.93005158\n    1020000000    38.85582205";
    tOutFile << ss.str();
    tOutFile.close();

    // no numerical values in table - outcome: empty table
    std::vector<std::vector<double>> tTable;
    Plato::read_table("test.txt", tTable);
    
    auto tTol = 1e-6;
    std::vector<std::vector<double>> tGold = 
        {{1000000000, 39.00415657},{1010000000, 38.93005158},{1020000000, 38.85582205}};
    EXPECT_EQ(3, tTable.size());
    for(auto& tRow : tTable)
    {
        EXPECT_EQ(2, tRow.size());
        auto tRowI = &tRow - &tTable[0];
        for(auto& tCol : tRow)
        {
            auto tColI = &tCol - &tRow[0];
            EXPECT_NEAR(tGold[tRowI][tColI], tCol, tTol);
            //printf("X(%d,%d)=%f ", tRowI, tColI, tCol);
        }
        //printf("\n");
    }

    auto tTrash = std::system("rm -f test.txt");
    Plato::Utils::ignore_unused(tTrash);
}

TEST(LocalOperation, parse_tokens)
{
    std::vector<char> tBuffer = {'#', 'f', '[', 'H', 'z', ']', ' ', 'S', 'E', '[', 'd', 'B', ']'};
    std::vector<std::string> tTokens;
    Plato::parse_tokens(tBuffer.data(), tTokens);
    EXPECT_EQ(2u, tTokens.size());
    EXPECT_STREQ("#f[Hz]", tTokens[0].c_str());

    // remove garbage from last token
    if(tTokens[1].size() > 6u)
    {
        tTokens[1].erase(tTokens[1].begin()+6, tTokens[1].end());
        //std::cout << tTokens[1] << "\n";
    }

    EXPECT_STREQ("SE[dB]", tTokens[1].c_str());
}

TEST(LocalOperation, compute)
{
    Plato::Table tTable;
    tTable.mCol = 1;
    tTable.mData = 
        {{1000000000, 39.00415657}, {1010000000, 38.93005158}, {1020000000, 38.85582205},
         {1030000000, 38.78149587}, {1040000000, 38.70709947}, {1050000000, 38.63265785} };

    // test max
    auto tTol = 1e-4;
    tTable.mOperation = "max";
    auto tOutput = Plato::compute(tTable);
    EXPECT_NEAR(39.00415657, tOutput, tTol);

    // test min
    tTable.mOperation = "min";
    tOutput = Plato::compute(tTable);
    EXPECT_NEAR(38.63265785, tOutput, tTol);

    // test min
    tTable.mOperation = "sum";
    tOutput = Plato::compute(tTable);
    EXPECT_NEAR(232.91128339, tOutput, tTol);
}

TEST(LocalOperation, HarvestDataFromFile)
{
    Plato::InputData tInputNode("Operation");
    tInputNode.add<std::string>("Column", "1");
    tInputNode.add<std::string>("Operation", "Max");
    tInputNode.add<std::string>("Name", "Harvest Gemma Data");
    tInputNode.add<std::string>("Function", "HarvestDataFromFile");
    tInputNode.add<std::string>("File", "matched_power_balance.dat");

    Plato::InputData tInput("Input");
    tInput.add<std::string>("ArgumentName", "shape_parameters");
    tInputNode.add<Plato::InputData>("Input", tInput);

    Plato::InputData tOutput("Output");
    tOutput.add<std::string>("ArgumentName", "criterion_value");
    tInputNode.add<Plato::InputData>("Output", tOutput);

    Plato::HarvestDataFromFile tOperation(nullptr, tInputNode);

    // test single-value inputs
    EXPECT_STREQ("1", tOperation.column().c_str());
    EXPECT_STREQ("Max", tOperation.operation().c_str());
    EXPECT_STREQ("Harvest Gemma Data", tOperation.name().c_str());
    EXPECT_STREQ("HarvestDataFromFile", tOperation.function().c_str());
    EXPECT_STREQ("matched_power_balance.dat", tOperation.file().c_str());

    // test input/output arguments 
    std::vector<Plato::LocalArg> tLocalArgs;
    tOperation.getArguments(tLocalArgs);
    EXPECT_EQ(2u, tLocalArgs.size());

    std::vector<std::string> tGoldNames = {"shape_parameters", "criterion_value"};
    for(auto tValue : tLocalArgs)
    {
        EXPECT_EQ(Plato::data::SCALAR, tValue.mLayout);
        auto tItr = std::find(tGoldNames.begin(), tGoldNames.end(), tValue.mName);
        EXPECT_TRUE(tItr != tGoldNames.end());
    }

    // test operator()()
    std::ofstream tOutFile;
    tOutFile.open("matched_power_balance.dat");
    std::stringstream tSS;
    tSS << "#f[Hz]  SE[dB]\n    1000000000    39.00415657\n    1010000000    38.93005158\n    1020000000    38.85582205";
    tOutFile << tSS.str();
    tOutFile.close();
    tOperation.unittest(true);
    tOperation.operator()();
    auto tResult = tOperation.result();
    auto tTol = 1e-4;
    EXPECT_NEAR(39.00415657, tResult, tTol);
    auto tTrash = std::system("rm -f matched_power_balance.dat");
    Plato::Utils::ignore_unused(tTrash);
}

TEST(EnforceBounds, applyBounds)
{
    Plato::InputData tNode;
    Plato::EnforceBounds tEnforceBounds(NULL, tNode);

    int tLength = 10;
    double tDataToBound[10] = {1, -3, 5, 10, 2, 7, 22, 33, 0, 0};
    double tLowerBound[10] =  {1.5, 0, 5, -10, 3, 7, -1, 14, -1, -1};
    double tUpperBound[10] =  {2, 1, 5, 1, 3.3, 7.5, 0, 15, 0, 1};

    tEnforceBounds.applyBounds(tLength, tLowerBound, tUpperBound, tDataToBound); 

    EXPECT_EQ(tDataToBound[0], 1.5);
    EXPECT_EQ(tDataToBound[1], 0);
    EXPECT_EQ(tDataToBound[2], 5);
    EXPECT_EQ(tDataToBound[3], 1);
    EXPECT_EQ(tDataToBound[4], 3);
    EXPECT_EQ(tDataToBound[5], 7);
    EXPECT_EQ(tDataToBound[6], 0);
    EXPECT_EQ(tDataToBound[7], 15);
    EXPECT_EQ(tDataToBound[8], 0);
    EXPECT_EQ(tDataToBound[9], 0);
}

} // end PlatoTestOperations namespace
