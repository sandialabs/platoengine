/*
 * XMLGeneratorStagesUtilities.cpp
 *
 *  Created on: Jan 14, 2022
 */
 
#include "XMLG_Macros.hpp"
#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorParserUtilities.hpp"
#include "XMLGeneratorStagesUtilities.hpp"
#include "XMLGeneratorServiceUtilities.hpp"
#include "XMLGeneratorSierraSDUtilities.hpp"
#include "XMLGeneratorSharedDataUtilities.hpp"
#include "XMLGeneratorStagesOperationsUtilities.hpp"
#include "XMLGeneratorSierraSDInputDeckUtilities.hpp"
#include "XMLGeneratorRandomInterfaceFileUtilities.hpp"
#include "XMLGeneratorPlatoMainConstraintValueOperationInterface.hpp"
#include "XMLGeneratorPlatoMainConstraintGradientOperationInterface.hpp"

namespace XMLGen
{

/******************************************************************************/
void append_design_volume_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY)
    {
        auto tStageNode = aDocument.append_child("Stage");
        XMLGen::append_children({"Name"}, {"Design Volume"}, tStageNode);
        XMLGen::append_design_volume_operation(aMetaData, tStageNode);
        auto tOutputNode = tStageNode.append_child("Output");
        XMLGen::append_children({"SharedDataName"}, {"Design Volume"}, tOutputNode);
    }
}
// function append_design_volume_stage
/******************************************************************************/

/******************************************************************************/
void append_initial_guess_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY && aMetaData.optimization_parameters().discretization() == "density")
    {
        auto tStageNode = aDocument.append_child("Stage");
        XMLGen::append_children({"Name"},{"Initial Guess"}, tStageNode);
        XMLGen::append_initial_field_operation(aMetaData, tStageNode);
        XMLGen::append_compute_normalization_factor_operation(aMetaData, tStageNode);
        auto tOutputNode = tStageNode.append_child("Output");
        XMLGen::append_children({"SharedDataName"},{"Control"}, tOutputNode);
    }
    else if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        auto tStageNode = aDocument.append_child("Stage");
        const std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();

        XMLGen::append_children({"Name"},{"Initialize Design Parameters"}, tStageNode);
        XMLGen::append_initial_values_operation(aMetaData, tStageNode);

        if (XMLGen::do_tet10_conversion(aMetaData) || XMLGen::have_auxiliary_mesh(aMetaData))
        {
            XMLGen::append_update_geometry_on_change_operation(tFirstPlatoMainPerformer, tStageNode);
        }
        else
        {
            XMLGen::append_initialize_geometry_operation(aMetaData, tStageNode);
        }

        if (XMLGen::have_auxiliary_mesh(aMetaData))
        {
            XMLGen::append_join_mesh_operation(tFirstPlatoMainPerformer, tStageNode);
            XMLGen::append_rename_mesh_operation(tFirstPlatoMainPerformer, tStageNode);
        }

        if (XMLGen::do_tet10_conversion(aMetaData))
        {
            XMLGen::append_tet10_conversion_operation(tFirstPlatoMainPerformer, tStageNode);
        }

        XMLGen::append_compute_normalization_factor_operation(aMetaData, tStageNode);
        auto tOutputNode = tStageNode.append_child("Output");
        XMLGen::append_children({"SharedDataName"},{"Design Parameters"}, tOutputNode);
    }
    else if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY && aMetaData.optimization_parameters().discretization() == "levelset")
    {
        const std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
        const std::string tFirstXTKPerformer = aMetaData.getFirstXTKMainPerformer();
        auto tStageNode = aDocument.append_child("Stage");
        XMLGen::append_children({"Name"},{"Initial Guess"}, tStageNode);

        XMLGen::append_generate_xtk_model_operation(aMetaData, false, true,tStageNode);

        auto tOutputNode = tStageNode.append_child("Output");
        XMLGen::append_children({"SharedDataName"},{"Initial Control"}, tOutputNode);
    }
    else
    {
        THROWERR(std::string("Invalid optimization and discretization combination. Supported types are topology+ density, topology+levelset, shape."  
            + aMetaData.optimization_parameters().optimization_type() + aMetaData.optimization_parameters().discretization() +"is not supported."));
    }
}
// function append_initial_guess_stage
/******************************************************************************/

/******************************************************************************/
void append_lower_bound_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    auto tStageNode = aDocument.append_child("Stage");
    XMLGen::append_children({"Name"}, {"Set Lower Bounds"}, tStageNode);
    if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY && aMetaData.optimization_parameters().discretization() == "density")
    {
        auto tInputNode = tStageNode.append_child("Input");
        XMLGen::append_children({"SharedDataName"}, {"Lower Bound Value"}, tInputNode);
        XMLGen::append_lower_bound_operation(aMetaData, tStageNode);
    }
    else if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY && aMetaData.optimization_parameters().discretization() == "levelset")
    {
        auto tOperation = tStageNode.append_child("Operation");
        XMLGen::append_children({"Name","PerformerName"}, {"Compute Lower Bounds", aMetaData.getFirstXTKMainPerformer()}, tOperation);

        auto tOutput = tOperation.append_child("Output");
        XMLGen::append_children({"SharedDataName","ArgumentName"}, {"Lower Bound Vector", "Lower Bound Vector"}, tOutput);
    }
    else if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        auto tOperationNode = tStageNode.append_child("Operation");
        XMLGen::append_children({"Name", "PerformerName"},{"Compute Lower Bounds", aMetaData.getFirstPlatoMainPerformer()}, tOperationNode);
        auto tOutputNode = tOperationNode.append_child("Output");
        XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Lower Bounds", "Lower Bound Vector"}, tOutputNode);
    }
    auto tOutputNode = tStageNode.append_child("Output");
    XMLGen::append_children({"SharedDataName"}, {"Lower Bound Vector"}, tOutputNode);
}
// function append_lower_bound_stage
/******************************************************************************/

