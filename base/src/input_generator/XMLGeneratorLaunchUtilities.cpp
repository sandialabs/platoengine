#include "XMLGeneratorCriterionMetadata.hpp"
#include "pugixml.hpp"

#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorDataStruct.hpp"
#include "XMLGeneratorLaunchUtilities.hpp"
#include "XMLGeneratorPostOptimizationRunFileUtilities.hpp"
#include <iostream>
#include <fstream>

namespace XMLGen
{
  void generate_engine_bash_script()
  {
    std::ofstream engineBash;
    engineBash.open("engine.sh");
    engineBash << "export PLATO_PERFORMER_ID=0\n";
    engineBash << "export PLATO_INTERFACE_FILE=interface.xml\n";
    engineBash << "export PLATO_APP_FILE=plato_main_operations.xml\n";
    engineBash << "\n";
    engineBash << "PlatoMain plato_main_input_deck.xml";
    engineBash.close();
  }

  void generate_analyze_bash_script()
  {
    std::ofstream analyzeBash;
    std::string filename = "analyze.sh";
    analyzeBash.open(filename);
    analyzeBash << "export PLATO_PERFORMER_ID=1\n";
    analyzeBash << "export PLATO_INTERFACE_FILE=interface.xml\n";
    analyzeBash << "export PLATO_APP_FILE=plato_analyze_operations.xml\n";
    analyzeBash << "\n";
    analyzeBash << "analyze_MPMD --input-config=plato_analyze_input_deck.xml\n";

    analyzeBash.close();
  }

  void generate_jsrun_script
    (const size_t& aNumPerformers, const std::string& performerName)
  {
    std::ofstream jsrun;
    jsrun.open("jsrun.source");
    jsrun << "1 : eng : bash engine.sh\n";
    jsrun << aNumPerformers << " : per : bash " << performerName << ".sh\n";
    jsrun.close();
  }

  void generate_batch_script
    (const size_t& aNumPerformers,
     const size_t& aNumProcessorsPerNode)
  {
    std::ofstream batchFile;
    batchFile.open ("plato.batch");
    batchFile << "#!/bin/bash\n";
    batchFile << "# LSF Directives\n";
    batchFile << "#BSUB -P <PROJECT>\n";
    batchFile << "#BSUB -W 0:00\n";

    size_t tNumNodesNeeded = XMLGen::Internal::compute_number_of_nodes_needed(aNumPerformers,aNumProcessorsPerNode);

    batchFile << "#BSUB -nnodes " << tNumNodesNeeded << "\n";
    batchFile << "#BSUB -J plato\n";
    batchFile << "#BSUB -o plato.%J\n";
    batchFile << "\n";
    batchFile << "cd <path/to/working/directory>\n";
    batchFile << "date\n";
    batchFile << "jsrun -A eng -n1 -a1 -c1 -g0\n";
    batchFile << "jsrun -A per -n" << aNumPerformers << " -a1 -c1 -g1\n";
    batchFile << "jsrun -f jsrun.source\n";
    
    batchFile.close();
  }

  void append_esp_initialization_line(const XMLGen::InputData& aInputData, FILE* aFile)
  {
    std::string tEnvString, tSeparationString, tLaunchString, tNumProcsString;
    XMLGen::determine_mpi_env_and_separation_strings(tEnvString, tSeparationString);
    XMLGen::determine_mpi_launch_strings(aInputData, tLaunchString, tNumProcsString);

    if(aInputData.optimization_parameters().csm_file() != "" &&
       aInputData.optimization_parameters().csm_opt_file() != "" &&
       aInputData.optimization_parameters().csm_tesselation_file() != "" &&
       aInputData.optimization_parameters().csm_exodus_file() != "")
    {
        fprintf(aFile,
            "plato-cli geometry esp --input %s --output-model %s --output-mesh %s --tesselation %s; \\\n",
            aInputData.optimization_parameters().csm_file().c_str(),
            aInputData.optimization_parameters().csm_opt_file().c_str(),
            aInputData.optimization_parameters().csm_exodus_file().c_str(),
            aInputData.optimization_parameters().csm_tesselation_file().c_str());
    }
  }

  void determine_mpi_env_and_separation_strings(std::string& envString, std::string& separationString)
  {
#ifndef USING_OPEN_MPI
    envString = "-env";
    separationString = " ";
#else
    envString = "-x";
    separationString = "=";
#endif
  }

