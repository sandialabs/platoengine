#include <vector>
#include "Plato_InputData.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/krino/LevelsetPrimitives.hpp"

using namespace plato::third_party_integration::krino;

namespace apps::krino_app
{

LevelsetPrimitives readLevelsetInitializationData(Plato::InputData &aAppFileData);
void createSpheresFromPattern(const Plato::InputData &aNode, LevelsetPrimitives &aPrims);
void createSphere(const Plato::InputData &aNode, LevelsetPrimitives &aPrims);
void createPlane(const Plato::InputData &aNode, LevelsetPrimitives &aPrims);
SpherePatternData readSpherePatternData(const Plato::InputData &aNode);
Sphere readSphereData(const Plato::InputData &aNode);
Plane readPlaneData(const Plato::InputData &aNode);

}
