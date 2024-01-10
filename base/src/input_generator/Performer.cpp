/*
 * Performer.cpp
 *
 *  Created on: April 14, 2022
 */

#include "Performer.hpp"
#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorPlatoMainOperationFileUtilities.hpp"
using namespace XMLGen;
namespace director
{
    Performer::Performer
    (const std::string& aName,
     const std::string& aCode,
     int aIdOffset,
     int aNumberRanks,
     int aConcurrentEvaluations) : 
     FileObject(aName, aConcurrentEvaluations),
     mCode(aCode),
     mIdOffset(aIdOffset),
     mNumberRanks(aNumberRanks)
    {
    }

    void Performer::write_interface
    (pugi::xml_node& aNode,
     std::string aEvaluationString)
    {
        auto tPerformerNode = aNode.append_child("Performer");
        addChild(tPerformerNode, "PerformerID", ID(aEvaluationString));
        addChild(tPerformerNode, "Name", name(aEvaluationString));
        addChild(tPerformerNode, "Code", mCode);
    }

    std::string Performer::return_mpirun(std::string aEvaluationString)
    {
        return std::string(": -np ") + 
                std::to_string(mNumberRanks) + 
                " -x PLATO_PERFORMER_ID=" + 
                ID(aEvaluationString) + 
                " \\\n";
    }
    std::string Performer::ID(const std::string& aEvaluationString)
    {
        if(evaluations() == 0)
            return std::to_string(mIdOffset);
        else if(aEvaluationString == "")
            return std::string("{E+") + std::to_string(mIdOffset) + "}";
        else
            return std::to_string(std::stoi(aEvaluationString) + mIdOffset);
    }
}