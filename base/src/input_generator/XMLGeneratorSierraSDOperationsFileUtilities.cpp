/*
 * XMLGeneratorSierraSDInputDeckUtilities.cpp
 *
 *  Created on: Feb 1, 2021
 */

#include "XMLGeneratorScenarioMetadata.hpp"
#include "XMLGeneratorServiceMetadata.hpp"
#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorSierraSDUtilities.hpp"
#include "XMLGeneratorSierraSDInputDeckUtilities.hpp"

namespace XMLGen
{

/**************************************************************************/
void append_cache_state_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    auto tOperationNode = aDocument.append_child("Operation");
    append_children({"Function","Name"}, {"Cache State","Cache State"}, tOperationNode);
    for(size_t k=0; k<aMetaData.objective.scenarioIDs.size(); ++k)
    {
        auto tCriterionID = aMetaData.objective.criteriaIDs[k];
        auto tScenarioID = aMetaData.objective.scenarioIDs[k];
        auto tServiceID = aMetaData.objective.serviceIDs[k];
        auto tCriterion = aMetaData.criterion(tCriterionID);
        if(tCriterion.type() != "modal_projection_error" &&
           tCriterion.type() != "modal_matching")
        {
            for(auto &tOutput : aMetaData.mOutputMetaData)
            {
                if(tOutput.serviceID() == tServiceID)
                {
                    XMLGen::Service tService = aMetaData.service(tServiceID);
                    if(aMetaData.objective.multi_load_case == "true")
                    {
                        auto &tScenario = aMetaData.scenario(aMetaData.objective.scenarioIDs[0]);
                        for(size_t i=0; i<tScenario.loadIDs().size(); ++i)
                        {
                            auto tLoadIndex = std::to_string(i);
                            for(auto &tCurData : tOutput.outputIDs())
                            {
                                auto tOutputNode = tOperationNode.append_child("Output");
                                XMLGen::append_children({"ArgumentName"}, {tCurData + tLoadIndex}, tOutputNode);
                            }
                        }                        
                    }
                    else
                    {
                        for(auto &tCurData : tOutput.outputIDs())
                        {
                            auto tOutputNode = tOperationNode.append_child("Output");
                            append_children({"ArgumentName"}, {tCurData + "0"}, tOutputNode);
                        }
                    }
                    break;
                } 
            }
        }
    }
}
/**************************************************************************/
void append_update_problem_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    auto tOperationNode = aDocument.append_child("Operation");
    append_children({"Function", "Name"}, {"Update Problem", "Update Problem"}, tOperationNode);
}
/**************************************************************************/
void append_SIMP_penalty_model
(const XMLGen::Scenario& aScenario,
 pugi::xml_node& aParentNode)
{
    auto tSIMPNode = aParentNode.append_child("SIMP");
    if(aScenario.materialPenaltyExponent() != "")
    {
        append_children({"PenaltyExponent"}, {aScenario.materialPenaltyExponent()}, tSIMPNode);
    }
    if(aScenario.minErsatzMaterialConstant() != "")
    {
        addChild(tSIMPNode, "MinimumValue", aScenario.minErsatzMaterialConstant());
    }
    else
    {
        addChild(tSIMPNode, "MinimumValue", "0.001");
    }
}
/**************************************************************************/
void append_displacement_operation
(const XMLGen::Scenario& aScenario,
 pugi::xml_document& aDocument)
{
    auto tOperationNode = aDocument.append_child("Operation");
    append_children({"Function", "Name","PenaltyModel"}, {"Displacement", "Compute Displacement","SIMP"}, tOperationNode);
    auto tTopologyNode = tOperationNode.append_child("Topology");
    append_children({"Name"}, {"Topology"}, tTopologyNode);
    append_SIMP_penalty_model(aScenario, tOperationNode);
}
/**************************************************************************/
void append_internal_energy_operation
(const XMLGen::Scenario& aScenario,
 pugi::xml_document& aDocument)
{
    auto tOperationNode = aDocument.append_child("Operation");
    append_children({"Function", "Name", "PenaltyModel"}, {"Compute Criterion Value", "Compute Objective", "SIMP"}, tOperationNode);
    auto tTopologyNode = tOperationNode.append_child("Topology");
    append_children({"Name"}, {"Topology"}, tTopologyNode);
    append_SIMP_penalty_model(aScenario, tOperationNode);
    auto tOutputNode = tOperationNode.append_child("OutputValue");
    append_children({"Name"}, {"Internal Energy"}, tOutputNode);
}
/******************************************************************************/
void append_compute_objective_gradient_operation_for_shape_problem
(const XMLGen::Scenario& aScenario,
 pugi::xml_document& aDocument)
{
    pugi::xml_node tmp_node = aDocument.append_child("Operation");
    addChild(tmp_node, "Name", "Compute Objective Gradient wrt CAD Parameters");
    addChild(tmp_node, "Function", "Compute Objective Gradient wrt CAD Parameters");
    pugi::xml_node tForNode = tmp_node.append_child("For");
    tForNode.append_attribute("var") = "I";
    tForNode.append_attribute("in") = "Parameters";
    pugi::xml_node tmp_node1 = tForNode.append_child("Input");
    addChild(tmp_node1, "ArgumentName", "Parameter Sensitivity {I}");
    tmp_node1 = tmp_node.append_child("Output");
    addChild(tmp_node1, "ArgumentName", "Internal Energy Gradient");
}
/**************************************************************************/
void append_internal_energy_gradient_operation
(const XMLGen::Scenario& aScenario,
 pugi::xml_document& aDocument)
{
    auto tOperationNode = aDocument.append_child("Operation");
    append_children({"Function", "Name","PenaltyModel"}, {"Compute Criterion Gradient", "Compute Gradient", "SIMP"}, tOperationNode);
    auto tTopologyNode = tOperationNode.append_child("Topology");
    append_children({"Name"}, {"Topology"}, tTopologyNode);
    append_SIMP_penalty_model(aScenario, tOperationNode);
    auto tOutputNode = tOperationNode.append_child("OutputGradient");
    append_children({"Name"}, {"Internal Energy Gradient"}, tOutputNode);
}
/**************************************************************************/
void append_internal_energy_hessian_operation
(const XMLGen::InputData& aMetaData,
 const XMLGen::Scenario& aScenario,
 pugi::xml_document& aDocument)
{
    auto tAlgorithm = aMetaData.optimization_parameters().optimization_algorithm();
    if(tAlgorithm =="ksbc" ||
       tAlgorithm == "ksal" ||
       tAlgorithm == "rol ksal" ||
       tAlgorithm == "rol ksbc")
    {
        auto tOperationNode = aDocument.append_child("Operation");
        append_children({"Function", "Name","PenaltyModel"}, {"Compute HessianTimesVector", "Compute HessianTimesVector", "SIMP"}, tOperationNode);
        auto tTopologyNode = tOperationNode.append_child("Topology");
        append_children({"Name"}, {"Topology"}, tTopologyNode);
        auto tDescentNode = tOperationNode.append_child("DescentDirection");
        append_children({"Name"}, {"Descent Direction"}, tDescentNode);
        append_SIMP_penalty_model(aScenario, tOperationNode);
        auto tOutputNode = tOperationNode.append_child("OutputHessian");
        append_children({"Name"}, {"HessianTimesVector"}, tOutputNode);
    }
}
/**************************************************************************/
void append_surface_area_operation
(const XMLGen::InputData& aMetaData,
 const XMLGen::Scenario& aScenario,
 pugi::xml_document& aDocument)
{
    auto tDiscretization = aMetaData.optimization_parameters().discretization();
    if(tDiscretization == "levelset")
    {
        auto tOperationNode = aDocument.append_child("Operation");
        append_children({"Function", "Name"}, {"Compute Surface Area", "Compute Surface Area"}, tOperationNode);
        auto tTopologyNode = tOperationNode.append_child("Topology");
        append_children({"Name"}, {"Topology"}, tTopologyNode);
        auto tOutputNode = tOperationNode.append_child("OutputValue");
        append_children({"Name"}, {"SurfaceArea"}, tOutputNode);
    }
}
/**************************************************************************/
void append_surface_area_gradient_operation
(const XMLGen::InputData& aMetaData,
 const XMLGen::Scenario& aScenario,
 pugi::xml_document& aDocument)
{
    auto tDiscretization = aMetaData.optimization_parameters().discretization();
    if(tDiscretization == "levelset")
    {
        auto tOperationNode = aDocument.append_child("Operation");
        append_children({"Function", "Name"}, {"Compute Surface Area Gradient", "Compute Surface Area Gradient"}, tOperationNode);
        auto tTopologyNode = tOperationNode.append_child("Topology");
        append_children({"Name"}, {"Topology"}, tTopologyNode);
        auto tOutputNode = tOperationNode.append_child("OutputValue");
        append_children({"Name"}, {"SurfaceAreaGradient"}, tOutputNode);
    }
}
/**************************************************************************/
void append_reinitialize_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    auto tReinitializeName = std::string("reinitialize_on_change_") + aMetaData.services()[0].performer();
    auto tOperationNode = aDocument.append_child("Operation");
    addChild(tOperationNode, "Name", tReinitializeName);
    addChild(tOperationNode, "Function", "Reinitialize");
}
/**************************************************************************/
void append_criterion_value_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument,
 const std::string& aCriterionId,
 const std::string& aIdentifierString)
{
    auto& tCriterion = aMetaData.criterion(aCriterionId);
    auto tCriterionType = Plato::tolower(tCriterion.type());

    auto tOperation = aDocument.append_child("Operation");
    auto tOperationName = std::string("Compute Criterion Value - ") + aIdentifierString;
    XMLGen::append_children({"Function", "Name"}, {"Compute Criterion Value", tOperationName}, tOperation);
    auto tOutput = tOperation.append_child("OutputValue");
    XMLGen::append_children({"Name"}, {aIdentifierString + std::string(" value")}, tOutput);
}
/**************************************************************************/
void append_objective_criterion_value_operations
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    XMLGen::Objective tObjective = aMetaData.objective;
    for (size_t i=0; i<tObjective.criteriaIDs.size(); ++i)
    {
        std::string tCriterionID = tObjective.criteriaIDs[i];
        std::string tServiceID = tObjective.serviceIDs[i];
        std::string tScenarioID = tObjective.scenarioIDs[i];

        if(tServiceID == aMetaData.services()[0].id())
        {
            ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
            auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);
            XMLGen::append_criterion_value_operation(aMetaData,aDocument,tCriterionID,tIdentifierString);
        }
    }
}
/**************************************************************************/
void append_constraint_criterion_value_operations
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    for (auto &tConstraint : aMetaData.constraints)
    {
        std::string tCriterionID = tConstraint.criterion();
        std::string tServiceID = tConstraint.service();
        std::string tScenarioID = tConstraint.scenario();

        if(tServiceID == aMetaData.services()[0].id())
        {
            ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
            auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);
            XMLGen::append_criterion_value_operation(aMetaData,aDocument,tCriterionID,tIdentifierString);
        }
    }
}
/**************************************************************************/
void append_criteria_operations
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    append_objective_criterion_value_operations(aMetaData,aDocument);
    append_constraint_criterion_value_operations(aMetaData,aDocument);
}
/**************************************************************************/
void add_operations_gradient_based_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument,
 const XMLGen::Scenario& aScenario)
{
    append_version_entry(aDocument);
    append_update_problem_operation(aMetaData, aDocument);
    append_cache_state_operation(aMetaData, aDocument);
    append_displacement_operation(aScenario, aDocument);
    append_internal_energy_operation(aScenario, aDocument);
    append_internal_energy_gradient_operation(aScenario, aDocument);
    if (aMetaData.optimization_parameters().optimizationType() == OT_SHAPE) {
        append_compute_objective_gradient_operation_for_shape_problem(aScenario, aDocument);
    }
    append_internal_energy_hessian_operation(aMetaData, aScenario, aDocument);
    append_surface_area_operation(aMetaData, aScenario, aDocument);
    append_surface_area_gradient_operation(aMetaData, aScenario, aDocument);
}
/**************************************************************************/
void add_operations_dakota_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    append_version_entry(aDocument);
    append_reinitialize_operation(aMetaData, aDocument);
    append_criteria_operations(aMetaData, aDocument);
}
/**************************************************************************/
void add_operations
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    if (aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY ||
        aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        for(size_t i=0; i<aMetaData.objective.serviceIDs.size(); ++i)
        {
            auto tServiceID = aMetaData.objective.serviceIDs[i];
            auto &tService = aMetaData.service(tServiceID);
            if(tService.code() == "sierra_sd" && tServiceID == aMetaData.services()[0].id())
            {
                auto tScenarioID = aMetaData.objective.scenarioIDs[i];
                auto &tScenario = aMetaData.scenario(tScenarioID);
                add_operations_gradient_based_problem(aMetaData, aDocument, tScenario);
            }
        }
    }
    else if (aMetaData.optimization_parameters().optimizationType() == OT_DAKOTA)
    {
        add_operations_dakota_problem(aMetaData, aDocument);
    }

}
/******************************************************************************/
void write_sierra_sd_operation_xml_file
(const XMLGen::InputData& aXMLMetaData)
{
    pugi::xml_document tDocument;
    XMLGen::append_include_defines_xml_data(aXMLMetaData, tDocument);
    add_operations(aXMLMetaData, tDocument);
    std::string tServiceID = aXMLMetaData.services()[0].id();
    std::string tFilename = std::string("sierra_sd_") + tServiceID + "_operations.xml";
    tDocument.save_file(tFilename.c_str(), "  ");
}
/******************************************************************************/


}
// namespace XMLGen
