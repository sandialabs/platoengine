#include "PlatoPythonApp.hpp"
#include "Plato_FreeFunctions.hpp"
#include "Plato_Macros.hpp"
#include "Plato_Communication.hpp"
#include "Plato_SharedValue.hpp"

#include <cstdio>
#include <glob.h>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <mpi.h>

namespace TestPythonApp
{

class PlatoTestPythonApp : public ::testing::Test
{
protected:
    void 
    SetUp() override
    {
        mAppFileName = "dummyApp.xml";
        mInputFileName = "dummyInput.xml";
        mEnvIsSet = setenv("PLATO_APP_FILE", mAppFileName.c_str(), true);
    }

    void 
    TearDown() override
    {
        std::string tDeleteCommand = "rm -rf " + mAppFileName + " " + mInputFileName;
        Plato::system(tDeleteCommand.c_str());
    }

    void 
    writeAppFile
    (const std::vector<std::string> & aContents = 
        {"<?xml version=\"1.0\"?> \n", "<Null> \n", "</Null> \n"})
    {
        this->writeFile(aContents, mAppFileName);
    }

    void 
    writeFile
    (const std::vector<std::string> & aContents,
     const std::string & aFileName)  
    {
        std::ofstream tAppFile;
        tAppFile.open(aFileName.c_str(), std::ofstream::out | std::ofstream::trunc);

        for(auto tLine : aContents)
        {
            tAppFile << tLine;
        }
    
        tAppFile.close();
    }

    void 
    writeInputFile
    (const std::vector<std::string> & aContents = 
        {"<?xml version=\"1.0\"?> \n", "<Null> \n", "</Null> \n"})
    {
        this->writeFile(aContents, mInputFileName);
    }

    std::unique_ptr<PlatoPythonApp>
    defaultApp()
    {
        MPI_Comm myComm;
        MPI_Comm_dup(MPI_COMM_WORLD, &myComm);

        int tArgC = 2;

        std::string tProgName = "DummyApp";
        char* tArgV[2] = {(char*)tProgName.c_str(), (char*)mInputFileName.c_str()};

        return std::make_unique<PlatoPythonApp>(tArgC, tArgV, myComm);
    }

    std::string 
    executeCommand(const char* aCommand) 
    {
        char tBuffer[128];
        std::string tResult = "";
        FILE* tPipe = popen(aCommand, "r");
        if (!tPipe) 
        {
            THROWERR( std::string("popen() failed when trying to run command: " + std::string(aCommand)) )
        }

        while (fgets(tBuffer, sizeof(tBuffer), tPipe) != NULL) 
        {
            size_t tLen = strlen(tBuffer);
            if (tLen > 0 && tBuffer[tLen-1] == '\n') 
            {
                tBuffer[tLen-1] = '\0';
            }
            tResult += tBuffer;
        }
        pclose(tPipe);

        return tResult;
    }

    std::string
    getNumpyPath()
    {
        std::string tNumpyPath = executeCommand("spack location -i py-numpy");
        std::string tModulePath = tNumpyPath + "/lib/python*/site-packages/";

        glob_t tGlobResult;
        int tGlobStatus = glob(tModulePath.c_str(), 0, nullptr, &tGlobResult);

        if (tGlobStatus != 0 || tGlobResult.gl_pathc == 0) {
            THROWERR("Error: could not find numpy build directory. Make sure spack env is sourced.")
        }
        std::string tPath = tGlobResult.gl_pathv[0];
        globfree(&tGlobResult);
        return tPath;
    }

protected:
    std::string mAppFileName;
    std::string mInputFileName;
    int mEnvIsSet;

};

TEST_F(PlatoTestPythonApp, EnvIsSetCorrectly)
{
    EXPECT_EQ(mEnvIsSet, 0);
}

TEST_F(PlatoTestPythonApp, Constructor_ErrorNotEnoughCommandLineArguments)
{
    writeAppFile();

    MPI_Comm myComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &myComm);
    EXPECT_ANY_THROW(PlatoPythonApp tApp(0, nullptr, myComm));
}

