#pragma once

#include "Plato_InputData.hpp"

#include <string>
#include <vector>

namespace pybind11
{
    class object;
}

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
    runPythonFunction(const pybind11::object & aObject) = 0;

    virtual const std::vector<double>&
    getOutputData() = 0;

    virtual void 
    setInputData(std::vector<double> aData);

    const std::string&
    name() const {return mName;};

    const std::string&
    inputDataName() const {return mInputData.name;};

    const std::vector<double>&
    inputDataVals() const {return mInputData.value;};

    const std::string&
    outputDataName() const {return mOutputData.name;};

protected:
    std::string mName;
    std::string mFunction;
    IOData mInputData;
    IOData mOutputData;
};