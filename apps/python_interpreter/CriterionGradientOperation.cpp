#include "CriterionGradientOperation.hpp"
#include "PlatoPythonOperation.hpp"

#include <vector>

#include <boost/python.hpp>

void
CriterionGradientOperation::runPythonFunction(const boost::python::object & aObject)
{
    boost::python::object tReturn = aObject.attr(mFunction.c_str())();
    mGradData.assign(boost::python::stl_input_iterator<double>(tReturn),
                     boost::python::stl_input_iterator<double>());
}

const std::vector<double>&
CriterionGradientOperation::getOutputData()
{
    return mGradData;
}