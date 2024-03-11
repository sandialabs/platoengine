/*
 * XMLGeneratorDakotaProblem.cpp
 *
 *  Created on: March 25, 2022
 */

#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorDataStruct.hpp"
#include "XMLGeneratorDakotaProblem.hpp"
#include "XMLGeneratorProblemUtilities.hpp"
#include "XMLGeneratorServiceUtilities.hpp"
#include "XMLGeneratorSierraSDUtilities.hpp"
#include "XMLGeneratorDefinesFileUtilities.hpp"
#include "XMLGeneratorLaunchScriptUtilities.hpp"
#include "XMLGeneratorPlatoESPInputFileUtilities.hpp"
#include "XMLGeneratorSierraSDInputDeckUtilities.hpp"
#include "XMLGeneratorPlatoMainInputFileUtilities.hpp"
#include "XMLGeneratorDakotaInterfaceFileUtilities.hpp"
#include "XMLGeneratorDakotaDriverInputFileUtilities.hpp"
#include "XMLGeneratorPlatoAnalyzeInputFileUtilities.hpp"
#include "XMLGeneratorPlatoESPOperationsFileUtilities.hpp"
#include "XMLGeneratorPlatoMainOperationFileUtilities.hpp"
#include "XMLGeneratorSierraSDOperationsFileUtilities.hpp"
#include "XMLGeneratorPlatoAnalyzeOperationsFileUtilities.hpp"


namespace XMLGen
{

namespace dakota
{

void write_performer_input_deck_file
(XMLGen::InputData& aMetaData)
{
    if(XMLGen::is_physics_performer(aMetaData))
    {
        std::string tMeshName = aMetaData.mesh.run_name;
        auto tEvaluations = std::stoi(aMetaData.optimization_parameters().concurrent_evaluations());
        for (int iEvaluation = 0; iEvaluation < tEvaluations; iEvaluation++)
        {
            std::string tTag = std::string("_") + std::to_string(iEvaluation);
            auto tAppendedMeshName = XMLGen::append_concurrent_tag_to_file_string(tMeshName,tTag);
            aMetaData.mesh.run_name = std::string("evaluations_") + std::to_string(iEvaluation) + std::string("/") + tAppendedMeshName;

            std::string tInputFileName = "";
            if(aMetaData.services()[0].code() == "plato_analyze")
            {
                auto tServiceID = XMLGen::get_plato_analyze_service_id(aMetaData);
                tInputFileName = std::string("plato_analyze_") + tServiceID + "_input_deck.xml";
                XMLGen::write_plato_analyze_input_deck_file(aMetaData);
            }
            else if(aMetaData.services()[0].code() == "sierra_sd")
            {
                auto tServiceID = XMLGen::get_salinas_service_id(aMetaData);
                tInputFileName = std::string("sierra_sd_") + tServiceID + "_input_deck.i";
                XMLGen::write_sierra_sd_input_deck(aMetaData);
            }
            XMLGen::Problem::move_input_deck_to_subdirectory(tInputFileName,iEvaluation);
        }
        aMetaData.mesh.run_name = tMeshName;
    }
}
// function write_performer_input_deck_file_dakota_problem

void write_performer_operation_xml_file
(const XMLGen::InputData& aMetaData)
{
    if(aMetaData.services().size() > 0)
    {
        if(aMetaData.services()[0].code() == "plato_analyze")
        {
            XMLGen::write_plato_analyze_operation_xml_file_dakota_problem(aMetaData);
        }
        else if(aMetaData.services()[0].code() == "sierra_sd")
        {
            XMLGen::write_sierra_sd_operation_xml_file(aMetaData);
        }
    }
}
//function write_performer_operation_xml_file_dakota_problem

void plato_esp_problem
(XMLGen::InputData& aMetaData,
 const std::vector<XMLGen::InputData>& aPreProcessedMetaData)
{
    XMLGen::Problem::create_concurrent_evaluation_subdirectories(aMetaData);
    XMLGen::write_define_xml_file(aMetaData);
    XMLGen::dakota::write_interface_xml_file(aMetaData);
    XMLGen::generate_launch_script(aMetaData);
    XMLGen::write_dakota_plato_main_operations_xml_file(aMetaData);
    XMLGen::write_amgx_input_file(aMetaData);
    XMLGen::write_plato_esp_input_deck_file(aMetaData);
    XMLGen::write_plato_esp_operations_file(aMetaData);

    XMLGen::Problem::write_plato_services_performer_input_deck_files(aMetaData);
    for(auto tCurMetaData : aPreProcessedMetaData)
    {
        XMLGen::dakota::write_performer_operation_xml_file(tCurMetaData);
        XMLGen::dakota::write_performer_input_deck_file(tCurMetaData);
    }

    XMLGen::write_plato_main_input_deck_file(aMetaData);
    XMLGen::write_dakota_driver_input_deck(aMetaData);
}
// function plato_esp_problem

void write_problem
(XMLGen::InputData& aMetaData,
 const std::vector<XMLGen::InputData>& aPreProcessedMetaData)
{
    auto tScenario = aMetaData.scenarios();
    for(unsigned int iScenario = 0 ; iScenario < tScenario.size(); iScenario++)
        XMLGen::dakota::plato_esp_problem(aMetaData, aPreProcessedMetaData);
}
// function write_problem

}
// namespace dakota

}
// namespace XMLGen