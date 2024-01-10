/*
 * XMLGeneratorSierraTFOperationsFileUtilities.cpp
 *
 *  Created on: July 5, 2022
 */

#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorSierraTFOperationsFileUtilities.hpp"
#include "XMLGeneratorOptimizationParametersMetadata.hpp"

namespace XMLGenSierraTF
{

/**************************************************************************/
void append_criterion_value_operation
(pugi::xml_document& aDocument)
{
    auto tOperationNode = aDocument.append_child("Operation");
    XMLGen::append_children({"Name"}, {"Compute Criterion"}, tOperationNode);
    auto tOutputNode = tOperationNode.append_child("OutputValue");
    XMLGen::append_children({"Name"}, {"Criterion"}, tOutputNode);
}
/**************************************************************************/
void append_criterion_gradient_wrt_CAD_parameters
(pugi::xml_document& aDocument)
{
    pugi::xml_node tmp_node = aDocument.append_child("Operation");
    XMLGen::addChild(tmp_node, "Name", "Compute Criterion Gradient wrt CAD Parameters");
    pugi::xml_node tForNode = tmp_node.append_child("For");
    tForNode.append_attribute("var") = "I";
    tForNode.append_attribute("in") = "Parameters";
    pugi::xml_node tmp_node1 = tForNode.append_child("Input");
    XMLGen::addChild(tmp_node1, "ArgumentName", "Parameter Sensitivity {I}");
    tmp_node1 = tmp_node.append_child("Output");
    XMLGen::addChild(tmp_node1, "ArgumentName", "Criterion Gradient wrt CAD Parameters");
}
/**************************************************************************/
void append_criterion_gradient_operation
(const XMLGen::InputData& /*aMetaData*/,
 pugi::xml_document& aDocument)
{
    auto tOperationNode = aDocument.append_child("Operation");
    XMLGen::append_children({"Name"}, {"Compute Criterion Gradient"}, tOperationNode);
    auto tOutputNode = tOperationNode.append_child("Output");
    XMLGen::append_children({"ArgumentName"}, {"DFDX"}, tOutputNode);
}
/**************************************************************************/
void add_operations_gradient_based_problem
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
//    XMLGen::append_version_entry(aDocument);
    append_criterion_value_operation(aDocument);
    append_criterion_gradient_operation(aMetaData, aDocument);
}
/**************************************************************************/
void add_operations
(const XMLGen::InputData& aMetaData,
 pugi::xml_document& aDocument)
{
    if (aMetaData.optimization_parameters().optimizationType() == XMLGen::OT_SHAPE)
    {
        for(size_t i=0; i<aMetaData.objective.serviceIDs.size(); ++i)
        {
            auto tServiceID = aMetaData.objective.serviceIDs[i];
            auto &tService = aMetaData.service(tServiceID);
            if(tService.code() == "sierra_tf" && tServiceID == aMetaData.services()[0].id())
            {
                add_operations_gradient_based_problem(aMetaData, aDocument);
            }
        }
    }
}
/******************************************************************************/
void write_sierra_tf_operation_xml_file
(const XMLGen::InputData& aXMLMetaData)
{
    pugi::xml_document tDocument;
    XMLGen::append_include_defines_xml_data(aXMLMetaData, tDocument);
    add_operations(aXMLMetaData, tDocument);
    std::string tServiceID = aXMLMetaData.services()[0].id();
    std::string tFilename = std::string("sierra_tf_") + tServiceID + "_operations.xml";
    tDocument.save_file(tFilename.c_str(), "  ");
}
/******************************************************************************/


}
// namespace XMLGenSierraTF
