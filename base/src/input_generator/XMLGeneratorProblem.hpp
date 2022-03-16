/*
 * XMLGeneratorProblem.hpp
 *
 *  Created on: Nov 20, 2020
 */

#pragma once

#include <iostream>

#include "Plato_FreeFunctions.hpp"
#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorDefinesFileUtilities.hpp"
#include "XMLGeneratorInterfaceFileUtilities.hpp"
#include "XMLGeneratorPlatoMainInputFileUtilities.hpp"
#include "XMLGeneratorPlatoMainOperationFileUtilities.hpp"
#include "XMLGeneratorPlatoESPInputFileUtilities.hpp"
#include "XMLGeneratorPlatoESPOperationsFileUtilities.hpp"
#include "XMLGeneratorPlatoXTKInputFile.hpp"
#include "XMLGeneratorPlatoXTKOperationFile.hpp"
#include "XMLGeneratorPlatoAnalyzeInputFileUtilities.hpp"
#include "XMLGeneratorPlatoAnalyzeOperationsFileUtilities.hpp"
#include "XMLGeneratorSierraSDInputDeckUtilities.hpp"
#include "XMLGeneratorSierraSDOperationsFileUtilities.hpp"
#include "XMLGeneratorSierraSDUtilities.hpp"
#include "XMLGeneratorLaunchScriptUtilities.hpp"
#include "XMLGeneratorPostOptimizationRunFileUtilities.hpp"
#include "XMLGeneratorDakotaInterfaceFileUtilities.hpp"
#include "XMLGeneratorDakotaDriverInputFileUtilities.hpp"

namespace XMLGen
{

namespace Problem
{

/******************************************************************************//**
 * \fn is_physics_performer
 * \brief check if service is physics performer
 * \param [in] aMetaData input metadata
**********************************************************************************/
inline bool is_physics_performer
(XMLGen::InputData& aMetaData)
{
    bool tReturn = false;
    if (aMetaData.services().size() > 0)
        if (aMetaData.services()[0].code() == "plato_analyze" || aMetaData.services()[0].code() == "sierra_sd")
            tReturn = true;

    return tReturn;
}

/******************************************************************************//**
 * \fn subdirectory_exists
 * \brief check if performer subdirectory exists
 * \param [in] aDirectoryName input name of directory
**********************************************************************************/
inline bool subdirectory_exists
(const std::string& aDirectoryName)
{
    bool tReturn = true;
    auto tCommand = std::string("[ -d \"") + aDirectoryName + std::string("\" ]");
    auto tExitStatus = Plato::system_with_status(tCommand.c_str());
    if (tExitStatus)
        tReturn = false;

    return tReturn;
}

/******************************************************************************//**
 * \fn create_subdirectory_for_evaluation
 * \brief create subdirectory and copy required files for performer
 * \param [in] aCsmFileName csm file name
 * \param [in] aEvaluation evaluation ID
**********************************************************************************/
inline void create_subdirectory_for_evaluation
(const std::string& aMeshFileName,
 const std::string& aCsmFileName,
 int aEvaluation)
{
    std::string tDirectoryName = std::string("evaluations_") + std::to_string(aEvaluation) + std::string("/");

    std::string tTag = std::string("_") + std::to_string(aEvaluation);
    auto tAppendedCsmFileName = XMLGen::append_concurrent_tag_to_file_string(aCsmFileName,tTag);
    auto tAppendedMeshFileName = XMLGen::append_concurrent_tag_to_file_string(aMeshFileName,tTag);

    std::string tCommand = std::string("mkdir ") + tDirectoryName;
    Plato::system_with_throw(tCommand.c_str());
    tCommand = std::string("cp ") + aCsmFileName + std::string(" ") + tDirectoryName + tAppendedCsmFileName;
    Plato::system_with_throw(tCommand.c_str());
    tCommand = std::string("cp ") + aMeshFileName + std::string(" ") + tDirectoryName + tAppendedMeshFileName;
    Plato::system_with_throw(tCommand.c_str());
}

/******************************************************************************//**
 * \fn create_concurrent_evaluation_subdirectories
 * \brief create directories for concurrent evaluation performers
 * \param [in] aMetaData input metadata
**********************************************************************************/
inline void create_concurrent_evaluation_subdirectories
(const XMLGen::InputData& aMetaData)
{
    std::string tMeshName = aMetaData.mesh.run_name;
    std::string tCsmFileName = aMetaData.optimization_parameters().csm_file();
    auto tEvaluations = std::stoi(aMetaData.optimization_parameters().concurrent_evaluations());
    for (int iEvaluation = 0; iEvaluation < tEvaluations; iEvaluation++)
    {
        if (!subdirectory_exists(std::string("evaluations_") + std::to_string(iEvaluation)))
            create_subdirectory_for_evaluation(tMeshName,tCsmFileName,iEvaluation);
    }
}

/******************************************************************************//**
 * \fn move_input_deck_to_subdirectory
 * \brief move performer input deck to subdirectory
 * \param [in] aInputFileName performer input file name
**********************************************************************************/
inline void move_input_deck_to_subdirectory
(const std::string& aInputFileName,
 int aEvaluation)
{
    std::string tDirectoryName = std::string("evaluations_") + std::to_string(aEvaluation) + std::string("/");

    std::string tTag = std::string("_") + std::to_string(aEvaluation);
    auto tAppendedInputFileName = XMLGen::append_concurrent_tag_to_file_string(aInputFileName,tTag);

    auto tCommand = std::string("mv ") + aInputFileName + std::string(" ") + tDirectoryName + tAppendedInputFileName;
    Plato::system_with_throw(tCommand.c_str());
}

/******************************************************************************//**
 * \fn write_plato_services_performer_input_deck_files
 * \brief Write the input deck file for the performer in the metada
 * \param [in] aMetaData input metadata
**********************************************************************************/
inline void write_plato_services_performer_input_deck_files
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
        move_input_deck_to_subdirectory(tInputFileName,iEvaluation);
    }
    aMetaData.mesh.run_name = tMeshName;
}

