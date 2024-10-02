#ifndef PLATO_KRINO_APP_UTILS_HPP
#define PLATO_KRINO_APP_UTILS_HPP

#include <vector>
#include "Plato_InputData.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/krino/LevelsetPrimitives.hpp"

using namespace plato::third_party_integration::krino;

namespace apps::krino_app
{

struct CommandLineOptions
{
    std::string mBackgroundMeshName{""};
    std::string mCutMeshName{""};
    std::string mKrinoOperationsFileName{"plato_krino_operations.xml"};
    std::string mFieldMeshName{""};
    std::string mFieldName{""};
    int mFieldDataTimeStep{0};
    bool mExecuteInitialMesh{false};
    bool mIncludeVoidRegion{false};
};

[[nodiscard]] bool parse_command_line_options(int aArgc, char *aArgv[], CommandLineOptions &aCommandLineOptions);
LevelsetPrimitives readLevelsetInitializationData(Plato::InputData &aAppFileData);
void createSpheresFromPattern(const Plato::InputData &aNode, LevelsetPrimitives &aPrims);
void createSphere(const Plato::InputData &aNode, LevelsetPrimitives &aPrims);
void createPlane(const Plato::InputData &aNode, LevelsetPrimitives &aPrims);
SpherePatternData readSpherePatternData(const Plato::InputData &aNode);
Sphere readSphereData(const Plato::InputData &aNode);
Plane readPlaneData(const Plato::InputData &aNode);

}

#endif // PLATO_KRINO_APP_UTILS_HPP
