/*
 * XMLGeneratorSharedDataUtilities.cpp
 *
 *  Created on: Jan 14, 2022
 */
 
#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorValidInputKeys.hpp"
#include "XMLGeneratorParserUtilities.hpp"
#include "XMLGeneratorSharedDataUtilities.hpp"

namespace XMLGen
{

/******************************************************************************/
void append_design_variables_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY)
    {
        XMLGen::append_control_shared_data(aMetaData, aDocument);
    }
    else if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        XMLGen::append_design_parameters_shared_data(aMetaData, aDocument);
    }
}
// function append_design_variables_shared_data
/******************************************************************************/

/******************************************************************************/
void append_control_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    auto tSharedData = aDocument.append_child("SharedData");
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    std::vector<std::string> tValues = {"Control", "Scalar", "Nodal Field", "IGNORE", tFirstPlatoMainPerformer, tFirstPlatoMainPerformer};
    XMLGen::append_children(tKeys, tValues, tSharedData);
    if(aMetaData.optimization_parameters().filterInEngine() == false)
    {
        for(auto& tService : aMetaData.services())
        {
            if(tService.code() == "plato_analyze")
            {
                tKeys = {"UserName"};
                tValues = {tService.performer()};
                XMLGen::append_children(tKeys, tValues, tSharedData);
            }
        }
    }
}
// function append_control_shared_data
/******************************************************************************/

/******************************************************************************/
void append_design_parameters_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    auto tTmpNode = aDocument.append_child("SharedData");
    addChild(tTmpNode, "Name", "Design Parameters");
    addChild(tTmpNode, "Type", "Scalar");
    addChild(tTmpNode, "Layout", "Global");
    addChild(tTmpNode, "Size", aMetaData.optimization_parameters().num_shape_design_variables());
    addChild(tTmpNode, "OwnerName", tFirstPlatoMainPerformer);
    addChild(tTmpNode, "UserName", tFirstPlatoMainPerformer);
    for(auto& tService : aMetaData.mPerformerServices)
    {
        if(tService.code() != "plato_esp")
        {
            addChild(tTmpNode, "UserName", tService.performer());
        }
    }
}
// append_design_parameters_shared_data
/******************************************************************************/

/******************************************************************************/
void append_lower_bounds_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    // shared data - lower bound value
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    std::vector<std::string> tValues = {"Lower Bound Value", "Scalar", "Global", "1", tFirstPlatoMainPerformer, tFirstPlatoMainPerformer};
    auto tSharedDataNode = aDocument.append_child("SharedData");
    XMLGen::append_children(tKeys, tValues, tSharedDataNode);

    // shared data - lower bound vector
    if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY && aMetaData.optimization_parameters().discretization() == "density")
    {
        tValues = {"Lower Bound Vector", "Scalar", "Nodal Field", "IGNORE", tFirstPlatoMainPerformer, tFirstPlatoMainPerformer};
        tSharedDataNode = aDocument.append_child("SharedData");
        XMLGen::append_children(tKeys, tValues, tSharedDataNode);
    }
    else if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        tValues = {"Lower Bound Vector", "Scalar", "Global", aMetaData.optimization_parameters().num_shape_design_variables(), tFirstPlatoMainPerformer, tFirstPlatoMainPerformer};
        tSharedDataNode = aDocument.append_child("SharedData");
        XMLGen::append_children(tKeys, tValues, tSharedDataNode);
    }
}
// function append_lower_bounds_shared_data
/******************************************************************************/

