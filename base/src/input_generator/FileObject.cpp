/*
 * FileObject.cpp
 *
 *  Created on: April 20, 2022
 */

#include "FileObject.hpp"
#include <regex>

namespace director
{

FileObject::FileObject
(const std::string& aName,
 int aConcurrentEvaluations) :
 mName(aName),
 mConcurrentEvaluations(aConcurrentEvaluations)
{
    mEvaluationTag = "{E}";
    mTagExpression = "\\{E\\}";

    if(mConcurrentEvaluations != 0)
        mName += "_" + mEvaluationTag;
}

std::string FileObject::name(std::string aEvaluationString)
{
    if(aEvaluationString=="")
        return mName;
    else
    {
        return std::regex_replace (mName,mTagExpression,aEvaluationString);
    }    
}

std::string FileObject::tag(std::string aEvaluationString)
{
    if(mConcurrentEvaluations == 0)
        return std::string("");
    if(aEvaluationString=="")
        return mEvaluationTag;
    else
        return aEvaluationString;
}

pugi::xml_node FileObject::forNode
(pugi::xml_node& aNode, 
 std::string aXMLLoopVectorName)
{
    if(mConcurrentEvaluations == 0)
        return aNode; 
    else
    {
        auto tForNode = aNode.append_child("For");
        tForNode.append_attribute("var") = "E";
        tForNode.append_attribute("in") = aXMLLoopVectorName.c_str();
        return tForNode;
    }
}

}