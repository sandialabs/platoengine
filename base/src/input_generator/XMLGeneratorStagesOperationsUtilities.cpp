/*
 * XMLGeneratorStagesOperationsUtilities.cpp
 *
 *  Created on: Jan 14, 2022
 */

#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorValidInputKeys.hpp"
#include "XMLGeneratorParserUtilities.hpp"
#include "XMLGeneratorOutputUtilities.hpp"
#include "XMLGeneratorStagesOperationsUtilities.hpp"

namespace XMLGen
{

/******************************************************************************/
std::string get_objective_value_operation_name(XMLGen::Service &aService)
{
    if(aService.code() == "sierra_sd")
    {
        return "Compute Objective";
    }
    else
    {
        return "Compute Objective Value";
    }
    return "";
}
// function get_objective_value_operation_name
/******************************************************************************/

/******************************************************************************/
std::string get_objective_value_operation_output_name(XMLGen::Service &aService)
{
    if(aService.code() == "sierra_sd")
    {
        return "Internal Energy";
    }
    else
    {
        return "Objective Value";
    }
    return "";
}
// function get_objective_value_operation_output_name
/******************************************************************************/

/******************************************************************************/
std::string get_objective_gradient_operation_name(XMLGen::Service &aService)
{
    if(aService.code() == "sierra_sd")
    {
        return "Compute Gradient";
    }
    else
    {
        return "Compute Objective Gradient";
    }
    return "";
}
// function get_objective_gradient_operation_name
/******************************************************************************/

/******************************************************************************/
std::string get_objective_gradient_operation_output_name(XMLGen::Service &aService)
{
    if(aService.code() == "sierra_sd")
    {
        return "Internal Energy Gradient";
    }
    else
    {
        return "Objective Gradient";
    }
    return "";
}
// function get_objective_gradient_operation_output_name
/******************************************************************************/

/******************************************************************************/
void append_design_volume_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"Design Volume", tFirstPlatoMainPerformer}, tOperationNode);
    auto tOutputNode = tOperationNode.append_child("Output");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Design Volume", "Design Volume"}, tOutputNode);
}
// function append_design_volume_operation
/******************************************************************************/

/******************************************************************************/
void append_initial_field_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"},{"Initialize Field", tFirstPlatoMainPerformer}, tOperationNode);
    auto tOutputNode = tOperationNode.append_child("Output");
    XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Initialized Field", "Control"}, tOutputNode);
}
// function append_initial_field_operation
/******************************************************************************/

/******************************************************************************/
void append_compute_normalization_factor_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    if(aMetaData.optimization_parameters().normalizeInAggregator())
    {
        if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY)
        {
            XMLGen::append_filter_control_operation(aMetaData, aParentNode);
        }
        XMLGen::append_objective_value_operation(aMetaData, aParentNode, true);
    }
}
// function append_compute_normalization_factor_operation
/******************************************************************************/

/******************************************************************************/
void append_initial_values_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"},{"Initialize Values", tFirstPlatoMainPerformer}, tOperationNode);
    auto tOutputNode = tOperationNode.append_child("Output");
    XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Values", "Design Parameters"}, tOutputNode);
    tOutputNode = tOperationNode.append_child("Output");
    XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Lower Bounds", "Lower Bound Vector"}, tOutputNode);
    tOutputNode = tOperationNode.append_child("Output");
    XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Upper Bounds", "Upper Bound Vector"}, tOutputNode);
}
// function append_initial_values_operation
/******************************************************************************/

/******************************************************************************/
void append_update_geometry_on_change_operation 
(const std::string &aFirstPlatoMainPerformer, 
 pugi::xml_node& aParentNode)
{
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"Update Geometry on Change", aFirstPlatoMainPerformer}, tOperationNode);
    auto tInputNode = tOperationNode.append_child("Input");
    XMLGen::append_children({"SharedDataName", "ArgumentName"}, {"Design Parameters", "Parameters"}, tInputNode);
}
// function append_update_geometry_on_change_operation
/******************************************************************************/

/******************************************************************************/
void append_initialize_geometry_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"},{"Initialize Geometry", tFirstPlatoMainPerformer}, tOperationNode);
}
// function append_initialize_geometry_operation
/******************************************************************************/

/******************************************************************************/
void append_join_mesh_operation
(const std::string &aFirstPlatoMainPerformer, 
 pugi::xml_node& aParentNode)
{
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"JoinMesh On Change", aFirstPlatoMainPerformer}, tOperationNode);
    auto tInputNode = tOperationNode.append_child("Input");
    XMLGen::append_children({"SharedDataName", "ArgumentName"}, {"Design Parameters", "Parameters"}, tInputNode);
}
// function append_join_mesh_operation
/******************************************************************************/

/******************************************************************************/
void append_rename_mesh_operation
(const std::string &aFirstPlatoMainPerformer, 
 pugi::xml_node& aParentNode)
{
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"RenameMesh On Change", aFirstPlatoMainPerformer}, tOperationNode);
    auto tInputNode = tOperationNode.append_child("Input");
    XMLGen::append_children({"SharedDataName", "ArgumentName"}, {"Design Parameters", "Parameters"}, tInputNode);
}
// function append_rename_mesh_operation
/******************************************************************************/

