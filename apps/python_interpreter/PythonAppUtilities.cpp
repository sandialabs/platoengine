#include "PythonAppUtilities.hpp"
#include "Plato_InputData.hpp"
#include "Plato_Parser.hpp"
#include "Plato_Macros.hpp"

#include <string>

namespace Plato
{

namespace PythonApp
{

std::string
read_xml_field
(const Plato::InputData & aOperationNode,
 const std::string & aFieldName)
{
    const std::string tValue = Plato::Get::String(aOperationNode, aFieldName);

    if( tValue.empty() )
        THROWERR(aFieldName + " field is missing in Operation definition.")

    return tValue;
}

}

}