/******************************************************************************/
void append_upper_bound_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    auto tStageNode = aDocument.append_child("Stage");
    XMLGen::append_children({"Name"}, {"Set Upper Bounds"}, tStageNode);
    if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY && aMetaData.optimization_parameters().discretization() == "density")
    {
        auto tInputNode = tStageNode.append_child("Input");
        XMLGen::append_children({"SharedDataName"}, {"Upper Bound Value"}, tInputNode);
        XMLGen::append_upper_bound_operation(aMetaData, tStageNode);
    }
    else if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY && aMetaData.optimization_parameters().discretization() == "levelset")
    {
        auto tOperation = tStageNode.append_child("Operation");
        XMLGen::append_children({"Name","PerformerName"}, {"Compute Upper Bounds", aMetaData.getFirstXTKMainPerformer()}, tOperation);

        auto tOutput = tOperation.append_child("Output");
        XMLGen::append_children({"SharedDataName","ArgumentName"}, {"Upper Bound Vector", "Upper Bound Vector"}, tOutput);
    }
    else if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        auto tOperationNode = tStageNode.append_child("Operation");
        XMLGen::append_children({"Name", "PerformerName"},{"Compute Upper Bounds", aMetaData.getFirstPlatoMainPerformer()}, tOperationNode);
        auto tOutputNode = tOperationNode.append_child("Output");
        XMLGen::append_children({"ArgumentName", "SharedDataName"},{"Upper Bounds", "Upper Bound Vector"}, tOutputNode);
    }
    auto tOutputNode = tStageNode.append_child("Output");
    XMLGen::append_children({"SharedDataName"}, {"Upper Bound Vector"}, tOutputNode);
}
// function append_upper_bound_stage
/******************************************************************************/

/******************************************************************************/
void append_plato_main_output_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    if (!aMetaData.mOutputMetaData.empty())
    {
        if (aMetaData.mOutputMetaData[0].isOutputDisabled())
        {
            return;
        }
        if (aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY)
        {
            auto tOutputStage = aDocument.append_child("Stage");
            XMLGen::append_children({"Name"}, {"Output To File"}, tOutputStage);
            XMLGen::append_enforce_bounds_operation(aMetaData, tOutputStage);
            XMLGen::append_write_output_operation(aMetaData, tOutputStage);
            XMLGen::append_compute_qoi_statistics_operation(aMetaData, tOutputStage);
            XMLGen::append_output_operation_to_interface_file(aMetaData, tOutputStage); // TODO: ME QUEDE AQUI
        }
        else if (aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
        {
            std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
            auto tOutputStage = aDocument.append_child("Stage");
            XMLGen::append_children({"Name"}, {"Output To File"}, tOutputStage);
            auto tOperation = tOutputStage.append_child("Operation");
            XMLGen::append_children({"Name", "PerformerName"}, {"CSMMeshOutput", tFirstPlatoMainPerformer}, tOperation);
        }
    }
}
// function append_plato_main_output_stage
/******************************************************************************/

