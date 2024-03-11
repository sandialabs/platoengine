/*
 * XMLGeneratorPlatoMainOperationFileUtilities.cpp
 *
 *  Created on: May 28, 2020
 */

#include <cstdlib>
#include <fstream>

#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorValidInputKeys.hpp"
#include "XMLGeneratorServiceMetadata.hpp"
#include "XMLGeneratorParserUtilities.hpp"
#include "XMLGeneratorServiceUtilities.hpp"
#include "XMLGeneratorSierraSDUtilities.hpp"
#include "XMLGeneratorFixedBlockUtilities.hpp"
#include "XMLGeneratorInterfaceFileUtilities.hpp"
#include "XMLGeneratorSierraSDInputDeckUtilities.hpp"
#include "XMLGeneratorPlatoMainOperationFileUtilities.hpp"

namespace XMLGen
{

/******************************************************************************/
void write_plato_main_operations_xml_file
(const XMLGen::InputData& aMetaData)
{
    pugi::xml_document tDocument;

    XMLGen::append_include_defines_xml_data(aMetaData, tDocument);
    XMLGen::append_filter_options_to_plato_main_operation(aMetaData, tDocument);
    XMLGen::append_output_to_plato_main_operation(aMetaData, tDocument);
    XMLGen::append_initialize_field_to_plato_main_operation(aMetaData, tDocument);
    XMLGen::append_set_lower_bounds_to_plato_main_operation(aMetaData, tDocument);
    XMLGen::append_set_upper_bounds_to_plato_main_operation(aMetaData, tDocument);
    XMLGen::append_copy_field_to_plato_main_operation(aMetaData, tDocument);
    XMLGen::append_copy_value_to_plato_main_operation(aMetaData, tDocument);
    XMLGen::append_design_volume_to_plato_main_operation(aMetaData, tDocument);
    XMLGen::append_compute_volume_to_plato_main_operation(aMetaData, tDocument);
    XMLGen::append_compute_volume_gradient_to_plato_main_operation(aMetaData, tDocument);
    if(aMetaData.needNegateOperation())
    {
        XMLGen::append_negate_field_to_plato_main_operation(aMetaData, tDocument);
        XMLGen::append_negate_value_to_plato_main_operation(aMetaData, tDocument);
    }

    if(XMLGen::is_robust_optimization_problem(aMetaData))
    {
        XMLGen::append_stochastic_objective_value_to_plato_main_operation(aMetaData, tDocument);
        XMLGen::append_stochastic_objective_gradient_to_plato_main_operation(aMetaData, tDocument);
        XMLGen::append_qoi_statistics_to_plato_main_operation(aMetaData, tDocument);
    }

    XMLGen::append_update_problem_to_plato_main_operation(aMetaData, tDocument);
    XMLGen::append_filter_control_to_plato_main_operation(aMetaData, tDocument);
    XMLGen::append_filter_gradient_to_plato_main_operation(aMetaData, tDocument);
    //if(!XMLGen::is_robust_optimization_problem(aMetaData))
    if(aMetaData.needToAggregate())
    {
        XMLGen::append_aggregate_data_to_plato_main_operation(aMetaData, tDocument);
    }

    XMLGen::append_csm_mesh_output_to_plato_main_operation(aMetaData, tDocument);
    XMLGen::append_initialize_geometry_operation_to_plato_main_operation(aMetaData, tDocument);
    XMLGen::append_update_geometry_on_change_operation_to_plato_main_operation(aMetaData, tDocument);
    XMLGen::append_enforce_bounds_operation_to_plato_main_operation(aMetaData, tDocument);
    XMLGen::append_chain_rule_operation_to_plato_main_operation(aMetaData, tDocument);
    
    if (XMLGen::do_tet10_conversion(aMetaData)) 
        XMLGen::append_tet10_conversion_operation_to_plato_main_operation(aMetaData, tDocument);
    
    if (XMLGen::have_auxiliary_mesh(aMetaData)) {
        XMLGen::append_mesh_join_operation_to_plato_main_operation(aMetaData, tDocument);
        XMLGen::append_mesh_rename_operation_to_plato_main_operation(aMetaData, tDocument);
    }

    tDocument.save_file("plato_main_operations.xml", "  ");
}
/******************************************************************************/

/******************************************************************************/
void write_dakota_plato_main_operations_xml_file
(const XMLGen::InputData& aMetaData)
{
    pugi::xml_document tDocument;
    append_update_geometry_on_change_operation_to_plato_main_operation(aMetaData, tDocument);
    append_reinitialize_operation_to_plato_main_operation(aMetaData, tDocument);
    append_compute_volume_criterion_value_operation_to_plato_main_operation(aMetaData, tDocument);
    append_decomp_operations_for_physics_performers_to_plato_main_operation(aMetaData, tDocument);
    if (XMLGen::create_subblock(aMetaData)) 
        XMLGen::append_subblock_creation_operation_to_plato_main_operation(aMetaData, tDocument);
    if (XMLGen::do_tet10_conversion(aMetaData))
        XMLGen::append_tet10_conversion_operation_to_plato_main_operation(aMetaData, tDocument);

    tDocument.save_file("plato_main_operations.xml", "  ");
}

/******************************************************************************/
bool is_volume_constraint_defined
(const XMLGen::InputData& aXMLMetaData)
{
    auto tVolumeConstraintDefined = false;
    for(auto& tConstraint : aXMLMetaData.constraints)
    {
        auto tCriterion = aXMLMetaData.criterion(tConstraint.criterion());
        auto tType = Plato::tolower(tCriterion.type());
        tVolumeConstraintDefined = tType.compare("volume") == 0;
        if (tVolumeConstraintDefined == true)
        {
            break;
        }
    }
    return tVolumeConstraintDefined;
}
// function is_volume_constraint_defined
/******************************************************************************/

/******************************************************************************/
bool is_volume_constraint_defined_and_computed_by_platomain
(const XMLGen::InputData& aXMLMetaData)
{
    auto tIsVolumeDefinedAndComputedByPlatoMain = false;
    for(auto& tConstraint : aXMLMetaData.constraints)
    {
        auto tCriterion = aXMLMetaData.criterion(tConstraint.criterion());
        auto tType = Plato::tolower(tCriterion.type());
        auto tService = aXMLMetaData.service(tConstraint.service());
        auto tCode = Plato::tolower(tService.code());

        auto tIsVolumeConstraintDefined = tType.compare("volume") == 0;
        auto tIsVolumeComputedByPlatoMain = tCode.compare("platomain") == 0;
        tIsVolumeDefinedAndComputedByPlatoMain = tIsVolumeConstraintDefined && tIsVolumeComputedByPlatoMain;
        if (tIsVolumeDefinedAndComputedByPlatoMain == true)
        {
            break;
        }
    }
    return tIsVolumeDefinedAndComputedByPlatoMain;
}
//function is_volume_constraint_defined_and_computed_by_platomain
/******************************************************************************/

/******************************************************************************/
bool get_volume_criterion_defined_and_computed_by_platomain
(const XMLGen::InputData& aXMLMetaData,
 std::string& aIdentifierString)
{
    auto tIsVolumeDefinedAndComputedByPlatoMain = false;

    XMLGen::Objective tObjective = aXMLMetaData.objective;
    for (size_t i=0; i<tObjective.criteriaIDs.size(); ++i)
    {
        std::string tCriterionID = tObjective.criteriaIDs[i];
        std::string tServiceID = tObjective.serviceIDs[i];
        std::string tScenarioID = tObjective.scenarioIDs[i];

        auto tType = Plato::tolower(aXMLMetaData.criterion(tCriterionID).type());
        auto tCode = Plato::tolower(aXMLMetaData.service(tServiceID).code());

        auto tIsVolumeConstraintDefined = tType.compare("volume") == 0;
        auto tIsVolumeComputedByPlatoMain = tCode.compare("platomain") == 0;
        tIsVolumeDefinedAndComputedByPlatoMain = tIsVolumeConstraintDefined && tIsVolumeComputedByPlatoMain;
        if (tIsVolumeDefinedAndComputedByPlatoMain == true)
        {
            ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
            aIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);
            return tIsVolumeDefinedAndComputedByPlatoMain;
        }
    }

    for(auto& tConstraint : aXMLMetaData.constraints)
    {
        std::string tCriterionID = tConstraint.criterion();
        std::string tServiceID = tConstraint.service();
        std::string tScenarioID = tConstraint.scenario();

        auto tType = Plato::tolower(aXMLMetaData.criterion(tCriterionID).type());
        auto tCode = Plato::tolower(aXMLMetaData.service(tServiceID).code());

        auto tIsVolumeConstraintDefined = tType.compare("volume") == 0;
        auto tIsVolumeComputedByPlatoMain = tCode.compare("platomain") == 0;
        tIsVolumeDefinedAndComputedByPlatoMain = tIsVolumeConstraintDefined && tIsVolumeComputedByPlatoMain;
        if (tIsVolumeDefinedAndComputedByPlatoMain == true)
        {
            ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
            aIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);
            return tIsVolumeDefinedAndComputedByPlatoMain;
        }
    }
    return tIsVolumeDefinedAndComputedByPlatoMain;
}
//function get_volume_criterion_defined_and_computed_by_platomain
/******************************************************************************/

