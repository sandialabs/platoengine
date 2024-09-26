#include "PlatoKrinoApp.hpp"
#include "plato/third_party_integration/krino/Parse.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"


using namespace Plato;

int runStandAlone(int aArgc, char *aArgv[], const CommandLineOptions &aOptions);
int runMPMD(int aArgc, char *aArgv[], const CommandLineOptions &aOptions);

/**********************************************************************/
int main(int aArgc, char *aArgv[])
/**********************************************************************/
{
    CommandLineOptions tCommandLineOptions;
    bool tStatus = parse_command_line_options(aArgc, aArgv, tCommandLineOptions);

    if(!tStatus || tCommandLineOptions.mBackgroundMeshName == "" || tCommandLineOptions.mCutMeshName == "")
    {
        std::stringstream tError;
        tError << std::endl << "ERROR: PlatoKrinoMain.cpp: You must specify the background mesh and cut mesh as command line arguments." << std::endl;
        ParsingException tParsingException(tError.str());
        throw tParsingException;
    }

    if(tCommandLineOptions.mExecuteInitialMesh)
    {
        return runStandAlone(aArgc, aArgv, tCommandLineOptions);
    }
    else
    {
        return runMPMD(aArgc, aArgv, tCommandLineOptions);
    }
}

int runStandAlone(int aArgc, char *aArgv[], const CommandLineOptions &aOptions)
{
    MPI_Init(&aArgc, &aArgv);

    initialize_environment_for_krino(MPI_COMM_WORLD);
    Kokkos::initialize(aArgc, aArgv);

    PlatoKrinoApp tMyApp(nullptr, aOptions);
    tMyApp.executeInitialMesh();

    MPI_Finalize();

    return 0;
}

int runMPMD(int aArgc, char *aArgv[], const CommandLineOptions &aOptions)
{
    MPI_Init(&aArgc, &aArgv);
    Interface tPlatoInterface;
    MPI_Comm tLocalComm;
    tPlatoInterface.getLocalComm(tLocalComm);

    initialize_environment_for_krino(tLocalComm);
    Kokkos::initialize(aArgc, aArgv);

    PlatoKrinoApp tMyApp(&tPlatoInterface, aOptions);
    tPlatoInterface.registerApplication(&tMyApp);

    try
    {
        tPlatoInterface.perform();
    }
    catch(...)
    {
    }

    tMyApp.finalize();

    return 0;
}




