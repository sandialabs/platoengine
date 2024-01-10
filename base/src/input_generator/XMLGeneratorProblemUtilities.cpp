/*
 * XMLGeneratorProblemUtilities.cpp
 *
 *  Created on: March 25, 2022
 */

#include "Plato_FreeFunctions.hpp"
#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorDataStruct.hpp"
#include "XMLGeneratorProblemUtilities.hpp"
#include "XMLGeneratorPlatoMainInputFileUtilities.hpp"

namespace XMLGen
{

namespace Problem
{

void create_subdirectory_for_evaluation
(const std::string& aCsmFileName,
 int aEvaluation)
{
    std::string tDirectoryName = std::string("evaluations_") + std::to_string(aEvaluation) + std::string("/");

    std::string tTag = std::string("_") + std::to_string(aEvaluation);
    auto tAppendedCsmFileName = XMLGen::append_concurrent_tag_to_file_string(aCsmFileName,tTag);

    std::string tCommand = std::string("mkdir ") + tDirectoryName;
    Plato::system_with_throw(tCommand.c_str());
    tCommand = std::string("cp ") + aCsmFileName + std::string(" ") + tDirectoryName + tAppendedCsmFileName;
    Plato::system_with_throw(tCommand.c_str());
}
// function create_subdirectory_for_evaluation

void create_concurrent_evaluation_subdirectories
(const XMLGen::InputData& aMetaData)
{
    std::string tCsmFileName = aMetaData.optimization_parameters().csm_file();
    auto tEvaluations = std::stoi(aMetaData.optimization_parameters().concurrent_evaluations());
    for (int iEvaluation = 0; iEvaluation < tEvaluations; iEvaluation++)
    {
        if (!XMLGen::subdirectory_exists(std::string("evaluations_") + std::to_string(iEvaluation)))
            XMLGen::Problem::create_subdirectory_for_evaluation(tCsmFileName,iEvaluation);
    }
}
// function create_concurrent_evaluation_subdirectories

void move_input_deck_to_subdirectory
(const std::string& aInputFileName,
 int aEvaluation)
{
    std::string tDirectoryName = std::string("evaluations_") + std::to_string(aEvaluation) + std::string("/");

    std::string tTag = std::string("_") + std::to_string(aEvaluation);
    auto tAppendedInputFileName = XMLGen::append_concurrent_tag_to_file_string(aInputFileName,tTag);

    auto tCommand = std::string("mv ") + aInputFileName + std::string(" ") + tDirectoryName + tAppendedInputFileName;
    Plato::system_with_throw(tCommand.c_str());
}
// function move_input_deck_to_subdirectory

void write_plato_services_performer_input_deck_files
(XMLGen::InputData& aMetaData)
{
    std::string tMeshName = aMetaData.mesh.run_name;
    std::string tInputFileName = "plato_main_input_deck.xml";
    auto tEvaluations = std::stoi(aMetaData.optimization_parameters().concurrent_evaluations());
    for (int iEvaluation = 0; iEvaluation < tEvaluations; iEvaluation++)
    {
        std::string tTag = std::string("_") + std::to_string(iEvaluation);
        auto tAppendedMeshName = XMLGen::append_concurrent_tag_to_file_string(tMeshName,tTag);
        aMetaData.mesh.run_name = std::string("evaluations_") + std::to_string(iEvaluation) + std::string("/") + tAppendedMeshName;
        XMLGen::write_plato_main_input_deck_file(aMetaData);
        XMLGen::Problem::move_input_deck_to_subdirectory(tInputFileName,iEvaluation);
    }
    aMetaData.mesh.run_name = tMeshName;
}
// function write_plato_services_performer_input_deck_files

}
// namespace Problem

}
// namespace XMLGen