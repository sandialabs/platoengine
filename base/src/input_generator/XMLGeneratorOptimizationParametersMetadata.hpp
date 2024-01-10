/*
 * XMLGeneratorOptimizationParametersMetadata.hpp
 *
 *  Created on: Jan 11, 2021
 */

#pragma once

#include <vector>
#include <string>
#include <unordered_map>

namespace XMLGen
{
    enum OptimizationType
    {
        OT_TOPOLOGY=0,
        OT_SHAPE,
        OT_DAKOTA,
        OT_UNKNOWN
    };

/******************************************************************************//**
 * \struct OptimizationParameters
 * \brief OptimizationParameters metadata for Plato problems.
**********************************************************************************/
struct OptimizationParameters
{
    struct ValueData
    {
        bool mIsDefault;
        std::vector<std::string> mValue;
    };

// private member data
private:
    bool mNormalizeInAggregator = false;
    bool mEnforceBounds = false;
    bool mFilterInEngine = true;
    OptimizationType mOptimizationType = OT_TOPOLOGY;
    bool mIsARestartRun = false;
    std::unordered_map<std::string, ValueData> mMetaData; /*!< OptimizationParameters metadata, map< tag, value > */
    //std::unordered_map<std::string, std::string> mMetaData; /*!< OptimizationParameters metadata, map< tag, value > */
    std::vector<std::string> mLevelsetNodesets;
    std::vector<std::string> mFixedBlockIDs;
    std::vector<std::string> mFixedSidesetIDs;
    std::vector<std::string> mFixedNodesetIDs;
    std::vector<std::string> mFixedBlockDomainValues;
    std::vector<std::string> mFixedBlockBoundaryValues;
    std::vector<std::string> mFixedBlockMaterialStates;
    std::vector<std::string> mSymmetryPlaneOrigin;
    std::vector<std::string> mSymmetryPlaneNormal;
    std::vector<std::string> mSymmetryPlaneLocationNames;

// private member functions
private:
    /******************************************************************************//**
     * \fn getValue
     * \brief Return first value on the list of strings with key defined by 'aTag'. \n 
     *   If key is not defined, returned and empty string.
     * \param [in] aTag key
     * \return first value
    **********************************************************************************/
    std::string getValue(const std::string& aTag) const;

    /******************************************************************************//**
     * \fn getValues
     * \brief Return list of strings with key defined by 'aTag'. If key is not defined, \n
     *   returned a list of size one. The only element in the list is defined by an \n
     *   empty string.
     * \param [in] aTag ket
     * \return list
    **********************************************************************************/
    std::vector<std::string> getValues(const std::string& aTag) const;

    /******************************************************************************//**
     * \fn getBool
     * \brief Return bool value for property with input tag; else, throw an error if \n
     * property is not defined in the metadata.
     * \param [in] aTag property tag
     * \return property bool value
    **********************************************************************************/
    bool getBool(const std::string& aTag) const;

public:
    /******************************************************************************//**
     * \fn normalizeInAggregator
     * \brief Return whether to normalize in the aggregator
     * \return whether to normalize in the aggregator
    **********************************************************************************/
    bool normalizeInAggregator() const { return mNormalizeInAggregator; }

    /******************************************************************************//**
     * \fn normalizeInAggregator
     * \brief Set whether to normalize in the aggregator
    **********************************************************************************/
    void normalizeInAggregator(const bool& aVal) { mNormalizeInAggregator = aVal; }

    /******************************************************************************//**
     * \fn enforceBounds
     * \brief Return whether to enforce bounds in the engine
     * \return whether to enforce bounds in the engine
    **********************************************************************************/
    bool enforceBounds() const { return mEnforceBounds; }

    /******************************************************************************//**
     * \fn enforceBounds
     * \brief Set whether to enforce bounds in engine type
    **********************************************************************************/
    void enforceBounds(const bool& aVal) { mEnforceBounds = aVal; }

    /******************************************************************************//**
     * \fn filterInEngine
     * \brief Return whether to filter in the engine
     * \return whether to filter in engine
    **********************************************************************************/
    bool filterInEngine() const { return mFilterInEngine; }

    /******************************************************************************//**
     * \fn filterInEngine
     * \brief Set whether to filter in engine type
    **********************************************************************************/
    void filterInEngine(const bool& aVal) { mFilterInEngine = aVal; }

    /******************************************************************************//**
     * \fn optimizationType
     * \brief Return the optimization type
     * \return optimization type
    **********************************************************************************/
    OptimizationType optimizationType() const { return mOptimizationType; }

