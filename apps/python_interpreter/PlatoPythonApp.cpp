#include "PlatoPythonApp.hpp"
#include "PythonAppUtilities.hpp"
#include "Plato_Macros.hpp"
#include "Plato_InputData.hpp"
#include "Plato_SharedData.hpp"
#include "Plato_Parser.hpp"
#include "PlatoPythonOperation.hpp"
#include "PythonOperationFactory.hpp"

#include <memory>
#include <string>

#include <Python.h>
#include <boost/python.hpp>

PlatoPythonApp::PlatoPythonApp
(int aArgc, 
 char **aArgv, 
 MPI_Comm& aLocalComm) :
 mLocalComm(aLocalComm),
 mAppfileData("Appfile Data"),
 mPythonObject(boost::python::object())
{
    this->storeAppFile();
    this->parseInputFile(aArgc, aArgv);
    this->parseOperations();
}

PlatoPythonApp::~PlatoPythonApp()
{
    if ( this->isInitialized() )
    {
        mPythonObject = boost::python::object();
        Py_Finalize();
    }
}

void
PlatoPythonApp::storeAppFile()
{
    const char* input_char = getenv("PLATO_APP_FILE");
    if ( input_char )
    {
        Plato::PugiParser tParser;
        mAppfileData = tParser.parseFile(input_char);
    }

    if ( mAppfileData.empty() )
    {
        THROWERR("PlatoPythonApp: App file is empty. Ensure that app file for PlatoPython performer"
                 " exists and that the PLATO_APP_FILE environment variable is set to the file name.")
    }
}

void
PlatoPythonApp::parseInputFile
(int aArgc, 
 char **aArgv)
{
    if (aArgc != 2)
        THROWERR("PlatoPythonApp: Expected one command line argument.")
 
    Plato::PugiParser tParser;
    auto tInputFileData = tParser.parseFile(aArgv[1]);

    if ( tInputFileData.empty() )
    {
        THROWERR("PlatoPythonApp: Input file is empty. Ensure that input file for PlatoPython performer"
                 " exists and is the first argument.")
    }

    this->parsePythonBlock(tInputFileData);
}

void
PlatoPythonApp::parsePythonBlock(const Plato::InputData & aInputFileData)
{
    const auto& tPythonNodes = aInputFileData.getByName<Plato::InputData>("Python");
    if ( tPythonNodes.size() != 1 )
        THROWERR("PlatoPythonApp: One Python block should be specified in input file.")

    const auto& tPythonNode = tPythonNodes[0];
    mPythonModule = Plato::PythonApp::read_xml_field(tPythonNode, "Module");
    mPythonClass = Plato::PythonApp::read_xml_field(tPythonNode, "Class");

    for( const auto& tPath : tPythonNode.getByName<std::string>("Path") )
    {
        mPythonPaths.push_back(tPath);
    }
}

void
PlatoPythonApp::parseOperations()
{
    PythonOperationFactory tFactory;
    for( const auto& tOperationNode : mAppfileData.getByName<Plato::InputData>("Operation") )
    {
        mOperations.push_back(tFactory.create(tOperationNode));
    }

    if ( mOperations.empty() )
        THROWERR("No Operations were provided for Plato PythonInterpreter App")
}

void 
PlatoPythonApp::finalize()
{
    if ( this->isInitialized() )
    {
        mPythonObject = boost::python::object();
        Py_Finalize();
    }
}

void 
PlatoPythonApp::initialize()
{
    Py_Initialize();
    this->setPythonPaths();
    this->constructPythonObject();
}

void 
PlatoPythonApp::setPythonPaths()
{
    auto sys = boost::python::import("sys");
    auto path = sys.attr("path");

    path.attr("append")("./");
    for( const auto& tPathStr : mPythonPaths)
    {
        path.attr("append")(tPathStr.c_str());
    }
}

void
PlatoPythonApp::constructPythonObject()
{
    auto tModule = boost::python::import(mPythonModule.c_str());
    mPythonObject = tModule.attr(mPythonClass.c_str())();
}

void 
PlatoPythonApp::compute(const std::string & aOperationName)
{
    const auto& tOperation = this->findOperation(aOperationName);
    tOperation->runPythonFunction(mPythonObject);
}

const std::unique_ptr<PlatoPythonOperation>&
PlatoPythonApp::findOperation(const std::string & aOperationName)
{
    const auto& tOperation = std::find_if(mOperations.begin(), mOperations.end(), 
        [&aOperationName](std::unique_ptr<PlatoPythonOperation>& aOperation){
        return aOperation->name() == aOperationName;
    });

    if(tOperation != mOperations.end())
    {
        return *(tOperation);
    }
    else
    {
        THROWERR("Failed to find operation with name '" + aOperationName + "'")
    }
}

void 
PlatoPythonApp::exportData
(const std::string & aArgumentName, 
 Plato::SharedData & aExportData)
{
    const auto tOperation = std::find_if(mOperations.begin(), mOperations.end(), 
        [&aArgumentName](std::unique_ptr<PlatoPythonOperation>& aOperation){
        return aOperation->outputDataName() == aArgumentName;
    });

    if(tOperation != mOperations.end())
    {
        auto tData = (*tOperation)->getOutputData();
        aExportData.setData(tData);
    }
    else
    {
        THROWERR("Failed to find SharedData with name '" + aArgumentName + "' when exporting")
    }
}

void 
PlatoPythonApp::importData
(const std::string & aArgumentName, 
 const Plato::SharedData & aImportData)
{
    const auto tOperation = std::find_if(mOperations.begin(), mOperations.end(), 
        [&aArgumentName](std::unique_ptr<PlatoPythonOperation>& aOperation){
        return aOperation->inputDataName() == aArgumentName;
    });

    if(tOperation != mOperations.end())
    {
        std::vector<double> tData;
        tData.resize(aImportData.size());
        aImportData.getData(tData);
        (*tOperation)->setInputData(tData);
    }
    else
    {
        THROWERR("Failed to find SharedData with name '" + aArgumentName + "' when importing")
    }
}

void 
PlatoPythonApp::exportDataMap
(const Plato::data::layout_t & /*aDataLayout*/, 
 std::vector<int> & /*aMyOwnedGlobalIDs*/)
{
}

const MPI_Comm& 
PlatoPythonApp::getComm() const
{
    return mLocalComm;
}

bool
PlatoPythonApp::isInitialized() const
{
    return Py_IsInitialized();
}

double 
PlatoPythonApp::getCriterionValue(const std::string & aOperationName)
{
    const auto& tOperation = this->findOperation(aOperationName);
    auto tData = tOperation->getOutputData();
    return tData[0];
}

std::vector<double> 
PlatoPythonApp::getCriterionGradient(const std::string & aOperationName)
{
    const auto& tOperation = this->findOperation(aOperationName);
    auto tData = tOperation->getOutputData();
    return tData;
}

std::vector<double> 
PlatoPythonApp::getOperationInput(const std::string & aOperationName)
{
    const auto& tOperation = this->findOperation(aOperationName);
    auto tData = tOperation->inputDataVals();
    return tData;
}
