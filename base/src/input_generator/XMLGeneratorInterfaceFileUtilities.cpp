/*
 * XMLGeneratorInterfaceFileUtilities.cpp
 *
 *  Created on: May 26, 2020
 */

#include <tuple>

#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorValidInputKeys.hpp"
#include "XMLGeneratorStagesUtilities.hpp"
#include "XMLGeneratorParserUtilities.hpp"
#include "XMLGeneratorSharedDataUtilities.hpp"
#include "XMLGeneratorPerformersUtilities.hpp"
#include "XMLGeneratorInterfaceFileUtilities.hpp"
#include "XMLGeneratorGradBasedOptimizerOptions.hpp"
#include "XMLGeneratorSierraSDInputDeckUtilities.hpp"
#include "XMLGeneratorRandomInterfaceFileUtilities.hpp"
#include "XMLGeneratorPlatoMainConstraintValueOperationInterface.hpp"
#include "XMLGeneratorPlatoMainConstraintGradientOperationInterface.hpp"

namespace XMLGen
{

/******************************************************************************/
void write_interface_xml_file
(const XMLGen::InputData& aMetaData)
{
    pugi::xml_document tDocument;

    XMLGen::append_include_defines_xml_data(aMetaData, tDocument);
    XMLGen::append_console_data(aMetaData, tDocument);
    XMLGen::append_performer_data(aMetaData, tDocument);
    XMLGen::append_shared_data(aMetaData, tDocument);
    XMLGen::append_stages(aMetaData, tDocument);
    XMLGen::append_method_options(aMetaData, tDocument);

    tDocument.save_file("interface.xml", "  ");
}
/******************************************************************************/

/******************************************************************************/
void append_performer_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    XMLGen::append_plato_main_performer(aMetaData, aParentNode);
    // note: multiperformer use case currently only works with Plato Analyze, and is only used currently with the robust optimization workflow
    if(XMLGen::is_robust_optimization_problem(aMetaData))
    {
        XMLGen::append_multiperformer_usecase(aMetaData, aParentNode);
    }
    else
    {
        XMLGen::append_uniperformer_usecase(aMetaData, aParentNode);
    }
    
}
/******************************************************************************/

/******************************************************************************/
void append_console_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    auto tNode = aParentNode.append_child("Console");
    XMLGen::append_children({"Enabled"}, {"true"}, tNode);
    XMLGen::append_children({"Verbose"}, {aMetaData.optimization_parameters().verbose()}, tNode);
}
/******************************************************************************/

/******************************************************************************/
void append_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    XMLGen::append_design_variables_shared_data(aMetaData, aDocument);
    XMLGen::append_lower_bounds_shared_data(aMetaData, aDocument);
    XMLGen::append_upper_bounds_shared_data(aMetaData, aDocument);
    XMLGen::append_design_volume_shared_data(aMetaData, aDocument);
    XMLGen::append_initial_control_shared_data(aMetaData, aDocument);
    XMLGen::append_parameter_sensitivity_shared_data(aMetaData, aDocument);
    XMLGen::append_gradient_based_criterion_shared_data(aMetaData, aDocument);
    XMLGen::append_gradient_based_constraint_shared_data(aMetaData, aDocument);
    XMLGen::append_gradient_based_objective_shared_data(aMetaData, aDocument);
    XMLGen::append_normalization_shared_data(aMetaData, aDocument);

    if(XMLGen::is_robust_optimization_problem(aMetaData))
    {
        XMLGen::append_qoi_statistics_shared_data(aMetaData, aDocument);
        XMLGen::append_multiperformer_qoi_shared_data(aMetaData, aDocument);
        XMLGen::append_multiperformer_topology_shared_data(aMetaData, aDocument);
        XMLGen::append_multiperformer_criterion_shared_data("Objective", aMetaData, aDocument);
    }
    else
    {
        XMLGen::append_qoi_shared_data(aMetaData, aDocument);
        XMLGen::append_topology_shared_data(aMetaData, aDocument);
    }
}
/******************************************************************************/