/******************************************************************************/
void append_upper_bounds_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    // shared data - upper bound value
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    std::vector<std::string> tValues = {"Upper Bound Value", "Scalar", "Global", "1", tFirstPlatoMainPerformer, tFirstPlatoMainPerformer};
    auto tSharedDataNode = aDocument.append_child("SharedData");
    XMLGen::append_children(tKeys, tValues, tSharedDataNode);

    // shared data - upper bound vector
    if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY  && aMetaData.optimization_parameters().discretization() == "density")
    {
        tValues = {"Upper Bound Vector", "Scalar", "Nodal Field", "IGNORE", tFirstPlatoMainPerformer, tFirstPlatoMainPerformer};
        tSharedDataNode = aDocument.append_child("SharedData");
        XMLGen::append_children(tKeys, tValues, tSharedDataNode);
    }
    else if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        tValues = {"Upper Bound Vector", "Scalar", "Global", aMetaData.optimization_parameters().num_shape_design_variables(), tFirstPlatoMainPerformer, tFirstPlatoMainPerformer};
        tSharedDataNode = aDocument.append_child("SharedData");
        XMLGen::append_children(tKeys, tValues, tSharedDataNode);
    }
    else
    {
        THROWERR(std::string("Invalid optimization and discretization combination. Supported types are topology+ density, shape." 
            + aMetaData.optimization_parameters().optimization_type() + aMetaData.optimization_parameters().discretization() +"is not supported."));
    }
}
// function append_upper_bounds_shared_data
/******************************************************************************/

/******************************************************************************/
void append_design_volume_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY)
    {
        std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
        std::vector<std::string> tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
        std::vector<std::string> tValues = {"Design Volume", "Scalar", "Global", "1", tFirstPlatoMainPerformer, tFirstPlatoMainPerformer};
        auto tSharedDataNode = aDocument.append_child("SharedData");
        XMLGen::append_children(tKeys, tValues, tSharedDataNode);
    }
}
// function append_design_volume_shared_data
/******************************************************************************/

/******************************************************************************/
void append_parameter_sensitivity_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
        auto tForNode = aDocument.append_child("For");
        tForNode.append_attribute("var") = "I";
        tForNode.append_attribute("in") = "Parameters";
        auto tTmpNode = tForNode.append_child("SharedData");
        addChild(tTmpNode, "Name", "Parameter Sensitivity {I}");
        addChild(tTmpNode, "Type", "Scalar");
        addChild(tTmpNode, "Layout", "Global");
        addChild(tTmpNode, "Dynamic", "true");
        addChild(tTmpNode, "OwnerName", "plato_esp_{I}");
        addChild(tTmpNode, "UserName", tFirstPlatoMainPerformer);
        for(auto& tService : aMetaData.mPerformerServices)
        {
            if(tService.code() != "plato_esp")
            {
                addChild(tTmpNode, "UserName", tService.performer());
            }
        }
    }
}
// function append_parameter_sensitivity_shared_data
/******************************************************************************/