/******************************************************************************/
void append_tet10_conversion_operation
(const std::string &aFirstPlatoMainPerformer, 
 pugi::xml_node& aParentNode)
{
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"ToTet10 On Change", aFirstPlatoMainPerformer}, tOperationNode);
    auto tInputNode = tOperationNode.append_child("Input");
    XMLGen::append_children({"SharedDataName", "ArgumentName"}, {"Design Parameters", "Parameters"}, tInputNode);
}
// function append_tet10_conversion_operation
/******************************************************************************/

/******************************************************************************/
void append_generate_xtk_model_operation
(const XMLGen::InputData& aMetaData,
 bool aInput,
 bool aOutput,
 pugi::xml_node& aParentNode)
{

    auto tOperation = aParentNode.append_child("Operation");

    std::vector<std::string> tKeys = {"Name","PerformerName"};
    std::vector<std::string> tVals = {"Update Problem",aMetaData.getFirstXTKMainPerformer()};
    XMLGen::append_children(tKeys,tVals, tOperation);

    if(aInput)
    {
        auto tInput = tOperation.append_child("Input");
        XMLGen::append_children({"ArgumentName","SharedDataName"},{"Topology","Topology"}, tInput);
    }
    if(aOutput)
    {
        auto tOutput = tOperation.append_child("Output");
        XMLGen::append_children({"SharedDataName","ArgumentName"},{"Initial Control","Initial Control"}, tOutput);
    }
}
// function append_generate_xtk_model_operation
/******************************************************************************/

/******************************************************************************/
void append_filter_control_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    if(aMetaData.optimization_parameters().filterInEngine() == true)
    {
        std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
        auto tOperationNode = aParentNode.append_child("Operation");
        XMLGen::append_children({"Name", "PerformerName"},{"Filter Control", tFirstPlatoMainPerformer}, tOperationNode);
        auto tInputNode = tOperationNode.append_child("Input");
        XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Field", "Control"}, tInputNode);
        auto tOutputNode = tOperationNode.append_child("Output");
        XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Filtered Field", "Topology"}, tOutputNode);
    }
    else if(aMetaData.optimization_parameters().filter_type() == "helmholtz")
    {
        std::string tHelmholtzPerformer = "";
        for(auto& tService : aMetaData.mPerformerServices)
        {
            if(tService.id() == "helmholtz")
            {
                tHelmholtzPerformer = tService.performer();
                break;
            }
        }
        auto tOperationNode = aParentNode.append_child("Operation");
        XMLGen::append_children({"Name", "PerformerName"},{"Filter Control", tHelmholtzPerformer}, tOperationNode);
        auto tInputNode = tOperationNode.append_child("Input");
        XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Topology", "Control"}, tInputNode);

        // append projection operation
        XMLGen::ValidProjectionKeys tValidProjectionKeys;
        auto tProjectionName = tValidProjectionKeys.value(aMetaData.optimization_parameters().projection_type());
        if(tProjectionName.empty())
        {
            auto tOutputNode = tOperationNode.append_child("Output");
            XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Topology", "Topology"}, tOutputNode);
        }
        else
        {
            auto tOutputNode = tOperationNode.append_child("Output");
            XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Topology", "Filtered Control"}, tOutputNode);

            std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
            auto tProjectionNode = aParentNode.append_child("Operation");
            XMLGen::append_children({"Name", "PerformerName"},{"Project Control", tFirstPlatoMainPerformer}, tProjectionNode);
            auto tInputNode2 = tProjectionNode.append_child("Input");
            XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Field", "Filtered Control"}, tInputNode2);
            tOutputNode = tProjectionNode.append_child("Output");
            XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Filtered Field", "Topology"}, tOutputNode);
        }
    }
}
// function append_filter_control_operation
/******************************************************************************/

/******************************************************************************/
void append_enforce_bounds_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    if(aMetaData.optimization_parameters().enforceBounds())
    {
        std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
        auto tOperationNode = aParentNode.append_child("Operation");
        XMLGen::append_children({"Name", "PerformerName"},{"EnforceBounds", tFirstPlatoMainPerformer}, tOperationNode);
        auto tInputNode = tOperationNode.append_child("Input");
        XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Lower Bound Vector", "Lower Bound Vector"}, tInputNode);
        tInputNode = tOperationNode.append_child("Input");
        XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Upper Bound Vector", "Upper Bound Vector"}, tInputNode);
        tInputNode = tOperationNode.append_child("Input");
        XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Topology", "Topology"}, tInputNode);
        auto tOutputNode = tOperationNode.append_child("Output");
        XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Clamped Topology", "Clamped Topology"}, tOutputNode);
    }
}
// function append_enforce_bounds_operation
/******************************************************************************/

/******************************************************************************/
void append_objective_value_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode,
 bool aCalculatingNormalizationFactor)
{
    pugi::xml_node tParentNode = aParentNode;
    XMLGen::Objective tObjective = aMetaData.objective;

    bool tMultiObjective = (tObjective.criteriaIDs.size() > 1 && tObjective.multi_load_case != "true");

    // If there is more than one sub-objective add an
    // outer "Operation" block so the sub-objectives
    // will be executed in parallel.
    if(tMultiObjective)
        tParentNode = aParentNode.append_child("Operation");
 
    if(tObjective.multi_load_case == "true")
    {
        XMLGen::append_objective_value_operation_for_multi_load_case(aMetaData, tParentNode, aCalculatingNormalizationFactor);
    }
    else
    {
        XMLGen::append_objective_value_operation_for_non_multi_load_case(aMetaData, tParentNode, aCalculatingNormalizationFactor);
    }
}
// function append_objective_value_operation
/******************************************************************************/

