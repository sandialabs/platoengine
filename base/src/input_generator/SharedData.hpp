/*
 * SharedData.hpp
 *
 *  Created on: April 12, 2022
 */
#pragma once

#include <vector>
#include <string>

#include "pugixml.hpp"

#include "FileObject.hpp"
#include "Performer.hpp"

namespace director
{

class SharedData : public FileObject
{
    
protected:
    std::shared_ptr<Performer> mOwnerPerformer;
    std::vector< std::shared_ptr<Performer>> mUserPerformers;
    
public:
    SharedData(const std::string& aName,
                           std::shared_ptr<Performer> aOwnerPerformer,
                           const std::vector<std::shared_ptr<Performer>>& aUserPerformers,
                           int aConcurrentEvaluations = 0);
    virtual ~SharedData() = default;
    virtual void write_interface(pugi::xml_node& aNode,
                                 std::string aEvaluationString = "") = 0;
    
};

class SharedDataGlobal : public SharedData
{
    
private:
    std::string mSize;
    
public:
    SharedDataGlobal(const std::string& aName,
                                 const std::string& aSize,
                                 std::shared_ptr<Performer> aOwnerPerformer,
                                 const std::vector<std::shared_ptr<Performer>>& aUserPerformers,
                                 int aConcurrentEvaluations = 0);
    void write_interface(pugi::xml_node& aNode,
                         std::string aEvaluationString = "") override;

    std::string size(){return mSize;}
};

class SharedDataNodalField : public SharedData
{
    
public:
    SharedDataNodalField(const std::string& aName,
                                     std::shared_ptr<Performer> aOwnerPerformer,
                                     const std::vector<std::shared_ptr<Performer>>& aUserPerformers,
                                     int aConcurrentEvaluations = 0);
    void write_interface(pugi::xml_node& aNode,
                         std::string aEvaluationString = "") override;
    
};



}