    /******************************************************************************//**
     * \fn optimizationType
     * \brief Set optimization type
    **********************************************************************************/
    void optimizationType(const OptimizationType& aVal) { mOptimizationType = aVal; }

    /******************************************************************************//**
     * \fn isARestartRun
     * \brief Return whether this is a restart run or not
     * \return whether this is a restart run or not
    **********************************************************************************/
    bool isARestartRun() const { return mIsARestartRun; }

    /******************************************************************************//**
     * \fn isARestartRun
     * \brief Set whether this is a restart run or not
    **********************************************************************************/
    void isARestartRun(const bool& aVal) { mIsARestartRun = aVal; }

    /******************************************************************************//**
     * \fn symmetryNormal
     * \brief Return scenario symmetry plane normal
     * \return scenario symmetry plane normal
    **********************************************************************************/
    std::vector<std::string> symmetryNormal() const {return mSymmetryPlaneNormal;}

    /******************************************************************************//**
     * \fn symmetryOrigin
     * \brief Return scenario symmetry plane origin
     * \return scenario symmetry plane origin
    **********************************************************************************/
    std::vector<std::string> symmetryOrigin() const {return mSymmetryPlaneOrigin;}

    /******************************************************************************//**
     * \fn symmetryOrigin
     * \brief Set scenario symmetry plane origin
     * \param [in] aOrigin symmetry plane origin
    **********************************************************************************/
    void symmetryOrigin(std::vector<std::string>& aOrigin) {mSymmetryPlaneOrigin = aOrigin;}

    /******************************************************************************//**
     * \fn symmetryNormal
     * \brief Set scenario symmetry plane normal
     * \param [in] aNormal symmetry plane normal
    **********************************************************************************/
    void symmetryNormal(std::vector<std::string>& aNormal) {mSymmetryPlaneNormal = aNormal;}

    /******************************************************************************//**
     * \fn symmetryOrigin
     * \brief Set entity set location names where symmetry plane constraints are applied.
     * \param [in] aInput entity set location names
    **********************************************************************************/
    void symmetryPlaneLocationNames(std::vector<std::string>& aInput) {mSymmetryPlaneLocationNames = aInput;}

    /******************************************************************************//**
     * \fn value
     * \brief Return first element on the list; else, throw an error if input key/tag is not defined.
     * \param [in] aTag property tag
     * \return value
    **********************************************************************************/
    std::string value(const std::string& aTag) const;

    /******************************************************************************//**
     * \fn value
     * \brief Return list of values; else, throw an error if input key/tag is not defined.
     * \param [in] aTag property tag
     * \return list
    **********************************************************************************/
    std::vector<std::string> values(const std::string& aTag) const;

    /******************************************************************************//**
     * \fn find
     * \brief Return sequence container associated with associative key.
     * \param [in] aKey associative key
     * \return sequence container. if not defined, return empty sequence container
    **********************************************************************************/
    std::vector<std::string> find(const std::string& aKey) const;

    /******************************************************************************//**
     * \fn tags
     * \brief Return list of parameter tags.
     * \return parameter tags
    **********************************************************************************/
    std::vector<std::string> tags() const;

    /******************************************************************************//**
     * \fn append
     * \brief Append parameter to metadata.
     * \param [in] aTag   parameter tag
     * \param [in] aValue parameter value
     * \param [in] aIsDefault parameter specifying whether this is a default or user set value
    **********************************************************************************/
    void append(const std::string& aTag, const std::string& aValue, const bool& aIsDefault=false);

    /******************************************************************************//**
     * \fn set
     * \brief Set metadata.
     * \param [in] aTag   parameter tag
     * \param [in] aList  parameter list
     * \param [in] aIsDefault parameter specifying whether this is a default or user set value
    **********************************************************************************/
    void set(const std::string& aTag, const std::vector<std::string>& aList, const bool& aIsDefault=false);

    /******************************************************************************//**
     * \fn filter_before_symmetry_enforcement 
     * \brief Return string value for keyword 'filter_before_symmetry_enforcement'.
     * \return value
    **********************************************************************************/
    std::string filter_before_symmetry_enforcement() const;

    /******************************************************************************//**
     * \fn mesh_map_filter_radius 
     * \brief Return string value for keyword 'mesh_map_filter_radius'.
     * \return value
    **********************************************************************************/
    std::string mesh_map_filter_radius() const;

