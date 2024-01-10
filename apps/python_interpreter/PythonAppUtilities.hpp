#include "Plato_InputData.hpp"

#include <string>

namespace Plato
{

namespace PythonApp
{

std::string
read_xml_field
(const Plato::InputData & aOperationNode,
 const std::string & aFieldName);

}

}