  void determine_mpi_launch_strings(const XMLGen::InputData& aInputData, std::string& aLaunchString, std::string& aNumProcsString)
  {
    if(aInputData.m_UseLaunch)
    {
      aLaunchString = "launch";
      aNumProcsString = "-n";
    }
    else
    {
      aLaunchString = "mpiexec";
#ifdef USING_OPEN_MPI
      aLaunchString += " --oversubscribe";
#endif
      aNumProcsString = "-np";
    }
  }

  void append_join_mesh_operation_line(const XMLGen::InputData& aInputData, FILE *aFile)
  {
    const std::string exodusFile(aInputData.optimization_parameters().csm_exodus_file());
    const std::string auxiliaryMeshFile(aInputData.mesh.auxiliary_mesh_name);
    const std::string joinedMeshFile(aInputData.mesh.joined_mesh_name);

    fprintf(aFile, "ejoin -output %s %s %s && /bin/cp %s %s; \\\n",
    joinedMeshFile.data(), exodusFile.data(), auxiliaryMeshFile.data(),
    joinedMeshFile.data(), exodusFile.data());
  }

  void append_tet10_conversion_operation_lines_for_dakota_workflow(FILE* aFile,const std::string &aEvaluations)
  {
    for (int iEvaluation = 0; iEvaluation < std::stoi(aEvaluations); iEvaluation++)
      fprintf(aFile, "cd evaluations_%d; cubit -input toTet10.jou -batch -nographics -nogui -noecho -nojournal -nobanner -information off; cd ..\n", iEvaluation);
  }

  void append_tet10_conversion_operation_line(FILE* aFile)
  {
    fprintf(aFile, "cubit -input toTet10.jou -batch -nographics -nogui -noecho -nojournal -nobanner -information off; \n");
  }

  void append_decomp_lines_for_prune_and_refine(const XMLGen::InputData& aInputData, FILE*& fp)
  {
    if(aInputData.optimization_parameters().isARestartRun())
    {
      int tNumberPruneAndRefineProcs = XMLGen::Internal::get_number_of_prune_and_refine_procs(aInputData);
      if(tNumberPruneAndRefineProcs > 1)
      {
        if(aInputData.mesh.name.empty())
          THROWERR("Missing input mesh name\n")
        XMLGen::append_decomp_line(fp,tNumberPruneAndRefineProcs,aInputData.mesh.name);
        if(aInputData.optimization_parameters().initial_guess_file_name() != "")
          XMLGen::append_decomp_line(fp,tNumberPruneAndRefineProcs,aInputData.optimization_parameters().initial_guess_file_name());
      }
    }
  }

  void append_decomp_line(FILE*& fp, const std::string& num_processors, const std::string& mesh_file_name)
  {
    fprintf(fp, "decomp -p %s %s\n", num_processors.c_str(), mesh_file_name.c_str());
  }

  void append_decomp_line(FILE*& fp, const int& num_processors, const std::string& mesh_file_name)
  {
    fprintf(fp, "decomp -p %d %s\n", num_processors, mesh_file_name.c_str());
  }

  void append_decomp_lines_for_dakota_workflow(FILE*& fp, const std::string& num_processors, int num_evaluations, const std::string& mesh_file_name)
  {
    for (int iEvaluation = 0; iEvaluation < num_evaluations; iEvaluation++)
    {
      std::string tTag = std::string("_") + std::to_string(iEvaluation);
      std::string appended_mesh_file_name = XMLGen::append_concurrent_tag_to_file_string(mesh_file_name,tTag);
      fprintf(fp, "cd evaluations_%d; decomp -p %s %s; cd ..\n", iEvaluation, num_processors.c_str(), appended_mesh_file_name.c_str());
    }
  }

  void append_prune_and_refine_lines_to_mpirun_launch_script(const XMLGen::InputData& aInputData, FILE*& fp)
  {
    int tNumRefines = XMLGen::Internal::get_number_of_refines(aInputData);
    bool need_to_transfer_prune_or_refine = tNumRefines > 0 || aInputData.optimization_parameters().isARestartRun();

    if(need_to_transfer_prune_or_refine)
    {
      XMLGen::append_prune_and_refine_command(aInputData, fp);
      XMLGen::append_concatenate_mesh_file_lines(aInputData,fp);
    }
  }