/******************************************************************************/
void append_cache_state_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    int tNumCacheStates = XMLGen::num_cache_states(aMetaData.services());
    if(tNumCacheStates > 0)
    {
        auto tStageNode = aDocument.append_child("Stage");
        XMLGen::append_children( { "Name" }, { "Cache State" }, tStageNode);
        auto tParentNode = tStageNode;
        if(tNumCacheStates > 1)
        {
            tParentNode = tStageNode.append_child("Operation");
        }
        for(size_t w=0; w<aMetaData.objective.serviceIDs.size(); ++w)
        {
            auto tServiceID = aMetaData.objective.serviceIDs[w];
            auto tCriterionID = aMetaData.objective.criteriaIDs[w];
            auto tCriterion = aMetaData.criterion(tCriterionID);
            auto tService = aMetaData.service(tServiceID);
            if (!tService.cacheState())
            {
                continue;
            }
            auto tOperationNode = tParentNode.append_child("Operation");
            std::vector<std::string> tKeys = { "Name", "PerformerName" };
            std::vector<std::string> tValues = { "Cache State", tService.performer() };
            XMLGen::append_children(tKeys, tValues, tOperationNode);
            if(tCriterion.type() != "modal_projection_error" &&
               tCriterion.type() != "modal_matching")
            {
                for(auto &tOutput : aMetaData.mOutputMetaData)
                {
                    auto tOutputService = tOutput.serviceID();
                    if(tOutputService == tService.id())
                    {
                        if(aMetaData.objective.multi_load_case == "true")
                        {
                            for(size_t i=0; i<aMetaData.objective.scenarioIDs.size(); ++i)
                            {
                                auto tScenarioIndex = std::to_string(i);
                                tKeys = {"ArgumentName", "SharedDataName"};
                                for(auto &tCurData : tOutput.deterministicIDs())
                                {
                                    auto tOutputNode = tOperationNode.append_child("Output");
                                    tValues = {tCurData + tScenarioIndex, tCurData + "_" + tService.performer() +
                                                  "_scenario_" + aMetaData.objective.scenarioIDs[i]};
                                    XMLGen::append_children(tKeys, tValues, tOutputNode);
                                }
                            }                        
                        }
                        else
                        {
                            tKeys = {"ArgumentName", "SharedDataName"};
                            for(auto &tCurData : tOutput.deterministicIDs())
                            {
                                auto tOutputNode = tOperationNode.append_child("Output");
                                tValues = {tCurData + "0", tCurData + "_" + tService.performer()};
                                XMLGen::append_children(tKeys, tValues, tOutputNode);
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
}
// function append_cache_state_stage
/******************************************************************************/

/******************************************************************************/
void append_update_problem_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    // always needed for levelset method
    if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY  && aMetaData.optimization_parameters().discretization() == "levelset") 
    {
        auto tStageNode = aDocument.append_child("Stage");
        XMLGen::append_children( { "Name" }, { "Update Problem" }, tStageNode);

        auto tInputNode = tStageNode.append_child("Input");
        XMLGen::append_children({"SharedDataName"},{"Control"},tInputNode);

        XMLGen::append_filter_control_operation(aMetaData, tStageNode);

        XMLGen::append_generate_xtk_model_operation(aMetaData,true, false,tStageNode);

        for (auto &tService : aMetaData.services())
        {
            std::cout<<"code = "<<tService.code()<<std::endl;
            if(tService.code() == "plato_analyze")
            {
                auto tOperationNode = tStageNode.append_child("Operation");
                std::vector<std::string> tKeys = { "Name", "PerformerName" };
                std::vector<std::string> tValues = { "Reload Mesh", tService.performer() };
                XMLGen::append_children(tKeys, tValues, tOperationNode);
            }
            if(tService.code() != "xtk" && tService.updateProblem())
            {
                auto tOperationNode = tStageNode.append_child("Operation");
                std::vector<std::string> tKeys = { "Name", "PerformerName" };
                std::vector<std::string> tValues = { "Update Problem", tService.performer() };
                XMLGen::append_children(tKeys, tValues, tOperationNode);
            }

        }
    }
    else if(XMLGen::need_update_problem_stage(aMetaData))
    {
        auto tStageNode = aDocument.append_child("Stage");
        XMLGen::append_children( { "Name" }, { "Update Problem" }, tStageNode);
        for (auto &tService : aMetaData.services())
        {
            if (!tService.updateProblem())
            {
                continue;
            }
            auto tOperationNode = tStageNode.append_child("Operation");
            std::vector<std::string> tKeys = { "Name", "PerformerName" };
            std::vector<std::string> tValues = { "Update Problem", tService.performer() };
            XMLGen::append_children(tKeys, tValues, tOperationNode);
        }
    }
}
// function append_update_problem_stage
/******************************************************************************/

/******************************************************************************/
void append_constraint_value_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY)
    {
        XMLGen::append_constraint_stage_for_topology_problem(aMetaData, aDocument);
    }
    else if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        XMLGen::append_constraint_stage_for_shape_problem(aMetaData, aDocument);
    }
}
// function append_constraint_value_stage
/******************************************************************************/

/******************************************************************************/
void append_constraint_stage_for_topology_problem 
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    auto tDesignVariableName = XMLGen::get_design_variable_name(aMetaData);
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    XMLGen::ConstraintValueOperation tValueOperationInterface;
    for(auto& tConstraint : aMetaData.constraints)
    {
        std::string tCriterionID = tConstraint.criterion();
        std::string tServiceID = tConstraint.service();
        std::string tScenarioID = tConstraint.scenario();
        ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
        auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);

        auto tStageNode = aDocument.append_child("Stage");
        auto tStageName = std::string("Compute Constraint Value ") + tConstraint.id();
        XMLGen::append_children({"Name"}, {tStageName}, tStageNode);
        auto tInputNode = tStageNode.append_child("Input");
        XMLGen::append_children({"SharedDataName"}, {"Control"}, tInputNode);

        auto tService = aMetaData.service(tConstraint.service()); 
        XMLGen::append_filter_control_operation(aMetaData, tStageNode);
        tValueOperationInterface.call(tConstraint, tService.performer(), tDesignVariableName, tService.code(), tStageNode);

        XMLGen::append_copy_value_operation(tFirstPlatoMainPerformer, std::string("Criterion Value - ") + tIdentifierString, std::string("Constraint Value ") + tConstraint.id(), tStageNode);

        if(tConstraint.greater_than())
        {
            XMLGen::append_negate_value_operation(tFirstPlatoMainPerformer, std::string("Constraint Value ") + tConstraint.id(), tStageNode);
        }

        auto tOutputNode = tStageNode.append_child("Output");
        auto tSharedDataName = std::string("Constraint Value ") + tConstraint.id();
        XMLGen::append_children({"SharedDataName"}, {tSharedDataName}, tOutputNode);
    }
}
// function append_constraint_stage_for_topology_problem
/******************************************************************************/

