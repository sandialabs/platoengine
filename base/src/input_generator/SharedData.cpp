/*
 * SharedData.cpp
 *
 *  Created on: April 14, 2022
 */

#include "SharedData.hpp"
#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorPlatoMainOperationFileUtilities.hpp"
using namespace XMLGen;
namespace director
{
    SharedData::SharedData
    (const std::string& aName,
     std::shared_ptr<Performer> aOwnerPerformer,
     const std::vector<std::shared_ptr<Performer>>& aUserPerformers,
     int aConcurrentEvaluations) :
     FileObject(aName,aConcurrentEvaluations)
    {
        mOwnerPerformer = aOwnerPerformer;
        mUserPerformers = aUserPerformers;
    }
    
    SharedDataGlobal::SharedDataGlobal
    (const std::string& aName,
     const std::string& aSize,
     std::shared_ptr<Performer> aOwnerPerformer,
     const std::vector<std::shared_ptr<Performer>>& aUserPerformers,
     int aConcurrentEvaluations) :
     SharedData(aName, aOwnerPerformer, aUserPerformers, aConcurrentEvaluations),
     mSize(aSize)
    {
    }

    void SharedDataGlobal::write_interface
    (pugi::xml_node& aNode,
     std::string aEvaluationString)
    {
        auto tSharedDataNode = aNode.append_child("SharedData");
        addChild(tSharedDataNode, "Name", name(aEvaluationString));
        addChild(tSharedDataNode, "Type", "Scalar");
        addChild(tSharedDataNode, "Layout", "Global");
        addChild(tSharedDataNode, "Size", mSize);   
        addChild(tSharedDataNode, "OwnerName", mOwnerPerformer->name(aEvaluationString) );
        for(unsigned int iUserPerformer = 0 ; iUserPerformer < mUserPerformers.size(); iUserPerformer++)
            addChild(tSharedDataNode, "UserName", mUserPerformers[iUserPerformer]->name(aEvaluationString));
    }

    SharedDataNodalField::SharedDataNodalField
    (const std::string& aName,
     std::shared_ptr<Performer> aOwnerPerformer,
     const std::vector<std::shared_ptr<Performer>>& aUserPerformers,
     int aConcurrentEvaluations) :
     SharedData(aName, aOwnerPerformer, aUserPerformers, aConcurrentEvaluations)
    {
    }

    void SharedDataNodalField::write_interface
    (pugi::xml_node& aNode,
     std::string aEvaluationString)
    {
        auto tSharedDataNode = aNode.append_child("SharedData");
        addChild(tSharedDataNode, "Name", name(aEvaluationString));
        addChild(tSharedDataNode, "Type", "Scalar");
        addChild(tSharedDataNode, "Layout", "Nodal Field");   
        addChild(tSharedDataNode, "OwnerName", mOwnerPerformer->name(aEvaluationString) );
        for(unsigned int iUserPerformer = 0 ; iUserPerformer < mUserPerformers.size(); iUserPerformer++)
            addChild(tSharedDataNode, "UserName", mUserPerformers[iUserPerformer]->name(aEvaluationString));
    }


}