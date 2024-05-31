#include <memory>
#include <string>
#include <istream>
#include <vector>
#include "PlatoKrinoEnums.hpp"

#pragma once 

namespace Plato::Krino
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

bool parseCommandLineOptions(int aArgc, char *aArgv[], CommandLineOptions &aCommandLineOptions);

}