/******************************************************************************/
std::string get_platomain_volume_constraint_id
(const XMLGen::InputData& aXMLMetaData)
{
    for(auto& tConstraint : aXMLMetaData.constraints)
    {
        auto tCriterion = aXMLMetaData.criterion(tConstraint.criterion());
        auto tType = Plato::tolower(tCriterion.type());
        auto tService = aXMLMetaData.service(tConstraint.service());
        auto tCode = Plato::tolower(tService.code());

        auto tIsVolumeConstraintDefined = tType.compare("volume") == 0;
        auto tIsVolumeComputedByPlatoMain = tCode.compare("platomain") == 0;
        bool tIsVolumeDefinedAndComputedByPlatoMain = tIsVolumeConstraintDefined && tIsVolumeComputedByPlatoMain;
        if (tIsVolumeDefinedAndComputedByPlatoMain == true)
        {
            return tConstraint.id();
        }
    }
    THROWERR("Volume constraint was assumed to be computed by platomain but was not.")
}
//function get_platomain_volume_constraint_id
/******************************************************************************/

/******************************************************************************/
void append_filter_options_to_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node &aParentNode)
{
    std::string tRadiusScale = aXMLMetaData.optimization_parameters().filter_radius_scale();
    std::string tRadiusAbsolute = aXMLMetaData.optimization_parameters().filter_radius_absolute();
    std::string tFilterPower = aXMLMetaData.optimization_parameters().filter_power();
    std::string tFilterType = aXMLMetaData.optimization_parameters().filter_type();

    std::vector<std::string> tKeys;
    std::vector<std::string> tValues;

    if(!tRadiusScale.empty())
    {
        tKeys.push_back("Scale");
        tValues.push_back(tRadiusScale);
    }
    if(!tRadiusAbsolute.empty())
    {
        tKeys.push_back("Absolute");
        tValues.push_back(tRadiusAbsolute);
    }
    if(!tFilterPower.empty())
    {
        tKeys.push_back("Power");
        tValues.push_back(tFilterPower);
    }
    if(tFilterType == "kernel_then_heaviside" ||
       tFilterType == "kernel_then_tanh")
    {
        tKeys.push_back("StartIteration");
        tValues.push_back(aXMLMetaData.optimization_parameters().filter_projection_start_iteration());
        tKeys.push_back("UpdateInterval");
        tValues.push_back(aXMLMetaData.optimization_parameters().filter_projection_update_interval());
        tKeys.push_back("UseAdditiveContinuation");
        tValues.push_back(aXMLMetaData.optimization_parameters().filter_use_additive_continuation());
        tKeys.push_back("HeavisideMin");
        tValues.push_back(aXMLMetaData.optimization_parameters().filter_heaviside_min());
        tKeys.push_back("HeavisideUpdate");
        tValues.push_back(aXMLMetaData.optimization_parameters().filter_heaviside_update());
        tKeys.push_back("HeavisideMax");
        tValues.push_back(aXMLMetaData.optimization_parameters().filter_heaviside_max());
    } 

    XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
    XMLGen::append_children(tKeys, tValues, aParentNode);
}
// function append_filter_options_to_operation
/******************************************************************************/

/******************************************************************************/
void append_projection_options_to_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node &aParentNode)
{
    std::vector<std::string> tKeys = {"StartIteration", "UpdateInterval",
        "UseAdditiveContinuation", "HeavisideMin", "HeavisideUpdate", "HeavisideMax"};

    std::vector<std::string> tValues = {aXMLMetaData.optimization_parameters().filter_projection_start_iteration(), aXMLMetaData.optimization_parameters().filter_projection_update_interval(),
        aXMLMetaData.optimization_parameters().filter_use_additive_continuation(), aXMLMetaData.optimization_parameters().filter_heaviside_min(),
        aXMLMetaData.optimization_parameters().filter_heaviside_update(), aXMLMetaData.optimization_parameters().filter_heaviside_max()};

    XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
    XMLGen::append_children(tKeys, tValues, aParentNode);
}
// function append_projection_options_to_operation
/******************************************************************************/

/******************************************************************************/
void append_filter_options_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document &aDocument)
{
    if(aXMLMetaData.optimization_parameters().filterInEngine() == true &&
       aXMLMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY)
    {
        XMLGen::ValidFilterKeys tValidFilterKeys;
        auto tValue = tValidFilterKeys.value(aXMLMetaData.optimization_parameters().filter_type());
        auto tFilterName = tValue.empty() ? "Kernel" : tValue;

        XMLGen::ValidProjectionKeys tValidProjectionKeys;
        auto tProjectionName = tValidProjectionKeys.value(aXMLMetaData.optimization_parameters().projection_type());

        auto tFilterNode = aDocument.append_child("Filter");
        if(tProjectionName.empty())
        {
            XMLGen::append_children({"Name"}, {tFilterName}, tFilterNode);
        }
        else
        {
            auto tFullName = tFilterName + "_then_" + aXMLMetaData.optimization_parameters().projection_type();
            tFilterName = tValidFilterKeys.value(tFullName);
            XMLGen::append_children({"Name"}, {tFilterName}, tFilterNode);
        }
        XMLGen::append_filter_options_to_operation(aXMLMetaData, tFilterNode);
    }
    else if(aXMLMetaData.optimization_parameters().filterInEngine() == false &&
       aXMLMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY &&
       aXMLMetaData.optimization_parameters().filter_type() == "helmholtz")
    {
        XMLGen::ValidProjectionKeys tValidKeys;
        auto tProjectionName = tValidKeys.value(aXMLMetaData.optimization_parameters().projection_type());

        if(!tProjectionName.empty())
        {
            auto tFilterNode = aDocument.append_child("Filter");
            XMLGen::append_children({"Name"}, {tProjectionName}, tFilterNode);
            XMLGen::append_projection_options_to_operation(aXMLMetaData, tFilterNode);
        }
    }
}
// function append_filter_options_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_enforce_bounds_operation_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document &aDocument)
{
    if(aXMLMetaData.optimization_parameters().enforceBounds())
    {
        auto tOperationNode = aDocument.append_child("Operation");
        XMLGen::append_children({"Name","Function"}, {"EnforceBounds","EnforceBounds"}, tOperationNode);
        auto tInputNode = tOperationNode.append_child("Input");
        XMLGen::append_children({"ArgumentName"}, {"Upper Bound Vector"}, tInputNode);
        tInputNode = tOperationNode.append_child("Input");
        XMLGen::append_children({"ArgumentName"}, {"Lower Bound Vector"}, tInputNode);
        tInputNode = tOperationNode.append_child("Input");
        XMLGen::append_children({"ArgumentName"}, {"Topology"}, tInputNode);
        auto tOutputNode = tOperationNode.append_child("Output");
        XMLGen::append_children({"ArgumentName"}, {"Clamped Topology"}, tOutputNode);
    }
}
// function append_enforce_bounds_operation_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_chain_rule_operation_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document &aDocument)
{
    if(aXMLMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        auto tOperationNode = aDocument.append_child("Operation");
        XMLGen::append_children({"Name","Function"}, {"Chain Rule","ChainRule"}, tOperationNode);
        auto tForNode = tOperationNode.append_child("For");
        XMLGen::append_attributes({"var", "in"}, {"I", "Parameters"}, tForNode);
        auto tInputNode = tForNode.append_child("Input");
        XMLGen::append_children({"ArgumentName"}, {"Parameter Sensitivity {I}"}, tInputNode);
        tInputNode = tOperationNode.append_child("Input");
        XMLGen::append_children({"ArgumentName"}, {"DFDX"}, tInputNode);
        auto tOutputNode = tOperationNode.append_child("Output");
        XMLGen::append_children({"ArgumentName"}, {"Full Gradient"}, tOutputNode);
    }
}
// function append_chain_rule_operation_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_constraint_gradient_to_output_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node &aParentNode)
{
    for(auto& tConstraint : aXMLMetaData.constraints)
    {
        auto tInput = aParentNode.append_child("Input");
        auto tArgumentName = std::string("constraint gradient ") + tConstraint.id();
        XMLGen::append_children( { "ArgumentName" }, { tArgumentName }, tInput);
    }
}
// function append_constraint_gradient_to_output_operation
/******************************************************************************/

/******************************************************************************/
void append_objective_gradient_to_output_operation
(const XMLGen::InputData& /*aXMLMetaData*/,
 pugi::xml_node &aParentNode)
{
/*
    bool tNeedToAggregate = aXMLMetaData.needToAggregate();

    if(tNeedToAggregate)
    {
*/
 // I think this will always be "objective gradient" but I am leaving the old code
 // in just in case we run into a situation when that isn't the case.
        auto tInput = aParentNode.append_child("Input");
        XMLGen::append_children( { "ArgumentName" }, { "objective gradient" }, tInput);
/*
    }
    else
    {
        for(size_t i=0; i<aXMLMetaData.objective.criteriaIDs.size(); ++i)
        {
            std::string tCriterionID = aXMLMetaData.objective.criteriaIDs[i];
            std::string tServiceID = aXMLMetaData.objective.serviceIDs[i];
            std::string tScenarioID = aXMLMetaData.objective.scenarioIDs[i];
            ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
            auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);

            auto tInput = aParentNode.append_child("Input");
            std::string tName = std::string("Criterion Gradient - ");
            tName += tIdentifierString;
            auto tArgumentName = XMLGen::to_lower(tName);
            XMLGen::append_children( { "ArgumentName" }, { tArgumentName }, tInput);
        }
    }
*/
}
// function append_objective_gradient_to_output_operation
/******************************************************************************/

