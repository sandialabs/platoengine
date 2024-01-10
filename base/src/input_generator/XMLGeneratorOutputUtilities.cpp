/*
 * XMLGeneratorOutputUtilities.cpp
 *
 *  Created on: Jan 17, 2022
 */

#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorValidInputKeys.hpp"
#include "XMLGeneratorOutputUtilities.hpp"

namespace XMLGen
{

/******************************************************************************/
void append_output_qoi_to_deterministic_write_output_operation
(const XMLGen::InputData& aMetaData,
 const XMLGen::Output& aOutputMetadata,
 pugi::xml_node& aParentNode)
{
    XMLGen::ValidPerformerOutputKeys tValidKeys;
    auto tServiceID = aOutputMetadata.serviceID();
    auto tCodeName = aMetaData.service(tServiceID).code();
    auto tPerformerName = aMetaData.service(tServiceID).performer();
    auto tOutputQoIs = aOutputMetadata.deterministicIDs();
    for(auto& tQoI : tOutputQoIs)
    {
        auto tOutput = aParentNode.append_child("Output");
        auto tArgumentName = tValidKeys.argument(tCodeName, tQoI);
        auto tSharedDataName = aOutputMetadata.deterministicSharedDataName(tQoI);
        tSharedDataName += std::string("_") + tPerformerName;
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, {tArgumentName, tSharedDataName}, tOutput);
    }
}
// function append_output_qoi_to_deterministic_write_output_operation
/******************************************************************************/

/******************************************************************************/
void append_output_qoi_to_nondeterministic_write_output_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    XMLGen::ValidPerformerOutputKeys tValidKeys;
    const XMLGen::Output &tOutputMetadata = aMetaData.mOutputMetaData[0];
    auto tServiceID = tOutputMetadata.serviceID();
    auto tCodeName = aMetaData.service(tServiceID).code();
    auto tOutputQoIs = tOutputMetadata.randomIDs();
    for(auto& tQoI : tOutputQoIs)
    {
        auto tOutput = aParentNode.append_child("Output");
        auto tArgumentName = tValidKeys.argument(tCodeName, tQoI);
        auto tSharedDataName = tOutputMetadata.randomSharedDataName(tQoI);
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, {tArgumentName, tSharedDataName}, tOutput);
    }
}
// function append_output_qoi_to_nondeterministic_write_output_operation
/******************************************************************************/

/******************************************************************************/
void append_default_qoi_to_output_operation_in_interface_file
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    auto tInput = aParentNode.append_child("Input");
    if(aMetaData.optimization_parameters().enforceBounds())
    {
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"topology", "Clamped Topology"}, tInput);
    }  
    else
    {
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"topology", "Topology"}, tInput);
    }
    tInput = aParentNode.append_child("Input");
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {"control", "Control"}, tInput);
    XMLGen::append_objective_gradient_qoi_to_output_operation(aMetaData, aParentNode);
    XMLGen::append_constraint_gradient_qoi_to_output_operation(aMetaData, aParentNode);
}
// function append_default_qoi_to_output_operation_in_interface_file
/******************************************************************************/

/******************************************************************************/
void append_objective_gradient_qoi_to_output_operation
(const XMLGen::InputData& /*aMetaData*/,
 pugi::xml_node& aParentNode)
{
    auto tInput = aParentNode.append_child("Input");
    auto tSharedDataName = std::string("Objective Gradient");
    auto tArgumentName  = XMLGen::to_lower(tSharedDataName);
    XMLGen::append_children({"ArgumentName", "SharedDataName"}, {tArgumentName, tSharedDataName}, tInput);
}
// function append_objective_gradient_qoi_to_output_operation
/******************************************************************************/

/******************************************************************************/
void append_constraint_gradient_qoi_to_output_operation
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    for(auto& tConstraint : aMetaData.constraints)
    {
        auto tInput = aParentNode.append_child("Input");
        auto tSharedDataName = std::string("Constraint Gradient ") + tConstraint.id();
        auto tArgumentName  = XMLGen::to_lower(tSharedDataName);
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, {tArgumentName, tSharedDataName}, tInput);
    }
}
// function append_constraint_gradient_qoi_to_output_operation
/******************************************************************************/

/******************************************************************************/
void append_deterministic_qoi_to_output_operation_in_interface_file
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    if(aMetaData.objective.multi_load_case == "true")
    {
        XMLGen::append_deterministic_qoi_to_output_operation_in_interface_file_for_multi_load_case(aMetaData, aParentNode);
    }
    else
    {
        XMLGen::append_deterministic_qoi_to_output_operation_in_interface_file_for_non_multi_load_case(aMetaData, aParentNode);
    }
}
// function append_deterministic_qoi_to_output_operation_in_interface_file
/******************************************************************************/