/******************************************************************************/
void append_constraint_stage_for_shape_problem 
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    auto tDesignVariableName = XMLGen::get_design_variable_name(aMetaData);
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    XMLGen::ConstraintValueOperation tValueOperationInterface;
    for(auto& tConstraint : aMetaData.constraints)
    {
        std::string tCriterionID = tConstraint.criterion();
        std::string tServiceID = tConstraint.service();
        std::string tScenarioID = tConstraint.scenario();
        ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
        auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);

        auto tStageNode = aDocument.append_child("Stage");
        auto tStageName = std::string("Compute Constraint Value ") + tConstraint.id();
        XMLGen::append_children({"Name"}, {tStageName}, tStageNode);
        auto tInputNode = tStageNode.append_child("Input");
        XMLGen::append_children({"SharedDataName"}, {"Design Parameters"}, tInputNode);

        auto tService = aMetaData.service(tConstraint.service()); 
        XMLGen::append_update_geometry_on_change_operation(tFirstPlatoMainPerformer, tStageNode);
        XMLGen::append_reinitialize_on_change_operation(tService.performer(), tStageNode);
        tValueOperationInterface.call(tConstraint, tService.performer(), tDesignVariableName, tService.code(), tStageNode);

        XMLGen::append_copy_value_operation(tFirstPlatoMainPerformer, std::string("Criterion Value - ") + tIdentifierString, std::string("Constraint Value ") + tConstraint.id(), tStageNode);

        auto tOutputNode = tStageNode.append_child("Output");
        auto tSharedDataName = std::string("Constraint Value ") + tConstraint.id();
        XMLGen::append_children({"SharedDataName"}, {tSharedDataName}, tOutputNode);
    }
}
// function append_constraint_stage_for_shape_problem
/******************************************************************************/

/******************************************************************************/
void append_constraint_gradient_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY  && aMetaData.optimization_parameters().discretization() == "density") 
    {
        XMLGen::append_constraint_gradient_stage_for_topology_problem(aMetaData, aDocument);
    }
    else if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE) 
    {
        XMLGen::append_constraint_gradient_stage_for_shape_problem(aMetaData, aDocument);
    }
    else if(aMetaData.optimization_parameters().optimization_type() == "topology" && aMetaData.optimization_parameters().discretization() == "levelset") 
    {
        XMLGen::append_constraint_gradient_stage_for_topology_levelset_problem(aMetaData, aDocument);
    }
    else
    {
        THROWERR(std::string("Invalid optimization and discretization combination. Supported types are topology+ density, topology+levelset, shape" 
            + aMetaData.optimization_parameters().optimization_type() + aMetaData.optimization_parameters().discretization() +"is not supported."));
    }
}
// function append_constraint_gradient_stage
/******************************************************************************/

/******************************************************************************/
void append_constraint_gradient_stage_for_topology_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    auto tDesignVariableName = XMLGen::get_design_variable_name(aMetaData);
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    XMLGen::ConstraintGradientOperation tGradOperationInterface;
    for(auto& tConstraint : aMetaData.constraints)
    {
        auto tStageNode = aDocument.append_child("Stage");
        auto tStageName = std::string("Compute Constraint Gradient ") + tConstraint.id();
        XMLGen::append_children({"Name"}, {tStageName}, tStageNode);
        auto tInputNode = tStageNode.append_child("Input");
        XMLGen::append_children({"SharedDataName"}, {"Control"}, tInputNode);

        auto tService = aMetaData.service(tConstraint.service()); 
        XMLGen::append_filter_control_operation(aMetaData, tStageNode);
        tGradOperationInterface.call(tConstraint, tService.performer(), tDesignVariableName, tService.code(), tStageNode);

        std::string tOutputSharedData = "Constraint Gradient " + tConstraint.id();
        if(aMetaData.optimization_parameters().filterInEngine() == false)
        {
            if(aMetaData.optimization_parameters().filter_type() == "helmholtz")
            {
                auto tSharedDataName = XMLGen::get_filter_constraint_criterion_gradient_input_shared_data_name(tConstraint);
                XMLGen::append_helmholtz_filter_criterion_gradient_operation(aMetaData, tSharedDataName, tOutputSharedData, tStageNode);
            }
            else
            {
                auto tSharedDataName = XMLGen::get_filter_constraint_criterion_gradient_input_shared_data_name(tConstraint);
                XMLGen::append_copy_field_operation(tFirstPlatoMainPerformer, tSharedDataName, tOutputSharedData, tStageNode);
            }
        }
        else
        {
            auto tSharedDataName = XMLGen::get_filter_constraint_criterion_gradient_input_shared_data_name(tConstraint);
            XMLGen::append_filter_criterion_gradient_operation(aMetaData, tSharedDataName, tOutputSharedData, tStageNode);
        }
        if(tConstraint.greater_than())
        {
            XMLGen::append_negate_field_operation(tFirstPlatoMainPerformer, std::string("Constraint Gradient ") + tConstraint.id(), tStageNode);
        }

        auto tOutputNode = tStageNode.append_child("Output");
        XMLGen::append_children({"SharedDataName"}, {tOutputSharedData}, tOutputNode);
    }
}
// function append_constraint_gradient_stage_for_topology_problem
/******************************************************************************/