/******************************************************************************/
void append_objective_value_operation_for_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode,
 bool aCalculatingNormalizationFactor)
{
    XMLGen::Objective tObjective = aMetaData.objective;
    if(tObjective.criteriaIDs.size() > 0)
    {
        std::string tCriterionID = tObjective.criteriaIDs[0];
        std::string tServiceID = tObjective.serviceIDs[0];
        std::string tCompoundScenarioID = XMLGen::get_compound_scenario_id(tObjective.scenarioIDs);
        XMLGen::Service tService = aMetaData.service(tServiceID); 

        auto tOperationNode = aParentNode.append_child("Operation");
        auto tOperationName = XMLGen::get_objective_value_operation_name(tService);
        XMLGen::append_children({"Name", "PerformerName"}, {tOperationName, tService.performer()}, tOperationNode);
        auto tDesignVariableName = XMLGen::get_design_variable_name(aMetaData);
        if(tDesignVariableName != "")
        {
            auto tOperationInput = tOperationNode.append_child("Input");
            XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Topology", tDesignVariableName}, tOperationInput);
        }
        auto tOperationOutput = tOperationNode.append_child("Output");

        ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tCompoundScenarioID);
        auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);
        auto tOutputSharedData = std::string("Criterion Value - ") + tIdentifierString;
        if(aCalculatingNormalizationFactor)
        {
            tOutputSharedData = std::string("Initial ") + tOutputSharedData;
        }
        auto tOperationOutputName = XMLGen::get_objective_value_operation_output_name(tService);
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, {tOperationOutputName, tOutputSharedData}, tOperationOutput);
    }
}
// function append_objective_value_operation_for_multi_load_case
/******************************************************************************/
  
/******************************************************************************/
void append_objective_value_operation_for_non_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode,
 bool aCalculatingNormalizationFactor)
{
    XMLGen::Objective tObjective = aMetaData.objective;
    for (size_t i=0; i<tObjective.criteriaIDs.size(); ++i)
    {
        std::string tCriterionID = tObjective.criteriaIDs[i];
        std::string tServiceID = tObjective.serviceIDs[i];
        std::string tScenarioID = tObjective.scenarioIDs[i];
        XMLGen::Service tService = aMetaData.service(tServiceID); 

        auto tOperationNode = aParentNode.append_child("Operation");
        auto tOperationName = XMLGen::get_objective_value_operation_name(tService);
        XMLGen::append_children({"Name", "PerformerName"}, {tOperationName, tService.performer()}, tOperationNode);
        auto tDesignVariableName = XMLGen::get_design_variable_name(aMetaData);
        if(tDesignVariableName != "")
        {
            auto tOperationInput = tOperationNode.append_child("Input");
            XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Topology", tDesignVariableName}, tOperationInput);
        }
        auto tOperationOutput = tOperationNode.append_child("Output");

        ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
        auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);
        auto tOutputSharedData = std::string("Criterion Value - ") + tIdentifierString;
        if(aCalculatingNormalizationFactor)
        {
            tOutputSharedData = std::string("Initial ") + tOutputSharedData;
        }
        auto tOperationOutputName = XMLGen::get_objective_value_operation_output_name(tService);
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, {tOperationOutputName, tOutputSharedData}, tOperationOutput);
    }
}
// function append_objective_value_operation_for_non_multi_load_case
/******************************************************************************/

/******************************************************************************/
void append_lower_bound_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"Compute Lower Bounds", tFirstPlatoMainPerformer}, tOperationNode);
    auto tInputNode = tOperationNode.append_child("Input");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Lower Bound Value", "Lower Bound Value"}, tInputNode);
    auto tOutputNode = tOperationNode.append_child("Output");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Lower Bound Vector", "Lower Bound Vector"}, tOutputNode);
}
// function append_lower_bound_operation
/******************************************************************************/

/******************************************************************************/
void append_upper_bound_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"Compute Upper Bounds", tFirstPlatoMainPerformer}, tOperationNode);
    auto tInputNode = tOperationNode.append_child("Input");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Upper Bound Value", "Upper Bound Value"}, tInputNode);
    auto tOutputNode = tOperationNode.append_child("Output");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Upper Bound Vector", "Upper Bound Vector"}, tOutputNode);
}
// function append_upper_bound_operation
/******************************************************************************/

/******************************************************************************/
void append_write_output_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    if(aMetaData.mOutputMetaData[0].isOutputDisabled())
    {
        return;
    }
    XMLGen::append_visualization_operation(aMetaData, aParentNode);
    XMLGen::append_write_output_operation_nondeterministic_usecase(aMetaData, aParentNode);
    XMLGen::append_write_output_operation_deterministic_usecase(aMetaData, aParentNode);
}
// function append_write_output_operation
/******************************************************************************/

