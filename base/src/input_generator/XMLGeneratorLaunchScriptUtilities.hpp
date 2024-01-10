#pragma once

#include "pugixml.hpp"

#include "XMLGeneratorDataStruct.hpp"

namespace XMLGen
{
    void generate_launch_script(const XMLGen::InputData& aInputData);
    void generate_summit_launch_scripts(const XMLGen::InputData& aInputData);
    void generate_mpirun_launch_script(const XMLGen::InputData& aInputData);
    void append_physics_performer_mpirun_lines(const XMLGen::InputData& aInputData, FILE*& aFile);
    void append_analyze_mpirun_commands_robust_optimization_problems(const XMLGen::InputData& aInputData, int &aNextPerformerID, FILE*& aFile);
    void append_physics_performer_mpirun_commands_gradient_based_problem(const XMLGen::InputData& aInputData, int &aNextPerformerID, FILE*& aFile);
    void append_analyze_mpirun_line(const XMLGen::InputData& aInputData, const XMLGen::Service& aService, int &aNextPerformerID, FILE*& aFile, const int aEvaluation = -1);
    void append_sierra_sd_mpirun_line (const XMLGen::InputData& aInputData, const XMLGen::Service& aService, int &aNextPerformerID, FILE*& aFile, const int aEvaluation = -1);
    void append_sierra_tf_mpirun_line (const XMLGen::InputData& aInputData, const XMLGen::Service& aService, int &aNextPerformerID, FILE*& aFile);
    void append_esp_mpirun_line(const XMLGen::InputData& aInputData, const XMLGen::Service& aService, int &aNextPerformerID, FILE*& aFile);
    void append_physics_performer_mpirun_commands_dakota_problem(const XMLGen::InputData& aInputData, int &aNextPerformerID, FILE*& aFile);
}