/******************************************************************************/
void append_constraint_gradient_stage_for_shape_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    auto tDesignVariableName = XMLGen::get_design_variable_name(aMetaData);
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    XMLGen::ConstraintGradientOperation tGradOperationInterface;
    for(auto& tConstraint : aMetaData.constraints)
    {
        std::string tCriterionID = tConstraint.criterion();
        std::string tServiceID = tConstraint.service();
        std::string tScenarioID = tConstraint.scenario();
        ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
        auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);
        auto tSharedDataName = std::string("Criterion Gradient - ") + tIdentifierString;
        auto tService = aMetaData.service(tConstraint.service()); 
        auto tStageNode = aDocument.append_child("Stage");
        auto tStageName = std::string("Compute Constraint Gradient ") + tConstraint.id();
        auto tStageOutputName = std::string("Constraint Gradient ") + tConstraint.id();
        XMLGen::append_children({"Name"}, {tStageName}, tStageNode);
        auto tInputNode = tStageNode.append_child("Input");
        XMLGen::append_children({"SharedDataName"}, {"Design Parameters"}, tInputNode);

        XMLGen::append_update_geometry_on_change_operation(tFirstPlatoMainPerformer, tStageNode);
        XMLGen::append_reinitialize_on_change_operation(tService.performer(), tStageNode);
        auto tOuterOperationNode = tStageNode.append_child("Operation");
        XMLGen::append_compute_constraint_gradient_operation(tConstraint, tService.performer(), tIdentifierString, tOuterOperationNode);
        XMLGen::append_compute_shape_sensitivity_on_change_operation(tOuterOperationNode);
        XMLGen::append_compute_criterion_sensitivity_operation(aMetaData, tService, tIdentifierString, tStageNode);
        XMLGen::append_copy_value_operation(tFirstPlatoMainPerformer, tSharedDataName, tStageOutputName, tStageNode);
        auto tOutputNode = tStageNode.append_child("Output");
        XMLGen::append_children({"SharedDataName"}, {"Constraint Gradient " + tConstraint.id()}, tOutputNode);
    }
}
// function append_constraint_gradient_stage_for_shape_problem
/******************************************************************************/

/******************************************************************************/
void append_constraint_gradient_stage_for_topology_levelset_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
 {
    auto tDesignVariableName = XMLGen::get_design_variable_name(aMetaData);
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    XMLGen::ConstraintGradientOperation tGradOperationInterface;

    std::string tFirstXTKPerformer       = aMetaData.getFirstXTKMainPerformer();

    if(tFirstXTKPerformer == "")
    {
        THROWERR(std::string("Levelset topology optimization in plato currently requires one xtk perfomer."));
    }
    for(auto& tConstraint : aMetaData.constraints)
    {
        auto tStageNode = aDocument.append_child("Stage");
        auto tStageName = std::string("Compute Constraint Gradient ") + tConstraint.id();
        XMLGen::append_children({"Name"}, {tStageName}, tStageNode);
        auto tInputNode = tStageNode.append_child("Input");
        XMLGen::append_children({"SharedDataName"}, {"Control"}, tInputNode);

        auto tService = aMetaData.service(tConstraint.service()); 
        XMLGen::append_filter_control_operation(aMetaData, tStageNode);

        auto tOperationNode = tStageNode.append_child("Operation");
        XMLGen::append_children({"Name", "PerformerName"}, {"Compute Constraint Gradient " + tConstraint.id(), tService.performer()}, tOperationNode);

        tOperationNode = tStageNode.append_child("Operation");
        XMLGen::append_children({"Name", "PerformerName"}, {"Load Constraint GradientX From HDF5", tFirstXTKPerformer}, tOperationNode);

        tOperationNode = tStageNode.append_child("Operation");
        XMLGen::append_children({"Name", "PerformerName"}, 
        {"Compute Constraint Gradient " + tConstraint.id() + " XTK", tFirstXTKPerformer}, tOperationNode);

        auto tOutputNode = tOperationNode.append_child("Output");
        auto tOutputDataName = XMLGen::get_filter_constraint_criterion_gradient_input_shared_data_name(tConstraint);
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, 
        {tOutputDataName, tOutputDataName}, tOutputNode);

        // Criterion Gradient - criterion_1_service_2_scenario_1

        std::string tOutputSharedData = "Constraint Gradient " + tConstraint.id();
        if(aMetaData.optimization_parameters().filterInEngine() == false)
        {
            if(aMetaData.optimization_parameters().filter_type() == "helmholtz")
            {
                auto tSharedDataName = XMLGen::get_filter_constraint_criterion_gradient_input_shared_data_name(tConstraint);
                XMLGen::append_helmholtz_filter_criterion_gradient_operation(aMetaData, tSharedDataName, tOutputSharedData, tStageNode);
            }
            else
            {
                auto tSharedDataName = XMLGen::get_filter_constraint_criterion_gradient_input_shared_data_name(tConstraint);
                XMLGen::append_copy_field_operation(tFirstPlatoMainPerformer, tSharedDataName, tOutputSharedData, tStageNode);
            }
        }
        else
        {
            auto tSharedDataName = XMLGen::get_filter_constraint_criterion_gradient_input_shared_data_name(tConstraint);
            XMLGen::append_filter_criterion_gradient_operation(aMetaData, tSharedDataName, tOutputSharedData, tStageNode);
        }

        tOutputNode = tStageNode.append_child("Output");
        XMLGen::append_children({"SharedDataName"}, {tOutputSharedData}, tOutputNode);
    }
}
// function append_constraint_gradient_stage_for_topology_levelset_problem
/******************************************************************************/

/******************************************************************************/
void append_objective_value_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY)
    {
        XMLGen::append_objective_value_stage_for_topology_problem(aMetaData, aDocument);
    }
    else if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        XMLGen::append_objective_value_stage_for_shape_problem(aMetaData, aDocument);
    }
}
// function append_objective_value_stage
/******************************************************************************/

