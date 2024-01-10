#pragma once

#include "PlatoPythonOperation.hpp"
#include "Plato_InputData.hpp"

#include <vector>

#include <boost/python.hpp>

class RunVoidFunctionOperation : public PlatoPythonOperation
{
public:
    RunVoidFunctionOperation(const Plato::InputData & aOperationNode);

protected:
    void
    runPythonFunction(const boost::python::object & aObject) override;

    const std::vector<double>&
    getOutputData() override;

    void 
    setInputData(std::vector<double> aData) override;

private:
    void
    setRunFlag(const std::vector<double> & aData);

private:
    bool mOnChange;
    bool mRunFlag;
    std::vector<double> mCurrentInputs;

};