TEST_F(PlatoTestPythonApp, Constructor_ErrorTooManyCommandLineArguments)
{
    writeAppFile();

    MPI_Comm myComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &myComm);
    int tArgC = 3;
    char* tArgV[3];
    EXPECT_ANY_THROW(PlatoPythonApp tApp(tArgC, tArgV, myComm));
}

TEST_F(PlatoTestPythonApp, Constructor_ErrorAppFileEnvVariableNotSet)
{
    mEnvIsSet = unsetenv("PLATO_APP_FILE");

    writeAppFile();
    writeInputFile();

    MPI_Comm myComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &myComm);
    EXPECT_THROW(PlatoPythonApp tApp(0, nullptr, myComm), std::runtime_error);
}

TEST_F(PlatoTestPythonApp, Constructor_ErrorAppFileEmpty)
{
    writeAppFile({""});
    writeInputFile();

    MPI_Comm myComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &myComm);
    EXPECT_ANY_THROW(PlatoPythonApp tApp(0, nullptr, myComm));
}

TEST_F(PlatoTestPythonApp, Constructor_ErrorInputFileNotFound)
{
    writeAppFile();

    MPI_Comm myComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &myComm);

    int tArgC = 2;
    std::string tProgName = "DummyApp";
    std::string tInputFileName = "nonexistant_file.xml";
    char* tArgV[2] = {(char*)tProgName.c_str(), (char*)tInputFileName.c_str()};

    EXPECT_ANY_THROW(PlatoPythonApp tApp(tArgC, tArgV, myComm));
}

TEST_F(PlatoTestPythonApp, Constructor_ErrorInputFileEmpty)
{
    writeAppFile();
    writeInputFile({""});

    EXPECT_ANY_THROW(const auto tApp = defaultApp());
}

TEST_F(PlatoTestPythonApp, Constructor_ErrorNoPythonBlockInInput)
{
    writeAppFile();
    writeInputFile();

    EXPECT_THROW(const auto tApp = defaultApp(), std::runtime_error);
}

TEST_F(PlatoTestPythonApp, Constructor_ErrorMultiplePythonBlocksInInput)
{
    std::vector<std::string> tFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "</Python> \n",
        "<Python> \n",
        "</Python> \n"
    };
    writeAppFile();
    writeInputFile(tFileContents);

    EXPECT_THROW(const auto tApp = defaultApp(), std::runtime_error);
}

TEST_F(PlatoTestPythonApp, Constructor_ErrorModuleNotProvidedInInput)
{
    std::vector<std::string> tFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Class>TestFunctions</Class> \n",
        "</Python> \n"
    };
    writeAppFile();
    writeInputFile(tFileContents);

    EXPECT_THROW(const auto tApp = defaultApp(), std::runtime_error);
}

TEST_F(PlatoTestPythonApp, Constructor_ErrorClassNotProvidedInInput)
{
    std::vector<std::string> tFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>test_functions</Module> \n",
        "</Python> \n"
    };
    writeAppFile();
    writeInputFile(tFileContents);

    EXPECT_THROW(const auto tApp = defaultApp(), std::runtime_error);
}

TEST_F(PlatoTestPythonApp, Constructor_ErrorNoOperationsDefined)
{
    writeAppFile();
    writeInputFile();

    EXPECT_THROW(const auto tApp = defaultApp(), std::runtime_error);
}

TEST_F(PlatoTestPythonApp, Constructor_ErrorFunctionNotProvidedInOperation)
{
    std::vector<std::string> tFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Name>ComputeObjectiveValue</Name> \n",
        "  <PyModule>test_functions</PyModule> \n",
        "  <PyFunction>return_val</PyFunction> \n",
        "</Operation> \n"
    };
    writeAppFile(tFileContents);
    writeInputFile();

    EXPECT_THROW(const auto tApp = defaultApp(), std::runtime_error);
}