/******************************************************************************/
void append_gradient_based_criterion_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();

    auto tConcretizedCriteria = aMetaData.getConcretizedCriteria();

    for(auto& tCriterion : tConcretizedCriteria)
    {      
        std::string tCriterionID = std::get<0>(tCriterion);
        std::string tServiceID = std::get<1>(tCriterion);
        std::string tScenarioID = std::get<2>(tCriterion);
        std::string tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tCriterion);

        XMLGen::Service tService = aMetaData.service(tServiceID);
        std::string tOwnerName = tService.performer();

        // shared data - deterministic criterion value
        std::string tTag = std::string("Criterion Value - ") + tIdentifierString;
        std::vector<std::string> tKeys = { "Name", "Type", "Layout", "Size", "OwnerName", "UserName" };
        std::vector<std::string> tValues = { tTag, "Scalar", "Global", "1", tOwnerName, tFirstPlatoMainPerformer };
        auto tSharedDataNode = aDocument.append_child("SharedData");
        XMLGen::append_children(tKeys, tValues, tSharedDataNode);

        // shared data - deterministic criterion gradient
        if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY)
        {
            if(aMetaData.optimization_parameters().discretization() == "density")
            {
                tTag = std::string("Criterion Gradient - ") + tIdentifierString;
                tValues = { tTag, "Scalar", "Nodal Field", "IGNORE", tOwnerName, tFirstPlatoMainPerformer };
                tSharedDataNode = aDocument.append_child("SharedData");
                XMLGen::append_children(tKeys, tValues, tSharedDataNode);

                if(aMetaData.optimization_parameters().filter_type() == "helmholtz")
                {
                    std::string tHelmholtzPerformer = "";
                    for(auto& tService2 : aMetaData.mPerformerServices)
                    {
                        if(tService2.id() == "helmholtz")
                        {
                            tHelmholtzPerformer = tService2.performer();
                            break;
                        }
                    }

                    XMLGen::ValidProjectionKeys tValidProjectionKeys;
                    auto tProjectionName = tValidProjectionKeys.value(aMetaData.optimization_parameters().projection_type());
                    if(tProjectionName.empty())
                    {
                        tKeys = {"UserName"};
                        tValues = {tHelmholtzPerformer};
                        XMLGen::append_children(tKeys, tValues, tSharedDataNode);
                    }
                    else
                    {
                        tSharedDataNode = aDocument.append_child("SharedData");
                        tTag = std::string("Projected Gradient - ") + tIdentifierString;
                        tKeys = { "Name", "Type", "Layout", "Size", "OwnerName", "UserName", "UserName" };
                        tValues = { tTag, "Scalar", "Nodal Field", "IGNORE", tFirstPlatoMainPerformer, tFirstPlatoMainPerformer, tHelmholtzPerformer };
                        XMLGen::append_children(tKeys, tValues, tSharedDataNode);
                    }
                }
            }
        }
        else if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
        {
                tKeys = { "Name", "Type", "Layout", "Dynamic", "OwnerName", "UserName" };
                tTag = std::string("Criterion GradientX - ") + tIdentifierString;
                tValues = { tTag, "Scalar", "Global", "true", tOwnerName, tFirstPlatoMainPerformer };
                tSharedDataNode = aDocument.append_child("SharedData");
                XMLGen::append_children(tKeys, tValues, tSharedDataNode);

                tKeys = { "Name", "Type", "Layout", "Size", "OwnerName", "UserName" };
                tTag = std::string("Criterion Gradient - ") + tIdentifierString;
                tValues = { tTag, "Scalar", "Global", aMetaData.optimization_parameters().num_shape_design_variables(), tFirstPlatoMainPerformer, 
                            tFirstPlatoMainPerformer };
                tSharedDataNode = aDocument.append_child("SharedData");
                XMLGen::append_children(tKeys, tValues, tSharedDataNode);
        }
    }
}
// function append_gradient_based_criterion_shared_data
/******************************************************************************/

/******************************************************************************/
void append_gradient_based_constraint_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    auto tConstraints = aMetaData.constraints;

    for(auto& tConstraint : tConstraints)
    {
        std::string tTag = std::string("Constraint Value ") + tConstraint.id();
        std::vector<std::string> tKeys = { "Name", "Type", "Layout", "Size", "OwnerName", "UserName" };
        std::vector<std::string> tValues = {tTag, "Scalar", "Global", "1", tFirstPlatoMainPerformer, tFirstPlatoMainPerformer };
        auto tSharedDataNode = aDocument.append_child("SharedData");
        XMLGen::append_children(tKeys, tValues, tSharedDataNode);

        if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY)
        {
            tKeys = { "Name", "Type", "Layout", "Size" };
            tTag = std::string("Constraint Gradient ") + tConstraint.id();
            tValues = { tTag, "Scalar", "Nodal Field", "IGNORE" };

            tSharedDataNode = aDocument.append_child("SharedData");
            XMLGen::append_children(tKeys, tValues, tSharedDataNode);

            if(aMetaData.optimization_parameters().filter_type() == "helmholtz")
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
                tKeys = { "OwnerName", "UserName", "UserName" };
                tValues = { tHelmholtzPerformer, tHelmholtzPerformer, tFirstPlatoMainPerformer };
                XMLGen::append_children(tKeys, tValues, tSharedDataNode);

                XMLGen::ValidProjectionKeys tValidProjectionKeys;
                auto tProjectionName = tValidProjectionKeys.value(aMetaData.optimization_parameters().projection_type());
                if(!tProjectionName.empty())
                {
                    tSharedDataNode = aDocument.append_child("SharedData");
                    tKeys = { "Name", "Type", "Layout", "Size", "OwnerName", "UserName", "UserName" };
                    tValues = { std::string("Projected ") + tTag, "Scalar", "Nodal Field", "IGNORE", tFirstPlatoMainPerformer, tFirstPlatoMainPerformer, tHelmholtzPerformer };
                    XMLGen::append_children(tKeys, tValues, tSharedDataNode);
                }
            }
            else
            {
                tKeys = { "OwnerName", "UserName" };
                tValues = { tFirstPlatoMainPerformer, tFirstPlatoMainPerformer };
                XMLGen::append_children(tKeys, tValues, tSharedDataNode);
            }
        }
        else if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
        {
            tTag = std::string("Constraint Gradient ") + tConstraint.id();
            tValues = { tTag, "Scalar", "Global", aMetaData.optimization_parameters().num_shape_design_variables(), tFirstPlatoMainPerformer, tFirstPlatoMainPerformer };
            tSharedDataNode = aDocument.append_child("SharedData");
            XMLGen::append_children(tKeys, tValues, tSharedDataNode);
        }
    }
}
// function append_gradient_based_constraint_shared_data
/******************************************************************************/