/******************************************************************************/
void append_qoi_statistics_to_output_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node &aParentNode)
{
    for(auto &tOutputMetadata : aXMLMetaData.mOutputMetaData)
    {
        auto tOutputQoIs = tOutputMetadata.randomIDs();
        for(auto& tOutputQoI : tOutputQoIs)
        {
            auto tLayout = tOutputMetadata.randomLayout(tOutputQoI);
            auto tValidLayout = XMLGen::check_data_layout(tLayout);
            auto tArgumentName = tOutputQoI + " mean";
            auto tInput= aParentNode.append_child("Input");
            XMLGen::append_children( { "ArgumentName", "Layout" }, { tArgumentName, tValidLayout }, tInput);

            tArgumentName = tOutputQoI + " standard deviation";
            tInput= aParentNode.append_child("Input");
            XMLGen::append_children( { "ArgumentName", "Layout" }, { tArgumentName, tValidLayout }, tInput);
        }
    }
}
/******************************************************************************/

/******************************************************************************/
void append_deterministic_qoi_to_output_operation_for_non_multi_load_case
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node &aParentNode)
{
    for(auto &tOutputMetadata : aXMLMetaData.mOutputMetaData)
    {
        auto tIDs = tOutputMetadata.deterministicIDs();
        for(auto& tID : tIDs)
        {
            auto tInput= aParentNode.append_child("Input");
            auto tLayout = tOutputMetadata.deterministicLayout(tID);
            auto tArgumentName = tOutputMetadata.deterministicArgumentName(tID);
            auto tPerformerString = std::string("_") + 
                                    aXMLMetaData.service(tOutputMetadata.serviceID()).performer();
            tArgumentName += tPerformerString;
            XMLGen::append_children( { "ArgumentName", "Layout" }, { tArgumentName, tLayout }, tInput);
        }
    }
}
/******************************************************************************/

/******************************************************************************/
void append_deterministic_qoi_to_output_operation_for_multi_load_case
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node &aParentNode)
{
    for(auto &tOutputMetadata : aXMLMetaData.mOutputMetaData)
    {
        auto tIDs = tOutputMetadata.deterministicIDs();
        for(auto& tID : tIDs)
        {
            auto tLayout = tOutputMetadata.deterministicLayout(tID);
            for(auto tScenarioID : aXMLMetaData.objective.scenarioIDs)
            {
                auto tArgumentName = tOutputMetadata.deterministicArgumentName(tID);
                auto tInput = aParentNode.append_child("Input");
                auto tPerformerString = std::string("_") + 
                                        aXMLMetaData.service(tOutputMetadata.serviceID()).performer();
                tArgumentName += tPerformerString;
                tArgumentName += "_scenario_" + tScenarioID;
                XMLGen::append_children( { "ArgumentName", "Layout" }, { tArgumentName, tLayout }, tInput);
            }
        }
    }
}
/******************************************************************************/

/******************************************************************************/
void append_deterministic_qoi_to_output_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node &aParentNode)
{
    if(aXMLMetaData.objective.multi_load_case == "true")
    {
        append_deterministic_qoi_to_output_operation_for_multi_load_case(aXMLMetaData, aParentNode);
    }
    else
    {
        append_deterministic_qoi_to_output_operation_for_non_multi_load_case(aXMLMetaData, aParentNode);
    }
}
// function append_deterministic_qoi_to_output_operation
/******************************************************************************/

/******************************************************************************/
void append_stochastic_qoi_to_output_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node &aParentNode)
{
    for(auto &tOutputMetadata : aXMLMetaData.mOutputMetaData)
    {
        if(tOutputMetadata.outputSamples() == false)
        {
            return;
        }

        auto tIDs = tOutputMetadata .randomIDs();
        for(auto& tID : tIDs)
        {
            auto tFor = aParentNode.append_child("For");
            XMLGen::append_attributes({"var", "in"}, {"SampleIndex", "Samples"}, tFor);

            auto tInput= tFor.append_child("Input");
            auto tLayout = tOutputMetadata .randomLayout(tID);
            auto tArgumentName = tID + " {SampleIndex}";
            XMLGen::append_children( { "ArgumentName", "Layout" }, { tArgumentName, tLayout }, tInput);
        }
    }
}
// function append_stochastic_qoi_to_output_operation
/******************************************************************************/

/******************************************************************************/
void append_children_to_output_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node &aParentNode)
{
    std::vector<std::string> tKeys = {"Function", "Name", "WriteRestart", "OutputFrequency", "MaxIterations", "RestartFieldName"};
    std::vector<std::string> tValues = {"PlatoMainOutput", "PlatoMainOutput", aXMLMetaData.optimization_parameters().write_restart_file(),
        aXMLMetaData.optimization_parameters().output_frequency(), aXMLMetaData.optimization_parameters().max_iterations(), aXMLMetaData.optimization_parameters().initial_guess_field_name()};
    XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
    XMLGen::append_children(tKeys, tValues, aParentNode);
}
// function append_children_to_output_operation
/******************************************************************************/

/******************************************************************************/
void append_default_qoi_to_output_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node &aParentNode)
{
    auto tInput = aParentNode.append_child("Input");
    auto tLayout = XMLGen::return_output_qoi_data_layout("topology");
    auto tValidLayoutKeyword = XMLGen::check_data_layout_keyword(tLayout);
    XMLGen::append_children({"ArgumentName", "Layout"}, {"topology", tValidLayoutKeyword}, tInput);

    tInput = aParentNode.append_child("Input");
    tLayout = XMLGen::return_output_qoi_data_layout("control");
    tValidLayoutKeyword = XMLGen::check_data_layout_keyword(tLayout);
    XMLGen::append_children({"ArgumentName", "Layout"}, {"control", tValidLayoutKeyword}, tInput);
    XMLGen::append_objective_gradient_to_output_operation(aXMLMetaData, aParentNode);
    XMLGen::append_constraint_gradient_to_output_operation(aXMLMetaData, aParentNode);
}
// function append_default_qoi_to_output_operation
/******************************************************************************/

/******************************************************************************/
void append_surface_extraction_to_output_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node &aParentNode)
{
    const std::vector<XMLGen::Scenario> &tScenarios = aXMLMetaData.scenarios();
    if (tScenarios.empty())
    {
        THROWERR("Scenarios list is empty. Scenarios list is requested to determine the number of spatial dimensions.")
    }

    if (tScenarios[0].dimensions() == "3")
    {
        std::string tOutputMethod = aXMLMetaData.optimization_parameters().output_method();
        if (!tOutputMethod.empty())
        {
            auto tSurfaceExtraction = aParentNode.append_child("SurfaceExtraction");
            std::vector<std::string> tKeys = {"OutputMethod", "Discretization"};
            std::string tStoredDiscretizationValue = aXMLMetaData.optimization_parameters().discretization();
            auto tDiscretization = tStoredDiscretizationValue.empty() ? "density" : tStoredDiscretizationValue;
            std::vector<std::string> tValues = {tOutputMethod, tDiscretization};
            XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
            XMLGen::append_children(tKeys, tValues, tSurfaceExtraction);

            auto tOutput = tSurfaceExtraction.append_child("Output");
            XMLGen::append_children({"Format"}, {"Exodus"}, tOutput);
        }
    }
}
// function append_surface_extraction_to_output_operation
/******************************************************************************/

/******************************************************************************/
void append_output_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document &aDocument)
{
    if(aXMLMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        return;
    }
    for(auto &tOutputMetadata : aXMLMetaData.mOutputMetaData)
    {
        if(tOutputMetadata.isOutputDisabled())
        {
            return;
        }
    }
    auto tOperation = aDocument.append_child("Operation");
    XMLGen::append_children_to_output_operation(aXMLMetaData, tOperation);
    XMLGen::append_default_qoi_to_output_operation(aXMLMetaData, tOperation);
    XMLGen::append_qoi_statistics_to_output_operation(aXMLMetaData, tOperation);
    XMLGen::append_stochastic_qoi_to_output_operation(aXMLMetaData, tOperation);
    XMLGen::append_deterministic_qoi_to_output_operation(aXMLMetaData, tOperation);
    XMLGen::append_surface_extraction_to_output_operation(aXMLMetaData, tOperation);
}
// function append_output_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_stochastic_objective_value_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document &aDocument)
{
    auto tOperation = aDocument.append_child("Operation");
    std::vector<std::string> tKeys = {"Function", "Name", "Layout"};
    std::vector<std::string> tValues = {"MeanPlusStdDev", "Compute Non-Deterministic Objective Value", "Scalar"};
    XMLGen::append_children(tKeys, tValues, tOperation);

    auto tOuterFor = tOperation.append_child("For");
    XMLGen::append_attributes({"var", "in"}, {"PerformerIndex", "Performers"}, tOuterFor);
    auto tInnerFor = tOuterFor.append_child("For");
    XMLGen::append_attributes({"var", "in"}, {"PerformerSampleIndex", "PerformerSamples"}, tInnerFor);
    auto tInnerForInput = tInnerFor.append_child("Input");
    tKeys = {"ArgumentName", "Probability"};
    tValues = {"Objective Value {PerformerIndex*NumSamplesPerPerformer+PerformerSampleIndex}",
        "{Probabilities[{PerformerIndex*NumSamplesPerPerformer+PerformerSampleIndex}]}"};
    XMLGen::append_children(tKeys, tValues, tInnerForInput);

    auto tOuterOutput = tOperation.append_child("Output");
    XMLGen::append_children({"Statistic", "ArgumentName"}, {"mean", "Objective Mean"}, tOuterOutput);
    tOuterOutput = tOperation.append_child("Output");
    XMLGen::append_children({"Statistic", "ArgumentName"}, {"std_dev", "Objective StdDev"}, tOuterOutput);
    tOuterOutput = tOperation.append_child("Output");
    auto tStatistics = std::string("mean_plus_") + aXMLMetaData.optimization_parameters().objective_number_standard_deviations() + "_std_dev";
    auto tArgumentName = std::string("Objective Mean Plus ") + aXMLMetaData.optimization_parameters().objective_number_standard_deviations() + " StdDev";
    XMLGen::append_children({"Statistic", "ArgumentName"}, {tStatistics, tArgumentName}, tOuterOutput);
}
// function append_stochastic_objective_value_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_stochastic_criterion_gradient_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node& aParentNode)
{
    auto tCriterionGradient = aParentNode.append_child("CriterionGradient");
    XMLGen::append_children({"Layout"}, {"Nodal Field"}, tCriterionGradient);

    auto tOuterFor = tCriterionGradient.append_child("For");
    XMLGen::append_attributes({"var", "in"}, {"PerformerIndex", "Performers"}, tOuterFor);
    auto tInnerFor = tOuterFor.append_child("For");
    XMLGen::append_attributes({"var", "in"}, {"PerformerSampleIndex", "PerformerSamples"}, tInnerFor);
    auto tInnerForInput = tInnerFor.append_child("Input");
    std::vector<std::string> tKeys = {"ArgumentName", "Probability"};
    std::vector<std::string> tValues = {"Objective Gradient {PerformerIndex*NumSamplesPerPerformer+PerformerSampleIndex}",
        "{Probabilities[{PerformerIndex*NumSamplesPerPerformer+PerformerSampleIndex}]}"};
    XMLGen::append_children(tKeys, tValues, tInnerForInput);

    auto tOuterOutput = tCriterionGradient.append_child("Output");
    auto tStatistics = std::string("mean_plus_") + aXMLMetaData.optimization_parameters().objective_number_standard_deviations() + "_std_dev";
    auto tArgumentName = std::string("Objective Mean Plus ")
        + aXMLMetaData.optimization_parameters().objective_number_standard_deviations() + " StdDev Gradient";
    XMLGen::append_children({"Statistic", "ArgumentName"}, {tStatistics, tArgumentName}, tOuterOutput);
}
// function append_stochastic_criterion_gradient_operation
/******************************************************************************/

