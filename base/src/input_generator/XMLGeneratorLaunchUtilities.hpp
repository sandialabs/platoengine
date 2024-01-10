#pragma once

#include "pugixml.hpp"

#include "XMLGeneratorDataStruct.hpp"

namespace XMLGen
{
  void generate_engine_bash_script();
  void generate_analyze_bash_script();
  void generate_jsrun_script(const size_t& aNumPerformers, const std::string& performerName);
  void generate_batch_script(const size_t& aNumPerformers,const size_t& aNumProcessorsPerNode = 6/*num gpus per node on summit*/);
  void append_esp_initialization_line(const XMLGen::InputData& aInputData, FILE* aFile);
  void append_copy_mesh_lines_for_dakota_workflow(FILE* aFile, const XMLGen::InputData& aInputData);
  void determine_mpi_env_and_separation_strings(std::string& envString, std::string& separationString);
  void determine_mpi_launch_strings(const XMLGen::InputData& aInputData, std::string& aLaunchString, std::string& aNumProcsString);
  void append_join_mesh_operation_line(const XMLGen::InputData& aInputData, FILE *aFile);
  void append_tet10_conversion_operation_lines_for_dakota_workflow(FILE* aFile,const std::string &aEvaluations);
  void append_tet10_conversion_operation_line(FILE* aFile);
  void append_subblock_creation_operation_lines_for_dakota_workflow(FILE* aFile,const std::string &aEvaluations);
  void append_decomp_lines_for_prune_and_refine(const XMLGen::InputData& aInputData, FILE*& fp);
  void append_decomp_line(FILE*& fp, const std::string& num_processors, const std::string& mesh_file_name);
  void append_decomp_line(FILE*& fp, const int& num_processors, const std::string& mesh_file_name);
  void append_decomp_lines_for_dakota_workflow(FILE*& fp, const std::string& num_processors, int num_evaluations, const std::string& mesh_file_name);
  void append_prune_and_refine_lines_to_mpirun_launch_script(const XMLGen::InputData& aInputData, FILE*& fp);
  void append_prune_and_refine_command(const XMLGen::InputData& aInputData, FILE*& fp);
  void append_concatenate_mesh_file_lines(const XMLGen::InputData& aInputData, FILE*& fp);
  void append_decomp_lines_to_mpirun_launch_script(const XMLGen::InputData& aInputData, FILE*& fp);
  void append_decomp_lines_for_optimizer(const XMLGen::InputData& aInputData,
      FILE*& fp,
      std::map<std::string,int>& hasBeenDecompedForThisNumberOfProcessors);
  void append_decomp_lines_for_performers(const XMLGen::InputData& aInputData, FILE*& fp,
      std::map<std::string,int>& hasBeenDecompedForThisNumberOfProcessors);
  void append_engine_mpirun_lines(const XMLGen::InputData& aInputData, FILE*& fp);
  void append_plato_analyze_code_path(const XMLGen::InputData& aInputData, FILE*& aFile, const std::string& aServiceID, const std::string& aDeviceID, const int aEvaluation = -1);
  void append_sierra_sd_code_path(const XMLGen::InputData& aInputData, FILE*& aFile, const std::string& aServiceID, const int aEvaluation = -1);
  void append_sierra_tf_code_path(const XMLGen::InputData& aInputData, FILE*& aFile, const std::string& aServiceID);
  void append_engine_services_mpirun_lines(const XMLGen::InputData& aInputData, int &aNextPerformerID, FILE*& fp);
  void append_post_optimization_run_lines(const XMLGen::InputData& aInputData, FILE*& fp);
  
  namespace Internal
  {
    int get_number_of_refines(const XMLGen::InputData& aInputData);
    int get_max_number_of_objective_procs(const XMLGen::InputData& aInputData);
    int get_number_of_prune_and_refine_procs(const XMLGen::InputData& aInputData);
    std::string get_num_opt_processors(const XMLGen::InputData& aInputData);
    std::string get_num_buffer_layers(const XMLGen::InputData& aInputData);
    std::string get_prune_string(const XMLGen::InputData& aInputData);
    std::string get_prune_threshold_string(const XMLGen::InputData& aInputData);
    std::string get_extension_string(const std::string& tNumberPruneAndRefineProcsString);
    std::string get_prune_and_refine_executable_path(const XMLGen::InputData& aInputData);
    size_t compute_number_of_nodes_needed(const size_t& aNumProcessorsNeeded, const size_t& aNumProcessorsPerNode);
  }
}
// namespace XMLGen