/******************************************************************************//**
 * \fn write_performer_operation_xml_file_gradient_based_problem
 * \brief Write the operations file for the performer in the metada
 * \param [in] aInputData input metadata
**********************************************************************************/
inline void write_performer_operation_xml_file_gradient_based_problem
(const XMLGen::InputData& aMetaData)
{
    if(aMetaData.services().size() > 0)
    {
        if(aMetaData.services()[0].code() == "plato_analyze")
        {
            if(aMetaData.services()[0].id() == "helmholtz")
            {
                XMLGen::write_plato_analyze_helmholtz_operation_xml_file(aMetaData);
            }
            else
            {
                XMLGen::write_plato_analyze_operation_xml_file(aMetaData);
            }
        }
        else if(aMetaData.services()[0].code() == "sierra_sd")
        {
            XMLGen::write_sierra_sd_operation_xml_file(aMetaData);
        }
    }
}

/******************************************************************************//**
 * \fn write_performer_operation_xml_file_dakota_problem
 * \brief Write the operations file for the performer in the metada
 * \param [in] aInputData input metadata
**********************************************************************************/
inline void write_performer_operation_xml_file_dakota_problem
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

/******************************************************************************//**
 * \fn write_performer_input_deck_file_gradient_based_problem
 * \brief Write the input deck file for the performer in the metada
 * \param [in] aInputData input metadata
**********************************************************************************/
inline void write_performer_input_deck_file_gradient_based_problem
(const XMLGen::InputData& aMetaData)
{
    if(aMetaData.services().size() > 0)
    {
        if(aMetaData.services()[0].code() == "plato_analyze")
        {
            if(aMetaData.services()[0].id() == "helmholtz")
            {
                XMLGen::write_plato_analyze_helmholtz_input_deck_file(aMetaData);
            }
            else
            {
                XMLGen::write_plato_analyze_input_deck_file(aMetaData);
            }
        }
        else if(aMetaData.services()[0].code() == "sierra_sd")
        {
            XMLGen::write_sierra_sd_input_deck(aMetaData);
        }
    }
}

/******************************************************************************//**
 * \fn write_performer_input_deck_file_dakota_problem
 * \brief Write the input deck file for the performer in the metada
 * \param [in] aMetaData input metadata
**********************************************************************************/
inline void write_performer_input_deck_file_dakota_problem
(XMLGen::InputData& aMetaData)
{
    if(is_physics_performer(aMetaData))
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
            move_input_deck_to_subdirectory(tInputFileName,iEvaluation);
        }
        aMetaData.mesh.run_name = tMeshName;
    }
}

/******************************************************************************//**
 * \fn write_optimization_problem
 * \brief Write input files needed to solve optimization problems with Plato Analyze.
 * \param [in] aInputData input metadata
**********************************************************************************/
inline void write_optimization_problem
(const XMLGen::InputData& aMetaData,
 const std::vector<XMLGen::InputData>& aPreProcessedMetaData)
{
    XMLGen::write_define_xml_file(aMetaData);
    XMLGen::write_interface_xml_file(aMetaData);
    XMLGen::generate_launch_script(aMetaData);
    XMLGen::write_plato_main_operations_xml_file(aMetaData);
    XMLGen::write_plato_main_input_deck_file(aMetaData);
    XMLGen::write_amgx_input_file(aMetaData);
    XMLGen::write_plato_esp_input_deck_file(aMetaData);
    XMLGen::write_plato_esp_operations_file(aMetaData);
    XMLGen::write_xtk_input_deck_file(aMetaData);
    XMLGen::write_xtk_operations_file(aMetaData);

    for(auto tCurMetaData : aPreProcessedMetaData)
    {
        write_performer_operation_xml_file_gradient_based_problem(tCurMetaData);
        write_performer_input_deck_file_gradient_based_problem(tCurMetaData);
    }

    XMLGen::write_post_optimization_run_files(aMetaData);
}

/******************************************************************************//**
 * \fn write_dakota_problem
 * \brief Write input files needed to solve problems with dakota.
 * \param [in] aInputData input metadata
**********************************************************************************/
inline void write_dakota_problem
(XMLGen::InputData& aMetaData,
 const std::vector<XMLGen::InputData>& aPreProcessedMetaData)
{
    create_concurrent_evaluation_subdirectories(aMetaData);
    XMLGen::write_define_xml_file(aMetaData);
    XMLGen::write_dakota_interface_xml_file(aMetaData);
    XMLGen::generate_launch_script(aMetaData);
    XMLGen::write_dakota_plato_main_operations_xml_file(aMetaData);
    XMLGen::write_amgx_input_file(aMetaData);
    XMLGen::write_plato_esp_input_deck_file(aMetaData);
    XMLGen::write_plato_esp_operations_file(aMetaData);
    XMLGen::write_xtk_input_deck_file(aMetaData);
    XMLGen::write_xtk_operations_file(aMetaData);

    write_plato_services_performer_input_deck_files(aMetaData);
    for(auto tCurMetaData : aPreProcessedMetaData)
    {
        write_performer_operation_xml_file_dakota_problem(tCurMetaData);
        write_performer_input_deck_file_dakota_problem(tCurMetaData);
    }

    XMLGen::write_plato_main_input_deck_file(aMetaData);
    XMLGen::write_dakota_driver_input_deck(aMetaData);
}

}
// namespace Problem

}
// namespace XMLGen