/******************************************************************************/
void append_gradient_based_objective_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();

    std::vector<std::string> tKeys = { "Name", "Type", "Layout", "Size", "OwnerName", "UserName" };
    std::vector<std::string> tValues = {"Objective Value", "Scalar", "Global", "1", tFirstPlatoMainPerformer, tFirstPlatoMainPerformer };
    auto tSharedDataNode = aDocument.append_child("SharedData");
    XMLGen::append_children(tKeys, tValues, tSharedDataNode);

    if(aMetaData.optimization_parameters().optimizationType() == OT_TOPOLOGY)
    {
        tKeys = { "Name", "Type", "Layout", "Size" };
        tValues = {"Objective Gradient", "Scalar", "Nodal Field", "IGNORE" };

        tSharedDataNode = aDocument.append_child("SharedData");
        XMLGen::append_children(tKeys, tValues, tSharedDataNode);

        if(aMetaData.optimization_parameters().filter_type() == "helmholtz")
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
            tKeys = { "OwnerName", "UserName", "UserName" };
            tValues = { tHelmholtzPerformer, tHelmholtzPerformer, tFirstPlatoMainPerformer };
            XMLGen::append_children(tKeys, tValues, tSharedDataNode);

            XMLGen::ValidProjectionKeys tValidProjectionKeys;
            auto tProjectionName = tValidProjectionKeys.value(aMetaData.optimization_parameters().projection_type());
            if(!tProjectionName.empty())
            {
                tSharedDataNode = aDocument.append_child("SharedData");
                tKeys = { "Name", "Type", "Layout", "Size", "OwnerName", "UserName", "UserName" };
                tValues = { "Projected Objective Gradient", "Scalar", "Nodal Field", "IGNORE", tFirstPlatoMainPerformer, tFirstPlatoMainPerformer, tHelmholtzPerformer };
                XMLGen::append_children(tKeys, tValues, tSharedDataNode);
            }
        }
        else
        {
            tKeys = { "OwnerName", "UserName" };
            tValues = { tFirstPlatoMainPerformer, tFirstPlatoMainPerformer };
            XMLGen::append_children(tKeys, tValues, tSharedDataNode);
        }

        if(aMetaData.needToAggregate())
        {
            tKeys = { "Name", "Type", "Layout", "Size", "OwnerName", "UserName" };
            tValues = { "Aggregate Gradient", "Scalar", "Nodal Field", "IGNORE", tFirstPlatoMainPerformer, tFirstPlatoMainPerformer };
            tSharedDataNode = aDocument.append_child("SharedData");
            XMLGen::append_children(tKeys, tValues, tSharedDataNode);
            if(aMetaData.optimization_parameters().filter_type() == "helmholtz")
            {
                XMLGen::ValidProjectionKeys tValidProjectionKeys;
                auto tProjectionName = tValidProjectionKeys.value(aMetaData.optimization_parameters().projection_type());
                if(tProjectionName.empty())
                {
                    std::string tHelmholtzPerformer = "";
                    for(auto& tService2 : aMetaData.mPerformerServices)
                    {
                        if(tService2.id() == "helmholtz")
                        {
                            tHelmholtzPerformer = tService2.performer();
                            break;
                        }
                    }
                    tKeys = {"UserName"};
                    tValues = {tHelmholtzPerformer};
                    XMLGen::append_children(tKeys, tValues, tSharedDataNode);
                }
            }
        }
    }
    else if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        tValues = {"Objective Gradient", "Scalar", "Global", aMetaData.optimization_parameters().num_shape_design_variables(), tFirstPlatoMainPerformer, tFirstPlatoMainPerformer };
        tSharedDataNode = aDocument.append_child("SharedData");
        XMLGen::append_children(tKeys, tValues, tSharedDataNode);
    }
}
// function append_gradient_based_objective_shared_data
/******************************************************************************/