/******************************************************************************/
void append_visualization_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
 {
     if (aMetaData.mOutputMetaData[0].randomIDs().empty())
     {
         for (auto &tOutputMetadata : aMetaData.mOutputMetaData)
         {
             if (!tOutputMetadata.value("native_service_output").empty() && tOutputMetadata.value("native_service_output") == "true")
             {
                 auto tServiceID = tOutputMetadata.serviceID();
                 auto tPerformerName = aMetaData.service(tServiceID).performer();
                 auto tOperation = aParentNode.append_child("Operation");
                 XMLGen::append_children({"Name", "PerformerName"}, {"Visualization", tPerformerName}, tOperation);
             }
         }
     }
 }
 // function append_visualization_operation
/******************************************************************************/

/******************************************************************************/
void append_write_output_operation_nondeterministic_usecase
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    const XMLGen::Output &tOutputMetadata = aMetaData.mOutputMetaData[0];
    if(tOutputMetadata.randomIDs().empty())
    {
        return;
    }

    auto tServiceID = tOutputMetadata.serviceID();
    auto &tService = aMetaData.service(tServiceID);
    if(tService.code() == "plato_analyze")
    {
        auto tForNode = aParentNode.append_child("For");
        XMLGen::append_attributes( { "var", "in" }, { "PerformerSampleIndex", "PerformerSamples" }, tForNode);
        auto tOperationNode = tForNode.append_child("Operation");
        tForNode = tOperationNode.append_child("For");
        XMLGen::append_attributes( { "var", "in" }, { "PerformerIndex", "Performers" }, tForNode);

        tOperationNode = tForNode.append_child("Operation");
        tServiceID = tOutputMetadata.serviceID();
        auto tBasePerformerName = aMetaData.service(tServiceID).performer();
        auto tPerformerName = tBasePerformerName + "_{PerformerIndex}";
        XMLGen::append_children( { "Name", "PerformerName" }, { "Write Output", tPerformerName }, tOperationNode);
        XMLGen::append_output_qoi_to_nondeterministic_write_output_operation(aMetaData, tOperationNode);
    }
}
// function append_write_output_operation_nondeterministic_usecase
/******************************************************************************/

/******************************************************************************/
void append_write_output_operation_deterministic_usecase
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    int tNumPlatoAnalyzeOutputs = 0;
    for(auto &tCurOutput : aMetaData.mOutputMetaData)
    {
        auto tServiceID = tCurOutput.serviceID();
        auto &tService = aMetaData.service(tServiceID);
        if(tService.code() == "plato_analyze")
        {
            tNumPlatoAnalyzeOutputs++;
        }
    }
    pugi::xml_node tCurParentNode = aParentNode;

    for(auto &tOutputMetadata : aMetaData.mOutputMetaData)
    {
        auto tServiceID = tOutputMetadata.serviceID();
        auto &tService = aMetaData.service(tServiceID);
        if(tService.code() == "plato_analyze")
        {
            if(!tOutputMetadata.deterministicIDs().empty())
            {
                auto tOperationNode = tCurParentNode.append_child("Operation");
                auto tPerformerName = aMetaData.service(tServiceID).performer();
                XMLGen::append_children( { "Name", "PerformerName" }, { "Write Output", tPerformerName }, tOperationNode);
                if(aMetaData.optimization_parameters().filterInEngine() == false &&
                   aMetaData.getFirstPlatoAnalyzePerformer() == tPerformerName)
                {
                    auto tTopologyNode = tOperationNode.append_child("Output");
                    XMLGen::append_children( { "ArgumentName", "SharedDataName" }, { "Topology", "Topology"}, tTopologyNode);
                }

                XMLGen::append_output_qoi_to_deterministic_write_output_operation(aMetaData, tOutputMetadata, tOperationNode);
            }
        }
    }
}
// function append_write_output_operation_deterministic_usecase
/******************************************************************************/

/******************************************************************************/
void append_compute_qoi_statistics_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    auto tQoIIDs = aMetaData.mOutputMetaData[0].randomIDs();
    for (auto &tQoIID : tQoIIDs)
    {
        auto tOperationNode = aParentNode.append_child("Operation");
        auto tOperationName = "compute " + tQoIID + " statistics";
        XMLGen::append_children({"Name", "PerformerName"}, {tOperationName, tFirstPlatoMainPerformer}, tOperationNode);

        auto tForNode = tOperationNode.append_child("For");
        XMLGen::append_attributes({"var", "in"}, {"PerformerIndex", "Performers"}, tForNode);
        tForNode = tForNode.append_child("For");
        XMLGen::append_attributes({"var", "in"}, {"PerformerSampleIndex", "PerformerSamples"}, tForNode);

        auto tInputNode = tForNode.append_child("Input");
        auto tDataName = tQoIID + " {PerformerIndex*NumSamplesPerPerformer+PerformerSampleIndex}";
        XMLGen::append_children({ "ArgumentName", "SharedDataName" }, { tDataName, tDataName }, tInputNode);

        auto tMeanName = tQoIID + " mean";
        auto tOutputNode = tOperationNode.append_child("Output");
        XMLGen::append_children({ "ArgumentName", "SharedDataName" }, { tMeanName, tMeanName }, tOutputNode);

        auto tStdDevName = tQoIID + " standard deviation";
        tOutputNode = tOperationNode.append_child("Output");
        XMLGen::append_children({ "ArgumentName", "SharedDataName" }, { tStdDevName, tStdDevName }, tOutputNode);
    }
}
// function append_compute_qoi_statistics_operation
/******************************************************************************/