TEST_F(PlatoTestPythonApp, Constructor_ErrorInvalidFunctionNameInOperation)
{
    std::vector<std::string> tFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeSomethingMadeUp</Function> \n",
        "  <Name>ComputeCriterionValue</Name> \n",
        "  <PyModule>test_functions</PyModule> \n",
        "  <PyFunction>return_val</PyFunction> \n",
        "</Operation> \n"
    };
    writeAppFile(tFileContents);
    writeInputFile();

    EXPECT_THROW(const auto tApp = defaultApp(), std::runtime_error);
}

TEST_F(PlatoTestPythonApp, Constructor_ErrorNameNotProvidedInOperation)
{
    std::vector<std::string> tFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionValue</Function> \n",
        "  <PyModule>test_functions</PyModule> \n",
        "  <PyFunction>return_val</PyFunction> \n",
        "</Operation> \n"
    };
    writeAppFile(tFileContents);
    writeInputFile();

    EXPECT_THROW(const auto tApp = defaultApp(), std::runtime_error);
}

TEST_F(PlatoTestPythonApp, Constructor_ErrorPyFunctionNotProvidedInOperation)
{
    std::vector<std::string> tFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionValue</Function> \n",
        "  <Name>ComputeObjectiveValue</Name> \n",
        "  <PyModule>test_functions</PyModule> \n",
        "</Operation> \n"
    };
    writeAppFile(tFileContents);
    writeInputFile();

    EXPECT_THROW(const auto tApp = defaultApp(), std::runtime_error);
}

TEST_F(PlatoTestPythonApp, Constructor_ErrorMultipleInputsInOperation)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionValue</Function> \n",
        "  <Name>ComputeCriterionValue</Name> \n",
        "  <PyFunction>return_val</PyFunction> \n",
        "  <Input> \n",
        "    <ArgumentName>Design Parameters 0</ArgumentName> \n",
        "  </Input> \n",
        "  <Input> \n",
        "    <ArgumentName>Design Parameters 1</ArgumentName> \n",
        "  </Input> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>test_functions</Module> \n",
        "  <Class>TestFunctions</Class> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    EXPECT_THROW(const auto tApp = defaultApp(), std::runtime_error);
}

TEST_F(PlatoTestPythonApp, Constructor_ErrorMultipleOutputsInOperation)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionValue</Function> \n",
        "  <Name>ComputeCriterionValue</Name> \n",
        "  <PyFunction>return_val</PyFunction> \n",
        "  <Output> \n",
        "    <ArgumentName>Objective Value</ArgumentName> \n",
        "  </Output> \n",
        "  <Output> \n",
        "    <ArgumentName>Objective Gradient</ArgumentName> \n",
        "  </Output> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>test_functions</Module> \n",
        "  <Class>TestFunctions</Class> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    EXPECT_THROW(const auto tApp = defaultApp(), std::runtime_error);
}

TEST_F(PlatoTestPythonApp, Constructor_MpiCommIsStoredCorrectly)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionValue</Function> \n",
        "  <Name>ComputeCriterionValue</Name> \n",
        "  <PyFunction>return_val</PyFunction> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>test_functions</Module> \n",
        "  <Class>TestFunctions</Class> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    const auto tApp = defaultApp();

    auto tAppComm = tApp->getComm();

    int appCommSize;
    MPI_Comm_size(tAppComm, &appCommSize);
    EXPECT_EQ(appCommSize, 1);

    int appCommRank;
    MPI_Comm_rank(tAppComm, &appCommRank);
    EXPECT_EQ(appCommRank, 0);
}

TEST_F(PlatoTestPythonApp, Constructor_InterpreterUninitializedUponConstruction)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionValue</Function> \n",
        "  <Name>ComputeCriterionValue</Name> \n",
        "  <PyFunction>return_val</PyFunction> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>test_functions</Module> \n",
        "  <Class>TestFunctions</Class> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    const auto tApp = defaultApp();

    EXPECT_EQ(tApp->isInitialized(), false);
}

