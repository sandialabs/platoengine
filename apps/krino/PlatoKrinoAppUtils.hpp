#include <vector>
#include "Plato_InputData.hpp"
#include "plato/krino_integration/PlatoKrinoUtilities.hpp"

namespace Plato::Krino
{

LevelsetPrimitives readLevelsetInitializationData(Plato::InputData &aAppFileData);
void createSpheresFromPattern(const Plato::InputData &aNode, LevelsetPrimitives &aPrims);
void createSphere(const Plato::InputData &aNode, LevelsetPrimitives &aPrims);
void createPlane(const Plato::InputData &aNode, LevelsetPrimitives &aPrims);
SpherePatternData readSpherePatternData(const Plato::InputData &aNode);
std::vector<Sphere> generateSpheres(const SpherePatternData &aData);
Sphere readSphereData(const Plato::InputData &aNode);
Plane readPlaneData(const Plato::InputData &aNode);
void checkForReasonableSpherePatternDefinition(const SpherePatternData &aData);
SphereLocatorData calculateSphereStartsAndSpacing(const SpherePatternData &aData);
std::pair<double, double> calculateOverlappingSingleSphereLocatorData(const SpherePatternData &aPatternData,
                                                                      const size_t &aDimension);
std::pair<double, double> calculateOverlappingManySphereLocatorData(const SpherePatternData &aPatternData,
                                                                    const size_t &aDimension);
std::pair<double, double> calculateNonOverlappingSphereLocatorData(const SpherePatternData &aPatternData,
                                                                   const size_t &aDimension);

}
