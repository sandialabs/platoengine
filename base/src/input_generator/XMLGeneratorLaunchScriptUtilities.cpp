#include <iostream>
#include <fstream>

#include "pugixml.hpp"

#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorDataStruct.hpp"
#include "XMLGeneratorSierraSDUtilities.hpp"
#include "XMLGeneratorLaunchUtilities.hpp"
#include "XMLGeneratorInterfaceFileUtilities.hpp"
#include "XMLGeneratorSierraSDInputDeckUtilities.hpp"
#include "XMLGeneratorLaunchScriptUtilities.hpp"

namespace XMLGen
{

void generate_launch_script(const XMLGen::InputData& aInputData)
{
    if(aInputData.m_Arch == XMLGen::Arch::SUMMIT)
        XMLGen::generate_summit_launch_scripts(aInputData);
    else
        XMLGen::generate_mpirun_launch_script(aInputData);
}

void generate_summit_launch_scripts(const XMLGen::InputData& aInputData)
{
    for(auto &tServiceID : aInputData.objective.serviceIDs)
    {
        XMLGen::Service tService = aInputData.service(tServiceID);
        if(tService.code() != "plato_analyze")
            THROWERR("ERROR: Summit output is only supported for Plato Analyze performers")
    }

    size_t tNumPerformers = aInputData.m_UncertaintyMetaData.numPerformers;

    XMLGen::generate_engine_bash_script();
    XMLGen::generate_analyze_bash_script();
    XMLGen::generate_jsrun_script(tNumPerformers, "analyze");
    XMLGen::generate_batch_script(tNumPerformers);
}

void generate_mpirun_launch_script(const XMLGen::InputData& aInputData)
{
    FILE *fp = fopen("mpirun.source", "w");

    if(aInputData.optimization_parameters().optimizationType() == XMLGen::OT_SHAPE)
    {
        XMLGen::append_esp_initialization_line(aInputData, fp);
        if (XMLGen::have_auxiliary_mesh(aInputData)) {
            XMLGen::append_join_mesh_operation_line(aInputData, fp);
        }
        if (XMLGen::do_tet10_conversion(aInputData)) {
            XMLGen::append_tet10_conversion_operation_line(fp);
        }
    }
    else if(aInputData.optimization_parameters().optimizationType() == XMLGen::OT_DAKOTA)
    {
        XMLGen::append_esp_initialization_line(aInputData, fp);
        XMLGen::append_copy_mesh_lines_for_dakota_workflow(fp,aInputData);
        if(XMLGen::create_subblock(aInputData))
            XMLGen::append_subblock_creation_operation_lines_for_dakota_workflow(fp,aInputData.optimization_parameters().concurrent_evaluations());
        if (XMLGen::do_tet10_conversion(aInputData))
            XMLGen::append_tet10_conversion_operation_lines_for_dakota_workflow(fp,aInputData.optimization_parameters().concurrent_evaluations());
    }
    XMLGen::append_decomp_lines_for_prune_and_refine(aInputData, fp);
    XMLGen::append_prune_and_refine_lines_to_mpirun_launch_script(aInputData, fp);
    XMLGen::append_decomp_lines_to_mpirun_launch_script(aInputData, fp);
    XMLGen::append_engine_mpirun_lines(aInputData, fp);
    XMLGen::append_physics_performer_mpirun_lines(aInputData, fp);
    XMLGen::append_post_optimization_run_lines(aInputData, fp);

    fclose(fp);
}

void append_physics_performer_mpirun_lines(const XMLGen::InputData& aInputData, FILE*& aFile)
{
    int tPerformerID=1;
    if(XMLGen::is_robust_optimization_problem(aInputData))
    {
        XMLGen::append_analyze_mpirun_commands_robust_optimization_problems(aInputData, tPerformerID, aFile);
    }
    else if(aInputData.optimization_parameters().optimizationType() == XMLGen::OT_TOPOLOGY ||
            aInputData.optimization_parameters().optimizationType() == XMLGen::OT_SHAPE)
    {
        XMLGen::append_physics_performer_mpirun_commands_gradient_based_problem(aInputData, tPerformerID, aFile);
    }
    else if(aInputData.optimization_parameters().optimizationType() == XMLGen::OT_DAKOTA)
    {
        XMLGen::append_physics_performer_mpirun_commands_dakota_problem(aInputData, tPerformerID, aFile);
    }
}

void append_analyze_mpirun_commands_robust_optimization_problems
(const XMLGen::InputData& aInputData,
 int &aNextPerformerID,
 FILE*& aFile)
{
    std::string tEnvString, tSeparationString, tLaunchString, tNumProcsString;
    XMLGen::determine_mpi_env_and_separation_strings(tEnvString, tSeparationString);
    XMLGen::determine_mpi_launch_strings(aInputData, tLaunchString, tNumProcsString);

    if(aInputData.m_UncertaintyMetaData.numPerformers <= 0)
    {
        THROWERR("Number of performers for uncertainty workflow must be greater than zero\n")
    }

    XMLGen::Service tService = aInputData.mPerformerServices[0];
    std::vector<std::string> tDeviceIDs = tService.deviceIDs();

    // If no device ids were specified just put all of the performers in one executable 
    // statement in the mpirun.source file
    if(tDeviceIDs.size() == 0)
    {
        fprintf(aFile,
            ": %s %s %s PLATO_PERFORMER_ID%s%d \\\n",
            tNumProcsString.c_str(),
            Plato::to_string(aInputData.m_UncertaintyMetaData.numPerformers).c_str(),
            tEnvString.c_str(),
            tSeparationString.c_str(),
            aNextPerformerID);
        aNextPerformerID++;
        if(aInputData.mPerformerServices.size() == 0)
        {
            THROWERR("Number of services must be greater than zero\n")
        }

        fprintf(aFile, "%s PLATO_INTERFACE_FILE%sinterface.xml \\\n", tEnvString.c_str(), tSeparationString.c_str());
        fprintf(aFile, "%s PLATO_APP_FILE%splato_analyze_%s_operations.xml \\\n", tEnvString.c_str(), tSeparationString.c_str(), tService.id().c_str());
        XMLGen::append_plato_analyze_code_path(aInputData, aFile, tService.id(), "");
    }
    // Id device ids were specified spread the performers out on the devices by putting them in 
    // separate executable statements and specifying the kokkos-device id.
    else
    {
        size_t tMinNumPerformersPerDevice = aInputData.m_UncertaintyMetaData.numPerformers/tDeviceIDs.size();
        size_t tRemainder = aInputData.m_UncertaintyMetaData.numPerformers%tDeviceIDs.size();
        for(size_t i=0; i<tDeviceIDs.size(); ++i)
        {
            int tNumPerformersOnThisDevice = tMinNumPerformersPerDevice;
            // On the last device add the remainder performers
            if(i == (tDeviceIDs.size()-1))
            {
                tNumPerformersOnThisDevice += tRemainder;
            }
            fprintf(aFile,
                ": %s %d %s PLATO_PERFORMER_ID%s%d \\\n",
                tNumProcsString.c_str(),
                tNumPerformersOnThisDevice,
                tEnvString.c_str(),
                tSeparationString.c_str(),
                aNextPerformerID);
            fprintf(aFile, "%s PLATO_INTERFACE_FILE%sinterface.xml \\\n", tEnvString.c_str(), tSeparationString.c_str());
            fprintf(aFile, "%s PLATO_APP_FILE%splato_analyze_%s_operations.xml \\\n", tEnvString.c_str(), tSeparationString.c_str(), tService.id().c_str());
            XMLGen::append_plato_analyze_code_path(aInputData, aFile, tService.id(), tDeviceIDs[i]);
        }
        aNextPerformerID++;
    }
}

void append_physics_performer_mpirun_commands_gradient_based_problem
(const XMLGen::InputData& aInputData,
 int &aNextPerformerID,
 FILE*& aFile)
{
    for(auto &tService : aInputData.mPerformerServices)
    {
        if(tService.code() == "plato_analyze")
        {
            append_analyze_mpirun_line(aInputData, tService, aNextPerformerID, aFile);
        }
        else if(tService.code() == "sierra_sd")
        {
            append_sierra_sd_mpirun_line(aInputData, tService, aNextPerformerID, aFile);
        }
        else if(tService.code() == "plato_esp")
        {
            append_esp_mpirun_line(aInputData, tService, aNextPerformerID, aFile);
        }
        aNextPerformerID++;
    }
}

void append_analyze_mpirun_line(const XMLGen::InputData& aInputData, 
                                const XMLGen::Service& aService,
                                int &aNextPerformerID, 
                                FILE*& aFile,
                                const int aEvaluation)
{
    std::string tEnvString, tSeparationString, tLaunchString, tNumProcsString;
    XMLGen::determine_mpi_env_and_separation_strings(tEnvString, tSeparationString);
    XMLGen::determine_mpi_launch_strings(aInputData, tLaunchString, tNumProcsString);

    std::vector<std::string> tDeviceIDs = aService.deviceIDs();
    std::string tDeviceID = "";
    if(tDeviceIDs.size() != 0)
    {
        tDeviceID = tDeviceIDs[0];
    }
    fprintf(aFile,
        ": %s %s %s PLATO_PERFORMER_ID%s%d \\\n",
        tNumProcsString.c_str(),
        aService.numberProcessors().c_str(),
        tEnvString.c_str(),
        tSeparationString.c_str(),
        aNextPerformerID);

    fprintf(aFile, "%s PLATO_INTERFACE_FILE%sinterface.xml \\\n", tEnvString.c_str(), tSeparationString.c_str());
    fprintf(aFile, "%s PLATO_APP_FILE%splato_analyze_%s_operations.xml \\\n", tEnvString.c_str(), tSeparationString.c_str(), aService.id().c_str());
    XMLGen::append_plato_analyze_code_path(aInputData, aFile, aService.id(), tDeviceID, aEvaluation);
}

void append_sierra_sd_mpirun_line
(const XMLGen::InputData& aInputData,
 const XMLGen::Service& aService,
 int &aNextPerformerID,
 FILE*& aFile,
 const int aEvaluation)
{
    std::string tEnvString, tSeparationString, tLaunchString, tNumProcsString;
    XMLGen::determine_mpi_env_and_separation_strings(tEnvString, tSeparationString);
    XMLGen::determine_mpi_launch_strings(aInputData, tLaunchString, tNumProcsString);

    std::vector<std::string> tDeviceIDs = aService.deviceIDs();
    std::string tDeviceID = "";
    if(tDeviceIDs.size() != 0)
    {
        tDeviceID = tDeviceIDs[0];
    }

    fprintf(aFile,
        ": %s %s %s PLATO_PERFORMER_ID%s%d \\\n",
        tNumProcsString.c_str(),
        aService.numberProcessors().c_str(),
        tEnvString.c_str(),
        tSeparationString.c_str(),
        aNextPerformerID);

    fprintf(aFile, "%s PLATO_INTERFACE_FILE%sinterface.xml \\\n", tEnvString.c_str(), tSeparationString.c_str());
    fprintf(aFile, "%s PLATO_APP_FILE%ssierra_sd_%s_operations.xml \\\n", tEnvString.c_str(), tSeparationString.c_str(), aService.id().c_str());
    XMLGen::append_sierra_sd_code_path(aInputData, aFile, aService.id(), aEvaluation);
}

void append_esp_mpirun_line(const XMLGen::InputData& aInputData, const XMLGen::Service& aService, int &aNextPerformerID, FILE*& aFile)
{
    std::string tEnvString, tSeparationString, tLaunchString, tNumProcsString;
    XMLGen::determine_mpi_env_and_separation_strings(tEnvString, tSeparationString);
    XMLGen::determine_mpi_launch_strings(aInputData, tLaunchString, tNumProcsString);

    fprintf(aFile,
        ": %s %s %s PLATO_PERFORMER_ID%s%d \\\n",
        tNumProcsString.c_str(),
        aService.numberProcessors().c_str(),
        tEnvString.c_str(),
        tSeparationString.c_str(),
        aNextPerformerID);

    fprintf(aFile, "%s PLATO_INTERFACE_FILE%sinterface.xml \\\n", tEnvString.c_str(), tSeparationString.c_str());
    fprintf(aFile, "%s PLATO_APP_FILE%splato_esp_operations.xml \\\n", tEnvString.c_str(), tSeparationString.c_str());
    fprintf(aFile, "PlatoESP plato_esp_input_deck.xml \\\n");
}

void append_physics_performer_mpirun_commands_dakota_problem
(const XMLGen::InputData& aInputData,
 int &aNextPerformerID,
 FILE*& aFile)
{
    for(auto& tService : aInputData.services())
    {
        auto tEvaluations = std::stoi(aInputData.optimization_parameters().concurrent_evaluations());
        for (int iEvaluation = 0; iEvaluation < tEvaluations; iEvaluation++)
        {
            if(tService.code() == "plato_analyze")
            {
                append_analyze_mpirun_line(aInputData, tService, aNextPerformerID, aFile, iEvaluation);
                aNextPerformerID++;
            }
            else if(tService.code() == "sierra_sd")
            {
                append_sierra_sd_mpirun_line(aInputData, tService, aNextPerformerID, aFile, iEvaluation);
                aNextPerformerID++;
            }
        }
    }
    XMLGen::append_engine_services_mpirun_lines(aInputData, aNextPerformerID, aFile);
}

}
// namespace XMLGen
