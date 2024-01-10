/*
 * XMLGeneratorServiceUtilities.cpp
 *
 *  Created on: Mar 23, 2022
 *
 */

#include "XMLG_Macros.hpp"
#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorDataStruct.hpp"
#include "XMLGeneratorServiceUtilities.hpp"

namespace XMLGen
{

/******************************************************************************/
std::vector<size_t> 
get_service_indices
(const std::string& aTargetCodeName,
 const XMLGen::InputData& aInputMetaData)
{
    std::vector<size_t> tTargetServiceIndices;
    auto tLowerTargetCodeName = XMLGen::to_lower(aTargetCodeName);
    for(auto& tService : aInputMetaData.services())
    {
        auto tLowerCode = XMLGen::to_lower(tService.code());
        if(tLowerCode == tLowerTargetCodeName)
        {
            size_t tIndex = &tService - &aInputMetaData.services()[0];
            tTargetServiceIndices.push_back(tIndex);
        }
    }
    return tTargetServiceIndices;
}
// function get_service_indices
/******************************************************************************/

/******************************************************************************/
std::vector<std::string> 
get_values_from_service_metadata
(const std::string& aCodeName,
 const std::string& aTargetKey,
 const XMLGen::InputData& aInputMetaData)
{
    std::vector<std::string> tValues;
    auto tLowerCodeName = XMLGen::to_lower(aCodeName);
    auto tLowerTargetKey = XMLGen::to_lower(aTargetKey);
    auto tServiceIndices = XMLGen::get_service_indices(tLowerCodeName, aInputMetaData);
    for(auto tIndex : tServiceIndices)
    {
        auto tValue = aInputMetaData.service(tIndex).value(tLowerTargetKey);
        tValues.push_back(tValue);
    }
    return tValues;
}
// function get_values_from_service_metadata
/******************************************************************************/

/******************************************************************************/
bool service_needs_decomp
(const XMLGen::Service& aService,
 std::map<std::string,int>& hasBeenDecompedForThisNumberOfProcessors)
{
    std::string num_procs = aService.numberProcessors();
    assert_is_positive_integer(num_procs);
    bool need_to_decompose = num_procs.compare("1") != 0;
    if(need_to_decompose)
    {
        bool has_been_decomposed = hasBeenDecompedForThisNumberOfProcessors[num_procs]++ != 0;
        if (hasBeenDecompedForThisNumberOfProcessors.size() > 1)
            THROWERR("MESH HAS ALREADY BEEN DECOMPED FOR A DIFFERENT NUMBER OF PROCESSORS.")

        if (!has_been_decomposed)
        {
            return true;
        }
    }
    return false;
}
// function service_needs_decomp
/******************************************************************************/

/******************************************************************************/
std::string get_unique_decomp_service
(const XMLGen::InputData& aMetaData)
{
    std::string tReturn = "";
    std::map<std::string,int> hasBeenDecompedForThisNumberOfProcessors;
    for(size_t i=0; i<aMetaData.objective.serviceIDs.size(); ++i)
    {
        XMLGen::Service tService = aMetaData.service(aMetaData.objective.serviceIDs[i]);
        if(tService.code() != "plato_analyze" && tService.code() != "platomain")
            if (XMLGen::service_needs_decomp(tService, hasBeenDecompedForThisNumberOfProcessors))
                tReturn = aMetaData.objective.serviceIDs[i];
    }
    for(auto& tConstraint : aMetaData.constraints)
    {
        XMLGen::Service tService = aMetaData.service(tConstraint.service());
        if(tService.code() != "plato_analyze" && tService.code() != "platomain")
            if (XMLGen::service_needs_decomp(tService, hasBeenDecompedForThisNumberOfProcessors))
                tReturn = tConstraint.service();
    }
    return tReturn;
}
// function get_unique_decomp_service
/******************************************************************************/

/******************************************************************************/
bool need_update_problem_stage
(const XMLGen::InputData& aMetaData)
{
    for (auto &tService : aMetaData.services())
    {
        if (tService.updateProblem())
        {
            return true;
        }
    }
    return false;
}
// function need_update_problem_stage
/******************************************************************************/

/******************************************************************************/
int num_cache_states
(const std::vector<XMLGen::Service> &aServices)
{
    int tNumCacheStates = 0;
    for(auto &tService : aServices)
    {
        if(tService.cacheState())
        {
            tNumCacheStates++;
        }
    }
    return tNumCacheStates;
}
// function num_cache_states
/******************************************************************************/

/******************************************************************************/
void check_plato_app_service_type(const std::string& aType)
{
    if( aType == "plato_app" )
    {
        THROWERR("The 'run system call' function cannot be used with service of type 'plato_app'. The 'run system call' function can only be used with services of type 'web_app' and 'system_call'.")
    }
}
// function check_plato_app_service_type
/******************************************************************************/

/******************************************************************************/
bool is_physics_performer
(XMLGen::InputData& aMetaData)
{
    bool tReturn = false;
    if (aMetaData.services().size() > 0)
        if (aMetaData.services()[0].code() == "plato_analyze" || aMetaData.services()[0].code() == "sierra_sd")
            tReturn = true;

    return tReturn;
}
// function is_physics_performer
/******************************************************************************/

/******************************************************************************/
std::string get_concretized_service_name
(const XMLGen::Service& aService)
{
    return (std::string("plato_services_") + aService.id());
}
// function get_concretized_service_name
/******************************************************************************/

}
// namespace XMLGen