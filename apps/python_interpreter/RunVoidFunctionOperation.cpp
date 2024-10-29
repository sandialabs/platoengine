#include "RunVoidFunctionOperation.hpp"
#include "PlatoPythonOperation.hpp"
#include "Plato_InputData.hpp"
#include "Plato_Parser.hpp"
#include "Plato_Macros.hpp"

#include <vector>

#include <boost/python.hpp>

RunVoidFunctionOperation::RunVoidFunctionOperation(const Plato::InputData & aOperationNode) :
 PlatoPythonOperation(aOperationNode),
 mRunFlag{true},
 mOnChange{Plato::Get::Bool(aOperationNode, "OnChange", false)},
 mUseInput{Plato::Get::Bool(aOperationNode, "UseInput", false)}
{}

void
RunVoidFunctionOperation::runPythonFunction(const boost::python::object & aObject)
{
    if ( mRunFlag )
    {
        if(mUseInput)
        {
            throwIfInputEmpty();
            aObject.attr(mFunction.c_str())(createPythonListFromInput());
        }
        else
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

void
RunVoidFunctionOperation::throwIfInputEmpty()
{
    if ( mInputData.value.empty() )
        THROWERR("Input Data has not been set for Operation with Name " + mName + ".")
}

boost::python::list
RunVoidFunctionOperation::createPythonListFromInput()
{
    boost::python::list inputs;
    for (auto val : mInputData.value)
    {
        inputs.append(val);
    }
    return inputs;
}