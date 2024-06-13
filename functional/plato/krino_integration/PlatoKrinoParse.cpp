#include "PlatoKrinoParse.hpp"

#include <Teuchos_CommandLineProcessor.hpp>
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <sstream>

namespace Plato::Krino
{

bool parseCommandLineOptions(int aArgc, char *aArgv[], CommandLineOptions &aCommandLineOptions)
{
    Teuchos::CommandLineProcessor tCLP;
    tCLP.setDocString("plato_krino_main options:");

    tCLP.setOption("bg_mesh", &(aCommandLineOptions.mBackgroundMeshName),
                   "Name of static background mesh that will hold levelset values.");
    tCLP.setOption("cut_mesh", &(aCommandLineOptions.mCutMeshName),
                   "Name of mesh that is cut out of the background mesh using levelset values.");
    tCLP.setOption("krino_operations_file", &(aCommandLineOptions.mKrinoOperationsFileName),
                   "Name of plato/krino operations file.");
    tCLP.setOption("field_mesh_name", &(aCommandLineOptions.mFieldMeshName),
                   "Name of mesh containing field for initializing levelset values.");
    tCLP.setOption("field_name", &(aCommandLineOptions.mFieldName),
                   "Name of field from which levelset values will be initialized.");
    tCLP.setOption("time_step", &(aCommandLineOptions.mFieldDataTimeStep),
                   "Time step from which to initialize levelset values.");
    tCLP.setOption("execute_initial_mesh", "", &(aCommandLineOptions.mExecuteInitialMesh),
                   "Specifies whether to create an initial cut mesh.");
    tCLP.setOption("include_void_region", "", &(aCommandLineOptions.mIncludeVoidRegion),
                   "Specifies whether to include the void regions in the output mesh.");

    Teuchos::CommandLineProcessor::EParseCommandLineReturn parseReturn =
        Teuchos::CommandLineProcessor::PARSE_SUCCESSFUL;
    try
    {
        parseReturn = tCLP.parse(aArgc, aArgv);
    }
    catch (std::exception &exc)
    {
        std::cout << "Failed to parse the command line arguments." << std::endl;
        return false;
    }

    if (parseReturn == Teuchos::CommandLineProcessor::PARSE_SUCCESSFUL) return true;

    return false;
}

}  // namespace Plato::Krino