/******************************************************************************/
void append_stochastic_criterion_value_operation
(pugi::xml_node& aParentNode)
{
    auto tCriterionValue = aParentNode.append_child("CriterionValue");
    XMLGen::append_children({"Layout"}, {"Global"}, tCriterionValue);

    auto tOuterFor = tCriterionValue.append_child("For");
    XMLGen::append_attributes({"var", "in"}, {"PerformerIndex", "Performers"}, tOuterFor);
    auto tInnerFor = tOuterFor.append_child("For");
    XMLGen::append_attributes({"var", "in"}, {"PerformerSampleIndex", "PerformerSamples"}, tInnerFor);
    auto tInnerForInput = tInnerFor.append_child("Input");
    std::vector<std::string> tKeys = {"ArgumentName", "Probability"};
    std::vector<std::string> tValues = {"Objective Value {PerformerIndex*NumSamplesPerPerformer+PerformerSampleIndex}",
        "{Probabilities[{PerformerIndex*NumSamplesPerPerformer+PerformerSampleIndex}]}"};
    XMLGen::append_children(tKeys, tValues, tInnerForInput);

    auto tOuterOutput = tCriterionValue.append_child("Output");
    XMLGen::append_children({"Statistic", "ArgumentName"}, {"mean", "Objective Mean"}, tOuterOutput);
    tOuterOutput = tCriterionValue.append_child("Output");
    XMLGen::append_children({"Statistic", "ArgumentName"}, {"std_dev", "Objective StdDev"}, tOuterOutput);
}
// function append_stochastic_criterion_value_operation
/******************************************************************************/

/******************************************************************************/
void append_stochastic_objective_gradient_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document &aDocument)
{
    auto tOperation = aDocument.append_child("Operation");
    std::vector<std::string> tKeys = {"Function", "Name", "Layout"};
    std::vector<std::string> tValues = {"MeanPlusStdDevGradient", "Compute Non-Deterministic Objective Gradient", "Nodal Field"};
    XMLGen::append_children(tKeys, tValues, tOperation);
    XMLGen::append_stochastic_criterion_value_operation(tOperation);
    XMLGen::append_stochastic_criterion_gradient_operation(aXMLMetaData, tOperation);
}
// function append_stochastic_objective_gradient_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_qoi_statistics_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document &aDocument)
{
    for(auto &tOutputMetadata : aXMLMetaData.mOutputMetaData)
    {
        auto tOutputDataIDs = tOutputMetadata.randomIDs();
        for(auto& tOutputDataID : tOutputDataIDs)
        {
            auto tDataLayout = tOutputMetadata.randomLayout(tOutputDataID);
            auto tSupportedDataLayout = XMLGen::check_data_layout(tDataLayout);
            auto tOperationName = "compute " + tOutputDataID + " statistics";
            std::vector<std::string> tKeys = {"Function", "Name" , "Layout"};
            std::vector<std::string> tValues = { "MeanPlusStdDev", tOperationName, tSupportedDataLayout };
            auto tOperation = aDocument.append_child("Operation");
            XMLGen::append_children(tKeys, tValues, tOperation);

            auto tOuterFor = tOperation.append_child("For");
            XMLGen::append_attributes({"var", "in"}, {"PerformerIndex", "Performers"}, tOuterFor);
            auto tInnerFor = tOuterFor.append_child("For");
            XMLGen::append_attributes({"var", "in"}, {"PerformerSampleIndex", "PerformerSamples"}, tInnerFor);
            auto tInput = tInnerFor.append_child("Input");
            tKeys = {"ArgumentName", "Probability"};
            auto tArgumentName = tOutputMetadata.randomArgumentName(tOutputDataID);
            tValues = {tArgumentName, "{Probabilities[{PerformerIndex*NumSamplesPerPerformer+PerformerSampleIndex}]}"};
            XMLGen::append_children(tKeys, tValues, tInput);

            auto tOutput = tOperation.append_child("Output");
            tArgumentName = tOutputDataID + " mean";
            XMLGen::append_children({"Statistic", "ArgumentName"}, {"mean", tArgumentName}, tOutput);
            tOutput = tOperation.append_child("Output");
            tArgumentName = tOutputDataID + " standard deviation";
            XMLGen::append_children({"Statistic", "ArgumentName"}, {"std_dev", tArgumentName}, tOutput);
        }
    }
}
// function append_qoi_statistics_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_aggregate_data_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    auto tOperation = aDocument.append_child("Operation");
    XMLGen::append_children({"Name", "Function"}, {"Aggregate Data", "Aggregator"}, tOperation);
    XMLGen::Objective tObjective = aXMLMetaData.objective;
    // Aggregate values
    auto tAggregateNode = tOperation.append_child("Aggregate");
    XMLGen::append_children({"Layout"}, {"Value"}, tAggregateNode);

    int tNumEntries = tObjective.criteriaIDs.size();
    if(tObjective.multi_load_case == "true")
    {
        tNumEntries = 1;
    }
    for (int i=0; i<tNumEntries; ++i)
    {
        auto tInput = tAggregateNode.append_child("Input");
        auto tArgName = std::string("Value ") + std::to_string(i+1);
        XMLGen::append_children({"ArgumentName"}, {tArgName}, tInput);
    }
    auto tOutput = tAggregateNode.append_child("Output");
    XMLGen::append_children({"ArgumentName"}, {"Value"}, tOutput);

    // Aggregate fields
    tAggregateNode = tOperation.append_child("Aggregate");
    XMLGen::append_children({"Layout"}, {"Nodal Field"}, tAggregateNode);

    for (int i=0; i<tNumEntries; ++i)
    {
        auto tInput = tAggregateNode.append_child("Input");
        auto tArgName = std::string("Field ") + std::to_string(i+1);
        XMLGen::append_children({"ArgumentName"}, {tArgName}, tInput);
    }
    tOutput = tAggregateNode.append_child("Output");
    XMLGen::append_children({"ArgumentName"}, {"Field"}, tOutput);

    // Weighting and normalization
    bool tNeedToDoWeightingInAggregator = aXMLMetaData.needToDoWeightingInAggregator();
    auto tWeightingNode = tOperation.append_child("Weighting");
    for (int i=0; i<tNumEntries; ++i)
    {
        auto tWeight = tWeightingNode.append_child("Weight");
        if(tNeedToDoWeightingInAggregator)
        {
            XMLGen::append_children({"Value"}, {tObjective.weights[i]}, tWeight);
        }
        else
        {
            XMLGen::append_children({"Value"}, {"1.0"}, tWeight);
        }
    }
    if(aXMLMetaData.optimization_parameters().normalizeInAggregator())
    {
        auto tNormals = tWeightingNode.append_child("Normals");
        for (int i=0; i<tNumEntries; ++i)
        {
            auto tInput = tNormals.append_child("Input");
            auto tArgName = std::string("Normal ") + std::to_string(i+1);
            XMLGen::append_children({"ArgumentName"}, {tArgName}, tInput);
        }
    }
}
// function append_aggregate_data_to_plato_main_operation
/******************************************************************************/
/******************************************************************************/
void append_update_problem_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    bool tNeedUpdate = false;
    for(auto &tService : aXMLMetaData.services())
    {
        if(tService.updateProblem())
        {
            tNeedUpdate = true;
            break;
        }
    }
    if(!tNeedUpdate)
    {
        return;
    }
    auto tOperation = aDocument.append_child("Operation");
    XMLGen::append_children({"Function", "Name"}, {"Update Problem", "Update Problem"}, tOperation);
}
// function append_update_problem_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_filter_control_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    if(aXMLMetaData.optimization_parameters().filterInEngine() == true &&
       aXMLMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY)
    {
        auto tOperation = aDocument.append_child("Operation");
        XMLGen::append_children({"Function", "Name", "Gradient"}, {"Filter", "Filter Control", "False"}, tOperation);
        auto tInput = tOperation.append_child("Input");
        XMLGen::append_children({"ArgumentName"}, {"Field"}, tInput);
        auto tOutput = tOperation.append_child("Output");
        XMLGen::append_children({"ArgumentName"}, {"Filtered Field"}, tOutput);
    }
    else if(aXMLMetaData.optimization_parameters().filterInEngine() == false &&
       aXMLMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY &&
       aXMLMetaData.optimization_parameters().filter_type() == "helmholtz")
    {
        XMLGen::ValidProjectionKeys tValidKeys;
        auto tProjectionName = tValidKeys.value(aXMLMetaData.optimization_parameters().projection_type());
        if(!tProjectionName.empty())
        {
            auto tOperation = aDocument.append_child("Operation");
            XMLGen::append_children({"Function", "Name", "Gradient"}, {"Filter", "Project Control", "False"}, tOperation);
            auto tInput = tOperation.append_child("Input");
            XMLGen::append_children({"ArgumentName"}, {"Field"}, tInput);
            auto tOutput = tOperation.append_child("Output");
            XMLGen::append_children({"ArgumentName"}, {"Filtered Field"}, tOutput);
        }
    }
}
// function append_filter_control_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_initialize_geometry_operation_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    if(aXMLMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        pugi::xml_node tmp_node = aDocument.append_child("Operation");
        addChild(tmp_node, "Function", "SystemCall");
        addChild(tmp_node, "Name", "Initialize Geometry");
        addChild(tmp_node, "Command", "plato-cli geometry esp");
        addChild(tmp_node, "Argument", std::string("--input ") + aXMLMetaData.optimization_parameters().csm_file());
        addChild(tmp_node, "Argument", std::string("--output-model ") + aXMLMetaData.optimization_parameters().csm_opt_file());
        addChild(tmp_node, "Argument", std::string("--output-mesh ") + aXMLMetaData.optimization_parameters().csm_exodus_file());
        addChild(tmp_node, "Argument", std::string("--tesselation ") + aXMLMetaData.optimization_parameters().csm_tesselation_file());
        addChild(tmp_node, "Argument", std::string("--workflow ") + aXMLMetaData.optimization_parameters().esp_workflow());
    }
}
// function append_initialize_geometry_operation_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void write_cubit_journal_file_tet10_conversion(std::string fileName, const std::string& meshName, std::vector<XMLGen::Block> blockList) {
    std::ofstream outfile(fileName);

    outfile << "import mesh \"" << meshName << "\" no_geom" << std::endl;
    for(auto ib : blockList) {
        outfile << "block " << ib.block_id << " element type tetra10" << std::endl;
    }
    outfile << "set exodus netcdf4 off" << std::endl;
    outfile << "set large exodus file on" << std::endl;
    outfile << "export mesh \"" << meshName << "\" overwrite" << std::endl;

    outfile.close();
}
// function write_cubit_journal_file_tet10_conversion
/******************************************************************************/

