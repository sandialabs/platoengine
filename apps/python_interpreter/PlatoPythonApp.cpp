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
#include <numeric>
#include <vector>

#include <Python.h>
#include <boost/python.hpp>

namespace 
{
const auto tDefaultErrorPolicy = [](const std::string& aOperationName){THROWERR("Failed to find operation with name '" + aOperationName + "'")};

const auto tDefaultLabel = [](const PlatoPythonOperation& aOperation){return aOperation.name();};

template <typename ExecutionPolicy, typename ErrorPolicy, typename OperationLabel>
void
findOperationAndExecute
(const std::vector<std::unique_ptr<PlatoPythonOperation>>& aOperations,
 const std::string & aOperationTag,
 const ExecutionPolicy& aFunc,
 const ErrorPolicy& aError,
 const OperationLabel& aLabel)
{
    const auto& tOperationIter = std::find_if(aOperations.begin(), aOperations.end(), 
        [&aOperationTag, &aLabel](const std::unique_ptr<PlatoPythonOperation>& aOperation){
        return aLabel(*(aOperation)) == aOperationTag;
    });

    if(tOperationIter != aOperations.end())
    {
        aFunc(**tOperationIter);
    }
    else
    {
        aError(aOperationTag);
    }
}
}

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
    this->getFieldSize();
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
PlatoPythonApp::getFieldSize()
{
    const std::string tFieldSizeOperationName{"Initialize Field Size"};
    findOperationAndExecute(mOperations, tFieldSizeOperationName,
        [this](PlatoPythonOperation& aOperation)
        {aOperation.runPythonFunction(mPythonObject);
         std::vector<double> tData = aOperation.getOutputData();
         assert(!tData.empty());
         mFieldSize = static_cast<int>(tData[0]);},
        [](const std::string & /*aOperationName*/)
        {},
        tDefaultLabel
    );
}

void 
PlatoPythonApp::compute(const std::string & aOperationName)
{
    findOperationAndExecute(mOperations, aOperationName,
        [this](PlatoPythonOperation& aOperation)
        {aOperation.runPythonFunction(mPythonObject);},
        tDefaultErrorPolicy,
        tDefaultLabel
    );
}

void 
PlatoPythonApp::exportData
(const std::string & aArgumentName, 
 Plato::SharedData & aExportData)
{
    findOperationAndExecute(mOperations, aArgumentName,
        [&aExportData](PlatoPythonOperation& aOperation)
        {auto tData = aOperation.getOutputData();
         aExportData.setData(tData);},
        [](const std::string & aOperationName)
        {THROWERR("Failed to find SharedData with name '" + aOperationName + "' when exporting")},
        [](const PlatoPythonOperation& aOperation){return aOperation.outputDataName();}
    );
}

void 
PlatoPythonApp::importData
(const std::string & aArgumentName, 
 const Plato::SharedData & aImportData)
{
    findOperationAndExecute(mOperations, aArgumentName,
        [&aImportData](PlatoPythonOperation& aOperation)
        {std::vector<double> tData;
         tData.resize(aImportData.size());
         aImportData.getData(tData);
         aOperation.setInputData(tData);},
        [](const std::string & aOperationName)
        {THROWERR("Failed to find SharedData with name '" + aOperationName + "' when importing")},
        [](const PlatoPythonOperation& aOperation){return aOperation.inputDataName();}
    );
}

void 
PlatoPythonApp::exportDataMap
(const Plato::data::layout_t & aDataLayout, 
 std::vector<int> & aMyOwnedGlobalIDs)
{
    if (aDataLayout == Plato::data::layout_t::SCALAR_FIELD)
    {
        this->throwIfFieldSizeNotSet();
        aMyOwnedGlobalIDs.resize(mFieldSize.value());
    }
    else if (aDataLayout == Plato::data::layout_t::ELEMENT_FIELD)
    {
        THROWERR("exportDataMap is not implemented for Element Field layout.")
    }
    std::iota(aMyOwnedGlobalIDs.begin(), aMyOwnedGlobalIDs.end(), 1);
}

void
PlatoPythonApp::throwIfFieldSizeNotSet()
{
    if ( !mFieldSize.has_value() )
        THROWERR("Field size for SharedData has not been set. Field size must be set by an operation with name 'Initialize Field Size'.")
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
    std::vector<double> tData;
    findOperationAndExecute(mOperations, aOperationName,
        [&tData](PlatoPythonOperation& aOperation)
        {tData = aOperation.getOutputData();},
        tDefaultErrorPolicy,
        tDefaultLabel
    );
    return tData[0];
}

std::vector<double> 
PlatoPythonApp::getCriterionGradient(const std::string & aOperationName)
{
    std::vector<double> tData;
    findOperationAndExecute(mOperations, aOperationName,
        [&tData](PlatoPythonOperation& aOperation)
        {tData = aOperation.getOutputData();},
        tDefaultErrorPolicy,
        tDefaultLabel
    );
    return tData;
}

std::vector<double> 
PlatoPythonApp::getOperationInput(const std::string & aOperationName)
{
    std::vector<double> tData;
    findOperationAndExecute(mOperations, aOperationName,
        [&tData](PlatoPythonOperation& aOperation)
        {tData = aOperation.inputDataVals();},
        tDefaultErrorPolicy,
        tDefaultLabel
    );
    return tData;
}
