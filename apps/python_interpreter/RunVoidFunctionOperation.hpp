#pragma once

#include "PlatoPythonOperation.hpp"
#include "Plato_InputData.hpp"

#include <vector>

namespace pybind11
{
    class object;
    class list;
}

class RunVoidFunctionOperation : public PlatoPythonOperation
{
public:
    RunVoidFunctionOperation(const Plato::InputData & aOperationNode);

protected:
    void
    runPythonFunction(const pybind11::object & aObject) override;

    const std::vector<double>&
    getOutputData() override;

    void 
    setInputData(std::vector<double> aData) override;

private:
    void
    setRunFlag(const std::vector<double> & aData);

    void
    throwIfInputEmpty();

    pybind11::list
    createPythonListFromInput();

private:
    bool mRunFlag;
    bool mOnChange;
    bool mUseInput;
    std::vector<double> mCurrentInputs;

};