/******************************************************************************/
void write_cubit_journal_file_subblock_from_bounding_box(std::string fileName, const std::string& meshName, std::vector<XMLGen::Block> blockList) 
{    
    if (blockList.size() != 2)
     THROWERR(std::string("write_cubit_journal_file_subblock_from_bounding_box: ") + 
              std::string("The sub_block capability is only implemented for the case where the input deck specifies 2 blocks: ") + 
              std::string("The first is the main block and the second defines properties of the sub-block to be created"))
    else
    {
        std::vector<double> tBoundingBox = blockList[0].bounding_box;
        std::ofstream outfile(fileName);
        outfile << "import mesh \"" << meshName << "\" no_geom" << std::endl;
        outfile << "delete block all" << std::endl;
        outfile << "block 2 tet with ";
        outfile << "x_coord >= " << tBoundingBox[0] << " and ";
        outfile << "y_coord >= " << tBoundingBox[1] << " and ";
        outfile << "z_coord >= " << tBoundingBox[2] << " and ";
        outfile << "x_coord <= " << tBoundingBox[3] << " and ";
        outfile << "y_coord <= " << tBoundingBox[4] << " and ";
        outfile << "z_coord <= " << tBoundingBox[5] << std::endl;
        outfile << "block 1 tet all" << std::endl;
        outfile << "set exodus netcdf4 off" << std::endl;
        outfile << "set large exodus file on" << std::endl;
        outfile << "export mesh \"" << meshName << "\" overwrite" << std::endl;
        outfile.close();
    }
}
// function write_cubit_journal_file_subblock_from_bounding_box
/******************************************************************************/

/******************************************************************************/
void append_tet10_conversion_operation_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{   
    std::string tOptions = "-batch -nographics -nogui -noecho -nojournal -nobanner -information off";

    if(aXMLMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        const std::string exodusFile(aXMLMetaData.optimization_parameters().csm_exodus_file());
        
        write_cubit_journal_file_tet10_conversion("toTet10.jou", exodusFile, aXMLMetaData.blocks);
        std::string tName = "ToTet10 On Change";  
        std::string tCommand =  std::string("cubit -input toTet10.jou ") + tOptions;
        std::string tNumParameters = std::to_string(XMLGen::get_number_of_shape_parameters(aXMLMetaData));
        append_cubit_systemcall_operation_commands(aDocument,tName,tCommand,tNumParameters);
    }
    else if(aXMLMetaData.optimization_parameters().optimizationType() == OT_DAKOTA)
    {
        auto tEvaluations = std::stoi(aXMLMetaData.optimization_parameters().concurrent_evaluations());
        for (int iEvaluation = 0; iEvaluation < tEvaluations; iEvaluation++)
        {
            std::string tTag = std::string("_") + std::to_string(iEvaluation);
            const std::string exodusFile = XMLGen::append_concurrent_tag_to_file_string(aXMLMetaData.optimization_parameters().csm_exodus_file(),tTag);
            
            write_cubit_journal_file_tet10_conversion("evaluations" + tTag + "/toTet10.jou", exodusFile, aXMLMetaData.blocks);
            std::string tName = std::string("convert_to_tet10") + tTag;
            std::string tCommand = std::string("cd evaluations") + tTag + std::string("; cubit -input toTet10.jou ") + tOptions;
            std::string tNumParameters = std::to_string(XMLGen::get_number_of_shape_parameters(aXMLMetaData));
            append_cubit_systemcall_operation_commands(aDocument,tName,tCommand,tNumParameters);
        }
    }
}
// function append_tet10_conversion_operation_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_subblock_creation_operation_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{   
    std::string tOptions = "-batch -nographics -nogui -noecho -nojournal -nobanner -information off";

    if(aXMLMetaData.optimization_parameters().optimizationType() == OT_DAKOTA)
    {
        auto tEvaluations = std::stoi(aXMLMetaData.optimization_parameters().concurrent_evaluations());
        for (int iEvaluation = 0; iEvaluation < tEvaluations; iEvaluation++)
        {
            std::string tTag = std::string("_") + std::to_string(iEvaluation);
            const std::string exodusFile = XMLGen::append_concurrent_tag_to_file_string(aXMLMetaData.optimization_parameters().csm_exodus_file(),tTag);
            
            write_cubit_journal_file_subblock_from_bounding_box("evaluations" + tTag + "/subBlock.jou", exodusFile, aXMLMetaData.blocks);
            std::string tName = std::string("create_sub_block") + tTag;
            std::string tCommand = std::string("cd evaluations") + tTag + std::string("; cubit -input subBlock.jou ") + tOptions;
            std::string tNumParameters = std::to_string(XMLGen::get_number_of_shape_parameters(aXMLMetaData));
            append_cubit_systemcall_operation_commands(aDocument,tName,tCommand,tNumParameters);
        }
    }
}
// function append_subblock_creation_operation_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_cubit_systemcall_operation_commands
(pugi::xml_document& aDocument, 
 const std::string &aName, 
 const std::string &aCommand,
 const std::string &aNumParameters)
{
    pugi::xml_node operationNode = aDocument.append_child("Operation");
    addChild(operationNode, "Function", "SystemCall");
    addChild(operationNode, "Name", aName);
    addChild(operationNode, "Command", aCommand);
    addChild(operationNode, "OnChange", "true");
    addChild(operationNode, "AppendInput", "false");
    auto tInputNode = operationNode.append_child("Input");
    XMLGen::append_children({"ArgumentName", "Layout", "Size"}, {"Parameters", "scalar", aNumParameters}, tInputNode);
}
// function append_cubit_systemcall_operation_commands
/******************************************************************************/

