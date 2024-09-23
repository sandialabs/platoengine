#ifndef PLATO_KRINO_INTEGRATION_PARSE
#define PLATO_KRINO_INTEGRATION_PARSE

#include <istream>
#include <memory>
#include <string>
#include <vector>

#include "Enums.hpp"

namespace plato::krino_integration
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

}  // namespace plato::krino_integration

#endif  // PLATO_KRINO_INTEGRATION_PARSE