TEST_F(PlatoTestPythonApp, Initialize_InterpreterInitializes)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionValue</Function> \n",
        "  <Name>ComputeCriterionValue</Name> \n",
        "  <PyFunction>return_val</PyFunction> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>test_functions</Module> \n",
        "  <Class>TestFunctions</Class> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    const auto tApp = defaultApp();

    tApp->initialize();
    EXPECT_EQ(tApp->isInitialized(), true);
    tApp->finalize();
}

TEST_F(PlatoTestPythonApp, Initialize_ErrorPythonModuleNotFound)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionValue</Function> \n",
        "  <Name>ComputeObjectiveValue</Name> \n",
        "  <PyFunction>return_val</PyFunction> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>nonexistant_functions</Module> \n",
        "  <Class>TestFunctions</Class> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    const auto tApp = defaultApp();
    EXPECT_ANY_THROW(tApp->initialize());
    tApp->finalize();
}

TEST_F(PlatoTestPythonApp, Initialize_ErrorPythonClassNotFound)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionValue</Function> \n",
        "  <Name>ComputeObjectiveValue</Name> \n",
        "  <PyFunction>return_val</PyFunction> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>test_functions</Module> \n",
        "  <Class>NothingBurger</Class> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    const auto tApp = defaultApp();
    EXPECT_ANY_THROW(tApp->initialize());
    tApp->finalize();
}

TEST_F(PlatoTestPythonApp, Finalize_InterpreterUninitializes)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionValue</Function> \n",
        "  <Name>ComputeCriterionValue</Name> \n",
        "  <PyFunction>return_val</PyFunction> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>test_functions</Module> \n",
        "  <Class>TestFunctions</Class> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    const auto tApp = defaultApp();

    tApp->initialize();
    EXPECT_EQ(tApp->isInitialized(), true);

    tApp->finalize();
    EXPECT_EQ(tApp->isInitialized(), false);
}

TEST_F(PlatoTestPythonApp, Compute_ErrorFunctionNotFound)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionValue</Function> \n",
        "  <Name>ComputeObjectiveValue</Name> \n",
        "  <PyFunction>nonexistant</PyFunction> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>test_functions</Module> \n",
        "  <Class>TestFunctions</Class> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    const auto tApp = defaultApp();

    tApp->initialize();
    EXPECT_ANY_THROW(tApp->compute("ComputeObjectiveValue"));
    tApp->finalize();
}

TEST_F(PlatoTestPythonApp, Compute_ErrorNoReturnValue)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionValue</Function> \n",
        "  <Name>ComputeObjectiveValue</Name> \n",
        "  <PyFunction>return_nothing</PyFunction> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>test_functions</Module> \n",
        "  <Class>TestFunctions</Class> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    const auto tApp = defaultApp();

    tApp->initialize();
    EXPECT_ANY_THROW(tApp->compute("ComputeObjectiveValue"));
    tApp->finalize();
}

TEST_F(PlatoTestPythonApp, Compute_ErrorListNotReturnedInCriterionGradientOperation)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionGradient</Function> \n",
        "  <Name>BadObjectiveGradient</Name> \n",
        "  <PyFunction>return_val</PyFunction> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>import_functions</Module> \n",
        "  <Class>ImportFunctions</Class> \n",
        "  <Path>./import</Path> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    const auto tApp = defaultApp();
    tApp->initialize();

    EXPECT_ANY_THROW(tApp->compute("BadObjectiveGradient"));

    tApp->finalize();
}

TEST_F(PlatoTestPythonApp, Compute_DummyCriterionValueOperation)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionValue</Function> \n",
        "  <Name>ComputeObjectiveValue</Name> \n",
        "  <PyFunction>return_val</PyFunction> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>test_functions</Module> \n",
        "  <Class>TestFunctions</Class> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    const auto tApp = defaultApp();

    tApp->initialize();
    tApp->compute("ComputeObjectiveValue");
    tApp->finalize();
    EXPECT_EQ(tApp->getCriterionValue("ComputeObjectiveValue"), 88);
}

