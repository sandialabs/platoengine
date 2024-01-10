#include "CriterionValueOperation.hpp"
#include "PlatoPythonOperation.hpp"

#include <vector>

#include <boost/python.hpp>

CriterionValueOperation::CriterionValueOperation(const Plato::InputData & aOperationNode) :
 PlatoPythonOperation(aOperationNode),
 mValue(1, 0.0)
{};

void
CriterionValueOperation::runPythonFunction(const boost::python::object & aObject)
{
    boost::python::object tReturn = aObject.attr(mFunction.c_str())();
    mValue[0] = boost::python::extract<double>(tReturn);
}

const std::vector<double>&
CriterionValueOperation::getOutputData()
{
    return mValue;
}