/******************************************************************************/
void append_objective_value_stage_for_topology_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    auto tObjective = aMetaData.objective;
    auto tStageNode = aDocument.append_child("Stage");
    XMLGen::append_children( { "Name" }, { "Compute Objective Value" }, tStageNode);
    auto tStageInputNode = tStageNode.append_child("Input");
    XMLGen::append_children( { "SharedDataName" }, { "Control" }, tStageInputNode);
    XMLGen::append_filter_control_operation(aMetaData, tStageNode);

    if(XMLGen::is_robust_optimization_problem(aMetaData))
        XMLGen::append_sample_objective_value_operation(aMetaData, tStageNode);
    else
        XMLGen::append_objective_value_operation(aMetaData, tStageNode, false);

    if(XMLGen::is_robust_optimization_problem(aMetaData))
    {
        auto tObjectiveName = "Objective Value";
        XMLGen::append_evaluate_nondeterministic_objective_value_operation(tObjectiveName, aMetaData, tStageNode);
    }

    if(!XMLGen::is_robust_optimization_problem(aMetaData))
    {
        if(aMetaData.needToAggregate())
        {
            XMLGen::append_aggregate_objective_value_operation(aMetaData, tStageNode);
        }
        else
        {
            std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
            std::string tCriterionID = aMetaData.objective.criteriaIDs[0];
            std::string tServiceID = aMetaData.objective.serviceIDs[0];
            std::string tScenarioID = "";
            if(aMetaData.objective.multi_load_case == "true")
            {
                tScenarioID = XMLGen::get_compound_scenario_id(aMetaData.objective.scenarioIDs);
            }
            else
            {
                tScenarioID = aMetaData.objective.scenarioIDs[0];
            }
            ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
            auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);
            auto tInputValue = "Criterion Value - " + tIdentifierString;

            XMLGen::append_copy_value_operation(tFirstPlatoMainPerformer, tInputValue, "Objective Value", tStageNode);
        }
    }

    auto tStageOutputNode = tStageNode.append_child("Output");
    XMLGen::append_children( { "SharedDataName" }, { "Objective Value" }, tStageOutputNode);
}
// function append_objective_value_stage_for_topology_problem
/******************************************************************************/

/******************************************************************************/
void append_objective_value_stage_for_shape_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    auto tObjective = aMetaData.objective;
    auto tStageNode = aDocument.append_child("Stage");
    XMLGen::append_children( { "Name" }, { "Compute Objective Value" }, tStageNode);
    auto tStageInputNode = tStageNode.append_child("Input");
    XMLGen::append_children( { "SharedDataName" }, { "Design Parameters" }, tStageInputNode);
    XMLGen::append_update_geometry_on_change_operation(tFirstPlatoMainPerformer, tStageNode);
    if (XMLGen::have_auxiliary_mesh(aMetaData)) {
        XMLGen::append_join_mesh_operation(tFirstPlatoMainPerformer, tStageNode);
        XMLGen::append_rename_mesh_operation(tFirstPlatoMainPerformer, tStageNode);
    }
    if (XMLGen::do_tet10_conversion(aMetaData)) {
        XMLGen::append_tet10_conversion_operation(tFirstPlatoMainPerformer, tStageNode);
    }
    int tNumReinits = XMLGen::count_number_of_reinitializations_needed(aMetaData, tObjective);
    auto tParentNode = tStageNode;
    if(tNumReinits > 0)
    {
        tParentNode = tStageNode.append_child("Operation");
    }
    for (size_t i=0; i<tObjective.criteriaIDs.size(); ++i)
    {
        std::string tCriterionID = tObjective.criteriaIDs[i];
        std::string tServiceID = tObjective.serviceIDs[i];
        std::string tScenarioID = tObjective.scenarioIDs[i];
        XMLGen::Service tService = aMetaData.service(tServiceID); 
        XMLGen::append_reinitialize_on_change_operation(tService.performer(), tParentNode);
    }

    if(XMLGen::is_robust_optimization_problem(aMetaData))
        XMLGen::append_sample_objective_value_operation(aMetaData, tStageNode);
    else
        XMLGen::append_objective_value_operation(aMetaData, tStageNode, false);


    if(XMLGen::is_robust_optimization_problem(aMetaData))
    {
        auto tObjectiveName = "Objective Value";
        XMLGen::append_evaluate_nondeterministic_objective_value_operation(tObjectiveName, aMetaData, tStageNode);
    }

    if(!XMLGen::is_robust_optimization_problem(aMetaData))
    {
        if(aMetaData.needToAggregate())
        {
            XMLGen::append_aggregate_objective_value_operation(aMetaData, tStageNode);
        }
        else
        {
            std::string tCriterionID = aMetaData.objective.criteriaIDs[0];
            std::string tServiceID = aMetaData.objective.serviceIDs[0];
            std::string tScenarioID = "";
            if(aMetaData.objective.multi_load_case == "true")
            {
                tScenarioID = XMLGen::get_compound_scenario_id(aMetaData.objective.scenarioIDs);
            }
            else
            {
                tScenarioID = aMetaData.objective.scenarioIDs[0];
            }
            ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
            auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);
            auto tInputValue = "Criterion Value - " + tIdentifierString;

            XMLGen::append_copy_value_operation(tFirstPlatoMainPerformer, tInputValue, "Objective Value", tStageNode);
        }
    }

    auto tStageOutputNode = tStageNode.append_child("Output");
    XMLGen::append_children( { "SharedDataName" }, { "Objective Value" }, tStageOutputNode);
}
// function append_objective_value_stage_for_shape_problem
/******************************************************************************/