TEST_F(PlatoTestPythonApp, Compute_DummyCriterionValueFromImportedModule)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionValue</Function> \n",
        "  <Name>ComputeObjectiveValue</Name> \n",
        "  <PyFunction>return_val</PyFunction> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>import_functions</Module> \n",
        "  <Class>ImportFunctions</Class> \n",
        "  <Path>./import</Path> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    const auto tApp = defaultApp();

    tApp->initialize();
    tApp->compute("ComputeObjectiveValue");
    tApp->finalize();
    EXPECT_EQ(tApp->getCriterionValue("ComputeObjectiveValue"), 77);
}

TEST_F(PlatoTestPythonApp, Compute_DummyCriterionGradientFromNumpyModule)
{
    std::string tPyPath = "<Path>" + getNumpyPath() + "</Path>";

    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionGradient</Function> \n",
        "  <Name>ComputeObjectiveGradient</Name> \n",
        "  <PyFunction>return_array</PyFunction> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>numpy_functions</Module> \n",
        "  <Class>NumpyFunctions</Class> \n",
        "  <Path>./import</Path> \n",
            tPyPath,
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    const auto tApp = defaultApp();
    tApp->initialize();

    tApp->compute("ComputeObjectiveGradient");

    auto tGrad = tApp->getCriterionGradient("ComputeObjectiveGradient");
    ASSERT_EQ(tGrad.size(), 3);
    EXPECT_EQ(tGrad[0], 91);
    EXPECT_EQ(tGrad[1], 71);
    EXPECT_EQ(tGrad[2], 14);

    tApp->finalize();
}

TEST_F(PlatoTestPythonApp, Compute_RunVoidFunctionToChangeStateForGradient)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionGradient</Function> \n",
        "  <Name>ComputeObjectiveGradient</Name> \n",
        "  <PyFunction>return_list</PyFunction> \n",
        "</Operation> \n",
        "<Operation> \n",
        "  <Function>RunVoidFunction</Function> \n",
        "  <Name>ChangeState</Name> \n",
        "  <PyFunction>change_state</PyFunction> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>import_functions</Module> \n",
        "  <Class>ImportFunctions</Class> \n",
        "  <Path>./import</Path> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    const auto tApp = defaultApp();

    tApp->initialize();

    tApp->compute("ComputeObjectiveGradient");
    auto tGrad = tApp->getCriterionGradient("ComputeObjectiveGradient");
    ASSERT_EQ(tGrad.size(), 3);
    EXPECT_EQ(tGrad[0], 19);
    EXPECT_EQ(tGrad[1], 17);
    EXPECT_EQ(tGrad[2], 44);

    tApp->compute("ChangeState");
    tApp->compute("ComputeObjectiveGradient");
    tGrad = tApp->getCriterionGradient("ComputeObjectiveGradient");
    ASSERT_EQ(tGrad.size(), 3);
    EXPECT_EQ(tGrad[0], 38);
    EXPECT_EQ(tGrad[1], 34);
    EXPECT_EQ(tGrad[2], 88);

    tApp->finalize();
}