/******************************************************************************/
void append_output_operation_to_interface_file
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    auto tOperation = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"},{"PlatoMainOutput", tFirstPlatoMainPerformer}, tOperation);
    XMLGen::append_default_qoi_to_output_operation_in_interface_file(aMetaData, tOperation);
    XMLGen::append_deterministic_qoi_to_output_operation_in_interface_file(aMetaData, tOperation);
    XMLGen::append_nondeterministic_qoi_to_output_operation_in_interface_file(aMetaData, tOperation);
    XMLGen::append_qoi_statistics_to_output_operation_in_interface_file(aMetaData, tOperation);
}
// function append_output_operation_to_interface_file
/******************************************************************************/

/******************************************************************************/
void append_copy_value_operation
(const std::string &aPlatoMainPerformer,
 const std::string &aInputSharedDataName,
 const std::string &aOutputSharedDataName,
 pugi::xml_node &aParentNode)
{
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"Copy Value", aPlatoMainPerformer}, tOperationNode);
    auto tInputNode = tOperationNode.append_child("Input");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"InputValue", aInputSharedDataName}, tInputNode);
    auto tOutputNode = tOperationNode.append_child("Output");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"OutputValue", aOutputSharedDataName}, tOutputNode);
}
// function append_copy_value_operation
/******************************************************************************/

/******************************************************************************/
void append_negate_value_operation
(const std::string &aPlatoMainPerformer,
 const std::string &aSharedDataName,
 pugi::xml_node &aParentNode)
{
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"Negate Value", aPlatoMainPerformer}, tOperationNode);
    auto tInputNode = tOperationNode.append_child("Input");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"InputValue", aSharedDataName}, tInputNode);
    auto tOutputNode = tOperationNode.append_child("Output");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"OutputValue", aSharedDataName}, tOutputNode);
}
// function append_negate_value_operation
/******************************************************************************/

/******************************************************************************/
void append_negate_field_operation
(const std::string &aPlatoMainPerformer,
 const std::string &aSharedDataName,
 pugi::xml_node &aParentNode)
{
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"Negate Field", aPlatoMainPerformer}, tOperationNode);
    auto tInputNode = tOperationNode.append_child("Input");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"InputField", aSharedDataName}, tInputNode);
    auto tOutputNode = tOperationNode.append_child("Output");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"OutputField", aSharedDataName}, tOutputNode);
}
// function append_negate_value_operation
/******************************************************************************/

/******************************************************************************/
void append_reinitialize_on_change_operation 
(const std::string &aPerformer, 
 pugi::xml_node& aParentNode)
{
    if (aPerformer.find("sierra_sd") != std::string::npos) {
        return;
    }
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"Reinitialize on Change", aPerformer}, tOperationNode);
    auto tInputNode = tOperationNode.append_child("Input");
    XMLGen::append_children({"SharedDataName", "ArgumentName"}, {"Design Parameters", "Parameters"}, tInputNode);
}
// function append_reinitialize_on_change_operation
/******************************************************************************/

/******************************************************************************/
void append_helmholtz_filter_criterion_gradient_operation
(const XMLGen::InputData& aMetaData,
 const std::string& aInputSharedDataName,
 const std::string& aOutputSharedDataName,
 pugi::xml_node& aParentNode)
{
    auto tFilterInputName = aInputSharedDataName;

    // append projection operation
    XMLGen::ValidProjectionKeys tValidProjectionKeys;
    auto tProjectionName = tValidProjectionKeys.value(aMetaData.optimization_parameters().projection_type());
    if(!tProjectionName.empty())
    {
        std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
        std::string tProjectionOutputName = std::string("Projected ") + aOutputSharedDataName;

        auto tProjectionNode = aParentNode.append_child("Operation");
        XMLGen::append_children({"Name", "PerformerName"},{"Project Gradient", tFirstPlatoMainPerformer}, tProjectionNode);
        auto tInputNode = tProjectionNode.append_child("Input");
        XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Field", "Filtered Control"}, tInputNode);
        tInputNode = tProjectionNode.append_child("Input");
        XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Gradient", aInputSharedDataName}, tInputNode);
        auto tOutputNode = tProjectionNode.append_child("Output");
        XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Filtered Gradient", tProjectionOutputName}, tOutputNode);
        tFilterInputName = tProjectionOutputName;
    }

    std::string tHelmholtzPerformer = "";
    for(auto& tService : aMetaData.mPerformerServices)
    {
        if(tService.id() == "helmholtz")
        {
            tHelmholtzPerformer = tService.performer();
            break;
        }
    }
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"},{"Filter Gradient", tHelmholtzPerformer}, tOperationNode);
    auto tInputNode = tOperationNode.append_child("Input");
    XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Topology", tFilterInputName}, tInputNode);
    auto tOutputNode = tOperationNode.append_child("Output");
    XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Topology", aOutputSharedDataName}, tOutputNode);
}
// function append_helmholtz_filter_criterion_gradient_operation
/******************************************************************************/

