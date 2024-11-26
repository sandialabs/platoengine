#pragma once

#include "PlatoPythonOperation.hpp"
#include "Plato_InputData.hpp"

#include <vector>

namespace pybind11
{
    class object;
}

class CriterionValueOperation : public PlatoPythonOperation
{
public:
    CriterionValueOperation(const Plato::InputData & aOperationNode);

protected:
    void
    runPythonFunction(const pybind11::object & aObject) override;

    const std::vector<double>&
    getOutputData() override;

private:
    std::vector<double> mValue;

};