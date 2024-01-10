/*
 * GemmaProblem.hpp
 *
 *  Created on: March 25, 2022
 */

#pragma once

#include <vector>
#include "Operation.hpp"
#include "Performer.hpp"
#include "SharedData.hpp"
#include "Stage.hpp"
#include "XMLGeneratorUtilities.hpp"

namespace director
{

class Problem
{
protected:
    std::shared_ptr<Performer> mPerformerMain;
    std::shared_ptr<Performer> mPerformer;

    std::vector<std::shared_ptr<Operation>> mOperations;
    std::vector<Stage> mStages;
    
    std::shared_ptr<Stage> mDakotaStage;

    std::vector<std::shared_ptr<SharedData>> mSharedData;

    std::string mInterfaceFileName;
    std::string mOperationsFileName;
    std::string mDefinesFileName;
    std::string mInputDeckName;
    std::string mMPISourceName;
    std::string mVerbose;

public:
    Problem();
    virtual ~Problem() = default;
    
    virtual void write_plato_main_operations(pugi::xml_document& aDocument) = 0;
    virtual void write_plato_main_input(pugi::xml_document& aDocument) = 0;
    virtual void write_interface(pugi::xml_document& aDocument) = 0;
    virtual void write_mpirun(std::string aFileName) = 0;
    virtual void write_defines() = 0;
};

class GemmaProblem : public Problem 
{
private:
    int mNumParams;

public:
    GemmaProblem(const XMLGen::InputData& aMetaData);
    void write_plato_main_operations(pugi::xml_document& aDocument) override;
    void write_plato_main_input(pugi::xml_document& aDocument) override;
    void write_interface(pugi::xml_document& aDocument) override;
    void write_mpirun(std::string aFileName) override;
    void write_defines() override;
    void create_evaluation_subdirectories_and_gemma_input(const XMLGen::InputData& aMetaData);
    void create_matched_power_balance_input_deck(const XMLGen::InputData& aMetaData);
};

}