/******************************************************************************/
void append_copy_field_operation
(const std::string &aPlatoMainPerformer,
 const std::string &aInputSharedDataName,
 const std::string &aOutputSharedDataName,
 pugi::xml_node &aParentNode)
{
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"Copy Field", aPlatoMainPerformer}, tOperationNode);
    auto tInputNode = tOperationNode.append_child("Input");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"InputField", aInputSharedDataName}, tInputNode);
    auto tOutputNode = tOperationNode.append_child("Output");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"OutputField", aOutputSharedDataName}, tOutputNode);
}
// function append_copy_field_operation
/******************************************************************************/

/******************************************************************************/
void append_filter_criterion_gradient_operation
(const XMLGen::InputData& aMetaData,
 const std::string& aInputSharedDataName,
 const std::string& aOutputSharedDataName,
 pugi::xml_node& aParentNode)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"},{"Filter Gradient", tFirstPlatoMainPerformer}, tOperationNode);
    auto tInputNode = tOperationNode.append_child("Input");
    XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Field", "Control"}, tInputNode);
    tInputNode = tOperationNode.append_child("Input");
    XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Gradient", aInputSharedDataName}, tInputNode);
    auto tOutputNode = tOperationNode.append_child("Output");
    XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Filtered Gradient", aOutputSharedDataName}, tOutputNode);
}
// function append_filter_criterion_gradient_operation
/******************************************************************************/

/******************************************************************************/
void append_compute_constraint_gradient_operation
(const XMLGen::Constraint &aConstraint,
 const std::string &aPerformer,
 pugi::xml_node &aParentNode)
{
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"Compute Constraint Gradient " + aConstraint.id(), aPerformer}, tOperationNode);
}
// function append_compute_constraint_gradient_operation
/******************************************************************************/

/******************************************************************************/
void append_compute_shape_sensitivity_on_change_operation
(pugi::xml_node &aParentNode)
{
    auto tForNode = aParentNode.append_child("For");
    XMLGen::append_attributes({"var", "in"}, {"I", "Parameters"}, tForNode);
    auto tOperationNode = tForNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"Compute Parameter Sensitivity on Change", "plato_esp_{I}"}, tOperationNode);
    auto tParamNode = tOperationNode.append_child("Parameter");
    XMLGen::append_children({"ArgumentName", "ArgumentValue"}, {"Parameter Index", "{I-1}"}, tParamNode);
    auto tInputNode = tOperationNode.append_child("Input");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Parameters", "Design Parameters"}, tInputNode);
    auto tOutputNode = tOperationNode.append_child("Output");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Parameter Sensitivity", "Parameter Sensitivity {I}"}, tOutputNode);
}
// function append_compute_shape_sensitivity_on_change_operation
/******************************************************************************/

/******************************************************************************/
void append_compute_constraint_sensitivity_operation
(const std::string &aPerformer,
 const std::string &aSharedDataName,
 pugi::xml_node &aParentNode)
{
    auto tOperationNode = aParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"Compute Constraint Sensitivity", aPerformer}, tOperationNode);
    auto tForNode = tOperationNode.append_child("For");
    XMLGen::append_attributes({"var", "in"}, {"I", "Parameters"}, tForNode);
    auto tInputNode = tForNode.append_child("Input");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Parameter Sensitivity {I}", "Parameter Sensitivity {I}"}, tInputNode);
    auto tOutputNode = tOperationNode.append_child("Output");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Criterion Sensitivity", aSharedDataName}, tOutputNode);
}
// function append_compute_constraint_sensitivity_operation
/******************************************************************************/

/******************************************************************************/
void append_aggregate_objective_value_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode)
{
    pugi::xml_node tParentNode = aParentNode;
    XMLGen::Objective tObjective = aMetaData.objective;
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();

    auto tOperationNode = tParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"Aggregate Data", tFirstPlatoMainPerformer}, tOperationNode);

    if(tObjective.multi_load_case == "true")
    {
        XMLGen::append_aggregate_objective_value_operation_for_multi_load_case(aMetaData, tOperationNode);
    }
    else
    {
        XMLGen::append_aggregate_objective_value_operation_for_non_multi_load_case(aMetaData, tOperationNode);
    }

    auto tOperationOutput = tOperationNode.append_child("Output");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Value", "Objective Value"}, tOperationOutput);
}
// function append_aggregate_objective_value_operation
/******************************************************************************/

/******************************************************************************/
void append_aggregate_objective_value_operation_for_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode)
{
    XMLGen::Objective tObjective = aMetaData.objective;
    if(tObjective.criteriaIDs.size() > 0)
    {
        std::string tCriterionID = tObjective.criteriaIDs[0];
        std::string tServiceID = tObjective.serviceIDs[0];
        std::string tCompoundScenarioID = get_compound_scenario_id(tObjective.scenarioIDs);
        ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tCompoundScenarioID);
        auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);
        auto tOperationInput = aParentNode.append_child("Input");
        auto tOutputSharedData = std::string("Criterion Value - ") + tIdentifierString;
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Value 1", tOutputSharedData}, tOperationInput);
        if(aMetaData.optimization_parameters().normalizeInAggregator())
        {
            tOperationInput = aParentNode.append_child("Input");
            tOutputSharedData = std::string("Initial ") + tOutputSharedData;
            XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Normal 1", tOutputSharedData}, tOperationInput);
        }
    }
}
// function append_aggregate_objective_value_operation_for_multi_load_case
/******************************************************************************/

