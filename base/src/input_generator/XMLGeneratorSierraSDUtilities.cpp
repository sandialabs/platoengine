/*
 * XMLGeneratorSierraSDUtilities.cpp
 *
 *  Created on: Jan 17, 2022
 */

#include "XMLGeneratorSierraSDUtilities.hpp"

namespace XMLGen
{

/******************************************************************************/
std::string get_salinas_service_id(const XMLGen::InputData &aMetaData)
{
    std::string tServiceID = "";
    for (auto &tID : aMetaData.objective.serviceIDs)
    {
        auto &tService = aMetaData.service(tID);
        if (tService.code() == "sierra_sd")
        {
            tServiceID = tID;
            break;
        }
    }
    if (tServiceID == "")
    {
        for (auto &tConstraint : aMetaData.constraints)
        {
            auto tID = tConstraint.service();
            auto &tService = aMetaData.service(tID);
            if (tService.code() == "sierra_sd")
            {
                tServiceID = tID;
                break;
            }
        }
    }
    return tServiceID;
}
// function get_salinas_service_id
/******************************************************************************/

/******************************************************************************/
bool do_tet10_conversion(const XMLGen::InputData& aMetaData)
{
    XMLGen::Service tService;
    XMLGen::Scenario tScenario;
    XMLGen::Criterion tCriterion;
    if (!XMLGen::extract_metadata_for_writing_sd_input_deck(aMetaData, tService, tScenario, tCriterion)) 
    {
        return false;
    }
    return tScenario.convert_to_tet10().length()>0;
}
// function do_tet10_conversion
/******************************************************************************/

/******************************************************************************/
bool extract_metadata_for_writing_sd_input_deck
(const XMLGen::InputData &aMetaData,
 XMLGen::Service &aService,
 XMLGen::Scenario &aScenario,
 XMLGen::Criterion &aCriterion)
{
    if(aMetaData.objective.serviceIDs.size() > 0)
    {
        auto tServiceID = aMetaData.objective.serviceIDs[0];
        aService = aMetaData.service(tServiceID);
        if(aService.code() == "sierra_sd")
        {
            auto tScenarioID = aMetaData.objective.scenarioIDs[0];
            aScenario = aMetaData.scenario(tScenarioID);
            auto tCriterionID = aMetaData.objective.criteriaIDs[0];
            aCriterion = aMetaData.criterion(tCriterionID);
            return true;
        }
    }
    return false;
}
// function extract_metadata_for_writing_sd_input_deck
/******************************************************************************/

}
// namespace XMLGen