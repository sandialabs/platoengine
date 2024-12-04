#include "PlatoKrinoApp.hpp"
#include "PlatoKrinoAppUtils.hpp"
#include "PlatoKrinoUtilities.hpp"

int runStandAlone(int aArgc, char *aArgv[], const apps::krino_app::CommandLineOptions &aOptions);
int runMPMD(int aArgc, char *aArgv[], const apps::krino_app::CommandLineOptions &aOptions);

/**********************************************************************/
int main(int aArgc, char *aArgv[])
/**********************************************************************/
{
    apps::krino_app::CommandLineOptions tCommandLineOptions;
    bool tStatus = apps::krino_app::parse_command_line_options(aArgc, aArgv, tCommandLineOptions);

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

int runStandAlone(int aArgc, char *aArgv[], const apps::krino_app::CommandLineOptions &aOptions)
{
    MPI_Init(&aArgc, &aArgv);

    Plato::initialize_environment_for_krino(MPI_COMM_WORLD);
    Kokkos::initialize(aArgc, aArgv);

    Plato::PlatoKrinoApp tMyApp(nullptr, aOptions);
    tMyApp.executeInitialMesh();

    MPI_Finalize();

    return 0;
}

int runMPMD(int aArgc, char *aArgv[], const apps::krino_app::CommandLineOptions &aOptions)
{
    MPI_Init(&aArgc, &aArgv);
    Plato::Interface tPlatoInterface;
    MPI_Comm tLocalComm;
    tPlatoInterface.getLocalComm(tLocalComm);

    Plato::initialize_environment_for_krino(tLocalComm);
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