/******************************************************************************/
void append_normalization_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    bool tNormalizeInAggregator = aMetaData.optimization_parameters().normalizeInAggregator();
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();

    if(tNormalizeInAggregator)
    {
        bool tMultiLoadCase = (aMetaData.objective.multi_load_case == "true");
        int tNumEntries = aMetaData.objective.criteriaIDs.size();
        if(tMultiLoadCase)
        {
            tNumEntries = 1;
        }
        for(int i=0; i<tNumEntries; ++i)
        {
            std::string tCriterionID = aMetaData.objective.criteriaIDs[i];
            std::string tServiceID = aMetaData.objective.serviceIDs[i];
            std::string tScenarioID = aMetaData.objective.scenarioIDs[i];
            if(tMultiLoadCase)
            {
                tScenarioID = XMLGen::get_compound_scenario_id(aMetaData.objective.scenarioIDs);
            }
            ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
            auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);
            auto tValueNameString = "Initial Criterion Value - " + tIdentifierString;
            auto &tService = aMetaData.service(tServiceID);
            auto tOwnerName = tService.performer();

            std::vector<std::string> tKeys = { "Name", "Type", "Layout", "Size", "OwnerName", "UserName" };
            std::vector<std::string> tValues = {tValueNameString, "Scalar", "Global", "1", tOwnerName, tFirstPlatoMainPerformer };
            auto tSharedDataNode = aDocument.append_child("SharedData");
            XMLGen::append_children(tKeys, tValues, tSharedDataNode);
        }
    }
}
// function append_normalization_shared_data
/******************************************************************************/

/******************************************************************************/
void append_qoi_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aParentNode)
{
    if(aMetaData.services().empty())
    {
        THROWERR("Append QoI Shared Data: list of 'services' is empty.")
    }

    if(aMetaData.objective.multi_load_case == "true")
    {
        XMLGen::append_qoi_shared_data_for_multi_load_case(aMetaData, aParentNode);
    }
    else
    {
        XMLGen::append_qoi_shared_data_for_non_multi_load_case(aMetaData, aParentNode);
    }
}
// function append_qoi_shared_data
/******************************************************************************/

/******************************************************************************/
void append_qoi_shared_data_for_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aParentNode)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    for(auto &tOutputMetaData : aMetaData.mOutputMetaData)
    {
        auto tOutputIDs = tOutputMetaData.deterministicIDs();
        auto tServiceID = tOutputMetaData.serviceID();
        auto tOwnerName = aMetaData.service(tServiceID).performer();
        for(auto& tID : tOutputIDs)
        {
            auto tLayout = tOutputMetaData.deterministicLayout(tID);
            for(size_t i=0; i<aMetaData.objective.scenarioIDs.size(); ++i)
            {
                auto tSharedDataName = tOutputMetaData.deterministicSharedDataName(tID);
                auto tScenarioIndex = std::to_string(i);
                auto tScenarioID = aMetaData.objective.scenarioIDs[i];
                tSharedDataName += std::string("_") + tOwnerName;
                tSharedDataName += "_scenario_" + tScenarioID;
                std::vector<std::string> tValues = {tSharedDataName, "Scalar", tLayout, "IGNORE", tOwnerName, tFirstPlatoMainPerformer};
                auto tSharedDataNode = aParentNode.append_child("SharedData");
                XMLGen::append_children(tKeys, tValues, tSharedDataNode);
            }                        
        }
    }
}
// function append_qoi_shared_data_for_multi_load_case
/******************************************************************************/