    /******************************************************************************//**
     * \fn mesh_map_search_tolerance 
     * \brief Return string value for keyword 'mesh_map_search_tolerance'.
     * \return value
    **********************************************************************************/
    std::string mesh_map_search_tolerance() const;

    /******************************************************************************//**
     * \fn descriptors
     * \brief Set optimizable parameter descriptors.
     * \param [in] aDescriptors sequence container
    **********************************************************************************/
    void descriptors(const std::vector<std::string> &aDescriptors);

    /******************************************************************************//**
     * \fn descriptors
     * \brief Return optimizable parameter descriptors.
     * \return sequence container. if not defined, return empty sequence container
    **********************************************************************************/
    std::vector<std::string> descriptors() const { return this->find("descriptors"); }

    /******************************************************************************//**
     * \fn lower_bounds
     * \brief Return lower bounds for optimization parameters.
     * \return sequence container. if not defined, return empty sequence container
    **********************************************************************************/
    std::vector<std::string> lower_bounds() const { return this->find("lower_bounds"); }

    /******************************************************************************//**
     * \fn upper_bounds
     * \brief Return lower bounds for optimization parameters.
     * \return sequence container. if not defined, return empty sequence container
    **********************************************************************************/
    std::vector<std::string> upper_bounds() const { return this->find("upper_bounds"); }

    /******************************************************************************//**
     * \fn lower_bounds
     * \brief Sets lower bounds for optimization parameters.
    **********************************************************************************/
    void lower_bounds(const std::vector<std::string> &aLowerBounds);

    /******************************************************************************//**
     * \fn upper_bounds
     * \brief Sets upper bounds for optimization parameters.
    **********************************************************************************/
    void upper_bounds(const std::vector<std::string> &aUpperBounds);

    /******************************************************************************//**
     * \fn mdps_bounds
     * \brief Sets mdps for optimization parameters.
    **********************************************************************************/
    void mdps_partitions(const std::vector<std::string> &aMDPSPartitions);

    /******************************************************************************//**
     * \fn mdps_partitions
     * \brief set lower bounds for optimization parameters.
     * 
    **********************************************************************************/
    std::vector<std::string> mdps_partitions() const { return this->find("mdps_partitions"); }



    /******************************************************************************//**
     * \fn setFixedBlockIDs
     * \brief Set the fixed block ids
     * \param [in] aFixedBlockIDs vector of strings with fixed block ids
    **********************************************************************************/
    void setFixedBlockIDs(const std::vector<std::string> &aFixedBlockIDs) { mFixedBlockIDs = aFixedBlockIDs; }

    /******************************************************************************//**
     * \fn setFixedSidesetIDs
     * \brief Set the fixed sideset ids
     * \param [in] aFixedSidesetIDs vector of strings with fixed sideset ids
    **********************************************************************************/
    void setFixedSidesetIDs(const std::vector<std::string> &aFixedSidesetIDs) { mFixedSidesetIDs = aFixedSidesetIDs; }

    /******************************************************************************//**
     * \fn setFixedNodesetIDs
     * \brief Set the fixed nodeset ids
     * \param [in] aFixedNodesetIDs vector of strings with fixed nodeset ids
    **********************************************************************************/
    void setFixedNodesetIDs(const std::vector<std::string> &aFixedNodesetIDs) { mFixedNodesetIDs = aFixedNodesetIDs; }

    /******************************************************************************//**
     * \fn setFixedBlockDomainValues
     * \brief Set density values for the nodes associated with the fixed block domains. 
     *        assign to the domain.
     * \param [in] aFixedBlockDomainValues list of homogeneous density values 
    **********************************************************************************/
    void setFixedBlockDomainValues(const std::vector<std::string> &aFixedBlockDomainValues) { mFixedBlockDomainValues = aFixedBlockDomainValues; }

    /******************************************************************************//**
     * \fn setFixedBlockBoundaryValues
     * \brief Set density values for the nodes associated with the fixed block boundaries.
     * \param [in] aFixedBlockBoundaryValues list of homogeneous density values 
    **********************************************************************************/
    void setFixedBlockBoundaryValues(const std::vector<std::string> &aFixedBlockBoundaryValues) { mFixedBlockBoundaryValues = aFixedBlockBoundaryValues; }

    /******************************************************************************//**
     * \fn setFixedBlockMaterialStates
     * \brief Set material state (options: solid and fluid) for each fixed block. 
     * \param [in] aFixedBlockMaterialStates list of material states 
    **********************************************************************************/
    void setFixedBlockMaterialStates(const std::vector<std::string> &aFixedBlockMaterialStates) { mFixedBlockMaterialStates = aFixedBlockMaterialStates; }