  void append_prune_and_refine_command(const XMLGen::InputData& aInputData, FILE*& fp)
  {
    if(aInputData.mesh.name.empty())
      THROWERR("No mesh name provided");
    if(aInputData.mesh.run_name.empty())
      THROWERR("No output mesh name provided");

    std::string tPruneString = XMLGen::Internal::get_prune_string(aInputData);
    std::string tNumRefinesString = Plato::to_string(XMLGen::Internal::get_number_of_refines(aInputData));
    std::string tNumBufferLayersString = XMLGen::Internal::get_num_buffer_layers(aInputData);
    int tNumberPruneAndRefineProcs = XMLGen::Internal::get_number_of_prune_and_refine_procs(aInputData);
    std::string tNumberPruneAndRefineProcsString = Plato::to_string(tNumberPruneAndRefineProcs);
    std::string tPruneAndRefineExe = XMLGen::Internal::get_prune_and_refine_executable_path(aInputData);

    std::string tCommand;
    if(aInputData.m_UseLaunch)
        tCommand = "launch -n " + tNumberPruneAndRefineProcsString + " " + tPruneAndRefineExe;
    else
        tCommand = "mpiexec -np " + tNumberPruneAndRefineProcsString + " " + tPruneAndRefineExe;
    if(aInputData.optimization_parameters().initial_guess_file_name() != "")
        tCommand += (" --mesh_with_variable=" + aInputData.optimization_parameters().initial_guess_file_name());
    tCommand += (" --mesh_to_be_pruned=" + aInputData.mesh.name);
    tCommand += (" --result_mesh=" + aInputData.mesh.run_name);
    if(aInputData.optimization_parameters().initial_guess_field_name() != "")
        tCommand += (" --field_name=" + aInputData.optimization_parameters().initial_guess_field_name());
    tCommand += (" --number_of_refines=" + tNumRefinesString);
    tCommand += (" --number_of_buffer_layers=" + tNumBufferLayersString);
    tCommand += (" --prune_mesh=" + tPruneString);

    fprintf(fp, "%s\n", tCommand.c_str());
  }

  void append_concatenate_mesh_file_lines(const XMLGen::InputData& aInputData, FILE*& fp)
  {
    int tNumberPruneAndRefineProcs = XMLGen::Internal::get_number_of_prune_and_refine_procs(aInputData);
    std::string tNumberPruneAndRefineProcsString = Plato::to_string(tNumberPruneAndRefineProcs);

    if(tNumberPruneAndRefineProcs > 1)
    {
      if(aInputData.mesh.run_name == "")
        THROWERR("mesh run_name not set")

          std::string tExtensionString = XMLGen::Internal::get_extension_string(tNumberPruneAndRefineProcsString);
      fprintf(fp, "epu -auto %s%s\n", aInputData.mesh.run_name.c_str(), tExtensionString.c_str());
    }
  }

  void append_decomp_lines_to_mpirun_launch_script(const XMLGen::InputData& aInputData, FILE*& fp)
  {
    std::map<std::string,int> hasBeenDecompedForThisNumberOfProcessors;

    XMLGen::append_decomp_lines_for_optimizer(aInputData, fp, hasBeenDecompedForThisNumberOfProcessors);
    XMLGen::append_decomp_lines_for_performers(aInputData, fp, hasBeenDecompedForThisNumberOfProcessors);
  }

  void append_decomp_lines_for_optimizer(const XMLGen::InputData& aInputData,
                                              FILE*& fp,
                                              std::map<std::string,int>& hasBeenDecompedForThisNumberOfProcessors)
  {
    if(aInputData.mesh.run_name.empty())
      THROWERR("Cannot add decomp line: Mesh name not provided\n")

    std::string num_opt_procs = XMLGen::Internal::get_num_opt_processors(aInputData);
    XMLGen::assert_is_positive_integer(num_opt_procs);
    
    bool need_to_decompose = num_opt_procs.compare("1") != 0;
    if(need_to_decompose)
    {
      if(hasBeenDecompedForThisNumberOfProcessors[num_opt_procs]++ == 0)
        XMLGen::append_decomp_line(fp, num_opt_procs, aInputData.mesh.run_name);
      if(aInputData.optimization_parameters().initial_guess_file_name() != "")
        XMLGen::append_decomp_line(fp, num_opt_procs, aInputData.optimization_parameters().initial_guess_file_name());
    }
  }

