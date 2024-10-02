#ifndef PLATO_THIRD_PARTY_INTEGRATION_KRINO_PARSE
#define PLATO_THIRD_PARTY_INTEGRATION_KRINO_PARSE

#include <istream>
#include <memory>
#include <string>
#include <vector>

#include "plato/third_party_integration/krino/Enums.hpp"

namespace plato::third_party_integration::krino
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

}  // namespace plato::third_party_integration::krino

#endif  // PLATO_THIRD_PARTY_INTEGRATION_KRINO_PARSE