/******************************************************************************/
void append_stages
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    // deterministic stages
    XMLGen::append_design_volume_stage(aMetaData, aDocument);
    XMLGen::append_initial_guess_stage(aMetaData, aDocument);
    XMLGen::append_lower_bound_stage(aMetaData, aDocument);
    XMLGen::append_upper_bound_stage(aMetaData, aDocument);
    XMLGen::append_plato_main_output_stage(aMetaData, aDocument);

    // nondeterministic stages
    if(XMLGen::is_robust_optimization_problem(aMetaData))
    {
        XMLGen::append_cache_state_stage_for_nondeterministic_usecase(aMetaData, aDocument);
        XMLGen::append_update_problem_stage_for_nondeterministic_usecase(aMetaData, aDocument);
    }
    else
    {
        XMLGen::append_cache_state_stage(aMetaData, aDocument);
        XMLGen::append_update_problem_stage(aMetaData, aDocument);
    }

    // constraint stages
    XMLGen::append_constraint_value_stage(aMetaData, aDocument);
    XMLGen::append_constraint_gradient_stage(aMetaData, aDocument);

    // objective stages
    XMLGen::append_objective_value_stage(aMetaData, aDocument);
    XMLGen::append_objective_gradient_stage(aMetaData, aDocument);
}
/******************************************************************************/

/******************************************************************************/
void append_method_options
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    auto tOptimizerNode = aParentNode.append_child("Optimizer");
    XMLGen::append_grad_based_optimizer_options(aMetaData, tOptimizerNode);
    XMLGen::append_optimizer_update_problem_stage_options(aMetaData, tOptimizerNode);
    XMLGen::append_optimizer_cache_stage_options(aMetaData, tOptimizerNode);
    XMLGen::append_grad_based_optimizer_output_options(aMetaData, tOptimizerNode);
    XMLGen::append_grad_based_optimizer_variables_options(aMetaData, tOptimizerNode);
    XMLGen::append_grad_based_optimizer_objective_options(aMetaData, tOptimizerNode);
    XMLGen::append_grad_based_optimizer_constraint_options(aMetaData, tOptimizerNode);
    XMLGen::append_bound_constraints_options_topology_optimization({"1.0", "0.0"}, tOptimizerNode);
}
// function append_method_options
/******************************************************************************/

/******************************************************************************/
int append_platoservice
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument,
 int aBasePerformerID)
{
    auto tCummulativeID = aBasePerformerID;
    std::vector<std::string> tKeywords = { "Name", "Code", "PerformerID" };
    auto tEvaluations = std::stoi(aMetaData.optimization_parameters().concurrent_evaluations());
    for (int iEvaluation = 0; iEvaluation < tEvaluations; iEvaluation++)
    {
        tCummulativeID++;
        auto tPerformerNode = aDocument.append_child("Performer");
        std::string tPerformerName = std::string("plato_services_") + std::to_string(iEvaluation);
        std::vector<std::string> tValues = { tPerformerName, std::string("plato_services"), std::to_string(tCummulativeID) };
        XMLGen::append_children( tKeywords, tValues, tPerformerNode);
    }
    return tCummulativeID;
}
// function append_method_options
/******************************************************************************/

/******************************************************************************/
int append_physics_performer
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument,
 int aBasePerformerID)
{
    auto tCummulativeID = aBasePerformerID;
    std::vector<std::string> tKeywords = { "Name", "Code", "PerformerID" };
    auto tEvaluations = std::stoi(aMetaData.optimization_parameters().concurrent_evaluations());
    for(auto& tService : aMetaData.services())
    {
        if(tService.value("type") == "plato_app" && tService.code() != "platomain")
        {
            for (int iEvaluation = 0; iEvaluation < tEvaluations; iEvaluation++)
            {
                tCummulativeID++;
                auto tPerformerNode = aDocument.append_child("Performer");
                std::string tPerformerName = tService.performer() + std::string("_") + std::to_string(iEvaluation);
                std::vector<std::string> tValues = { tPerformerName, tService.code(), std::to_string(tCummulativeID) };
                XMLGen::append_children( tKeywords, tValues, tPerformerNode);
            }
        }
    }
    return tCummulativeID;
}
// function append_physics_performer
/******************************************************************************/


}
// namespace XMLGen