/******************************************************************************/
void append_mesh_join_operation_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    if(aXMLMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        const std::string exodusFile(aXMLMetaData.optimization_parameters().csm_exodus_file());
        const std::vector<XMLGen::Block> blockList(aXMLMetaData.blocks);

        const std::string auxiliaryMeshFile(aXMLMetaData.mesh.auxiliary_mesh_name);
        const std::string joinedMeshFile(aXMLMetaData.mesh.joined_mesh_name);

        pugi::xml_node operationNode = aDocument.append_child("Operation");
        addChild(operationNode, "Function", "SystemCallMPI");
        addChild(operationNode, "Name", "JoinMesh On Change");
        addChild(operationNode, "Command", "ejoin");
        addChild(operationNode, "OnChange", "true");
        std::string tNumRanks = "1";
        for (const auto& iService : aXMLMetaData.services())
        {
          if(iService.code() == "platomain")
          {
            tNumRanks = iService.numberProcessors();
          }
        }
        addChild(operationNode, "NumRanks", tNumRanks);
        addChild(operationNode, "Argument", "-output");
        addChild(operationNode, "Argument", joinedMeshFile);
        addChild(operationNode, "Argument", exodusFile);
        addChild(operationNode, "Argument", auxiliaryMeshFile);
        addChild(operationNode, "AppendInput", "false");
        auto tInputNode = operationNode.append_child("Input");
        std::string tNumParameters = std::to_string(XMLGen::get_number_of_shape_parameters(aXMLMetaData));
        XMLGen::append_children({"ArgumentName", "Layout", "Size"}, {"Parameters", "scalar", tNumParameters}, tInputNode);
    }
}
// function append_mesh_join_operation_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_mesh_rename_operation_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    if(aXMLMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        const std::string exodusFile(aXMLMetaData.optimization_parameters().csm_exodus_file());
        const std::vector<XMLGen::Block> blockList(aXMLMetaData.blocks);

        const std::string joinedMeshFile(aXMLMetaData.mesh.joined_mesh_name);

        std::stringstream moveCmd;
        moveCmd << "while lsof -u `id -u -n` | grep " << joinedMeshFile << "; do sleep 1; done; ";
        moveCmd << "/bin/cp " << joinedMeshFile << " " << exodusFile;

        pugi::xml_node operationNode = aDocument.append_child("Operation");
        addChild(operationNode, "Function", "SystemCall");
        addChild(operationNode, "Name", "RenameMesh On Change");
        addChild(operationNode, "Command", moveCmd.str());
        addChild(operationNode, "OnChange", "true");
        addChild(operationNode, "AppendInput", "false");
        auto tInputNode = operationNode.append_child("Input");
        std::string tNumParameters = std::to_string(XMLGen::get_number_of_shape_parameters(aXMLMetaData));
        XMLGen::append_children({"ArgumentName", "Layout", "Size"}, {"Parameters", "scalar", tNumParameters}, tInputNode);
    }
}
// function append_mesh_rename_operation_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_update_geometry_on_change_operation_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    if(aXMLMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        pugi::xml_node tmp_node = aDocument.append_child("Operation");
        XMLGen::append_update_geometry_on_change_operation_commands(aXMLMetaData, tmp_node, "Update Geometry on Change", "plato-cli geometry esp", "");
    }
    else if(aXMLMetaData.optimization_parameters().optimizationType() == OT_DAKOTA)
    {
        auto tEvaluations = std::stoi(aXMLMetaData.optimization_parameters().concurrent_evaluations());
        for (int iEvaluation = 0; iEvaluation < tEvaluations; iEvaluation++)
        {
            pugi::xml_node tmp_node = aDocument.append_child("Operation");
            std::string tTag = std::string("_") + std::to_string(iEvaluation);
            std::string tName = std::string("update_geometry_on_change") + tTag;
            std::string tCommand = std::string("cd evaluations") + tTag + std::string("; plato-cli geometry esp");
            XMLGen::append_update_geometry_on_change_operation_commands(aXMLMetaData, tmp_node, tName, tCommand, tTag);
        }
    }
}
// function append_update_geometry_on_change_operation_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_update_geometry_on_change_operation_commands
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node& aParentNode,
 const std::string& aName,
 const std::string& aCommand,
 const std::string& aTag)
{
    addChild(aParentNode, "Function", "SystemCall");
    addChild(aParentNode, "Name", aName);
    addChild(aParentNode, "Command", aCommand);
    addChild(aParentNode, "OnChange", "true");
    addChild(aParentNode, "Argument", std::string("--input ") + XMLGen::append_concurrent_tag_to_file_string(aXMLMetaData.optimization_parameters().csm_file(),aTag));
    addChild(aParentNode, "Argument", std::string("--output-model ") + XMLGen::append_concurrent_tag_to_file_string(aXMLMetaData.optimization_parameters().csm_opt_file(),aTag));
    addChild(aParentNode, "Argument", std::string("--output-mesh ") + XMLGen::append_concurrent_tag_to_file_string(aXMLMetaData.optimization_parameters().csm_exodus_file(),aTag));
    addChild(aParentNode, "Argument", std::string("--tesselation ") + XMLGen::append_concurrent_tag_to_file_string(aXMLMetaData.optimization_parameters().csm_tesselation_file(),aTag));
    addChild(aParentNode, "Argument", std::string("--workflow ") + aXMLMetaData.optimization_parameters().esp_workflow());
    if (Plato::tolower(aXMLMetaData.optimization_parameters().mesh_morph()) == "true")
    {
        addChild(aParentNode, "Argument", std::string("--morph true"));
        addChild(aParentNode, "Argument", std::string("--precision 16"));
    }
    addChild(aParentNode, "Argument", "--parameters");
    addChild(aParentNode, "AppendInput", "true");
    pugi::xml_node aInputNode = aParentNode.append_child("Input");
    addChild(aInputNode, "ArgumentName", "Parameters");
    addChild(aInputNode, "Layout", "scalar");
    addChild(aInputNode, "Size", std::to_string(XMLGen::get_number_of_shape_parameters(aXMLMetaData)));
}
// function append_update_geometry_on_change_operation_commands
/******************************************************************************/

/******************************************************************************/
void append_reinitialize_operation_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    if(aXMLMetaData.optimization_parameters().optimizationType() == OT_DAKOTA)
    {
        pugi::xml_node tOperation = aDocument.append_child("Operation");
        XMLGen::append_children({"Function", "Name"}, {"Reinitialize", "reinitialize_on_change_plato_services"}, tOperation);
    }
}
// function append_reinitialize_operation_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_csm_mesh_output_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    if(aXMLMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        pugi::xml_node tmp_node = aDocument.append_child("Operation");
        addChild(tmp_node, "Function", "CSMMeshOutput");
        addChild(tmp_node, "Name", "CSMMeshOutput");
        addChild(tmp_node, "BaseMeshName", aXMLMetaData.optimization_parameters().csm_exodus_file());
        addChild(tmp_node, "OutputFrequency", aXMLMetaData.optimization_parameters().output_frequency());
        addChild(tmp_node, "MaxIterations", aXMLMetaData.optimization_parameters().max_iterations());
    }
}
// function append_csm_mesh_output_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_filter_gradient_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    if(aXMLMetaData.optimization_parameters().filterInEngine() == true &&
       aXMLMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY)
    {
        auto tOperation = aDocument.append_child("Operation");
        XMLGen::append_children({"Function", "Name", "Gradient"}, {"Filter", "Filter Gradient", "True"}, tOperation);
        auto tInput = tOperation.append_child("Input");
        XMLGen::append_children({"ArgumentName"}, {"Field"}, tInput);
        tInput = tOperation.append_child("Input");
        XMLGen::append_children({"ArgumentName"}, {"Gradient"}, tInput);
        auto tOutput = tOperation.append_child("Output");
        XMLGen::append_children({"ArgumentName"}, {"Filtered Gradient"}, tOutput);
    }
    else if(aXMLMetaData.optimization_parameters().filterInEngine() == false &&
       aXMLMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY &&
       aXMLMetaData.optimization_parameters().filter_type() == "helmholtz")
    {
        XMLGen::ValidProjectionKeys tValidKeys;
        auto tProjectionName = tValidKeys.value(aXMLMetaData.optimization_parameters().projection_type());
        if(!tProjectionName.empty())
        {
            auto tOperation = aDocument.append_child("Operation");
            XMLGen::append_children({"Function", "Name", "Gradient"}, {"Filter", "Project Gradient", "True"}, tOperation);
            auto tInput = tOperation.append_child("Input");
            XMLGen::append_children({"ArgumentName"}, {"Field"}, tInput);
            tInput = tOperation.append_child("Input");
            XMLGen::append_children({"ArgumentName"}, {"Gradient"}, tInput);
            auto tOutput = tOperation.append_child("Output");
            XMLGen::append_children({"ArgumentName"}, {"Filtered Gradient"}, tOutput);
        }
    }
}
// function append_filter_gradient_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_initialize_density_field_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    auto tOperation = aDocument.append_child("Operation");
    XMLGen::append_children({"Function", "Name", "Method"}, {"InitializeField", "Initialize Field", "Uniform"}, tOperation);

    auto tMethod = tOperation.append_child("Uniform");
    std::string tInitialDensityValue = aXMLMetaData.optimization_parameters().initial_density_value();
    auto tValue = tInitialDensityValue.empty() ? "0.5" : tInitialDensityValue;
    XMLGen::append_children({"Value"}, {tValue}, tMethod);

    auto tOutput = tOperation.append_child("Output");
    XMLGen::append_children({"ArgumentName"}, {"Initialized Field"}, tOutput);
}
// function append_initialize_density_field_operation
/******************************************************************************/