/******************************************************************************/
void append_objective_gradient_stage
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY  && aMetaData.optimization_parameters().discretization() == "density") 
    {
        XMLGen::append_objective_gradient_stage_for_topology_problem(aMetaData, aDocument);
    }
    else if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        XMLGen::append_objective_gradient_stage_for_shape_problem(aMetaData, aDocument);
    }
    else if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY  && aMetaData.optimization_parameters().discretization() == "levelset") 
    {
        XMLGen::append_objective_gradient_stage_for_topology_levelset_problem(aMetaData, aDocument);
    }
}
// function append_objective_gradient_stage
/******************************************************************************/

/******************************************************************************/
void append_objective_gradient_stage_for_topology_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    auto tObjective = aMetaData.objective;
    auto tStageNode = aDocument.append_child("Stage");
    XMLGen::append_children({"Name"}, {"Compute Objective Gradient"}, tStageNode);
    auto tStageInputNode = tStageNode.append_child("Input");
    XMLGen::append_children({"SharedDataName"}, {"Control"}, tStageInputNode);
    XMLGen::append_filter_control_operation(aMetaData, tStageNode);

    if(XMLGen::is_robust_optimization_problem(aMetaData))
    {
        XMLGen::append_sample_objective_gradient_operation(aMetaData, tStageNode);
        auto tSharedDataName = std::string("Objective Gradient");
        XMLGen::append_evaluate_nondeterministic_objective_gradient_operation(tSharedDataName, aMetaData, tStageNode);
    }
    else
    {
        pugi::xml_node tParentNode = tStageNode;

        // If there is more than one sub-objective add an
        // outer "Operation" block so the sub-objectives
        // will be executed in parallel.
        bool tMultiObjective = (tObjective.criteriaIDs.size() > 1 && tObjective.multi_load_case != "true");
        if(tMultiObjective) { tParentNode = tParentNode.append_child("Operation"); }

        XMLGen::append_objective_gradient_operation(aMetaData, tParentNode);
        if(aMetaData.needToAggregate())
        {
            XMLGen::append_aggregate_objective_gradient_operation(aMetaData, tStageNode);
        }
    }

    if(aMetaData.optimization_parameters().filterInEngine() == true)
    {
        auto tInputMetaDataTag = XMLGen::get_filter_objective_criterion_gradient_input_shared_data_name(aMetaData);
        XMLGen::append_filter_criterion_gradient_operation(aMetaData, tInputMetaDataTag, "Objective Gradient", tStageNode);
    }

    if(aMetaData.optimization_parameters().filterInEngine() == false)
    {
        if(aMetaData.optimization_parameters().filter_type() == "helmholtz")
        {
            auto tInputMetaDataTag = XMLGen::get_filter_objective_criterion_gradient_input_shared_data_name(aMetaData);
            XMLGen::append_helmholtz_filter_criterion_gradient_operation(aMetaData, tInputMetaDataTag, "Objective Gradient", tStageNode);
        }
        else
        {
            if(aMetaData.needToAggregate())
            {
                std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
                XMLGen::append_copy_field_operation(tFirstPlatoMainPerformer, "Aggregate Gradient", "Objective Gradient", tStageNode);
            }
            else
            {
                std::string tCriterionID = tObjective.criteriaIDs[0];
                std::string tServiceID = tObjective.serviceIDs[0];
                std::string tScenarioID = tObjective.scenarioIDs[0];
                ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
                auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);

                std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
                XMLGen::append_copy_field_operation(tFirstPlatoMainPerformer, std::string("Criterion Gradient - ") + tIdentifierString, "Objective Gradient", tStageNode);
            }
        }
    }

    auto tStageOutputNode = tStageNode.append_child("Output");
    XMLGen::append_children({"SharedDataName"}, {"Objective Gradient"}, tStageOutputNode);
}
// filter append_objective_gradient_stage_for_topology_problem
/******************************************************************************/

