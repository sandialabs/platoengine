#include "PlatoKrinoApp.hpp"
#include "PlatoKrinoParse.hpp"
#include "PlatoKrinoUtilities.hpp"


using namespace Plato::Krino;

int runStandAlone(int aArgc, char *aArgv[], const CommandLineOptions &aOptions);
int runMPMD(int aArgc, char *aArgv[], const CommandLineOptions &aOptions);

/**********************************************************************/
int main(int aArgc, char *aArgv[])
/**********************************************************************/
{
    CommandLineOptions tCommandLineOptions;
    bool tStatus = parseCommandLineOptions(aArgc, aArgv, tCommandLineOptions);

    if(!tStatus || tCommandLineOptions.mBackgroundMeshName == "" || tCommandLineOptions.mCutMeshName == "")
    {
        std::stringstream tError;
        tError << std::endl << "ERROR: PlatoKrinoMain.cpp: You must specify the background mesh and cut mesh as command line arguments." << std::endl;
        Plato::ParsingException tParsingException(tError.str());
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

    initializeSTKEnvironment(MPI_COMM_WORLD);
    initializeKrinoLogging();
    Kokkos::initialize(aArgc, aArgv);

    Plato::PlatoKrinoApp tMyApp(nullptr, aOptions);
    tMyApp.executeInitialMesh();

    MPI_Finalize();

    return 0;
}

int runMPMD(int aArgc, char *aArgv[], const CommandLineOptions &aOptions)
{
    MPI_Init(&aArgc, &aArgv);
    Plato::Interface tPlatoInterface;
    MPI_Comm tLocalComm;
    tPlatoInterface.getLocalComm(tLocalComm);

    initializeSTKEnvironment(tLocalComm);

    initializeKrinoLogging();
   
    Kokkos::initialize(aArgc, aArgv);

    Plato::PlatoKrinoApp tMyApp(&tPlatoInterface, aOptions);
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




