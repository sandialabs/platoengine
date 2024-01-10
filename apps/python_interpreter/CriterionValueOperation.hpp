#pragma once

#include "PlatoPythonOperation.hpp"
#include "Plato_InputData.hpp"

#include <vector>

#include <boost/python.hpp>

class CriterionValueOperation : public PlatoPythonOperation
{
public:
    CriterionValueOperation(const Plato::InputData & aOperationNode);

protected:
    void
    runPythonFunction(const boost::python::object & aObject) override;

    const std::vector<double>&
    getOutputData() override;

private:
    std::vector<double> mValue;

};