/******************************************************************************/
void append_objective_gradient_stage_for_shape_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    auto tObjective = aMetaData.objective;
    auto tStageNode = aDocument.append_child("Stage");
    XMLGen::append_children({"Name"}, {"Compute Objective Gradient"}, tStageNode);
    auto tStageInputNode = tStageNode.append_child("Input");
    XMLGen::append_children({"SharedDataName"}, {"Design Parameters"}, tStageInputNode);
    XMLGen::append_update_geometry_on_change_operation(tFirstPlatoMainPerformer, tStageNode);
    if (XMLGen::have_auxiliary_mesh(aMetaData)) 
    {
        XMLGen::append_join_mesh_operation(tFirstPlatoMainPerformer, tStageNode);
        XMLGen::append_rename_mesh_operation(tFirstPlatoMainPerformer, tStageNode);
    }
    if (XMLGen::do_tet10_conversion(aMetaData)) 
    {
        XMLGen::append_tet10_conversion_operation(tFirstPlatoMainPerformer, tStageNode);
    }
    int tNumReinits = XMLGen::count_number_of_reinitializations_needed(aMetaData, tObjective);
    auto tParentNode = tStageNode;
    if(tNumReinits > 0)
    {
        tParentNode = tStageNode.append_child("Operation");
    }
    for (size_t i=0; i<tObjective.criteriaIDs.size(); ++i)
    {
        std::string tCriterionID = tObjective.criteriaIDs[i];
        std::string tServiceID = tObjective.serviceIDs[i];
        std::string tScenarioID = tObjective.scenarioIDs[i];
        XMLGen::Service tService = aMetaData.service(tServiceID); 
        XMLGen::append_reinitialize_on_change_operation(tService.performer(), tParentNode);
    }

    if(XMLGen::is_robust_optimization_problem(aMetaData))
    {
        XMLGen::append_sample_objective_gradient_operation(aMetaData, tStageNode);
        auto tSharedDataName = std::string("Objective Gradient");
        XMLGen::append_evaluate_nondeterministic_objective_gradient_operation(tSharedDataName, aMetaData, tStageNode);
    }
    else
    {
        auto tOuterOperationNode = tStageNode.append_child("Operation");
        XMLGen::append_objective_gradient_operation(aMetaData, tOuterOperationNode);
        XMLGen::append_compute_shape_sensitivity_on_change_operation(tOuterOperationNode);
        tOuterOperationNode = tStageNode.append_child("Operation");
        for (size_t i=0; i<tObjective.criteriaIDs.size(); ++i)
        {
            std::string tCriterionID = tObjective.criteriaIDs[i];
            std::string tServiceID = tObjective.serviceIDs[i];
            std::string tScenarioID = tObjective.scenarioIDs[i];
            XMLGen::Service tService = aMetaData.service(tServiceID); 
            ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
            auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);
            XMLGen::append_compute_criterion_sensitivity_operation(aMetaData, tService, tIdentifierString, tOuterOperationNode);
        }
        if(aMetaData.needToAggregate())
        {
            XMLGen::append_aggregate_objective_gradient_operation(aMetaData, tStageNode);
        }
    }

    if(!aMetaData.needToAggregate())
    {
        std::string tCriterionID = tObjective.criteriaIDs[0];
        std::string tServiceID = tObjective.serviceIDs[0];
        std::string tScenarioID = "";
        if(aMetaData.objective.multi_load_case == "true")
        {
            tScenarioID = XMLGen::get_compound_scenario_id(aMetaData.objective.scenarioIDs);
        }
        else
        {
            tScenarioID = aMetaData.objective.scenarioIDs[0];
        }
        ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
        auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);
        auto tSharedDataName = "Criterion Gradient - " + tIdentifierString;
        XMLGen::append_copy_value_operation(tFirstPlatoMainPerformer, tSharedDataName, "Objective Gradient", tStageNode);

    }

    auto tStageOutputNode = tStageNode.append_child("Output");
    XMLGen::append_children({"SharedDataName"}, {"Objective Gradient"}, tStageOutputNode);
}
// function append_objective_gradient_stage_for_shape_problem
/******************************************************************************/

/******************************************************************************/
void append_objective_gradient_stage_for_topology_levelset_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    std::string tFirstXTKMainPerformer = aMetaData.getFirstXTKMainPerformer();
    auto tObjective = aMetaData.objective;
    auto tStageNode = aDocument.append_child("Stage");
    XMLGen::append_children({"Name"}, {"Compute Objective Gradient"}, tStageNode);
    auto tStageInputNode = tStageNode.append_child("Input");
    XMLGen::append_children({"SharedDataName"}, {"Control"}, tStageInputNode);
    XMLGen::append_filter_control_operation(aMetaData, tStageNode);

    std::string tServiceID = tObjective.serviceIDs[0];
    XMLGen::Service tService = aMetaData.service(tServiceID);

    auto tOperationNode = tStageNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"Compute Objective Gradient", tService.performer()}, tOperationNode);

    // load from hdf5
    tOperationNode = tStageNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"Load Objective GradientX From HDF5", tFirstXTKMainPerformer}, tOperationNode);

    tOperationNode = tStageNode.append_child("Operation");
    XMLGen::append_children({"Name", "PerformerName"}, {"Compute Objective Gradient XTK", tFirstXTKMainPerformer}, tOperationNode);

    auto tOutputNode = tOperationNode.append_child("Output");
    auto tOutputDataName = XMLGen::get_filter_objective_criterion_gradient_input_shared_data_name(aMetaData);
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, 
    {tOutputDataName, tOutputDataName}, tOutputNode);

    if(aMetaData.optimization_parameters().filterInEngine())
    {
        auto tInputMetaDataTag = XMLGen::get_filter_objective_criterion_gradient_input_shared_data_name(aMetaData);
        XMLGen::append_filter_criterion_gradient_operation(aMetaData, tInputMetaDataTag, "Objective Gradient", tStageNode);
    }

    if(aMetaData.optimization_parameters().filterInEngine() == false)
    {
        if(aMetaData.optimization_parameters().filter_type() == "helmholtz")
        {
            auto tInputMetaDataTag = XMLGen::get_filter_objective_criterion_gradient_input_shared_data_name(aMetaData);
            XMLGen::append_helmholtz_filter_criterion_gradient_operation(aMetaData, tInputMetaDataTag, "Objective Gradient", tStageNode);
        }
        else if(aMetaData.optimization_parameters().filter_type() != "helmholtz" && !aMetaData.needToAggregate())
        {
            tServiceID = tObjective.serviceIDs[0];
            std::string tCriterionID = tObjective.criteriaIDs[0];
            std::string tScenarioID = tObjective.scenarioIDs[0];
            ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
            auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);

            std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
            append_copy_field_operation(tFirstPlatoMainPerformer, std::string("Criterion Gradient - ") + tIdentifierString, "Objective Gradient", tStageNode);
        }
    }

    auto tStageOutputNode = tStageNode.append_child("Output");
    XMLGen::append_children({"SharedDataName"}, {"Objective Gradient"}, tStageOutputNode);
}
// function append_objective_gradient_stage_for_topology_levelset_problem
/******************************************************************************/

}
