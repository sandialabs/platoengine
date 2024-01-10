#include "RunVoidFunctionOperation.hpp"
#include "PlatoPythonOperation.hpp"
#include "Plato_InputData.hpp"
#include "Plato_Parser.hpp"
#include "Plato_Macros.hpp"

#include <vector>

#include <boost/python.hpp>

RunVoidFunctionOperation::RunVoidFunctionOperation(const Plato::InputData & aOperationNode) :
 PlatoPythonOperation(aOperationNode),
 mRunFlag(true)
{
    mOnChange = Plato::Get::Bool(aOperationNode, "OnChange", false);
}

void
RunVoidFunctionOperation::runPythonFunction(const boost::python::object & aObject)
{
    if ( mRunFlag )
    {
        aObject.attr(mFunction.c_str())();
    }
}

const std::vector<double>&
RunVoidFunctionOperation::getOutputData()
{
    THROWERR("Operation with Name " + mName + " cannot have output to export")
}

void
RunVoidFunctionOperation::setInputData(std::vector<double> aData)
{
    this->setRunFlag(aData);

    mInputData.value = std::move(aData);
}

void
RunVoidFunctionOperation::setRunFlag(const std::vector<double> & aData)
{
    if ( !mOnChange )
    {
        mRunFlag = true;
        return;
    }

    if ( mInputData.value.size() == 0 )
    {
        mRunFlag = true;
        mCurrentInputs = aData;
        return;
    }

    if ( mCurrentInputs == aData)
    {
        mRunFlag = false;
    }
    else
    {
        mRunFlag = true;
        mCurrentInputs = aData;
    }
}