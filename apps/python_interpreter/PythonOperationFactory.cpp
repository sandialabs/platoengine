#include "PythonOperationFactory.hpp"
#include "Plato_InputData.hpp"
#include "Plato_Parser.hpp"
#include "Plato_Macros.hpp"

#include "PlatoPythonOperation.hpp"
#include "CriterionValueOperation.hpp"
#include "CriterionGradientOperation.hpp"
#include "RunVoidFunctionOperation.hpp"

#include <memory>
#include <string>

std::unique_ptr<PlatoPythonOperation>
PythonOperationFactory::create(const Plato::InputData & aOperationNode)
{
    const std::string tOpName = Plato::Get::String(aOperationNode, "Name");
    const std::string tFunction = Plato::Get::String(aOperationNode, "Function");

    if (tFunction == "ComputeCriterionValue")
    {
        return std::make_unique<CriterionValueOperation>(aOperationNode);
    }
    else if (tFunction == "ComputeCriterionGradient")
    {
        return std::make_unique<CriterionGradientOperation>(aOperationNode);
    }
    else if (tFunction == "RunVoidFunction")
    {
        return std::make_unique<RunVoidFunctionOperation>(aOperationNode);
    }
    else
    {
        THROWERR("Unsupported Function in Operation with Name: " + tOpName)
    }

}
