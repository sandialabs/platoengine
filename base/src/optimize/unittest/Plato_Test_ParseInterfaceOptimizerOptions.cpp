#include "InterfaceTestFixture.hpp"

#include "Plato_Interface.hpp"
#include "Plato_OptimizerEngineStageData.hpp"

#include "Plato_SharedDataInfo.hpp"
#include "Plato_Communication.hpp"

#include "Plato_DistributedVectorROL.hpp"

#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

class PlatoTestParseInterfaceOptimizerOptions : public Plato::InterfaceTestFixture
{
};

TEST_F(PlatoTestParseInterfaceOptimizerOptions, ParseGradientCheckParameters)
{
    // construct Plato interface
    const std::vector<std::string> tFileContents = {
        "<Performer> \n",
        "  <Name>PlatoMain</Name> \n",
        "  <Code>Plato_Main</Code> \n",
        "  <PerformerID>0</PerformerID> \n",
        "</Performer> \n",
        "<Optimizer> \n",
        "  <Options>\n",
        "    <ROLGradientCheckPerturbationScale>2.0</ROLGradientCheckPerturbationScale>\n",
        "    <ROLGradientCheckSeed>123</ROLGradientCheckSeed>\n",
        "    <ROLGradientCheckSteps>4</ROLGradientCheckSteps>\n",
        "    <ROLGradientCheckStepSize>2</ROLGradientCheckStepSize>\n",
        "  </Options>\n",
        "</Optimizer> \n"
    };
    writeInterfaceFile(tFileContents);

    auto tEngineData = constructOptimizerData();
     
    const auto tPerturbationScale = tEngineData.getROLPerturbationScale();
    const auto tCheckGradientSeed = tEngineData.getROLCheckGradientSeed();

    EXPECT_EQ(tPerturbationScale, 2.0);
    EXPECT_EQ(tCheckGradientSeed, 123);
    
    const auto tCheckGradientSteps = tEngineData.getROLCheckGradientSteps();
    const auto tCheckGradientStepSize = tEngineData.getROLCheckGradientStepSize();
    EXPECT_EQ(tCheckGradientSteps, 4);
    EXPECT_EQ(tCheckGradientStepSize, 2);
}

TEST_F(PlatoTestParseInterfaceOptimizerOptions, ConstructControlsFromInterfaceFile)
{
    // construct Plato interface
    const std::vector<std::string> tFileContents = {
        "<Performer> \n",
        "  <Name>PlatoMain</Name> \n",
        "  <Code>Plato_Main</Code> \n",
        "  <PerformerID>0</PerformerID> \n",
        "</Performer> \n",
        "<SharedData> \n",
        "  <Name>Design Parameters</Name> \n",
        "  <Type>Scalar</Type> \n",
        "  <Layout>Global</Layout> \n",
        "  <Size>5</Size> \n",
        "</SharedData> \n",
        "<Optimizer> \n",
        "  <OptimizationVariables>\n",
        "    <ValueName>Design Parameters</ValueName>\n",
        "    <InitializationStage></InitializationStage>\n",
        "  </OptimizationVariables>\n",
        "  <InitialGuess>\n",
        "    <Value>88</Value>\n",
        "  </InitialGuess>\n",
        "</Optimizer> \n"
    };
    writeInterfaceFile(tFileContents);

    //  construct optimizer engine stage data
    auto tEngineData = constructOptimizerData();
     
    // test control name
    constexpr size_t tCONTROL_VECTOR_INDEX = 0;
    std::string tControlName = tEngineData.getControlName(tCONTROL_VECTOR_INDEX);

    EXPECT_EQ(tControlName, "Design Parameters");

    // test initial guess (computing like it is in setInitialGuess method of ROL interface)
    EXPECT_EQ(tEngineData.getInitializationStageName().empty(), true);

    std::vector<double> tInitialGuess = tEngineData.getInitialGuess();
    ASSERT_EQ(tInitialGuess.size(), 1);
    EXPECT_EQ(tInitialGuess[0], 88);

    // construct shared data info for data layer
    constexpr std::string_view tDataName = "Design Parameters";
    constexpr int tDataSize = 2;
    constexpr bool tIsDynamic = false;
    constexpr std::string_view tLayout = "GLOBAL";
    const std::vector<std::string> tOwnerNames = {"platomain_1"};
    const std::vector<std::string> tUserNames = {"platomain_1"};
    constexpr Plato::communication::broadcast_t tBroadcastType = Plato::communication::broadcast_t::SENDER_AND_RECEIVER;

    Plato::SharedDataInfo tInfo;
    tInfo.setSharedDataMap(tOwnerNames, tUserNames);
    tInfo.setMyBroadcast(tBroadcastType);
    tInfo.setSharedDataSize(static_cast<std::string>(tDataName), tDataSize);
    tInfo.setSharedDataDynamic(static_cast<std::string>(tDataName), tIsDynamic);
    tInfo.setSharedDataIdentifiers(static_cast<std::string>(tDataName), static_cast<std::string>(tLayout));

    // set interface data layer
    const Plato::CommunicationData tDummyCommData;
    mInterface->setDataLayer(tInfo, tDummyCommData);

    // construct control through interface
    const size_t tNumControls = mInterface->size(tControlName);

    MPI_Comm tLocalComm;
    mInterface->getLocalComm(tLocalComm);
    
    Teuchos::RCP<Plato::DistributedVectorROL<double>> tControls =
            Teuchos::rcp(new Plato::DistributedVectorROL<double>(tLocalComm, tNumControls));

    // The below lines are in the ROL interface, but the end result is the fill call that's here
    // auto tControlBoundsMng = this->setControlBounds(tNumControls);
    // this->setInitialGuess(tControlName, *tControls);
    tControls->fill(tInitialGuess[0]);

    // test control
    std::vector<double> tControlValues = tControls->vector();
    size_t tControlSize = tControlValues.size();
    ASSERT_EQ(tControlSize, 2);

    for(unsigned int iControl=0; iControl<tControlSize; iControl++)
    {
        EXPECT_EQ(tControlValues[iControl], 88);
    }
}

