#include "CriterionValueOperation.hpp"
#include "PlatoPythonOperation.hpp"

#include <vector>
#include <iostream>

#include <pybind11/embed.h>

CriterionValueOperation::CriterionValueOperation(const Plato::InputData & aOperationNode) :
 PlatoPythonOperation(aOperationNode),
 mValue(1, 0.0)
{}

void
CriterionValueOperation::runPythonFunction(const pybind11::object & aObject)
{
    pybind11::object tReturn = aObject.attr(mFunction.c_str())();
    const auto tValue = tReturn.cast<double>();
    std::cout << "Criterion value:  " << tValue << "\n";
    mValue[0] = tValue;
}

const std::vector<double>&
CriterionValueOperation::getOutputData()
{
    return mValue;
}
