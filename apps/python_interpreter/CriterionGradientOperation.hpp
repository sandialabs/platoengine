#pragma once

#include "PlatoPythonOperation.hpp"
#include "Plato_InputData.hpp"

#include <vector>

namespace pybind11
{
    class object;
}

class CriterionGradientOperation : public PlatoPythonOperation
{
public:
    CriterionGradientOperation(const Plato::InputData & aOperationNode) :
     PlatoPythonOperation(aOperationNode)
    {}

protected:
    void
    runPythonFunction(const pybind11::object & aObject) override;

    const std::vector<double>&
    getOutputData() override;

private:
    std::vector<double> mGradData;

};