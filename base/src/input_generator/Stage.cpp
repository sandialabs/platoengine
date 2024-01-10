/*
 * Stage.cpp
 *
 *  Created on: April 14, 2022
 */

#include "Stage.hpp"
#include "XMLGeneratorUtilities.hpp"

using namespace XMLGen;

namespace director
{
    
Stage::Stage
(const std::string& aName,
 const std::vector<std::shared_ptr<Operation>>& aOperations,
 std::shared_ptr<SharedData> aInputSharedData,
 std::shared_ptr<SharedData> aOutputSharedData) :
 FileObject(aName),
 mInputSharedData(aInputSharedData),
 mOutputSharedData(aOutputSharedData),
 mOperations(aOperations)
{
}

void Stage::write
(pugi::xml_document& aDocument)
{
    auto tStageNode = aDocument.append_child("Stage");
    
    addChild(tStageNode, "Name", name());

    this->appendInput(tStageNode);

    for(unsigned int iOperation = 0; iOperation < mOperations.size(); ++iOperation)
    {
        auto tForOrStageNode = mOperations[iOperation]->forNode(tStageNode,"Parameters");
        mOperations[iOperation]->write_interface(tForOrStageNode); 
    }

    this->appendOutput(tStageNode);
}

void Stage::appendInput
(pugi::xml_node& aNode,
 const std::string& aTag)
{
    if(mInputSharedData)
    {
        auto tForOrStageNode = mInputSharedData->forNode(aNode,"Parameters");
        auto tSharedDataNode = tForOrStageNode.append_child("Input");
        if (aTag == "")
            XMLGen::append_children({"SharedDataName"},{mInputSharedData->name()},tSharedDataNode);
        else
            XMLGen::append_children({"Tag", "SharedDataName"},{aTag, mInputSharedData->name()},tSharedDataNode);
    }
}

void Stage::appendOutput(pugi::xml_node& aNode)
{
    if(mOutputSharedData)
    {
        auto tForOrStageNode = mOutputSharedData->forNode(aNode,"Parameters");
        auto tSharedDataNode = tForOrStageNode.append_child("Output");
        XMLGen::append_children({"SharedDataName"},{mOutputSharedData->name()},tSharedDataNode);
    }
}

std::string Stage::inputSharedDataName()
{
    if(mInputSharedData)
        return mInputSharedData->name();
    else 
        return "";
}

std::string Stage::outputSharedDataName()
{
    if(mOutputSharedData)
        return mOutputSharedData->name();
    else 
        return "";
}

void Stage::write_dakota
(pugi::xml_node& aDocument,
 const std::string& aStageTag)
{
    auto tStageNode = aDocument.append_child("Stage");
    
    addChild(tStageNode, "StageTag", aStageTag);
    addChild(tStageNode, "StageName", name());

    this->appendInput(tStageNode,"continuous");
    this->appendOutput(tStageNode);
}

}