/******************************************************************************/
void append_qoi_shared_data_for_non_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aParentNode)
{
    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();
    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName"};
    for(auto &tOutputMetaData : aMetaData.mOutputMetaData)
    {
        auto tOutputIDs = tOutputMetaData.deterministicIDs();
        auto tServiceID = tOutputMetaData.serviceID();
        auto tOwnerName = aMetaData.service(tServiceID).performer();
        for(auto& tID : tOutputIDs)
        {
            auto tLayout = tOutputMetaData.deterministicLayout(tID);
            auto tSharedDataName = tOutputMetaData.deterministicSharedDataName(tID);
            tSharedDataName += std::string("_") + tOwnerName;
            std::vector<std::string> tValues = {tSharedDataName, "Scalar", tLayout, "IGNORE", tOwnerName, tFirstPlatoMainPerformer};
            auto tSharedDataNode = aParentNode.append_child("SharedData");
            XMLGen::append_children(tKeys, tValues, tSharedDataNode);
        }
    }
}
// function append_qoi_shared_data_for_non_multi_load_case
/******************************************************************************/

/******************************************************************************/
void append_topology_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    if(aMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
    {
        return;
    }
    if(aMetaData.services().empty())
    {
        THROWERR("Append Topology Shared Data: Services list is empty.")
    }

    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();

    auto tSharedData = aDocument.append_child("SharedData");

    std::vector<std::string> tKeys = {"Name", "Type", "Layout", "Size"};
    std::vector<std::string> tValues = {"Topology", "Scalar", "Nodal Field", "IGNORE"};
    XMLGen::append_children(tKeys, tValues, tSharedData);

    if(aMetaData.optimization_parameters().filterInEngine() == false)
    {
        if(aMetaData.optimization_parameters().filter_type() == "helmholtz")
        {
            XMLGen::ValidProjectionKeys tValidProjectionKeys;
            auto tProjectionName = tValidProjectionKeys.value(aMetaData.optimization_parameters().projection_type());
            if(tProjectionName.empty())
            {
                XMLGen::append_helmholtz_topology_shared_data(aMetaData, tSharedData);
            }
            else
            {
                auto tFilteredControlSharedData = aDocument.append_child("SharedData");
                XMLGen::append_projection_helmholtz_topology_shared_data(aMetaData, tSharedData, tFilteredControlSharedData);
            }
        }
        else
        {
            std::string tFirstPlatoAnalyzePerformer = aMetaData.getFirstPlatoAnalyzePerformer();
            tKeys = {"OwnerName"};
            tValues = {tFirstPlatoAnalyzePerformer};
            XMLGen::append_children(tKeys, tValues, tSharedData);
            tKeys = {"UserName"};
            tValues = {tFirstPlatoAnalyzePerformer};
            XMLGen::append_children(tKeys, tValues, tSharedData);
            tKeys = {"UserName"};
            tValues = {tFirstPlatoMainPerformer};
            XMLGen::append_children(tKeys, tValues, tSharedData);
        }
    }
    else
    {
        tKeys = {"OwnerName"};
        tValues = {tFirstPlatoMainPerformer};
        XMLGen::append_children(tKeys, tValues, tSharedData);

        for(auto& tService : aMetaData.services())
        {
            tKeys = {"UserName"};
            tValues = {tService.performer()};
            XMLGen::append_children(tKeys, tValues, tSharedData);
        }
    }
 
    // Add the "Clamped Topology" shared data that will be used for output.
    if(aMetaData.optimization_parameters().enforceBounds())
    {
        tSharedData = aDocument.append_child("SharedData");
        tKeys = {"Name", "Type", "Layout", "Size"};
        tValues = {"Clamped Topology", "Scalar", "Nodal Field", "IGNORE"};
        XMLGen::append_children(tKeys, tValues, tSharedData);
        tKeys = {"OwnerName"};
        tValues = {tFirstPlatoMainPerformer};
        XMLGen::append_children(tKeys, tValues, tSharedData);
        tKeys = {"UserName"};
        tValues = {tFirstPlatoMainPerformer};
        XMLGen::append_children(tKeys, tValues, tSharedData);
    }
}
// function append_topology_shared_data
/******************************************************************************/

