#pragma once

#include "Plato_Application.hpp"
#include "PlatoPythonOperation.hpp"
#include "Plato_InputData.hpp"
#include "Plato_SharedData.hpp"

#include <string>
#include <vector>
#include <optional>

#include <mpi.h>

#include <pybind11/embed.h>

class __attribute__ ((visibility("hidden"))) PlatoPythonApp : public Plato::Application
{
public:
    PlatoPythonApp
    (int aArgc, 
     char **aArgv, 
     MPI_Comm& aLocalComm);

    ~PlatoPythonApp() = default;

    PlatoPythonApp(const PlatoPythonApp& aApp) = delete;

    PlatoPythonApp&
    operator=(const PlatoPythonApp& aApp) = delete;

    PlatoPythonApp(PlatoPythonApp&& aApp) = delete;

    PlatoPythonApp&
    operator=(PlatoPythonApp&& aApp) = delete;

    void 
    finalize() override;

    void 
    initialize() override;

    void 
    compute(const std::string & aOperationName) override;

    void 
    exportData
    (const std::string & aArgumentName, 
     Plato::SharedData & aExportData) override;

    void 
    importData
    (const std::string & aArgumentName, 
     const Plato::SharedData & aImportData) override;

    void 
    exportDataMap
    (const Plato::data::layout_t & aDataLayout, 
     std::vector<int> & aMyOwnedGlobalIDs) override;

    const MPI_Comm& 
    getComm() const;

    double 
    getCriterionValue(const std::string & aArgumentName);

    std::vector<double>
    getCriterionGradient(const std::string & aArgumentName);

    std::vector<double>
    getOperationInput(const std::string & aArgumentName);

private:
    void
    storeAppFile();

    void
    parseInputFile
    (int aArgc, 
     char **aArgv);

    void
    parsePythonBlock(const Plato::InputData & aInputFileData);

    void
    parseOperations();

    void
    setPythonPaths();

    void
    constructPythonObject();

    void
    getFieldSize();

    void
    throwIfFieldSizeNotSet();

private:
    pybind11::scoped_interpreter mInterpreter;
    MPI_Comm mLocalComm;
    Plato::InputData mAppfileData;
    pybind11::object mPythonObject;
    std::string mPythonModule;
    std::string mPythonClass;
    std::vector<std::string> mPythonPaths;
    std::vector<std::unique_ptr<PlatoPythonOperation>> mOperations;
    std::optional<int> mFieldSize;
};