/******************************************************************************/
void append_initialize_field_from_file_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    if(aXMLMetaData.optimization_parameters().initial_guess_file_name().empty())
    {
        THROWERR(std::string("Append Initialize Field From File Operation: ")
            + "Initial guess was supposed to be initialized by reading it from an user-specified file. "
            + "However, the 'filename' keyword is empty.")
    }

    if(aXMLMetaData.optimization_parameters().initial_guess_field_name().empty())
    {
        THROWERR(std::string("Append Initialize Field From File Operation: ")
            + "Initial guess was supposed to be initialized by reading it from an user-specified field. "
            + "However, the field's 'name' keyword is empty.")
    }

    auto tOperation = aDocument.append_child("Operation");
    std::vector<std::string> tKeys = {"Function", "Name", "Method"};
    std::vector<std::string> tValues = {"InitializeField", "Initialize Field", "FromFieldOnInputMesh"};
    XMLGen::append_children(tKeys, tValues, tOperation);

    auto tMethod = tOperation.append_child("FromFieldOnInputMesh");
    tKeys = {"Name", "VariableName", "Iteration"};
    tValues = {aXMLMetaData.optimization_parameters().initial_guess_file_name(), aXMLMetaData.optimization_parameters().initial_guess_field_name(), aXMLMetaData.optimization_parameters().restart_iteration()};
    XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
    XMLGen::append_children(tKeys, tValues, tMethod);
}
// function append_initialize_field_from_file_operation
/******************************************************************************/

/******************************************************************************/
void append_initialize_field_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    XMLGen::ValidDiscretizationKeys tValidKeys;
    auto tValue = tValidKeys.value(aXMLMetaData.optimization_parameters().discretization());
    if(tValue.empty())
    {
        THROWERR(std::string("Append Initialize Field to Plato Main Operation: ") + "Discretization method '" + tValue + "' is not supported.")
    }

    if(tValue.compare("density") == 0)
    {
        XMLGen::append_initialize_density_field_operation(aXMLMetaData, aDocument);
    }
}
// function append_initialize_field_operation
/******************************************************************************/

/******************************************************************************/
void append_initialize_field_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    if(aXMLMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        append_initialize_data_for_shape_problem(aXMLMetaData, aDocument);
    }
    else if(aXMLMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY)
    {
        append_initialize_data_for_topology_problem(aXMLMetaData, aDocument);
    }
}
// function append_initialize_field_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_initialize_data_for_topology_problem
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    if(!aXMLMetaData.optimization_parameters().isARestartRun())
    {
        XMLGen::append_initialize_field_operation(aXMLMetaData, aDocument);
    }
    else
    {
        XMLGen::append_initialize_field_from_file_operation(aXMLMetaData, aDocument);
    }
}
// function append_initialize_data_for_topology_problem
/******************************************************************************/

/******************************************************************************/
void append_initialize_data_for_shape_problem
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    pugi::xml_node tOperation = aDocument.append_child("Operation");
    addChild(tOperation, "Function", "InitializeValues");
    addChild(tOperation, "Name", "Initialize Values");
    pugi::xml_node tOutput = tOperation.append_child("Output");
    addChild(tOutput, "ArgumentName", "Values");
    addChild(tOperation, "Method", "ReadFromCSMFile");
    addChild(tOperation, "CSMFileName", aXMLMetaData.optimization_parameters().csm_file());
}
// function append_initialize_data_for_topology_problem
/******************************************************************************/

/******************************************************************************/
void append_design_volume_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    if(XMLGen::is_volume_constraint_defined(aXMLMetaData))
    {
        auto tOperation = aDocument.append_child("Operation");
        XMLGen::append_children({"Function", "Name"}, {"DesignVolume", "Design Volume"}, tOperation);
        auto tOutput = tOperation.append_child("Output");
        XMLGen::append_children({"ArgumentName"}, {"Design Volume"}, tOutput);
    }
}
// function append_design_volume_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_compute_volume_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    if(XMLGen::is_volume_constraint_defined_and_computed_by_platomain(aXMLMetaData))
    {
        std::string tConstraintID = XMLGen::get_platomain_volume_constraint_id(aXMLMetaData);

        auto tOperation = aDocument.append_child("Operation");
        std::vector<std::string> tKeys = {"Function", "Name", "PenaltyModel"};
        std::vector<std::string> tValues = {"ComputeVolume", "Compute Constraint Value " + tConstraintID, "SIMP"};
        XMLGen::append_children(tKeys, tValues, tOperation);

        tKeys = {"ArgumentName"}; tValues = {"Topology"};
        auto tInput = tOperation.append_child("Input");
        XMLGen::append_children(tKeys, tValues, tInput);

        tKeys = {"ArgumentName"}; tValues = {"Volume"};
        auto tOutput = tOperation.append_child("Output");
        XMLGen::append_children(tKeys, tValues, tOutput);

        tKeys = {"ArgumentName"}; tValues = {"Volume Gradient"};
        tOutput = tOperation.append_child("Output");
        XMLGen::append_children(tKeys, tValues, tOutput);

        tKeys = {"PenaltyExponent", "MinimumValue"}; tValues = {"1.0", "0.0"};
        auto tSIMP = tOperation.append_child("SIMP");
        XMLGen::append_children(tKeys, tValues, tSIMP);
    }
}
// function append_compute_volume_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_compute_volume_gradient_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    if(XMLGen::is_volume_constraint_defined_and_computed_by_platomain(aXMLMetaData))
    {
        std::string tConstraintID = XMLGen::get_platomain_volume_constraint_id(aXMLMetaData);

        auto tOperation = aDocument.append_child("Operation");
        std::vector<std::string> tKeys = {"Function", "Name", "PenaltyModel"};
        std::vector<std::string> tValues = {"ComputeVolume", "Compute Constraint Gradient " + tConstraintID, "SIMP"};
        XMLGen::append_children(tKeys, tValues, tOperation);

        tKeys = {"ArgumentName"}; tValues = {"Topology"};
        auto tInput = tOperation.append_child("Input");
        XMLGen::append_children(tKeys, tValues, tInput);

        tKeys = {"ArgumentName"}; tValues = {"Volume"};
        auto tOutput = tOperation.append_child("Output");
        XMLGen::append_children(tKeys, tValues, tOutput);

        tKeys = {"ArgumentName"}; tValues = {"Volume Gradient"};
        tOutput = tOperation.append_child("Output");
        XMLGen::append_children(tKeys, tValues, tOutput);

        tKeys = {"PenaltyExponent", "MinimumValue"}; tValues = {"1.0", "0.0"};
        auto tSIMP = tOperation.append_child("SIMP");
        XMLGen::append_children(tKeys, tValues, tSIMP);
    }
}
// function append_compute_volume_gradient_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_compute_volume_criterion_value_operation_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    std::string tIdentifierString = "";
    if (get_volume_criterion_defined_and_computed_by_platomain(aXMLMetaData, tIdentifierString))
    {
        auto tOperation = aDocument.append_child("Operation");
        std::vector<std::string> tKeys = {"Function", "Name"};
        std::vector<std::string> tValues = {"DesignVolume", std::string("Compute Criterion Value - ") + tIdentifierString};
        XMLGen::append_children(tKeys, tValues, tOperation);

        tKeys = {"ArgumentName"}; tValues = {tIdentifierString + std::string(" value")};
        auto tOutput = tOperation.append_child("Output");
        XMLGen::append_children(tKeys, tValues, tOutput);
    }
}
// function append_compute_volume_criterion_value_operation_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_decomp_operations_for_physics_performers_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    auto tDecompServiceID = XMLGen::get_unique_decomp_service(aXMLMetaData);
    if (tDecompServiceID.size() > 0)
    {
        auto tService = aXMLMetaData.service(tDecompServiceID);
        auto tEvaluations = std::stoi(aXMLMetaData.optimization_parameters().concurrent_evaluations());
        for (int iEvaluation = 0; iEvaluation < tEvaluations; iEvaluation++)
            append_decomp_operations(aXMLMetaData, aDocument, tService, iEvaluation);
    }
}
// function append_decomp_operations_for_physics_performers_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_decomp_operations
(const XMLGen::InputData&,
 pugi::xml_document&,
 const XMLGen::Service&,
 int)
{
    THROWERR("Dakota with multiple CPUs per physics performer is not supported.");
}
// function append_decomp_operations
/******************************************************************************/

/******************************************************************************/
void append_fixed_blocks_identification_numbers_to_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node& aParentNode)
{
    const auto& tOptParams = aXMLMetaData.optimization_parameters();
    if(tOptParams.fixed_block_ids().size() > 0)
    {
        XMLGen::FixedBlock::check_fixed_block_arrays(tOptParams);

        auto tFixedBlockIDs = tOptParams.fixed_block_ids();
        auto tDomainValues = tOptParams.fixed_block_domain_values();
        auto tBoundaryValues = tOptParams.fixed_block_boundary_values();
        auto tMaterialStates = tOptParams.fixed_block_material_states();

        for(auto& tID : tFixedBlockIDs)
        {
            auto tIndex = &tID - &tFixedBlockIDs[0];
            auto tFixedBlocks = aParentNode.append_child("FixedBlocks");
            XMLGen::append_children({"Index"}, {tID}, tFixedBlocks);
            XMLGen::append_children({"DomainValue"}, {tDomainValues[tIndex]}, tFixedBlocks);
            XMLGen::append_children({"BoundaryValue"}, {tBoundaryValues[tIndex]}, tFixedBlocks);
            XMLGen::append_children({"MaterialState"}, {tMaterialStates[tIndex]}, tFixedBlocks);
        }
    }
}
// function append_fixed_blocks_identification_numbers_to_operation
/******************************************************************************/

