/*
 * XMLGeneratorParseOptimizationParameters.hpp
 *
 *  Created on: Jan 11, 2021
 */

#pragma once

#include "XMLGeneratorParseMetadata.hpp"
#include "XMLGeneratorParserUtilities.hpp"
#include "XMLGeneratorOptimizationParametersMetadata.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \class ParseOptimizationParameters
 * \brief Parse inputs in OptimizationParameters block and store values in XMLGen::OptimizationParameters.
**********************************************************************************/
class ParseOptimizationParameters : public XMLGen::ParseMetadata<std::vector<XMLGen::OptimizationParameters>>
{
private:
    std::vector<XMLGen::OptimizationParameters> mData; /*!< OptimizationParameterss metadata */
    /*!< map from main optimization parameters tags to pair< valid tokens, pair<value,default> >, \n
     * i.e. map< tag, pair<valid tokens, pair<value,default>> > */
    XMLGen::MetaDataTags mTags;

private:
    /******************************************************************************//**
     * \fn setMeshMapData
     * \brief Set Scenario mesh map data
     * \param [in] aScenario Scenario metadata
    **********************************************************************************/
    void setMeshMapData(XMLGen::OptimizationParameters &aMetadata);

    /******************************************************************************//**
     * \fn allocate
     * \brief Allocate map from valid tags to valid tokens-value pair
    **********************************************************************************/
    void allocate();

    /******************************************************************************//**
     * \fn setTags
     * \brief Set optimization parameters metadata.
     * \param [in] aOptimizationParameters optimization parameters metadata
    **********************************************************************************/
    void setTags(XMLGen::OptimizationParameters& aOptimizationParameters);

    /******************************************************************************//**
     * \fn setFixedBlockIDs
     * \brief Set the fixed block metadata
     * \param [in] aMetadata optimization parameters metadata
    **********************************************************************************/
    void setFixedBlockIDs(XMLGen::OptimizationParameters &aMetadata);

    /******************************************************************************//**
     * \fn setFixedNodesetIDs
     * \brief Set the fixed nodeset metadata
     * \param [in] aMetadata optimization parameters metadata
    **********************************************************************************/
    void setFixedNodesetIDs(XMLGen::OptimizationParameters &aMetadata);

    /******************************************************************************//**
     * \fn setFixedSidesetIDs
     * \brief Set the fixed sideset metadata
     * \param [in] aMetadata optimization parameters metadata
    **********************************************************************************/
    void setFixedSidesetIDs(XMLGen::OptimizationParameters &aMetadata);

    /******************************************************************************//**
     * \fn setFixedBlockDomainValues
     * \brief Set density values corresponding to the fixed block domains 
     * \param [in] aMetadata optimization parameters metadata
    **********************************************************************************/
    void setFixedBlockDomainValues(XMLGen::OptimizationParameters &aMetadata);

    /******************************************************************************//**
     * \fn setFixedBlockBoundaryValues
     * \brief Set density values corresponding to the fixed block boundaries 
     * \param [in] aMetadata optimization parameters metadata
    **********************************************************************************/
    void setFixedBlockBoundaryValues(XMLGen::OptimizationParameters &aMetadata);

    /******************************************************************************//**
     * \fn setSymmetryPlaneLocationNames
     * \brief Set entity set location names where symmetry plane are applied.
     * \param [in] aMetadata optimization parameters metadata
    **********************************************************************************/
    void setSymmetryPlaneLocationNames(XMLGen::OptimizationParameters &aMetadata);

    /******************************************************************************//**
     * \fn setFixedBlockMaterialStates
     * \brief Set material state for each fixed block, options are solid and fluid. 
     * \param [in] aMetadata optimization parameters metadata
    **********************************************************************************/
    void setFixedBlockMaterialStates(XMLGen::OptimizationParameters &aMetadata);

    /******************************************************************************//**
     * \fn setDakotaDescriptorsAndBounds
     * \brief Set descriptors, lower and upper bounds and mdps_partitions for dakota
     * \param [in/out] aInputFile parsed input metadata
    **********************************************************************************/
    void setDakotaDescriptorsAndBounds(XMLGen::OptimizationParameters& aMetadata);

    /******************************************************************************//**
     * \fn checkHeavisideFilterParams
     * \brief Check for valid heaviside filter params
     * \param [in] aMetadata optimization parameters metadata
    **********************************************************************************/
    void checkHeavisideFilterParams(XMLGen::OptimizationParameters &aMetadata);

    /******************************************************************************//**
     * \fn setMetaData
     * \brief Set the metadata from the tag values
     * \param [in] aMetadata optimization parameters metadata
    **********************************************************************************/
    void setMetaData(XMLGen::OptimizationParameters &aMetadata);

    /******************************************************************************//**
     * \fn setCSMParameters
     * \brief Set the csm parameters 
     * \param [in] aMetadata optimization parameters metadata
    **********************************************************************************/
    void setCSMParameters(XMLGen::OptimizationParameters &aMetadata);

    /******************************************************************************//**
     * \fn autoFillRestartParameters
     * \brief Auto populate restart parameters based on user input
     * \param [in] aMetadata optimization parameters metadata
    **********************************************************************************/
    void autoFillRestartParameters(XMLGen::OptimizationParameters &aMetadata);

    /******************************************************************************//**
     * \fn checkROLSubProblemModel
     * \brief Auto populate ROL subproblem parameter based on user input
     * \param [in] aMetadata optimization parameters metadata
    **********************************************************************************/
    void checkROLSubProblemModel(XMLGen::OptimizationParameters &aMetadata);

    /******************************************************************************//**
     * \fn checkROLHessianType
     * \brief Auto populate hessian type parameter based on user input
     * \param [in] aMetadata optimization parameters metadata
    **********************************************************************************/
    void checkROLHessianType(XMLGen::OptimizationParameters &aMetadata);

    /******************************************************************************//**
     * \fn setOptimizationType
     * \brief Set the optimization type for this problem
     * \param [in] aMetadata optimization parameters metadata
    **********************************************************************************/
    void setOptimizationType(XMLGen::OptimizationParameters &aMetadata);

    /******************************************************************************//**
     * \fn setFilterInEngine
     * \brief Set whether to filter in the engine
     * \param [in] aMetadata optimization parameters metadata
    **********************************************************************************/
    void setFilterInEngine(XMLGen::OptimizationParameters &aMetadata);

    /******************************************************************************//**
     * \fn setEnforceBounds
     * \brief Set whether to enforce bounds in the engine
     * \param [in] aMetadata optimization parameters metadata
    **********************************************************************************/
    void setEnforceBounds(XMLGen::OptimizationParameters &aMetadata);

    /******************************************************************************//**
     * \fn setNormalizeInAggregator
     * \brief Set whether to normalize in the aggregator
     * \param [in] aMetadata optimization parameters metadata
    **********************************************************************************/
    void setNormalizeInAggregator(XMLGen::OptimizationParameters &aMetadata);
public:

    /******************************************************************************//**
     * \fn data
     * \brief Return optimization parameters metadata.
     * \return metadata
    **********************************************************************************/
    std::vector<XMLGen::OptimizationParameters> data() const override;

    /******************************************************************************//**
     * \fn parse
     * \brief Parse OptimizationParameters metadata.
     * \param [in] aInputFile input file metadata.
    **********************************************************************************/
    void parse(std::istream &aInputFile) override;
};

}
// namespace XMLGen