TEST_F(PlatoTestPythonApp, Compute_RunVoidFunctionToChangeStateForGradientWhenInputsChange)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionGradient</Function> \n",
        "  <Name>ComputeObjectiveGradient</Name> \n",
        "  <PyFunction>return_list</PyFunction> \n",
        "</Operation> \n",
        "<Operation> \n",
        "  <Function>RunVoidFunction</Function> \n",
        "  <Name>ChangeState</Name> \n",
        "  <OnChange>true</OnChange> \n",
        "  <PyFunction>change_state</PyFunction> \n",
        "  <Input> \n",
        "    <ArgumentName>Parameters</ArgumentName> \n",
        "  </Input> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>import_functions</Module> \n",
        "  <Class>ImportFunctions</Class> \n",
        "  <Path>./import</Path> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    Plato::CommunicationData tDummyCommData;
    Plato::SharedValue tSharedData("dumbo", {"nada"}, tDummyCommData, Plato::data::SCALAR, 1, true);
    std::vector<double> tVals = {21.0, 56.0};
    tSharedData.setData(tVals);

    const auto tApp = defaultApp();

    tApp->initialize();

    tApp->compute("ComputeObjectiveGradient");
    auto tGrad = tApp->getCriterionGradient("ComputeObjectiveGradient");
    ASSERT_EQ(tGrad.size(), 3);
    EXPECT_EQ(tGrad[0], 19);
    EXPECT_EQ(tGrad[1], 17);
    EXPECT_EQ(tGrad[2], 44);

    tApp->importData("Parameters", tSharedData);
    tApp->compute("ChangeState");
    tApp->compute("ComputeObjectiveGradient");
    tGrad = tApp->getCriterionGradient("ComputeObjectiveGradient");
    ASSERT_EQ(tGrad.size(), 3);
    EXPECT_EQ(tGrad[0], 38);
    EXPECT_EQ(tGrad[1], 34);
    EXPECT_EQ(tGrad[2], 88);

    tApp->importData("Parameters", tSharedData);
    tApp->compute("ChangeState");
    tApp->compute("ComputeObjectiveGradient");
    tGrad = tApp->getCriterionGradient("ComputeObjectiveGradient");
    ASSERT_EQ(tGrad.size(), 3);
    EXPECT_EQ(tGrad[0], 38);
    EXPECT_EQ(tGrad[1], 34);
    EXPECT_EQ(tGrad[2], 88);

    tVals[0] = 71.0;
    tSharedData.setData(tVals);
    tApp->importData("Parameters", tSharedData);
    tApp->compute("ChangeState");
    tApp->compute("ComputeObjectiveGradient");
    tGrad = tApp->getCriterionGradient("ComputeObjectiveGradient");
    ASSERT_EQ(tGrad.size(), 3);
    EXPECT_EQ(tGrad[0], 76);
    EXPECT_EQ(tGrad[1], 68);
    EXPECT_EQ(tGrad[2], 176);

    tApp->finalize();
}

TEST_F(PlatoTestPythonApp, ImportData_ErrorWrongSharedDataName)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>RunVoidFunction</Function> \n",
        "  <Name>ChangeState</Name> \n",
        "  <PyFunction>change_state</PyFunction> \n",
        "  <Input> \n",
        "    <ArgumentName>Parameters</ArgumentName> \n",
        "  </Input> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>import_functions</Module> \n",
        "  <Class>ImportFunctions</Class> \n",
        "  <Path>./import</Path> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    Plato::CommunicationData tDummyCommData;
    Plato::SharedValue tSharedData("dumbo", {"nada"}, tDummyCommData, Plato::data::SCALAR, 1, false);

    const auto tApp = defaultApp();

    tApp->initialize();
    EXPECT_THROW(tApp->importData("Design Values", tSharedData), std::runtime_error);
}

TEST_F(PlatoTestPythonApp, ImportData_InputStoredCorrectly)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>RunVoidFunction</Function> \n",
        "  <Name>ChangeState</Name> \n",
        "  <PyFunction>change_state</PyFunction> \n",
        "  <Input> \n",
        "    <ArgumentName>Parameters</ArgumentName> \n",
        "  </Input> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>import_functions</Module> \n",
        "  <Class>ImportFunctions</Class> \n",
        "  <Path>./import</Path> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    Plato::CommunicationData tDummyCommData;
    Plato::SharedValue tSharedData("dumbo", {"nada"}, tDummyCommData, Plato::data::SCALAR, 1, true);
    std::vector<double> tVals = {21.0, 56.0};
    tSharedData.setData(tVals);

    const auto tApp = defaultApp();

    tApp->initialize();
    tApp->importData("Parameters", tSharedData);
    auto tInputData = tApp->getOperationInput("ChangeState");
    ASSERT_EQ(tInputData.size(), 2);
    EXPECT_EQ(tInputData[0], 21);
    EXPECT_EQ(tInputData[1], 56);
}

