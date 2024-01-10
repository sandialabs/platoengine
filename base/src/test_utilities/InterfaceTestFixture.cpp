#include "InterfaceTestFixture.hpp"

#include "Plato_Interface.hpp"
#include "Plato_OptimizerEngineStageData.hpp"
#include "Plato_Parser.hpp"
#include "Plato_OptimizerUtilities.hpp"

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>

#include <stdlib.h>

namespace Plato
{
void
InterfaceTestFixture::SetUp()
{
    mInterfaceFilePath = "./dummyInterface.xml";
    mEnvIsSet = setenv("PLATO_INTERFACE_FILE", mInterfaceFilePath.c_str(), true);
    mIDIsSet = setenv("PLATO_PERFORMER_ID", "0", true);
}

void 
InterfaceTestFixture::TearDown()
{
    std::filesystem::remove(mInterfaceFilePath);
}

void
InterfaceTestFixture::setInterface()
{
    mInterface = std::make_unique<Plato::Interface>();
}

void 
InterfaceTestFixture::writeInterfaceFile
(const std::vector<std::string> & aContents)
{
    this->writeFile(aContents, mInterfaceFilePath);
}

void 
InterfaceTestFixture::writeFile
(const std::vector<std::string> & aContents,
 const std::filesystem::path & aFilePath)  
{
    std::ofstream tFile;
    tFile.open(aFilePath, std::ofstream::out | std::ofstream::trunc);

    tFile << "<?xml version=\"1.0\"?> \n";
    std::copy(aContents.cbegin(), aContents.cend(), std::ostream_iterator<std::string>(tFile, ""));

    tFile.close();

    this->setInterface();
}

void 
InterfaceTestFixture::useExistingInterfaceFile(const std::filesystem::path & aFilePath) 
{
    std::filesystem::copy(aFilePath, mInterfaceFilePath);
    this->setInterface();
}

Plato::OptimizerEngineStageData
InterfaceTestFixture::constructOptimizerData()
{
    std::vector<size_t> tDummyIndex = {0};
    auto tOptimizerNode = Plato::getOptimizerNode(mInterface.get(), tDummyIndex);

    Plato::OptimizerEngineStageData tEngineData;
    Plato::Parse::parseOptimizationVariablesNames(tOptimizerNode, tEngineData);
    Plato::Parse::parseInitialGuess(tOptimizerNode, tEngineData);
    Plato::Parse::parseOptimizerOptions(tOptimizerNode, tEngineData);

    return tEngineData;
}
}