  void append_decomp_lines_for_performers(const XMLGen::InputData& aInputData, FILE*& fp,
                                          std::map<std::string,int>& hasBeenDecompedForThisNumberOfProcessors)
  {
    if(aInputData.mesh.run_name.empty())
      THROWERR("Cannot add decomp line: Mesh name not provided\n")

    for(size_t i=0; i<aInputData.objective.serviceIDs.size(); ++i)
    {
        XMLGen::Service tService = aInputData.service(aInputData.objective.serviceIDs[i]);
        if(tService.code() != "plato_analyze")
        {
            XMLGen::Criterion tCriterion = aInputData.criterion(aInputData.objective.criteriaIDs[i]);
            std::string num_procs = tService.numberProcessors();
    
            XMLGen::assert_is_positive_integer(num_procs);

            bool need_to_decompose = num_procs.compare("1") != 0;
            if(need_to_decompose)
            {
                if(hasBeenDecompedForThisNumberOfProcessors[num_procs]++ == 0) {
                  if (aInputData.optimization_parameters().optimizationType() == OT_DAKOTA)
                  {
                    auto tMeshName = aInputData.optimization_parameters().csm_exodus_file();
                    int tNumEvaluations = std::stoi(aInputData.optimization_parameters().concurrent_evaluations());
                    XMLGen::append_decomp_lines_for_dakota_workflow(fp, num_procs, tNumEvaluations, tMeshName);
                  }
                  else
                    XMLGen::append_decomp_line(fp, num_procs, aInputData.mesh.run_name);
                }

                if(tCriterion.value("ref_data_file").length() > 0)
                  XMLGen::append_decomp_line(fp, num_procs, tCriterion.value("ref_data_file"));
            }
        }
    }
  }

  void append_engine_mpirun_lines(const XMLGen::InputData& aInputData, FILE*& fp)
  {
    std::string envString, separationString, tLaunchString, tNumProcsString, tPlatoEngineName;

    XMLGen::determine_mpi_env_and_separation_strings(envString,separationString);
    XMLGen::determine_mpi_launch_strings(aInputData,tLaunchString,tNumProcsString);
    tPlatoEngineName = "PlatoMain";   

    std::string num_opt_procs = XMLGen::Internal::get_num_opt_processors(aInputData);
    XMLGen::assert_is_positive_integer(num_opt_procs);

    // Now add the main mpirun call.
    fprintf(fp, "%s %s %s %s PLATO_PERFORMER_ID%s0 \\\n", tLaunchString.c_str(), 
                                tNumProcsString.c_str(), num_opt_procs.c_str(),
                                envString.c_str(),separationString.c_str());
    fprintf(fp, "%s PLATO_INTERFACE_FILE%sinterface.xml \\\n", envString.c_str(),separationString.c_str());
    fprintf(fp, "%s PLATO_APP_FILE%splato_main_operations.xml \\\n", envString.c_str(),separationString.c_str());

    auto tPlatoMainServiceId = aInputData.getFirstPlatoMainId();
    if(aInputData.service(tPlatoMainServiceId).path().length() != 0)
      fprintf(fp, "%s plato_main_input_deck.xml \\\n", aInputData.service(tPlatoMainServiceId).path().c_str());
    else
      fprintf(fp, "%s plato_main_input_deck.xml \\\n", tPlatoEngineName.c_str());
  }

  void append_plato_analyze_code_path(const XMLGen::InputData& aInputData, FILE*& aFile, const std::string& aServiceID, const std::string& aDeviceID, const int aEvaluation)
  {
    // Build id string based on the performer id.
    std::string tIDString;
    if(aServiceID != "")
    {
        tIDString = "_" + aServiceID + "_";
    }
    else
    {
        tIDString = "_";
    }

    // Add executable name.
    if(aInputData.service(aServiceID).path().length() != 0)
    {
        fprintf(aFile, "%s ", aInputData.service(aServiceID).path().c_str());
    }
    else
    {
        fprintf(aFile, "analyze_MPMD ");
    }

    // Add kokkos-device setting if requested.
    if(aDeviceID != "")
    {
        fprintf(aFile, "--kokkos-device-id=%s ", aDeviceID.c_str());
    }

    // Add the input deck syntax.
    if (aEvaluation == -1)
    {
        fprintf(aFile, "--input-config=plato_analyze%sinput_deck.xml \\\n", tIDString.c_str());
    }
    else
    {
        auto tEvaluationString = std::to_string(aEvaluation);
        fprintf(aFile, "--input-config=evaluations_%s/plato_analyze%sinput_deck_%s.xml \\\n", tEvaluationString.c_str(), tIDString.c_str(), tEvaluationString.c_str());
    }
  }