    /******************************************************************************//**
     * \fn setLevelsetNodesets
     * \brief Set the levelset nodeset ids
     * \param [in] aLevelsetNodesets vector of strings with levelset nodeset ids
    **********************************************************************************/
    void setLevelsetNodesets(const std::vector<std::string> &aLevelsetNodesets) { mLevelsetNodesets = aLevelsetNodesets; }

    /******************************************************************************//**
     * \fn addFixedBlockID
     * \brief Add a fixed block id to the fixed block id list
     * \param [in] aFixedBlockID string with the fixed block id
    **********************************************************************************/
    void addFixedBlockID(const std::string &aFixedBlockID) { mFixedBlockIDs.push_back(aFixedBlockID); }

    /******************************************************************************//**
     * \fn addFixedNodesetID
     * \brief Add a fixed nodeset id to the fixed nodeset id list
     * \param [in] aFixedNodesetID string with the fixed nodeset id
    **********************************************************************************/
    void addFixedNodesetID(const std::string &aFixedNodesetID) {  mFixedNodesetIDs.push_back(aFixedNodesetID); }

    /******************************************************************************//**
     * \fn addFixedSidesetID
     * \brief Add a fixed sideset id to the fixed sideset id list
     * \param [in] aFixedSidesetID string with the fixed sideset id
    **********************************************************************************/
    void addFixedSidesetID(const std::string &aFixedSidesetID) { mFixedSidesetIDs.push_back(aFixedSidesetID); }

    /******************************************************************************//**
     * \fn addLevelsetNodeset
     * \brief Add a levelset nodeset id to the levelset nodeset id list
     * \param [in] aLevelsetNodesetID string with the levelset nodeset id
    **********************************************************************************/
    void addLevelsetNodeset(const std::string &aLevelsetNodesetID) { mLevelsetNodesets.push_back(aLevelsetNodesetID); }

    /******************************************************************************//**
     * \fn needsMeshMap 
     * \brief Return bool stating whether a mesh map is needed
     * \return value
    **********************************************************************************/
    bool needsMeshMap() const;