TEST_F(PlatoTestPythonApp, ExportData_ErrorWrongSharedDataName)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionValue</Function> \n",
        "  <Name>ComputeObjectiveValue</Name> \n",
        "  <PyFunction>return_val</PyFunction> \n",
        "  <Output> \n",
        "    <ArgumentName>Objective Value</ArgumentName> \n",
        "  </Output> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>test_functions</Module> \n",
        "  <Class>TestFunctions</Class> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    Plato::CommunicationData tDummyCommData;
    Plato::SharedValue tSharedData("dumbo", {"nada"}, tDummyCommData, Plato::data::SCALAR, 1, false);

    const auto tApp = defaultApp();

    tApp->initialize();
    EXPECT_THROW(tApp->exportData("Objection Value", tSharedData), std::runtime_error);
}

TEST_F(PlatoTestPythonApp, ExportData_StoreValueInSharedValueClass)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionValue</Function> \n",
        "  <Name>ComputeObjectiveValue</Name> \n",
        "  <PyFunction>return_val</PyFunction> \n",
        "  <Output> \n",
        "    <ArgumentName>Objective Value</ArgumentName> \n",
        "  </Output> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>test_functions</Module> \n",
        "  <Class>TestFunctions</Class> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    Plato::CommunicationData tDummyCommData;
    Plato::SharedValue tSharedData("dumbo", {"nada"}, tDummyCommData, Plato::data::SCALAR, 1, false);

    const auto tApp = defaultApp();

    tApp->initialize();
    tApp->compute("ComputeObjectiveValue");
    tApp->exportData("Objective Value", tSharedData);
    tApp->finalize();

    std::vector<double> tReturnVals(1);
    tSharedData.getData(tReturnVals);
    ASSERT_EQ(tReturnVals.size(), 1);
    EXPECT_EQ(tReturnVals[0], 88);
}

TEST_F(PlatoTestPythonApp, ExportData_StoreGradientInSharedValueClass)
{
    std::vector<std::string> tAppFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Operation> \n",
        "  <Function>ComputeCriterionGradient</Function> \n",
        "  <Name>ComputeObjectiveGradient</Name> \n",
        "  <PyFunction>return_list</PyFunction> \n",
        "  <Output> \n",
        "    <ArgumentName>Objective Gradient</ArgumentName> \n",
        "  </Output> \n",
        "</Operation> \n"
    };
    writeAppFile(tAppFileContents);

    std::vector<std::string> tInputFileContents = {
        "<?xml version=\"1.0\"?> \n",
        "<Python> \n",
        "  <Module>import_functions</Module> \n",
        "  <Class>ImportFunctions</Class> \n",
        "  <Path>./import</Path> \n",
        "</Python> \n"
    };
    writeInputFile(tInputFileContents);

    Plato::CommunicationData tDummyCommData;
    bool tIsDynamic = true;
    Plato::SharedValue tSharedData("dumbo", {"nada"}, tDummyCommData, Plato::data::SCALAR, 1, tIsDynamic);

    const auto tApp = defaultApp();

    tApp->initialize();
    tApp->compute("ComputeObjectiveGradient");
    tApp->exportData("Objective Gradient", tSharedData);
    tApp->finalize();

    std::vector<double> tReturnVals;
    tSharedData.getData(tReturnVals);
    ASSERT_EQ(tReturnVals.size(), 3);
    EXPECT_EQ(tReturnVals[0], 19);
    EXPECT_EQ(tReturnVals[1], 17);
    EXPECT_EQ(tReturnVals[2], 44);
}

}