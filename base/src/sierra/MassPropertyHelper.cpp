#include "MassPropertyHelper.hpp"

#include <array>
#include <string>
#include <string_view>

#include "Plato_SerializationLoadSave.hpp"

namespace Plato::Sierra {

void save_to_xml(const std::string& aFilename, const MassPropertyHelper& aMP) {
  Plato::saveToXML(aMP, Plato::XMLFileName(aFilename),
                   Plato::XMLNodeName(std::string(MASS_PROPERTY_SERIALIZATION_NAME)));
}

MassPropertyHelper load_from_xml(const std::string& aFilename) {
  MassPropertyHelper tMP;
  Plato::loadFromXML(tMP, Plato::XMLFileName(aFilename),
                     Plato::XMLNodeName(std::string(MASS_PROPERTY_SERIALIZATION_NAME)));
  return tMP;
}

}  // namespace Plato::Sierra