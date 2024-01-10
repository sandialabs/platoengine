/*
 * Stage.hpp
 *
 *  Created on: April 14, 2022
 */
#pragma once

#include <vector>
#include <string>
#include "pugixml.hpp"

#include "FileObject.hpp"
#include "Operation.hpp"
#include "SharedData.hpp"

namespace director
{

class Stage : public FileObject
{
    
private:
    
    std::shared_ptr<SharedData> mInputSharedData;
    std::shared_ptr<SharedData> mOutputSharedData;
    std::vector<std::shared_ptr<Operation>> mOperations;

    void appendInput(pugi::xml_node& aNode,
                     const std::string& aTag = "");
    void appendOutput(pugi::xml_node& aNode);

public:
    Stage(const std::string& aName,
                      const std::vector<std::shared_ptr<Operation>>& aOperations,
                      std::shared_ptr<SharedData> aInputSharedData = nullptr,
                      std::shared_ptr<SharedData> aOutputSharedData = nullptr);

    void write(pugi::xml_document& aDocument);
    void write_dakota(pugi::xml_node& aDocument,
                      const std::string& aStageTag);

    std::string inputSharedDataName();
    std::string outputSharedDataName();
};

}