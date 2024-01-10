#pragma once

#include "Plato_InputData.hpp"

#include <string>
#include <vector>

#include <boost/python.hpp>

struct IOData
{
    std::string name;
    std::vector<double> value;
};

class PlatoPythonOperation
{
public:
    PlatoPythonOperation(const Plato::InputData & aOperationNode);

    virtual ~PlatoPythonOperation() = default;

    virtual void
    runPythonFunction(const boost::python::object & aObject) = 0;

    virtual const std::vector<double>&
    getOutputData() = 0;

    virtual void 
    setInputData(std::vector<double> aData);

    std::string
    name(){return mName;};

    std::string
    inputDataName(){return mInputData.name;};

    std::vector<double>
    inputDataVals(){return mInputData.value;};

    std::string
    outputDataName(){return mOutputData.name;};

protected:
    std::string mName;
    std::string mFunction;
    IOData mInputData;
    IOData mOutputData;
};