  void append_sierra_sd_code_path(const XMLGen::InputData& aInputData, FILE*& aFile, const std::string& aServiceID, const int aEvaluation)
  {
    if(aInputData.service(aServiceID).path().length() != 0)
    {
        fprintf(aFile, "%s --beta -i ", aInputData.service(aServiceID).path().c_str());
    }
    else
    {
        fprintf(aFile, "plato_sd_main --beta -i ");
    }
    if (aEvaluation == -1)
    {
        fprintf(aFile, "sierra_sd_%s_input_deck.i \\\n", aServiceID.c_str());
    }
    else
    {
        auto tEvaluationString = std::to_string(aEvaluation);
        fprintf(aFile, "evaluations_%s/sierra_sd_%s_input_deck_%s.i \\\n", tEvaluationString.c_str(), aServiceID.c_str(), tEvaluationString.c_str());
    }
  }

  void append_engine_services_mpirun_lines(const XMLGen::InputData& aInputData, int &aNextPerformerID, FILE*& fp)
  {
    std::string envString, separationString, tLaunchString, tNumProcsString, tPlatoServicesPath;

    XMLGen::determine_mpi_env_and_separation_strings(envString,separationString);
    XMLGen::determine_mpi_launch_strings(aInputData,tLaunchString,tNumProcsString);

    std::string tPlatoServicesName = "PlatoEngineServices";   
    auto tPlatoMainService = aInputData.service(aInputData.getFirstPlatoMainId());
    std::string tPlatoMainPath = tPlatoMainService.path();
    if(tPlatoMainPath.length() != 0)
    {
      size_t tPos = tPlatoMainPath.rfind("/");
      std::string tPathToExecutable = tPlatoMainPath.substr(0, tPos);
      tPlatoServicesPath = tPathToExecutable + std::string("/") + tPlatoServicesName;
    }
    else
      tPlatoServicesPath = tPlatoServicesName;

    auto tEvaluations = std::stoi(aInputData.optimization_parameters().concurrent_evaluations());
    for (int iEvaluation = 0; iEvaluation < tEvaluations; iEvaluation++)
    {
      fprintf(fp, ": %s %s %s PLATO_PERFORMER_ID%s%d \\\n", tNumProcsString.c_str(), tPlatoMainService.numberProcessors().c_str(),
                                  envString.c_str(),separationString.c_str(), aNextPerformerID);
      fprintf(fp, "%s PLATO_INTERFACE_FILE%sinterface.xml \\\n", envString.c_str(),separationString.c_str());
      fprintf(fp, "%s PLATO_APP_FILE%splato_main_operations.xml \\\n", envString.c_str(),separationString.c_str());

      auto tEvaluationString = std::to_string(iEvaluation);
      fprintf(fp, "%s evaluations_%s/plato_main_input_deck_%s.xml \\\n", tPlatoServicesPath.c_str(), tEvaluationString.c_str(), tEvaluationString.c_str());

      aNextPerformerID++;
    }
  }

  void append_post_optimization_run_lines(const XMLGen::InputData& aInputData, FILE*& fp)
  {
      // This newline is important because the performer lines always end with a
      // "\" so that the command continues on the next line so we need to 
      // add a newline to end the previous command.
      fprintf(fp, "\n");
      for(const XMLGen::Run &tCurRun : aInputData.runs())
      {
          if(tCurRun.command().empty())
          {
              std::string tType = tCurRun.type();
              if(tType == "modal_analysis")
              {
                  int tNumProcs = 1;
                  std::string tServiceID = tCurRun.service();
                  if(!tServiceID.empty())
                  {
                      XMLGen::Service tService = aInputData.service(tServiceID);
                      std::string tNumProcsString = tService.numberProcessors(); 
                      tNumProcs = std::atoi(tNumProcsString.c_str());
                      if(tNumProcs > 1)
                      {
                          append_decomp_line(fp, tNumProcs, aInputData.mesh.run_name);
                      }
                  }
                  std::string tInputDeckName = build_post_optimization_run_input_deck_name(tCurRun);
                  std::string tLaunchString, tTempNumProcsString;
                  XMLGen::determine_mpi_launch_strings(aInputData,tLaunchString,tTempNumProcsString);
                  fprintf(fp, "%s %s %d salinas -i %s\n", tLaunchString.c_str(), tTempNumProcsString.c_str(), 
                                                       tNumProcs, tInputDeckName.c_str());
              }
          }
          else
          {
              fprintf(fp, "%s\n", tCurRun.command().c_str());
          }
      } 
  }

