/*
 * XMLGeneratorDakotaDriverInputFileUtilities.cpp
 *
 *  Created on: Jan 24, 2022
 */

#include "XMLGeneratorDakotaDriverInputFileUtilities.hpp"
#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorParserUtilities.hpp"
#include "Plato_ParseCSMUtilities.hpp"

namespace XMLGen
{

/******************************************************************************/
void write_dakota_driver_input_deck
(const XMLGen::InputData& aMetaData)
{
    FILE *fp = fopen("plato_dakota_input_deck.i", "w");

    XMLGen::check_dakota_workflow(aMetaData);
    XMLGen::append_dakota_driver_title(aMetaData,fp);
    XMLGen::append_dakota_driver_environment_block(aMetaData,fp);
    XMLGen::append_dakota_driver_method_blocks(aMetaData,fp);
    XMLGen::append_dakota_driver_model_blocks(aMetaData,fp);
    XMLGen::append_dakota_driver_variables_block(aMetaData,fp);
    XMLGen::append_dakota_driver_interface_block(aMetaData,fp);
    XMLGen::append_dakota_driver_responses_block(aMetaData,fp);

    fclose(fp);
}

/******************************************************************************/
void check_dakota_workflow
(const XMLGen::InputData& aMetaData)
{
    auto tWorkflow = aMetaData.optimization_parameters().dakota_workflow();
    if (tWorkflow.empty())
        THROWERR("Optimization parameter dakota_workflow must be specified for optimization_type dakota")
    if (tWorkflow != "mdps" && tWorkflow != "sbgo")
        THROWERR("Dakota workflow '" + tWorkflow + "' is not supported.")
}

/******************************************************************************/
void append_dakota_driver_title
(const XMLGen::InputData& aMetaData,
 FILE*& fp)
{
    auto tWorkflow = aMetaData.optimization_parameters().dakota_workflow();
    if (tWorkflow == "mdps")
        fprintf(fp, "# input file for dakota multi-dimensional parameter study (MDPS) workflow\n");
    else if (tWorkflow == "sbgo")
        fprintf(fp, "# input file for dakota surrogate-based global optimization (SBGO) workflow\n");
}

/******************************************************************************/
void append_dakota_driver_environment_block
(const XMLGen::InputData& aMetaData,
 FILE*& fp)
{
    auto tWorkflow = aMetaData.optimization_parameters().dakota_workflow();

    fprintf(fp, "\n environment\n");
    fprintf(fp, "   tabular_data\n");
    if (tWorkflow == "mdps")
    {
        fprintf(fp, "       tabular_data_file = 'dakota_multi_dimensional_parameter_study.dat'\n");
    }
    else if (tWorkflow == "sbgo")
    {
        fprintf(fp, "       tabular_data_file = 'dakota_surrogate_based_global_optimization.dat'\n");
        fprintf(fp, "   top_method_pointer = 'SBGO'\n");
    }
}

/******************************************************************************/
void append_dakota_driver_method_blocks
(const XMLGen::InputData& aMetaData,
 FILE*& fp)
{
    auto tWorkflow = aMetaData.optimization_parameters().dakota_workflow();
    if (tWorkflow == "mdps")
    {
        XMLGen::append_multidim_parameter_study_method_block(aMetaData,fp);
    }
    else if (tWorkflow == "sbgo")
    {
        XMLGen::append_surrogate_based_global_method_block(aMetaData,fp);
        XMLGen::append_moga_method_block(aMetaData,fp);
        XMLGen::append_sampling_method_block(aMetaData,fp);
    }
}

/******************************************************************************/
void append_multidim_parameter_study_method_block
(const XMLGen::InputData& aMetaData,
 FILE*& fp)
{
    auto tPartitions = aMetaData.optimization_parameters().mdps_partitions();

    fprintf(fp, "\n method\n");
    fprintf(fp, "   multidim_parameter_study\n");

    size_t tNumParameters = XMLGen::get_number_of_shape_parameters(aMetaData);

    fprintf(fp, "       partitions =");

    if(tPartitions.size()==1)
    while(tPartitions.size() < tNumParameters)
        tPartitions.push_back(tPartitions[0]);
    for (size_t iParameter = 0; iParameter < tNumParameters; iParameter++)
        fprintf(fp, " %s ", tPartitions[iParameter].c_str());
    fprintf(fp, "\n");
}

/******************************************************************************/
void append_surrogate_based_global_method_block
(const XMLGen::InputData& aMetaData,
 FILE*& fp)
{
    auto tMaxIterations = aMetaData.optimization_parameters().sbgo_max_iterations();

    fprintf(fp, "\n method\n");
    fprintf(fp, "   id_method = 'SBGO'\n");
    fprintf(fp, "   surrogate_based_global\n");
    fprintf(fp, "       model_pointer = 'SURROGATE'\n");
    fprintf(fp, "       method_pointer = 'MOGA'\n");
    fprintf(fp, "       max_iterations = %s\n", tMaxIterations.c_str());
    fprintf(fp, "       replace_points\n");
    fprintf(fp, "   output quiet\n");
}

/******************************************************************************/
void append_moga_method_block
(const XMLGen::InputData& aMetaData,
 FILE*& fp)
{
    auto tPopulationSize = aMetaData.optimization_parameters().moga_population_size();
    auto tMaxEvaluations = aMetaData.optimization_parameters().moga_max_function_evaluations();
    auto tNichingDistance = aMetaData.optimization_parameters().moga_niching_distance();

    fprintf(fp, "\n method\n");
    fprintf(fp, "   id_method = 'MOGA'\n");
    fprintf(fp, "   moga\n");
    fprintf(fp, "       seed = 10983\n");
    fprintf(fp, "       population_size = %s\n", tPopulationSize.c_str());
    fprintf(fp, "       max_function_evaluations = %s\n", tMaxEvaluations.c_str());
    fprintf(fp, "       initialization_type unique_random\n");
    fprintf(fp, "       crossover_type shuffle_random\n");
    fprintf(fp, "           num_offspring = 2 num_parents = 2\n");
    fprintf(fp, "           crossover_rate = 0.8\n");
    fprintf(fp, "       mutation_type replace_uniform\n");
    fprintf(fp, "           mutation_rate = 0.1\n");
    fprintf(fp, "       fitness_type domination_count\n");
    fprintf(fp, "       replacement_type elitist\n");
    fprintf(fp, "       niching_type distance %s\n", tNichingDistance.c_str());
    fprintf(fp, "       postprocessor_type\n");
    fprintf(fp, "           orthogonal_distance 0.01\n");
    fprintf(fp, "       convergence_type metric_tracker\n");
    fprintf(fp, "           percent_change = 0.01 num_generations = 10\n");
    fprintf(fp, "   output normal\n");
    fprintf(fp, "   scaling\n");
}

/******************************************************************************/
void append_sampling_method_block
(const XMLGen::InputData& aMetaData,
 FILE*& fp)
{
    auto tNumSamples = aMetaData.optimization_parameters().num_sampling_method_samples();

    fprintf(fp, "\n method\n");
    fprintf(fp, "   id_method = 'SAMPLING'\n");
    fprintf(fp, "   sampling\n");
    fprintf(fp, "       samples = %s\n", tNumSamples.c_str());
    fprintf(fp, "       seed = 777\n");
    fprintf(fp, "       sample_type lhs\n");
    fprintf(fp, "       model_pointer = 'TRUTH'\n");
}

/******************************************************************************/
void append_dakota_driver_model_blocks
(const XMLGen::InputData& aMetaData,
 FILE*& fp)
{
    auto tWorkflow = aMetaData.optimization_parameters().dakota_workflow();
    if (tWorkflow == "sbgo")
    {
        XMLGen::append_surrogate_model_block(aMetaData, fp);
        XMLGen::append_single_model_block(fp);
    }
}

/******************************************************************************/
void append_surrogate_model_block
(const XMLGen::InputData& aMetaData,
 FILE*& fp)
{
    fprintf(fp, "\n model\n");
    fprintf(fp, "   id_model = 'SURROGATE'\n");
    fprintf(fp, "   surrogate global\n");
    fprintf(fp, "       dace_method_pointer = 'SAMPLING'\n");
    fprintf(fp, "       gaussian_process surfpack\n");

    auto tSurrogateModelName = aMetaData.optimization_parameters().sbgo_surrogate_output_name();
    if (!tSurrogateModelName.empty())
    {
        fprintf(fp, "       export_model\n");
        fprintf(fp, "           filename_prefix = '%s'\n", tSurrogateModelName.c_str());
        fprintf(fp, "           formats binary_archive\n");
    }
}

/******************************************************************************/
void append_single_model_block
(FILE*& fp)
{
    fprintf(fp, "\n model\n");
    fprintf(fp, "   id_model = 'TRUTH'\n");
    fprintf(fp, "   single\n");
    fprintf(fp, "       interface_pointer = 'TRUE_FN'\n");
}

/******************************************************************************/
void append_dakota_driver_variables_block
(const XMLGen::InputData& aMetaData,
 FILE*& fp)
{
    std::vector<std::string> tVariablesStrings = {
        "       descriptors",
        "       lower_bounds",
        "       upper_bounds",
        "       initial_point"};
    
    std::vector<std::string> tDescriptors;
    auto tCsmFileName = aMetaData.optimization_parameters().csm_file();

    size_t tNumParameters = 0;
    if(tCsmFileName != "")
        tNumParameters = XMLGen::get_variable_strings_from_csm_file(tVariablesStrings, tCsmFileName, aMetaData);
    else if(aMetaData.optimization_parameters().descriptors().size() != 0)
        tNumParameters = XMLGen::get_variable_strings_from_optimization_parameters(tVariablesStrings, aMetaData);

    fprintf(fp, "\n variables\n");
    fprintf(fp, "   continuous_design = %lu\n", tNumParameters);
    for (auto& tString : tVariablesStrings)
    {
        tString += "\n";
        fprintf(fp, "%s", tString.c_str());
    }
}

/******************************************************************************/
size_t get_variable_strings_from_csm_file
(std::vector<std::string>& aVariablesStrings,
 const std::string& aCsmFileName,
 const XMLGen::InputData& aMetaData)
{
    std::vector<double> tInitialValues, tLowerBounds, tUpperBounds;
    std::vector<std::string> tDescriptors;
    
    Plato::ParseCSM::getValuesFromCSMFile(aCsmFileName, tInitialValues, tLowerBounds, tUpperBounds);
    Plato::ParseCSM::getDescriptorsFromCSMFile(aCsmFileName, tDescriptors);

    for(auto iDescriptor : tDescriptors)
        aVariablesStrings[0] += "  '" + iDescriptor + "'  ";
    
    for(auto iLowerBounds : tLowerBounds)
        aVariablesStrings[1] += "  " + std::to_string(iLowerBounds) + "  ";
    
    for(auto iUpperBounds : tUpperBounds)
        aVariablesStrings[2] += "  " + std::to_string(iUpperBounds) + "  ";

    for(auto iInitialValue : tInitialValues)
        aVariablesStrings[3] += "  " + std::to_string(iInitialValue) + "  ";

    if ((int)tDescriptors.size() != std::stoi(aMetaData.optimization_parameters().num_shape_design_variables()))
        THROWERR("Number of design variables specified in input deck does not match number of descriptors.")

    return tDescriptors.size();
}

/******************************************************************************/
size_t get_variable_strings_from_optimization_parameters
(std::vector<std::string>& aVariablesStrings,
 const XMLGen::InputData& aMetaData)
{
    auto tDescriptors = aMetaData.optimization_parameters().descriptors();
    auto tLowerBounds = aMetaData.optimization_parameters().lower_bounds();
    auto tUpperBounds = aMetaData.optimization_parameters().upper_bounds();

    for(auto iDescriptor : tDescriptors )
        aVariablesStrings[0] += "  '" + iDescriptor + "'  ";
    
    for(auto iLowerBounds : tLowerBounds )
        aVariablesStrings[1] += "  " + iLowerBounds + "  ";
    
    for(auto iUpperBounds : tUpperBounds )
        aVariablesStrings[2] += "  " + iUpperBounds + "  ";

    aVariablesStrings.pop_back();

    return tDescriptors.size();
}

/******************************************************************************/
void append_dakota_driver_interface_block
(const XMLGen::InputData& aMetaData,
 FILE*& fp)
{
    auto tWorkflow = aMetaData.optimization_parameters().dakota_workflow();
    auto tEvaluations = aMetaData.optimization_parameters().concurrent_evaluations();

    fprintf(fp, "\n interface\n");
    if (tWorkflow == "sbgo")
        fprintf(fp, "   id_interface = 'TRUE_FN'\n");
    fprintf(fp, "   analysis_drivers = 'plato_dakota_plugin'\n");
    fprintf(fp, "   direct\n");
    fprintf(fp, "   asynchronous evaluation_concurrency %s\n", tEvaluations.c_str());
}

/******************************************************************************/
void append_dakota_driver_responses_block
(const XMLGen::InputData& aMetaData,
 FILE*& fp)
{
    auto tWorkflow = aMetaData.optimization_parameters().dakota_workflow();

    fprintf(fp, "\n responses\n");
    if (tWorkflow == "mdps")
    {
        auto tResponseFunctions = aMetaData.optimization_parameters().mdps_response_functions();
        fprintf(fp, "   response_functions = %s\n", tResponseFunctions.c_str());
    }
    else if (tWorkflow == "sbgo")
    {
        XMLGen::append_surrogate_based_global_responses_block(aMetaData,fp);
    }

    fprintf(fp, "   no_gradients\n");
    fprintf(fp, "   no_hessians\n");
}

/******************************************************************************/
void append_surrogate_based_global_responses_block
(const XMLGen::InputData& aMetaData,
 FILE*& fp)
{
    std::string tDescriptorString = "   descriptors";
    std::string tScalesString = "   primary_scales =";

    XMLGen::Objective tObjective = aMetaData.objective;
    for (size_t i=0; i<tObjective.criteriaIDs.size(); ++i)
    {
        std::string tCriterionId = tObjective.criteriaIDs[i];
        std::string tScenarioID = tObjective.scenarioIDs[i];
        auto& tCriterion = aMetaData.criterion(tCriterionId);
        auto tCriterionType = Plato::tolower(tCriterion.type());
        tDescriptorString += std::string(" '") + tCriterionType + std::string("_scenario") + tScenarioID + std::string("'");
        tScalesString += std::string(" ") + tObjective.weights[i];
    }
    fprintf(fp, "   objective_functions = %zu\n", tObjective.criteriaIDs.size());

    tDescriptorString += "\n";
    tScalesString += "\n";
    fprintf(fp, "%s", tDescriptorString.c_str());
    fprintf(fp, "%s", tScalesString.c_str());
}

}
// namespace XMLGen
