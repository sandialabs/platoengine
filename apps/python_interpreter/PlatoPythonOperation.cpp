#include "PlatoPythonOperation.hpp"
#include "PythonAppUtilities.hpp"
#include "Plato_InputData.hpp"
#include "Plato_Parser.hpp"
#include "Plato_Macros.hpp"

PlatoPythonOperation::PlatoPythonOperation(const Plato::InputData & aOperationNode) :
 mName( Plato::PythonApp::read_xml_field(aOperationNode, "Name") ),
 mFunction( Plato::PythonApp::read_xml_field(aOperationNode, "PyFunction") )
{
    auto tOutputNodes = aOperationNode.getByName<Plato::InputData>("Output");
    if ( tOutputNodes.size() == 1 )
        mOutputData.name = Plato::Get::String(tOutputNodes[0], "ArgumentName");
    else if ( tOutputNodes.size() > 1 )
        THROWERR("Only one output can be prescribed in Operation with Name " + mName)

    auto tInputNodes = aOperationNode.getByName<Plato::InputData>("Input");
    if ( tInputNodes.size() == 1 )
        mInputData.name = Plato::Get::String(tInputNodes[0], "ArgumentName");
    else if ( tInputNodes.size() > 1 )
        THROWERR("Only one input can be prescribed in Operation with Name " + mName)
}

void
PlatoPythonOperation::setInputData(std::vector<double> aData)
{
    mInputData.value = std::move(aData);
}