  namespace Internal
  {
    int get_number_of_refines(const XMLGen::InputData& aInputData)
    {
      int tNumRefines = 0;
      if(aInputData.optimization_parameters().number_refines() != "" && aInputData.optimization_parameters().number_refines() != "0")
      {
        XMLGen::assert_is_positive_integer(aInputData.optimization_parameters().number_refines());
        tNumRefines = std::atoi(aInputData.optimization_parameters().number_refines().c_str());
      }
      return tNumRefines;
    }

    int get_max_number_of_objective_procs(const XMLGen::InputData& aInputData)
    {
      int tMaxNumObjProcs = 0;
      for(size_t i=0; i<aInputData.objective.serviceIDs.size(); ++i)
      {
          XMLGen::Service tService = aInputData.service(aInputData.objective.serviceIDs[i]);
          std::string num_procs = tService.numberProcessors();
          if(!num_procs.empty())
          {
              XMLGen::assert_is_positive_integer(num_procs);
              int tNumProcs = std::atoi(num_procs.c_str());
              if(tNumProcs > tMaxNumObjProcs)
                tMaxNumObjProcs = tNumProcs;
          }
      }

      return tMaxNumObjProcs;
    }

    int get_number_of_prune_and_refine_procs(const XMLGen::InputData& aInputData)
    {
      int tNumberPruneAndRefineProcs = 1;
      if(aInputData.optimization_parameters().number_prune_and_refine_processors() != "" &&
              aInputData.optimization_parameters().number_prune_and_refine_processors() != "0")
      {
          std::string tNumberPruneAndRefineProcsString = aInputData.optimization_parameters().number_prune_and_refine_processors();
          XMLGen::assert_is_positive_integer(tNumberPruneAndRefineProcsString);
          tNumberPruneAndRefineProcs = std::atoi(tNumberPruneAndRefineProcsString.c_str());
      }
      else
        tNumberPruneAndRefineProcs = XMLGen::Internal::get_max_number_of_objective_procs(aInputData);

      return tNumberPruneAndRefineProcs;
    }

    std::string get_num_opt_processors(const XMLGen::InputData& aInputData)
    {
        std::string num_opt_procs = "1";
        for(auto tService : aInputData.services())
        {
            // Find the first platomain service
            if(tService.code() == "platomain")
            {
                num_opt_procs = tService.numberProcessors();
                break;
            } 
        }
        return num_opt_procs;
    }

    std::string get_num_buffer_layers(const XMLGen::InputData& aInputData)
    {
      std::string tNumBufferLayersString = "2";
      if(aInputData.optimization_parameters().number_buffer_layers() != "")
        tNumBufferLayersString = aInputData.optimization_parameters().number_buffer_layers();
      if(tNumBufferLayersString != "0")
        XMLGen::assert_is_positive_integer(tNumBufferLayersString);
      return tNumBufferLayersString;
    }

    std::string get_prune_string(const XMLGen::InputData& aInputData)
    {
      std::string tPruneString = "0";
      if(aInputData.optimization_parameters().prune_mesh() == "true")
        tPruneString = "1";
      return tPruneString;
    }

    std::string get_extension_string(const std::string& tNumberPruneAndRefineProcsString)
    {
      std::string tExtensionString = "." + tNumberPruneAndRefineProcsString + ".";
      for(size_t g=0; g<tNumberPruneAndRefineProcsString.length(); ++g)
        tExtensionString += "0";
      return tExtensionString;
    }

    std::string get_prune_and_refine_executable_path(const XMLGen::InputData& aInputData)
    {
      std::string tPruneAndRefineExe = "prune_and_refine";
      if(aInputData.optimization_parameters().prune_and_refine_path().length() > 0)
        tPruneAndRefineExe = aInputData.optimization_parameters().prune_and_refine_path();
      return tPruneAndRefineExe;
    }

    size_t compute_number_of_nodes_needed(const size_t& aNumProcessorsNeeded, const size_t& aNumProcessorsPerNode)
    {
      if(aNumProcessorsPerNode == 0)
        THROWERR("Error: zero processors per node specified")
      size_t tNumNodesNeeded = aNumProcessorsNeeded/aNumProcessorsPerNode;
      if(aNumProcessorsNeeded % aNumProcessorsPerNode != 0)
        ++tNumNodesNeeded;
      return tNumNodesNeeded;
    }
  }
}
// namespace XMLGen
