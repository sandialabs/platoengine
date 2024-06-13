/*
 * XMLGeneratorFixedBlockUtilities.hpp
 *
 *  Created on: Jul 9, 2021
 */

#pragma once

#include "Plato_FreeFunctions.hpp"
#include "XMLGeneratorOptimizationParametersMetadata.hpp"
#include <algorithm>

namespace XMLGen
{

namespace FixedBlock
{

/******************************************************************************//**
 * \fn is_material_state_solid
 * \brief Return true if all the elemement blocks are set to a solid material state. 
 * \param [in] aMetadata optimization block metadata
 * \return boolean flag (true or false)
**********************************************************************************/
inline bool is_material_state_solid
(XMLGen::OptimizationParameters& aMetadata)
{
    auto tFixedBlockMaterialStates = aMetadata.fixed_block_material_states();
    if(tFixedBlockMaterialStates.empty())
    {
        THROWERR("List of fixed block material states is empty. Material states must be defined.")
    }
    auto tIsSolidState = std::find(tFixedBlockMaterialStates.begin(), tFixedBlockMaterialStates.end(), "fluid") == tFixedBlockMaterialStates.end();
    return tIsSolidState;
}

/******************************************************************************//**
 * \fn check_fixed_block_material_states
 * \brief Check if material states are defined for the fixed blocks. 
 * \param [in] aMetadata optimization block metadata
**********************************************************************************/
inline void check_fixed_block_material_states
(XMLGen::OptimizationParameters &aMetadata)
{
    auto tFixedBlockMaterialStates = aMetadata.fixed_block_material_states();
    if ( tFixedBlockMaterialStates.empty() )
    {
        auto tNumFixedBlocks = aMetadata.fixed_block_ids().size();
        tFixedBlockMaterialStates.resize(tNumFixedBlocks);
        std::fill(tFixedBlockMaterialStates.begin(), tFixedBlockMaterialStates.end(), "solid");
        aMetadata.setFixedBlockMaterialStates(tFixedBlockMaterialStates);
    }
}

inline void check_fixed_blocks_values(
XMLGen::OptimizationParameters& aMetadata,
std::vector<std::string> &aValuesIn,
const std::string &aSolidFillValue, 
const std::string &aFluidsFillValue1,
const std::string &aFluidsFillValue2)
{
    if (aValuesIn.empty())
    {
        if(XMLGen::FixedBlock::is_material_state_solid(aMetadata))
        {
            auto tNumFixedBlocks = aMetadata.fixed_block_ids().size();
            aValuesIn.resize(tNumFixedBlocks);
            std::fill(aValuesIn.begin(), aValuesIn.end(), aSolidFillValue);
        }
        else
        {
            auto tFixedBlockMaterialStates = aMetadata.fixed_block_material_states();
            for(auto& tMaterialState : tFixedBlockMaterialStates)
            {
                auto tMaterialStateLower = Plato::tolower(tMaterialState);
                std::string tDomainValue = tMaterialStateLower == "solid" ? aFluidsFillValue1 : aFluidsFillValue2;
                aValuesIn.push_back(tDomainValue);
            }
        }
    }
}

/******************************************************************************//**
 * \fn check_fixed_blocks_domain_values
 * \brief Check if the design variables values 
 *        of the inner fixed block nodes were set by the user. If the values 
 *        were not set, set values to defaults based on the material state.
 * \param [in] aMetadata optimization block metadata
**********************************************************************************/
inline void check_fixed_blocks_domain_values
(XMLGen::OptimizationParameters& aMetadata)
{
    const std::string tSolidFillValue = "1.0";
    const std::string tFluidsValue1 = "0.0";
    const std::string tFluidsValue2 = "1.0";
    auto tFixedBlockDomainLowerValues = aMetadata.fixed_block_domain_lower_values();
    if (tFixedBlockDomainLowerValues.empty())
    {
        check_fixed_blocks_values(aMetadata, tFixedBlockDomainLowerValues, tSolidFillValue, tFluidsValue1, tFluidsValue2);  
        aMetadata.setFixedBlockDomainLowerValues(tFixedBlockDomainLowerValues);
    }
    auto tFixedBlockDomainUpperValues = aMetadata.fixed_block_domain_upper_values();
    if (tFixedBlockDomainUpperValues.empty())
    {
        check_fixed_blocks_values(aMetadata, tFixedBlockDomainUpperValues, tSolidFillValue, tFluidsValue1, tFluidsValue2);  
        aMetadata.setFixedBlockDomainUpperValues(tFixedBlockDomainUpperValues);
    }
}

/******************************************************************************//**
 * \fn check_fixed_blocks_boundary_values
 * \brief Check if the design variables values of the 
 *        nodes on the fixed block boundaries were set by the user. If the values 
 *        were not set, set values to defaults based on the material state.
 * \param [in,out] aMetadata optimization block metadata
**********************************************************************************/
inline void check_fixed_blocks_boundary_values
(XMLGen::OptimizationParameters& aMetadata)
{
    auto tFixedBlockBoundaryLowerValues = aMetadata.fixed_block_boundary_lower_values();
    if (tFixedBlockBoundaryLowerValues.empty())
    {
        const std::string tSolidFillValue = "0.5001";
        const std::string tFluidsValue1 = "0.4999";
        const std::string tFluidsValue2 = "0.5001";
        check_fixed_blocks_values(aMetadata, tFixedBlockBoundaryLowerValues, tSolidFillValue, tFluidsValue1, tFluidsValue2);  
        aMetadata.setFixedBlockBoundaryLowerValues(tFixedBlockBoundaryLowerValues);
    }
    auto tFixedBlockBoundaryUpperValues = aMetadata.fixed_block_boundary_upper_values();
    if (tFixedBlockBoundaryUpperValues.empty())
    {
        const std::string tSolidFillValue = "1.0";
        const std::string tFluidsValue1 = "0.4999";
        const std::string tFluidsValue2 = "0.5001";
        check_fixed_blocks_values(aMetadata, tFixedBlockBoundaryUpperValues, tSolidFillValue, tFluidsValue1, tFluidsValue2);  
        aMetadata.setFixedBlockBoundaryUpperValues(tFixedBlockBoundaryUpperValues);
    }
}

/******************************************************************************//**
 * \fn set_fixed_sideset_ids
 * \brief Set fixed sideset ids.
 * \param [in] aTokens parsed tokens
 * \param [out] aMetadata optimization block metadata
**********************************************************************************/
inline void set_fixed_sideset_ids
(const std::string & aTokens, 
 XMLGen::OptimizationParameters& aMetadata)
{
    char tValuesBuffer[10000];
    strcpy(tValuesBuffer, aTokens.c_str());
    std::vector<std::string> tFixedNodesetIDs;
    XMLGen::parse_tokens(tValuesBuffer, tFixedNodesetIDs);
    aMetadata.setFixedSidesetIDs(tFixedNodesetIDs);
}

/******************************************************************************//**
 * \fn set_fixed_nodeset_ids
 * \brief Set fixed nodesets ids.
 * \param [in] aTokens parsed tokens
 * \param [out] aMetadata optimization block metadata
**********************************************************************************/
inline void set_fixed_nodeset_ids
(const std::string & aTokens, 
 XMLGen::OptimizationParameters& aMetadata)
{
    char tValuesBuffer[10000];
    strcpy(tValuesBuffer, aTokens.c_str());
    std::vector<std::string> tFixedSidesetIDs;
    XMLGen::parse_tokens(tValuesBuffer, tFixedSidesetIDs);
    aMetadata.setFixedNodesetIDs(tFixedSidesetIDs);
}

/******************************************************************************//**
 * \fn set_fixed_block_ids
 * \brief Set fixed block ids.
 * \param [in] aTokens parsed tokens
 * \param [out] aMetadata optimization block metadata
**********************************************************************************/
inline void set_fixed_block_ids
(const std::string & aTokens, 
 XMLGen::OptimizationParameters& aMetadata)
{
    char tValuesBuffer[10000];
    strcpy(tValuesBuffer, aTokens.c_str());
    std::vector<std::string> tFixedBlocksetIDs;
    XMLGen::parse_tokens(tValuesBuffer, tFixedBlocksetIDs);
    aMetadata.setFixedBlockIDs(tFixedBlocksetIDs);
}

/******************************************************************************//**
 * \fn set_fixed_block_domain_lower_values
 * \brief Set the lower bound for the inner fixed block design variable values.
 * \param [in] aTokens parsed tokens
 * \param [out] aMetadata optimization block metadata
**********************************************************************************/
inline void set_fixed_block_domain_lower_values
(const std::string & aTokens, 
 XMLGen::OptimizationParameters& aMetadata)
{
    char tValuesBuffer[10000];
    strcpy(tValuesBuffer, aTokens.c_str());
    std::vector<std::string> tFixedBlockDomainLowerValues;
    XMLGen::parse_tokens(tValuesBuffer, tFixedBlockDomainLowerValues);
    aMetadata.setFixedBlockDomainLowerValues(tFixedBlockDomainLowerValues);
}

/******************************************************************************//**
 * \fn set_fixed_block_domain_upper_values
 * \brief Set the upper bound for the inner fixed block design variable values.
 * \param [in] aTokens parsed tokens
 * \param [out] aMetadata optimization block metadata
**********************************************************************************/
inline void set_fixed_block_domain_upper_values
(const std::string & aTokens, 
 XMLGen::OptimizationParameters& aMetadata)
{
    char tValuesBuffer[10000];
    strcpy(tValuesBuffer, aTokens.c_str());
    std::vector<std::string> tFixedBlockDomainUpperValues;
    XMLGen::parse_tokens(tValuesBuffer, tFixedBlockDomainUpperValues);
    aMetadata.setFixedBlockDomainUpperValues(tFixedBlockDomainUpperValues);
}

/******************************************************************************//**
 * \fn set_fixed_block_boundary_lower_values
 * \brief Set the lower bound for the design variable values on fixed block boundaries.
 * \param [in] aTokens parsed tokens
 * \param [out] aMetadata optimization block metadata
**********************************************************************************/
inline void set_fixed_block_boundary_lower_values
(const std::string & aTokens, 
 XMLGen::OptimizationParameters& aMetadata)
{
    char tValuesBuffer[10000];
    strcpy(tValuesBuffer, aTokens.c_str());
    std::vector<std::string> tFixedBlockBoundaryLowerValues;
    XMLGen::parse_tokens(tValuesBuffer, tFixedBlockBoundaryLowerValues);
    aMetadata.setFixedBlockBoundaryLowerValues(tFixedBlockBoundaryLowerValues);
}

/******************************************************************************//**
 * \fn set_fixed_block_boundary_upper_values
 * \brief Set the upper bound for the design variable values on fixed block boundaries.
 * \param [in] aTokens parsed tokens
 * \param [out] aMetadata optimization block metadata
**********************************************************************************/
inline void set_fixed_block_boundary_upper_values
(const std::string & aTokens, 
 XMLGen::OptimizationParameters& aMetadata)
{
    char tValuesBuffer[10000];
    strcpy(tValuesBuffer, aTokens.c_str());
    std::vector<std::string> tFixedBlockBoundaryUpperValues;
    XMLGen::parse_tokens(tValuesBuffer, tFixedBlockBoundaryUpperValues);
    aMetadata.setFixedBlockBoundaryUpperValues(tFixedBlockBoundaryUpperValues);
}

/******************************************************************************//**
 * \fn set_fixed_block_material_states
 * \brief Set fixed block material states (e.g. solid, fluid).
 * \param [in] aTokens parsed tokens
 * \param [out] aMetadata optimization block metadata
**********************************************************************************/
inline void set_fixed_block_material_states
(const std::string & aTokens, 
 XMLGen::OptimizationParameters& aMetadata)
{
    char tValuesBuffer[10000];
    strcpy(tValuesBuffer, aTokens.c_str());
    std::vector<std::string> tFixedBlockMaterialStates;
    XMLGen::parse_tokens(tValuesBuffer, tFixedBlockMaterialStates);
    aMetadata.setFixedBlockMaterialStates(tFixedBlockMaterialStates);
}

/******************************************************************************//**
 * \fn check_domain_values_array
 * \brief Check if the number of domain values matches the number of fixed blocks. 
 *        Function throws error if numbers do not match. 
 * \param [in] aMetadata optimization block metadata
**********************************************************************************/
inline void check_domain_values_array
(const XMLGen::OptimizationParameters& aMetadata)
{
     if (aMetadata.fixed_block_ids().size() != aMetadata.fixed_block_domain_lower_values().size())
     {
         THROWERR("Number of domain lower values '" + std::to_string(aMetadata.fixed_block_domain_lower_values().size()) + "' does not match the number of fixed blocks '" + std::to_string(aMetadata.fixed_block_ids().size()) + "'.")
     }
     if (aMetadata.fixed_block_ids().size() != aMetadata.fixed_block_domain_upper_values().size())
     {
         THROWERR("Number of domain upper values '" + std::to_string(aMetadata.fixed_block_domain_upper_values().size()) + "' does not match the number of fixed blocks '" + std::to_string(aMetadata.fixed_block_ids().size()) + "'.")
     }
}

/******************************************************************************//**
 * \fn check_boundary_values_array
 * \brief Check if the number of boundary values matches the number of fixed blocks. 
 *        Function throws error if numbers do not match. 
 * \param [in] aMetadata optimization block metadata
**********************************************************************************/
inline void check_boundary_values_array
(const XMLGen::OptimizationParameters& aMetadata)
{
    size_t tNumFixedBlocks = aMetadata.fixed_block_ids().size();
    size_t tNumFixedBlockBoundaryLowerValues = aMetadata.fixed_block_boundary_lower_values().size();
    size_t tNumFixedBlockBoundaryUpperValues = aMetadata.fixed_block_boundary_upper_values().size();

    if (tNumFixedBlocks != tNumFixedBlockBoundaryLowerValues)
    {
        THROWERR("Number of boundary lower values '" + std::to_string(tNumFixedBlockBoundaryLowerValues) + "' does not match the number of fixed blocks '" + std::to_string(tNumFixedBlocks) + "'.")
    }
    if (tNumFixedBlocks != tNumFixedBlockBoundaryUpperValues)
    {
        THROWERR("Number of boundary upper values '" + std::to_string(tNumFixedBlockBoundaryUpperValues) + "' does not match the number of fixed blocks '" + std::to_string(tNumFixedBlocks) + "'.")
    }
}

/******************************************************************************//**
 * \fn check_boundary_values_array
 * \brief Check if the number of material states matches the number of fixed blocks. 
 *        Function throws error if numbers do not match. 
 * \param [in] aMetadata optimization block metadata
**********************************************************************************/
inline void check_material_states_array
(const XMLGen::OptimizationParameters &aMetadata)
{
    auto tBlockIDs = aMetadata.fixed_block_ids();
    auto tMaterialStates = aMetadata.fixed_block_material_states();

    auto tNumFixedBlocks = tBlockIDs.size();
    if (tNumFixedBlocks != tMaterialStates.size())
    {
        THROWERR("Number of material states '" + std::to_string(tMaterialStates.size()) + "' does not match the number of fixed blocks '" + std::to_string(tNumFixedBlocks) + "'.")
    }
}

/******************************************************************************//**
 * \fn check_fixed_block_arrays
 * \brief Check domain_values_array, boundary_values_array, and material_states_array size. 
 *        Function throws error if numbers do not match. 
 * \param [in] aMetadata optimization block metadata
**********************************************************************************/
inline void check_fixed_block_arrays
(const XMLGen::OptimizationParameters &aMetadata)
{
    XMLGen::FixedBlock::check_domain_values_array(aMetadata);
    XMLGen::FixedBlock::check_boundary_values_array(aMetadata);
    XMLGen::FixedBlock::check_material_states_array(aMetadata);
}

/******************************************************************************//**
 * \fn check_fixed_block_metadata
 * \brief Check if fixed block metadata was properly defined. If metadata was not 
 *        properly defined, set values to defaults. 
 * \param [in] aTokens parsed tokens
 * \param [out] aMetadata optimization block metadata
**********************************************************************************/
inline void check_fixed_block_metadata
(XMLGen::OptimizationParameters& aMetadata)
{
    if( !aMetadata.fixed_block_ids().empty() )
    {
        XMLGen::FixedBlock::check_fixed_block_material_states(aMetadata);
        XMLGen::FixedBlock::check_fixed_blocks_domain_values(aMetadata);
        XMLGen::FixedBlock::check_fixed_blocks_boundary_values(aMetadata);
    }
}

}
// namespace FixedBlock

}
// namespace XMLGen