/******************************************************************************/
void append_deterministic_qoi_to_output_operation_in_interface_file_for_multi_load_case
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    for(auto &tOutputMetaData : aMetaData.mOutputMetaData)
    {
        auto tQoIIDs = tOutputMetaData.deterministicIDs();
        for(auto& tID : tQoIIDs)
        {
            for(auto tScenarioID : aMetaData.objective.scenarioIDs)
            {
                auto tInput = aParentNode.append_child("Input");
                auto tArgumentName = tOutputMetaData.deterministicArgumentName(tID);
                auto tSharedDataName = tOutputMetaData.deterministicSharedDataName(tID);
                auto tOwnerString = std::string("_") + aMetaData.service(tOutputMetaData.serviceID()).performer();
                tSharedDataName += tOwnerString;
                tSharedDataName += "_scenario_" + tScenarioID;
                tArgumentName += tOwnerString;
                tArgumentName += "_scenario_" + tScenarioID;
                XMLGen::append_children({"ArgumentName", "SharedDataName"}, {tArgumentName, tSharedDataName}, tInput);
            }
        }
    }
}
// function append_deterministic_qoi_to_output_operation_in_interface_file_for_multi_load_case
/******************************************************************************/

/******************************************************************************/
void append_deterministic_qoi_to_output_operation_in_interface_file_for_non_multi_load_case 
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    for(auto &tOutputMetaData : aMetaData.mOutputMetaData)
    {
        auto tQoIIDs = tOutputMetaData.deterministicIDs();
        for(auto& tID : tQoIIDs)
        {
            auto tInput = aParentNode.append_child("Input");
            auto tArgumentName = tOutputMetaData.deterministicArgumentName(tID);
            auto tSharedDataName = tOutputMetaData.deterministicSharedDataName(tID);
            auto tOwnerString = std::string("_") + aMetaData.service(tOutputMetaData.serviceID()).performer();
            tSharedDataName += tOwnerString;
            tArgumentName += tOwnerString;
            XMLGen::append_children({"ArgumentName", "SharedDataName"}, {tArgumentName, tSharedDataName}, tInput);
        }
    }
}
// function append_deterministic_qoi_to_output_operation_in_interface_file_for_non_multi_load_case
/******************************************************************************/

/******************************************************************************/
void append_nondeterministic_qoi_to_output_operation_in_interface_file
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    const XMLGen::Output &tOutputMetadata = aMetaData.mOutputMetaData[0];
    if(tOutputMetadata.outputSamples() == false)
    {
        return;
    }

    auto tQoIIDs = tOutputMetadata.randomIDs();
    for(auto& tID : tQoIIDs)
    {
        auto tOuterFor = aParentNode.append_child("For");
        XMLGen::append_attributes({"var", "in"}, {"PerformerIndex", "Performers"}, tOuterFor);
        auto tInnerFor = tOuterFor.append_child("For");
        XMLGen::append_attributes({"var", "in"}, {"PerformerSampleIndex", "PerformerSamples"}, tInnerFor);
        auto tInput = tInnerFor.append_child("Input");
        auto tArgumentName = tOutputMetadata.randomArgumentName(tID);
        auto tSharedDataName = tOutputMetadata.randomSharedDataName(tID);
        XMLGen::append_children({"ArgumentName", "SharedDataName"}, {tArgumentName, tSharedDataName}, tInput);
    }
}
// function append_nondeterministic_qoi_to_output_operation_in_interface_file
/******************************************************************************/

/******************************************************************************/
void append_qoi_statistics_to_output_operation_in_interface_file
(const XMLGen::InputData& aMetaData,
 pugi::xml_node& aParentNode)
{
    auto tQoIIDs = aMetaData.mOutputMetaData[0].randomIDs();
    for (auto &tID : tQoIIDs)
    {
        auto tMeanName = tID + " mean";
        auto tInput = aParentNode.append_child("Input");
        XMLGen::append_children( { "ArgumentName", "SharedDataName" }, { tMeanName, tMeanName }, tInput);

        auto tStdDevName = tID + " standard deviation";
        tInput = aParentNode.append_child("Input");
        XMLGen::append_children( { "ArgumentName", "SharedDataName" }, { tStdDevName, tStdDevName }, tInput);
    }
}
// function append_qoi_statistics_to_output_operation_in_interface_file
/******************************************************************************/

}
// namespace XMLGen