/******************************************************************************/
void append_aggregate_objective_value_operation_for_non_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode)
{
    XMLGen::Objective tObjective = aMetaData.objective;
    for (size_t i=0; i<tObjective.criteriaIDs.size(); ++i)
    {
        std::string tCriterionID = tObjective.criteriaIDs[i];
        std::string tServiceID = tObjective.serviceIDs[i];
        std::string tScenarioID = tObjective.scenarioIDs[i];
        ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
        auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);
        auto tOperationInput = aParentNode.append_child("Input");
        auto tArgName = std::string("Value ") + std::to_string(i+1);
        auto tOutputSharedData = std::string("Criterion Value - ") + tIdentifierString;
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, {tArgName, tOutputSharedData}, tOperationInput);
        if(aMetaData.optimization_parameters().normalizeInAggregator())
        {
            tOperationInput = aParentNode.append_child("Input");
            tArgName = std::string("Normal ") + std::to_string(i+1);
            tOutputSharedData = std::string("Initial ") + tOutputSharedData;
            XMLGen::append_children({"ArgumentName", "SharedDataName"}, {tArgName, tOutputSharedData}, tOperationInput);
        }
    }
}
// function append_aggregate_objective_value_operation_for_non_multi_load_case
/******************************************************************************/

/******************************************************************************/
void append_compute_objective_sensitivity_operation
(const std::string &aPerformer,
 const std::string &aSharedDataName,
 pugi::xml_node &aParentNode)
{
    auto tOperationNode = aParentNode.append_child("Operation");
    if(aPerformer.find("sierra_sd") != std::string::npos) 
    {
        XMLGen::append_children({"Name", "PerformerName"}, {"Compute Objective Gradient wrt CAD Parameters", aPerformer}, tOperationNode);
    }
    else {
        XMLGen::append_children({"Name", "PerformerName"}, {"Compute Objective Sensitivity", aPerformer}, tOperationNode);
    }
    auto tForNode = tOperationNode.append_child("For");
    XMLGen::append_attributes({"var", "in"}, {"I", "Parameters"}, tForNode);
    auto tInputNode = tForNode.append_child("Input");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Parameter Sensitivity {I}", "Parameter Sensitivity {I}"}, tInputNode);
    auto tOutputNode = tOperationNode.append_child("Output");
    if(aPerformer.find("sierra_sd") != std::string::npos) 
    {
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Internal Energy Gradient", aSharedDataName}, tOutputNode);
    }
    else
    {
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Criterion Sensitivity", aSharedDataName}, tOutputNode);
    }
}
// function append_compute_objective_sensitivity_operation
/******************************************************************************/

/******************************************************************************/
void append_aggregate_objective_gradient_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode)
{
    pugi::xml_node tParentNode = aParentNode;
    XMLGen::Objective tObjective = aMetaData.objective;
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();

    std::string tType;
    if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
        tType = "Value";
    else
        tType = "Field";

    auto tOperationNode = tParentNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"Aggregate Data", tFirstPlatoMainPerformer}, tOperationNode);
    
    if(tObjective.multi_load_case == "true")
    {
        XMLGen::append_aggregate_objective_gradient_operation_for_multi_load_case(aMetaData, tOperationNode, tType);
    }
    else
    {
        XMLGen::append_aggregate_objective_gradient_operation_for_non_multi_load_case(aMetaData, tOperationNode, tType);
    }
}
// function append_aggregate_objective_gradient_operation
/******************************************************************************/

/******************************************************************************/
void append_aggregate_objective_gradient_operation_for_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode,
 std::string &aType)
{
    XMLGen::Objective tObjective = aMetaData.objective;
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();

    if(tObjective.criteriaIDs.size() > 0)
    {
        std::string tCriterionID = tObjective.criteriaIDs[0];
        std::string tServiceID = tObjective.serviceIDs[0];
        std::string tCompoundScenarioID = XMLGen::get_compound_scenario_id(tObjective.scenarioIDs);
        ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tCompoundScenarioID);
        auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);

        auto tOperationInput = aParentNode.append_child("Input");
        std::string tArgName = aType + " 1";
        auto tInputSharedData = std::string("Criterion Gradient - ") + tIdentifierString;
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, {tArgName, tInputSharedData}, tOperationInput);
        if(aMetaData.optimization_parameters().normalizeInAggregator())
        {
            tOperationInput = aParentNode.append_child("Input");
            tArgName = "Normal 1";
            tInputSharedData = std::string("Initial Criterion Value - ") + tIdentifierString;
            XMLGen::append_children({"ArgumentName", "SharedDataName"}, {tArgName, tInputSharedData}, tOperationInput);
        }
    }

    auto tOperationOutput = aParentNode.append_child("Output");
    if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY)
    {
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, {aType, "Aggregate Gradient"}, tOperationOutput);
    }
    else if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, {aType, "Objective Gradient"}, tOperationOutput);
    }
}
// function append_aggregate_objective_gradient_operation_for_multi_load_case
/******************************************************************************/