    // Functions to get member values
    std::string optimization_algorithm() const {return value("optimization_algorithm");} 
    std::string normalize_in_aggregator() const {return value("normalize_in_aggregator");} 
    std::string number_refines() const {return value("number_refines");}
    std::string number_prune_and_refine_processors() const {return value("number_prune_and_refine_processors");}
    std::string prune_and_refine_path() const {return value("prune_and_refine_path");}
    std::string number_buffer_layers() const {return value("number_buffer_layers");}
    std::string prune_mesh() const {return value("prune_mesh");}
    std::string prune_threshold() const {return value("prune_threshold");}
    std::string num_shape_design_variables() const {return value("num_shape_design_variables");}
    std::string initial_guess_file_name() const {return value("initial_guess_file_name");}
    std::string initial_guess_field_name() const {return value("initial_guess_field_name");}
    std::string verbose() const {return value("verbose");}
    std::string optimization_type() const {return value("optimization_type");}
    std::string check_gradient() const {return value("check_gradient");}
    std::string check_hessian() const {return value("check_hessian");}
    std::string max_iterations() const {return value("max_iterations");}
    std::string filter_in_engine() const {return value("filter_in_engine");}
    void filter_in_engine(const std::string& aValue) { append("filter_in_engine", aValue, true); }
    std::string output_method() const {return value("output_method");}
    std::string discretization() const {return value("discretization");}
    std::string initial_density_value() const {return value("initial_density_value");}
    std::string restart_iteration() const {return value("restart_iteration");}
    std::string create_levelset_spheres() const {return value("create_levelset_spheres");}
    std::string levelset_initialization_method() const {return value("levelset_initialization_method");}
    std::string levelset_material_box_min() const {return value("levelset_material_box_min");}
    std::string levelset_material_box_max() const {return value("levelset_material_box_max");}
    std::string levelset_sphere_radius() const {return value("levelset_sphere_radius");}
    std::string levelset_sphere_packing_factor() const {return value("levelset_sphere_packing_factor");}
    std::vector<std::string> levelset_nodesets() const {return mLevelsetNodesets;}
    std::vector<std::string> fixed_block_ids() const {return mFixedBlockIDs;}
    std::vector<std::string> fixed_nodeset_ids() const {return mFixedNodesetIDs;}
    std::vector<std::string> fixed_sideset_ids() const {return mFixedSidesetIDs;}
    std::vector<std::string> fixed_block_domain_values() const {return mFixedBlockDomainValues;}
    std::vector<std::string> fixed_block_boundary_values() const {return mFixedBlockBoundaryValues;}
    std::vector<std::string> fixed_block_material_states() const {return mFixedBlockMaterialStates;}
    std::vector<std::string> symmetry_plane_location_names() const {return mSymmetryPlaneLocationNames;}
    std::string hessian_type() const {return value("hessian_type");}
    std::string limited_memory_storage() const {return value("limited_memory_storage");}
    std::string problem_update_frequency() const {return value("problem_update_frequency");}
    std::string objective_number_standard_deviations() const {return value("objective_number_standard_deviations");}
    std::string filter_radius_scale() const {return value("filter_radius_scale");}
    std::string filter_radius_absolute() const {return value("filter_radius_absolute");}
    std::string boundary_sticking_penalty() const {return value("boundary_sticking_penalty");}
    std::string filter_projection_start_iteration() const {return value("filter_projection_start_iteration");}
    std::string filter_projection_update_interval() const {return value("filter_projection_update_interval");}
    std::string filter_use_additive_continuation() const {return value("filter_use_additive_continuation");}
    std::string filter_power() const {return value("filter_power");}
    std::string filter_heaviside_scale() const {return value("filter_heaviside_scale");}
    std::string filter_heaviside_min() const {return value("filter_heaviside_min");}
    std::string filter_heaviside_update() const {return value("filter_heaviside_update");}
    std::string filter_heaviside_max() const {return value("filter_heaviside_max");}
    std::string filter_type() const {return value("filter_type");}
    std::string filter_service() const {return value("filter_service");}
    std::string projection_type() const {return value("projection_type");}
    std::string write_restart_file() const {return value("write_restart_file");}
    std::string output_frequency() const {return value("output_frequency");}
    std::string csm_file() const {return value("csm_file");}
    std::string csm_exodus_file() const {return value("csm_exodus_file");}
    std::string csm_tesselation_file() const {return value("csm_tesselation_file");}
    std::string csm_opt_file() const {return value("csm_opt_file");}
    std::string enforce_bounds() const {return value("enforce_bounds");}
    std::string reset_algorithm_on_update() const {return value("reset_algorithm_on_update");}
    std::string rol_subproblem_model() const {return value("rol_subproblem_model");}
    std::string rol_lin_more_cauchy_initial_step_size() const {return value("rol_lin_more_cauchy_initial_step_size");}
    std::string rol_initial_trust_region_radius() const {return value("rol_initial_trust_region_radius");}
    std::string rol_gradient_tolerance() const {return value("rol_gradient_tolerance");}
    std::string rol_constraint_tolerance() const {return value("rol_constraint_tolerance");}
    std::string rol_step_tolerance() const {return value("rol_step_tolerance");}

    std::string dakota_workflow() const {return value("dakota_workflow");}
    std::string concurrent_evaluations() const {return value("concurrent_evaluations");}
    
    std::string mdps_response_functions() const {return value("mdps_response_functions");}
    std::string sbgo_max_iterations() const {return value("sbgo_max_iterations");}
    std::string moga_population_size() const {return value("moga_population_size");}
    std::string moga_niching_distance() const {return value("moga_niching_distance");}
    std::string num_sampling_method_samples() const {return value("num_sampling_method_samples");}
    std::string moga_max_function_evaluations() const {return value("moga_max_function_evaluations");}
    std::string sbgo_surrogate_output_name() const {return value("sbgo_surrogate_output_name");}
    std::string amgx_solver_tolerance() const {return value("amgx_solver_tolerance");}
    std::string amgx_max_iterations() const {return value("amgx_max_iterations");}
    std::string amgx_print_solver_stats() const {return value("amgx_print_solver_stats");}
    std::string amgx_solver_type() const {return value("amgx_solver_type");}

    std::string rol_gradient_check_perturbation_scale() const {return value("rol_gradient_check_perturbation_scale");}
    std::string rol_gradient_check_steps() const {return value("rol_gradient_check_steps");}
    std::string rol_gradient_check_random_seed() const {return value("rol_gradient_check_random_seed");}

    std::string esp_workflow() const {return value("esp_workflow");}
    std::string mesh_morph() const {return value("mesh_morph");}

};
// struct OptimizationParameters

}