/******************************************************************************/
void append_helmholtz_topology_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
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

    std::vector<std::string> tKeys = {"OwnerName"};
    std::vector<std::string> tValues = {tHelmholtzPerformer};
    XMLGen::append_children(tKeys, tValues, aParentNode);
    for(auto& tService : aMetaData.services())
    {
        tKeys = {"UserName"};
        tValues = {tService.performer()};
        XMLGen::append_children(tKeys, tValues, aParentNode);
    }

}
// function append_helmholtz_topology_shared_data
/******************************************************************************/

/******************************************************************************/
void append_projection_helmholtz_topology_shared_data
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aTopologyNode,
 pugi::xml_node& aFilteredControlNode)
{

    std::string tFirstPlatoMainPerformer = aMetaData.getFirstPlatoMainPerformer();

    std::string tHelmholtzPerformer = "";
    for(auto& tService : aMetaData.mPerformerServices)
    {
        if(tService.id() == "helmholtz")
        {
            tHelmholtzPerformer = tService.performer();
            break;
        }
    }

    std::vector<std::string> tKeys = {"OwnerName"};
    std::vector<std::string> tValues = {tFirstPlatoMainPerformer};
    XMLGen::append_children(tKeys, tValues, aTopologyNode);
    for(auto& tService : aMetaData.services())
    {
        tKeys = {"UserName"};
        tValues = {tService.performer()};
        XMLGen::append_children(tKeys, tValues, aTopologyNode);
    }

    tKeys = {"Name", "Type", "Layout", "Size", "OwnerName", "UserName", "UserName"};
    tValues = {"Filtered Control", "Scalar", "Nodal Field", "IGNORE", tHelmholtzPerformer, tHelmholtzPerformer, tFirstPlatoMainPerformer};
    XMLGen::append_children(tKeys, tValues, aFilteredControlNode);
}
// function append_projection_helmholtz_topology_shared_data
/******************************************************************************/

/******************************************************************************/
std::string get_filter_constraint_criterion_gradient_input_shared_data_name
(const XMLGen::Constraint &aConstraint)
{
    std::string tCriterionID = aConstraint.criterion();
    std::string tServiceID = aConstraint.service();
    std::string tScenarioID = aConstraint.scenario();
    ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
    auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);

    auto tReturnString = std::string("Criterion Gradient - ") + tIdentifierString;
    return tReturnString;
}
// function get_filter_constraint_criterion_gradient_input_shared_data_name
/******************************************************************************/

/******************************************************************************/
std::string get_filter_objective_criterion_gradient_input_shared_data_name
(const XMLGen::InputData& aMetaData)
{
    std::string tReturnString;
    if(!XMLGen::is_robust_optimization_problem(aMetaData))
    {
        std::string tCriterionID = aMetaData.objective.criteriaIDs[0];
        std::string tServiceID = aMetaData.objective.serviceIDs[0];
        std::string tScenarioID = "";
        if(aMetaData.objective.multi_load_case == "true")
        {
            tScenarioID = get_compound_scenario_id(aMetaData.objective.scenarioIDs);
        }
        else
        {
            tScenarioID = aMetaData.objective.scenarioIDs[0];
        }
        ConcretizedCriterion tConcretizedCriterion(tCriterionID,tServiceID,tScenarioID);
        auto tIdentifierString = XMLGen::get_concretized_criterion_identifier_string(tConcretizedCriterion);
        if(aMetaData.needToAggregate())
        {
            tReturnString = "Aggregate Gradient";
        }
        else
        {
            tReturnString = "Criterion Gradient - " + tIdentifierString;
        }
    }
    else
    {
        tReturnString = "Objective Gradient";
    }
    return tReturnString;
}
// function get_filter_objective_criterion_gradient_input_shared_data_name
/******************************************************************************/

}
// namespace XMLGen
