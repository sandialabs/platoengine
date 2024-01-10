/*
 * Performer.hpp
 *
 *  Created on: April 12, 2022
 */
#pragma once

#include <vector>
#include <string>
#include <regex>

#include "FileObject.hpp"

#include "pugixml.hpp"


namespace director
{

class Performer : public FileObject
{
    
private:
    std::string mPerformerID;
    std::string mCode;
    int mIdOffset;
    int mNumberRanks;
       
public:
    Performer(const std::string& aName,
                          const std::string& aCode,
                          int aIdOffset = 0,
                          int aNumberRanks = 1,
                          int aConcurrentEvalutions = 0);

    void write_interface(pugi::xml_node& aNode,
                         std::string aEvaluationString = "");

    std::string return_mpirun(std::string aEvaluationString = "");
    
    int numberRanks(){return mNumberRanks;}
    
    std::string ID(const std::string& aEvaluationString);
};

}