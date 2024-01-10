/*
 * FileObject.hpp
 *
 *  Created on: April 20, 2022
 */
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <regex>

#include "pugixml.hpp"

namespace director
{

class FileObject
{
private:
    std::string mName;
    int mConcurrentEvaluations;
    std::string mEvaluationTag;
    std::regex mTagExpression;

public:
    FileObject(const std::string& aName,
                           int aConcurrentEvaluations = 0);

    virtual ~FileObject() = default;

    int evaluations(){return mConcurrentEvaluations;}
    std::string name(std::string aEvaluationString = "");
    std::string tag(std::string aEvaluationString = "");

    std::string loopVar(){return "E";}
    pugi::xml_node forNode(pugi::xml_node& aNode,
                           std::string aXMLLoopVectorName);
};

}