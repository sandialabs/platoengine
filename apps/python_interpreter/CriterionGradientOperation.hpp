#pragma once

#include "PlatoPythonOperation.hpp"
#include "Plato_InputData.hpp"

#include <vector>

#include <boost/python.hpp>

class CriterionGradientOperation : public PlatoPythonOperation
{
public:
    CriterionGradientOperation(const Plato::InputData & aOperationNode) :
     PlatoPythonOperation(aOperationNode)
    {}

protected:
    void
    runPythonFunction(const boost::python::object & aObject) override;

    const std::vector<double>&
    getOutputData() override;

private:
    std::vector<double> mGradData;

};