/******************************************************************************/
void append_fixed_sidesets_identification_numbers_to_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node& aParentNode)
{
    if(aXMLMetaData.optimization_parameters().fixed_sideset_ids().size() > 0)
    {
        auto tFixedSideSet = aParentNode.append_child("FixedSidesets");
        for(auto& tID : aXMLMetaData.optimization_parameters().fixed_sideset_ids())
        {
            XMLGen::append_children({"Index"}, {tID}, tFixedSideSet);
        }
    }
}
// function append_fixed_sidesets_identification_numbers_to_operation
/******************************************************************************/

/******************************************************************************/
void append_fixed_nodesets_identification_numbers_to_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node& aParentNode)
{
    if(aXMLMetaData.optimization_parameters().fixed_nodeset_ids().size() > 0)
    {
        auto tFixedNodeSet = aParentNode.append_child("FixedNodesets");
        for(auto& tID : aXMLMetaData.optimization_parameters().fixed_nodeset_ids())
        {
            XMLGen::append_children({"Index"}, {tID}, tFixedNodeSet);
        }
    }
}
// function append_fixed_nodesets_identification_numbers_to_operation
/******************************************************************************/

/******************************************************************************/
void append_set_lower_bounds_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    if(aXMLMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY )
    {
        // TODO: THIS CODE ASSUMES THAT ONLY ONE SCENARIO BLOCK IS DEFINED AND WILL NOT WORK 
        // IF MULTIPLE ANALYZE SCENARIO BLOCKS ARE DEFINED. THIS CODE WILL NEED REFACTORING 
        // IF THIS USE CASE IS NEEDED IN THE FUTURE.
        XMLGen::ValidPhysicsKeys tValidPhysicsKeys;
        auto tPhysics = aXMLMetaData.scenario(0).physics();
        auto tMainMaterialStateForApplication = tValidPhysicsKeys.material_state(tPhysics);

        auto tOperation = aDocument.append_child("Operation");
        auto tDiscretization = aXMLMetaData.optimization_parameters().discretization();
        std::vector<std::string> tKeys = {"Function", "Name", "UseCase", "Discretization"};
        std::vector<std::string> tValues = {"SetLowerBounds", "Compute Lower Bounds", tMainMaterialStateForApplication, tDiscretization};
        XMLGen::append_children(tKeys, tValues, tOperation);

        auto tInput = tOperation.append_child("Input");
        XMLGen::append_children({"ArgumentName"}, {"Lower Bound Value"}, tInput);
        auto tOutput = tOperation.append_child("Output");
        XMLGen::append_children({"ArgumentName"}, {"Lower Bound Vector"}, tOutput);

        XMLGen::append_fixed_blocks_identification_numbers_to_operation(aXMLMetaData, tOperation);
        XMLGen::append_fixed_sidesets_identification_numbers_to_operation(aXMLMetaData, tOperation);
        XMLGen::append_fixed_nodesets_identification_numbers_to_operation(aXMLMetaData, tOperation);
    }
    else if(aXMLMetaData.optimization_parameters().optimizationType() == OT_SHAPE )
    {
        auto tOperation = aDocument.append_child("Operation");
        addChild(tOperation, "Function", "InitializeValues");
        addChild(tOperation, "Name", "Compute Lower Bounds");
        pugi::xml_node tOutput = tOperation.append_child("Output");
        addChild(tOutput, "ArgumentName", "Lower Bounds");
        addChild(tOperation, "Method", "ReadFromCSMFile");
        addChild(tOperation, "CSMFileName", aXMLMetaData.optimization_parameters().csm_file());
    }
}
// function append_set_lower_bounds_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_copy_field_to_plato_main_operation
(const XMLGen::InputData& /*aXMLMetaData*/,
 pugi::xml_document& aDocument)
{
    auto tOperation = aDocument.append_child("Operation");
    std::vector<std::string> tKeys = {"Function", "Name"};
    std::vector<std::string> tValues = {"CopyField", "Copy Field"};
    XMLGen::append_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.append_child("Input");
    XMLGen::append_children({"ArgumentName"}, {"InputField"}, tInput);
    auto tOutput = tOperation.append_child("Output");
    XMLGen::append_children({"ArgumentName"}, {"OutputField"}, tOutput);
}
// function append_copy_field_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_copy_value_to_plato_main_operation
(const XMLGen::InputData& /*aXMLMetaData*/,
 pugi::xml_document& aDocument)
{
    auto tOperation = aDocument.append_child("Operation");
    std::vector<std::string> tKeys = {"Function", "Name"};
    std::vector<std::string> tValues = {"CopyValue", "Copy Value"};
    XMLGen::append_children(tKeys, tValues, tOperation);

    auto tInput = tOperation.append_child("Input");
    XMLGen::append_children({"ArgumentName"}, {"InputValue"}, tInput);
    auto tOutput = tOperation.append_child("Output");
    XMLGen::append_children({"ArgumentName"}, {"OutputValue"}, tOutput);
}
// function append_copy_value_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_negate_value_to_plato_main_operation
(const XMLGen::InputData& /*aXMLMetaData*/,
 pugi::xml_document& aDocument)
{
    auto tOperation = aDocument.append_child("Operation");
    std::vector<std::string> tKeys = {"Function", "Name"};
    std::vector<std::string> tValues = {"Aggregator", "Negate Value"};
    XMLGen::append_children(tKeys, tValues, tOperation);

    auto tAggregate = tOperation.append_child("Aggregate");
    XMLGen::append_children({"Layout"}, {"Value"}, tAggregate);
    auto tInput = tAggregate.append_child("Input");
    XMLGen::append_children({"ArgumentName"}, {"InputValue"}, tInput);
    auto tOutput = tAggregate.append_child("Output");
    XMLGen::append_children({"ArgumentName"}, {"OutputValue"}, tOutput);

    auto tWeighting = tOperation.append_child("Weighting");
    auto tWeight = tWeighting.append_child("Weight");
    XMLGen::append_children({"Value"}, {"-1.0"}, tWeight);
}
// function append_negate_value_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_negate_field_to_plato_main_operation
(const XMLGen::InputData& /*aXMLMetaData*/,
 pugi::xml_document& aDocument)
{
    auto tOperation = aDocument.append_child("Operation");
    std::vector<std::string> tKeys = {"Function", "Name"};
    std::vector<std::string> tValues = {"Aggregator", "Negate Field"};
    XMLGen::append_children(tKeys, tValues, tOperation);

    auto tAggregate = tOperation.append_child("Aggregate");
    XMLGen::append_children({"Layout"}, {"Nodal Field"}, tAggregate);
    auto tInput = tAggregate.append_child("Input");
    XMLGen::append_children({"ArgumentName"}, {"InputField"}, tInput);
    auto tOutput = tAggregate.append_child("Output");
    XMLGen::append_children({"ArgumentName"}, {"OutputField"}, tOutput);

    auto tWeighting = tOperation.append_child("Weighting");
    auto tWeight = tWeighting.append_child("Weight");
    XMLGen::append_children({"Value"}, {"-1.0"}, tWeight);
}
// function append_negate_value_to_plato_main_operation
/******************************************************************************/

/******************************************************************************/
void append_set_upper_bounds_to_plato_main_operation
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_document& aDocument)
{
    if(aXMLMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY)
    {
        // TODO: THIS CODE ASSUMES THAT ONLY ONE SCENARIO BLOCK IS DEFINED AND WILL NOT WORK 
        // IF MULTIPLE ANALYZE SCENARIO BLOCKS ARE DEFINED. THIS CODE WILL NEED REFACTORING 
        // IF THIS USE CASE IS NEEDED IN THE FUTURE.
        XMLGen::ValidPhysicsKeys tValidPhysicsKeys;
        auto tPhysics = aXMLMetaData.scenario(0).physics();
        auto tMainMaterialStateForApplication = tValidPhysicsKeys.material_state(tPhysics);

        auto tOperation = aDocument.append_child("Operation");
        auto tDiscretization = aXMLMetaData.optimization_parameters().discretization();
        std::vector<std::string> tKeys = {"Function", "Name", "UseCase", "Discretization"};
        std::vector<std::string> tValues = {"SetUpperBounds", "Compute Upper Bounds", tMainMaterialStateForApplication, tDiscretization};
        XMLGen::append_children(tKeys, tValues, tOperation);

        auto tInput = tOperation.append_child("Input");
        XMLGen::append_children({"ArgumentName"}, {"Upper Bound Value"}, tInput);
        auto tOutput = tOperation.append_child("Output");
        XMLGen::append_children({"ArgumentName"}, {"Upper Bound Vector"}, tOutput);

        XMLGen::append_fixed_blocks_identification_numbers_to_operation(aXMLMetaData, tOperation);
        XMLGen::append_fixed_sidesets_identification_numbers_to_operation(aXMLMetaData, tOperation);
        XMLGen::append_fixed_nodesets_identification_numbers_to_operation(aXMLMetaData, tOperation);
    }
    else if(aXMLMetaData.optimization_parameters().optimizationType() == OT_SHAPE )
    {
        auto tOperation = aDocument.append_child("Operation");
        addChild(tOperation, "Function", "InitializeValues");
        addChild(tOperation, "Name", "Compute Upper Bounds");
        pugi::xml_node tOutput = tOperation.append_child("Output");
        addChild(tOutput, "ArgumentName", "Upper Bounds");
        addChild(tOperation, "Method", "ReadFromCSMFile");
        addChild(tOperation, "CSMFileName", aXMLMetaData.optimization_parameters().csm_file());
    }
}
// function append_set_upper_bounds_to_plato_main_operation
/******************************************************************************/

}
// namespace XMLGen