/******************************************************************************/
void append_aggregate_objective_gradient_operation_for_non_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode,
 std::string &aType)
{
    XMLGen::Objective tObjective = aMetaData.objective;
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();

    for (size_t i=0; i<tObjective.criteriaIDs.size(); ++i)
    {
        std::string tCriterionID = tObjective.criteriaIDs[i];
        std::string tServiceID = tObjective.serviceIDs[i];
        std::string tScenarioID = tObjective.scenarioIDs[i];
        ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
        auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);

        auto tOperationInput = aParentNode.append_child("Input");
        auto tArgName = aType + " " + std::to_string(i+1);
        auto tInputSharedData = std::string("Criterion Gradient - ") + tIdentifierString;
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, {tArgName, tInputSharedData}, tOperationInput);
        if(aMetaData.optimization_parameters().normalizeInAggregator())
        {
            tOperationInput = aParentNode.append_child("Input");
            tArgName = std::string("Normal ") + std::to_string(i+1);
            tInputSharedData = std::string("Initial Criterion Value - ") + tIdentifierString;
            XMLGen::append_children({"ArgumentName", "SharedDataName"}, {tArgName, tInputSharedData}, tOperationInput);
        }
    }

    auto tOperationOutput = aParentNode.append_child("Output");
    if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY)
    {
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, {aType, "Aggregate Gradient"}, tOperationOutput);
    }
    else if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, {aType, "Objective Gradient"}, tOperationOutput);
    }
}
// function append_aggregate_objective_gradient_operation_for_non_multi_load_case
/******************************************************************************/

/******************************************************************************/
void append_objective_gradient_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode)
{
    XMLGen::Objective tObjective = aMetaData.objective;

    if(tObjective.multi_load_case == "true")
    {
        XMLGen::append_objective_gradient_operation_for_multi_load_case(aMetaData, aParentNode);
    }
    else
    {
        XMLGen::append_objective_gradient_operation_for_non_multi_load_case(aMetaData, aParentNode);
    }
}
// function append_objective_gradient_operation
/******************************************************************************/

/******************************************************************************/
void append_objective_gradient_operation_for_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode)
{
    XMLGen::Objective tObjective = aMetaData.objective;
    auto tDesignVariableName = XMLGen::get_design_variable_name(aMetaData);
    if(tObjective.scenarioIDs.size() > 0)
    {
        std::string tCriterionID = tObjective.criteriaIDs[0];
        std::string tServiceID = tObjective.serviceIDs[0];
        std::string tScenarioID = XMLGen::get_compound_scenario_id(aMetaData.objective.scenarioIDs);
        ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
        auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);
        XMLGen::Service tService = aMetaData.service(tServiceID); 
        auto tOperationNode = aParentNode.append_child("Operation");
        auto tOperationName = XMLGen::get_objective_gradient_operation_name(tService);
        XMLGen::append_children({"Name", "PerformerName"}, {tOperationName, tService.performer()}, tOperationNode);
        if(tDesignVariableName != "")
        {
            auto tOperationInput = tOperationNode.append_child("Input");
            XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Topology", tDesignVariableName}, tOperationInput);
            auto tOperationOutput = tOperationNode.append_child("Output");
            auto tOutputSharedData = std::string("Criterion Gradient - ") + tIdentifierString;
            auto tOperationOutputName = XMLGen::get_objective_gradient_operation_output_name(tService);
            XMLGen::append_children({"ArgumentName", "SharedDataName"}, {tOperationOutputName, tOutputSharedData}, tOperationOutput);
        }
    }
}
// function append_objective_gradient_operation_for_multi_load_case
/******************************************************************************/

/******************************************************************************/
void append_objective_gradient_operation_for_non_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node &aParentNode)
{
    XMLGen::Objective tObjective = aMetaData.objective;
    auto tDesignVariableName = XMLGen::get_design_variable_name(aMetaData);
    for (size_t i=0; i<tObjective.criteriaIDs.size(); ++i)
    {
        std::string tCriterionID = tObjective.criteriaIDs[i];
        std::string tServiceID = tObjective.serviceIDs[i];
        std::string tScenarioID = tObjective.scenarioIDs[i];
        ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
        auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);
        XMLGen::Service tService = aMetaData.service(tObjective.serviceIDs[i]); 
        auto tOperationNode = aParentNode.append_child("Operation");
        auto tOperationName = XMLGen::get_objective_gradient_operation_name(tService);
        XMLGen::append_children({"Name", "PerformerName"}, {tOperationName, tService.performer()}, tOperationNode);
        if(tDesignVariableName != "")
        {
            auto tOperationInput = tOperationNode.append_child("Input");
            XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"Topology", tDesignVariableName}, tOperationInput);
            auto tOperationOutput = tOperationNode.append_child("Output");
            auto tOutputSharedData = std::string("Criterion Gradient - ") + tIdentifierString;
            auto tOperationOutputName = XMLGen::get_objective_gradient_operation_output_name(tService);
            XMLGen::append_children({"ArgumentName", "SharedDataName"}, {tOperationOutputName, tOutputSharedData}, tOperationOutput);
        }
    }
}
// function append_objective_gradient_operation_for_non_multi_load_case
/******************************************************************************/

}
// namespace XMLGen
