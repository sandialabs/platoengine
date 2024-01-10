#pragma once

#include "Plato_Interface.hpp"
#include "Plato_OptimizerEngineStageData.hpp"

#include <string>
#include <vector>
#include <filesystem>
#include <memory>

#include <gtest/gtest.h>

namespace Plato
{
class InterfaceTestFixture : public ::testing::Test
{
protected:
    void 
    SetUp() override;

    void 
    TearDown() override;

    void
    setInterface();

    void 
    writeInterfaceFile
    (const std::vector<std::string> & aContents = 
        {"<Null> \n", "</Null> \n"});

    void 
    writeFile
    (const std::vector<std::string> & aContents,
     const std::filesystem::path & aFilePath);

    void 
    useExistingInterfaceFile(const std::filesystem::path & aFilePath);

    Plato::OptimizerEngineStageData
    constructOptimizerData();

protected:
    std::filesystem::path mInterfaceFilePath;
    int mEnvIsSet;
    int mIDIsSet;
    std::unique_ptr<Plato::Interface> mInterface;
};
}
