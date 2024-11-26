#include "CriterionGradientOperation.hpp"
#include "PlatoPythonOperation.hpp"

#include <vector>

#include <pybind11/embed.h>
#include <pybind11/stl.h>

void
CriterionGradientOperation::runPythonFunction(const pybind11::object & aObject)
{
    pybind11::object tReturn = aObject.attr(mFunction.c_str())();
    mGradData = tReturn.cast<std::vector<double>>();
}

const std::vector<double>&
CriterionGradientOperation::getOutputData()
{
    return mGradData;
}