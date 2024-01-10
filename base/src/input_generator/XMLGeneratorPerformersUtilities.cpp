/*
 * XMLGeneratorPerformersUtilities.cpp
 *
 *  Created on: Jan 14, 2022
 */

#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorPerformersUtilities.hpp"

namespace XMLGen
{

/******************************************************************************/
int append_plato_main_performer
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node& aNode,
 int aBasePerformerID)
{
    // The platomain optimizer should always be the first service in the list.
    auto tPerformerID = std::to_string(aBasePerformerID);
    const XMLGen::Service &tService = aXMLMetaData.service(0);
    auto tPerformerNode = aNode.append_child("Performer");
    XMLGen::append_children( {"Name", "Code", "PerformerID"}, {tService.performer(), tService.code(), tPerformerID}, tPerformerNode);
    return std::stoi(tPerformerID);
}
// function append_plato_main_performer
/******************************************************************************/

/******************************************************************************/
void append_uniperformer_usecase
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node& aParentNode)
{
    if(aXMLMetaData.mPerformerServices.empty())
    {
        THROWERR("Appending uniperformer use case to interface file: Service metadata list is empty.")
    }

    int tPerformerID=1;
    std::vector<std::string> tKeywords = { "Name", "Code", "PerformerID" };
    for(auto& tService : aXMLMetaData.mPerformerServices)
    {
        auto tPerformerNode = aParentNode.append_child("Performer");
        if(tService.code() != "plato_esp")
        {
            std::vector<std::string> tValues = { tService.performer(), tService.code(), std::to_string(tPerformerID) };
            XMLGen::append_children( tKeywords, tValues, tPerformerNode);
        }
        else
        {
            if(aXMLMetaData.optimization_parameters().optimizationType() == OT_SHAPE)
            {
                addChild(tPerformerNode, "PerformerID", std::to_string(tPerformerID));
                auto tForNode = tPerformerNode.append_child("For");
                tForNode.append_attribute("var") = "I";
                tForNode.append_attribute("in") = "Parameters";
                addChild(tForNode, "Name", "plato_esp_{I}");
            }
        }
        tPerformerID++;
    }
}
// function append_uniperformer_usecase
/******************************************************************************/

/******************************************************************************/
void append_multiperformer_usecase
(const XMLGen::InputData& aXMLMetaData,
 pugi::xml_node& aNode)
{
    if(aXMLMetaData.services().empty())
    {
        THROWERR("Appending multiperformer use case to interface file: Service metadata list is empty.")
    }

    int tNextPerformerID=1;
    for(auto& tService : aXMLMetaData.services())
    {
        if(tService.code() != "plato_esp")  
        {
            const int tID = (&tService - &aXMLMetaData.services()[0]);
            // The PlatoMain optimizer should always be the first service in the list
            // so skip it.
            if(tID > 0)
            {
                auto tPerformerNode = aNode.append_child("Performer");
                XMLGen::append_children( { "PerformerID" }, { std::to_string(tNextPerformerID) }, tPerformerNode);
                tNextPerformerID++;
                auto tForNode = tPerformerNode.append_child("For");
                XMLGen::append_attributes( { "var", "in" }, { "PerformerIndex", "Performers" }, tForNode);
                auto tPerformerName = tService.performer() + "_{PerformerIndex}";
                XMLGen::append_children( { "Name", "Code" }, { tPerformerName, tService.code() }, tForNode);
            }
        }
    }
}
// function append_multiperformer_usecase
/******************************************************************************/

}
// namespace XMLGen
