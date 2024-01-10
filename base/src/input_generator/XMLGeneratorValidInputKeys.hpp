/*
 * XMLGeneratorValidInputKeys.hpp
 *
 *  Created on: May 29, 2020
 */

#pragma once


#include <set>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "XMLG_Macros.hpp"
#include "XMLGeneratorParserUtilities.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \fn return_supported_value
 * \brief Return supported keyword.
 * \param [in] aKey input file keyword
 * \return supported value/keyword.
**********************************************************************************/
inline std::string return_supported_value(const std::string& aKey, const std::vector<std::string>& aKeys)
{
    auto tLowerKey = XMLGen::to_lower(aKey);
    auto tItr = std::find(aKeys.begin(), aKeys.end(), tLowerKey);
    if(tItr == aKeys.end())
    {
        return ("");
    }
    return tItr.operator*();
}
// function return_supported_value

/******************************************************************************//**
 * \fn return_supported_value
 * \brief Return supported random category keyword.
 * \param [in] aKey input file keyword
 * \return supported random category keyword. If key is not supported, return an empty string.
**********************************************************************************/
inline std::string return_supported_value(const std::string& aKey, const std::unordered_map<std::string, std::string>& aKeys)
{
    auto tLowerKey = XMLGen::to_lower(aKey);
    auto tItr = aKeys.find(tLowerKey);
    if(tItr == aKeys.end())
    {
        return ("");
    }
    return tItr->second;
}
// function return_supported_value

struct ValidKeys
{
    std::vector<std::string> mKeys;
};

struct ValidCriterionParameterKeys
{
    std::vector<std::string> mKeys = 
    {
        "type", 
        "normalize", 
        "normalization_value",
        "stress_p_norm_exponent",
        "stress_p_norm_measure",
        "stress_p_norm_volume_scaling",
        "mechanical_weighting_factor",
        "thermal_weighting_factor",
        "material_penalty_model", 
        "material_penalty_exponent", 
        "minimum_ersatz_material_value",
        "stress_limit",
        "scmm_initial_penalty",
        "scmm_penalty_expansion_multiplier",
        "scmm_penalty_upper_bound",
        "scmm_stress_weight",
        "scmm_mass_weight",
        "scmm_constraint_exponent",
        "criterion_ids",
        "criterion_weights",
        "relative_stress_limit",
        "relaxed_stress_ramp_factor",
        "conductivity_ratios",
        "location_name",
        "location_type",
        "local_measure",
        "spatial_weighting_function",
        "location_name",
        "displacement_direction",
        "measure_magnitude",
        "target",
        "target_solution_vector",
        "target_magnitude",
        "target_solution",
        "block",
        /* These are all related to stress-constrained mass minimization problems with Sierra/SD */
        "volume_misfit_target",
        "limit_power_min",
        "limit_power_max",
        "limit_power_feasible_bias",
        "limit_power_feasible_slope",
        "limit_power_infeasible_bias",
        "limit_power_infeasible_slope",
        "limit_reset_subfrequency",
        "limit_reset_count",
        "inequality_allowable_feasibility_upper",
        "inequality_feasibility_scale",
        "inequality_infeasibility_scale",
        "stress_inequality_power",
        "stress_favor_final",
        "stress_favor_updates",
        "volume_penalty_power",
        "volume_penalty_divisor",
        "volume_penalty_bias",
        "surface_area_sideset_id",
        // Sierra/SD modal objectives
        "num_modes_compute",
        "modes_to_exclude",
        "eigen_solver_shift",
        "camp_solver_tol",
        "camp_max_iter",
        "ref_data_file",
        "match_nodesets",
        // Sierra/TF keywords
        "search_nodesets",
        "temperature_field_name",
        // mass properties
        "mass",
        "cgx", 
        "cgy", 
        "cgz", 
        "ixx", 
        "iyy", 
        "izz", 
        "ixz", 
        "iyz", 
        "ixy",
        // system call
        "data_file",
        "data_group",
        "data_extraction_operation"
    };
};

struct ValidBoolKeys
{
private:
    /*!<
     * \brief Valid plato xml generator parser boolean keys.
     **/
    std::vector<std::string> mKeys = {"true", "false"};

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return supported boolean keyword.
     * \param [in] aKey input file keyword
     * \return supported criterion keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const std::string& aKey) const
    {
        return (XMLGen::return_supported_value(aKey, mKeys));
    }
};
// struct ValidBoolKeys

struct ValidPlatoInputFileMetaDataBlockKeys
{
    /*!<
     * \brief plato input file metadata block names
     **/
    std::vector<std::string> mKeys =
    {
        "service", 
        "objective", 
        "constraint", 
        "material", 
        "block", 
        "uncertainty", 
        "mesh", 
        "output"
    };
};
// struct ValidPlatoInputFileMetaDataBlockKeys

struct ValidObjectiveTypeKeys
{
    /*!<
     * \brief Valid plato input deck criterion keywords.
     **/
    std::vector<std::string> mKeys = {"single_criterion", "weighted_sum", "multi_objective"};
};
// struct ValidObjectiveTypeKeys

struct ValidCriterionTypeKeys
{
private:
    /*!<
     * \brief Supported criterion keywords.
     **/
    std::vector<std::string> mKeys =
    {
        "composite", 
        "mechanical_compliance", 
        "thermal_compliance",
        "electrical_compliance",
        "thermomechanical_compliance", 
        "electromechanical_compliance", 
        "total_work", 
        "elastic_work",
        "plastic_work",
        "thermoplasticity_thermal_energy",
        "volume", 
        "volume_average", 
        "mass",
        "effective_energy", 
        "surface_area", 
        "flux_p-norm", 
        "stress", 
        "stress_p-norm", 
        "stress_constraint",  
        "stress_constraint_general",  
        "stress_constraint_quadratic",  
        "stress_and_mass",
        "flux", 
        "frf_mismatch", 
        "limit_stress",
        "compliance_and_volume_min",
        "mean_temperature", 
        "mean_surface_pressure",
        "mean_surface_temperature",
        "flow_rate",
        "fluid_thermal_compliance",
        "maximize_fluid_thermal_flux",
        "modal_matching",
        "temperature_matching",
        "modal_projection_error",
        "mass_properties",
        "displacement",
        "volume_average_von_mises",
        "system_call"
    };

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return supported criterion keyword.
     * \param [in] aKey input file keyword
     * \return supported criterion keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const std::string& aKey) const
    {
        return (XMLGen::return_supported_value(aKey, mKeys));
    }

    /******************************************************************************//**
     * \fn size
     * \brief Return const reference to criteria list.
     * \return criteria list
    **********************************************************************************/
    const decltype(mKeys)& list() const
    { return mKeys; }
};
// struct ValidCriterionTypeKeys

struct ValidPNormMeasureKeys
{
private:
    /*!<
     * \brief Valid p-norm measures.
     **/
    std::vector<std::string> mKeys = {"vonmises"};

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return supported p-norm measure keyword.
     * \param [in] aKey input file keyword
     * \return supported criterion keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const std::string& aKey) const
    {
        return (XMLGen::return_supported_value(aKey, mKeys));
    }
};
// struct ValidPNormMeasureKeys

struct ValidPNormMeasureKeyMap
{
    /*!<
     * valid p-norm measure key map \n
     * \brief map from p-norm measure keyword to identification keyword for Plato Analyze input deck, i.e. map<measure_key, identification_key>.
     *
     **/
    std::unordered_map<std::string, std::string> mKeys = { { "vonmises", "Von Mises" } };
};
// ValidPNormMeasureKeyMap

struct ValidRandomCategoryKeys
{
private:
    /*!<
     * \brief Valid plato input deck random categories keywords.
     **/
    std::vector<std::string> mKeys = {"load", "material"};

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return supported categories for random variables/vectors.
     * \param [in] aKey input file keyword
     * \return supported categories for random variables/vectors keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const std::string& aKey) const
    {
        return (XMLGen::return_supported_value(aKey, mKeys));
    }
};
// struct ValidRandomCategoryKeys

struct ValidRandomPropertyKeys
{
private:
    /*!<
     * \brief Valid random tags for random variables/vectors.
     **/
    std::vector<std::string> mKeys = 
    { 
        "angle_variation", 
        "youngs_modulus", 
        "poissons_ratio", 
        "mass_density", 
        "youngs_modulus_x", 
        "youngs_modulus_y",
        "youngs_modulus_z", 
        "poissons_ratio_xy", 
        "poissons_ratio_xz", 
        "poissons_ratio_yz", 
        "shear_modulus_xy", 
        "shear_modulus_xz",
        "shear_modulus_yz", 
        "dielectric_permittivity_11", 
        "dielectric_permittivity_33", 
        "piezoelectric_coupling_15", 
        "piezoelectric_coupling_33",
        "piezoelectric_coupling_31", 
        "thermal_conductivity", 
        "specific_heat", 
        "reference_temperature", 
        "thermal_expansivity",
        "yield_stress" 
    };

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return supported tags for random variables/vectors.
     * \param [in] aKey input file keyword
     * \return supported tags for random variables/vectors keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const std::string& aKey) const
    {
        return (XMLGen::return_supported_value(aKey, mKeys));
    }
};
// struct ValidRandomPropertyKeys

struct ValidRandomAttributeKeys
{
private:
    /*!<
     * \brief Valid plato input deck random attributes keywords.
     **/
    std::vector<std::string> mKeys = {"x", "y", "z", "homogeneous"};

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return supported attributes for random variables/vectors.
     * \param [in] aKey input file keyword
     * \return supported attributes for random variables/vectors keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const std::string& aKey) const
    {
        return (XMLGen::return_supported_value(aKey, mKeys));
    }
};
// struct ValidRandomAttributeKeys

struct ValidRandomIdentificationKeys
{
private:
    /*!<
     * \brief Valid plato input deck identification keywords.
     * Map from random 'category' keyword to identification keyword, i.e. map<category_key, identification_key>.
     **/
    std::unordered_map<std::string, std::string> mKeys = { { "load", "load_id" }, { "material", "material_id" } };

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return supported random category keyword.
     * \param [in] aKey input file keyword
     * \return supported random category keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const std::string& aKey) const
    {
        return (XMLGen::return_supported_value(aKey, mKeys));
    }
};
// struct ValidStatisticalDistributionKeys

struct ValidStatisticalDistributionKeys
{
private:
    /*!<
     * \brief Valid plato input deck statistical distribution keywords.
     **/
    std::vector<std::string> mKeys = {"normal", "beta", "uniform"};

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return supported probability distribution function keyword.
     * \param [in] aKey input file keyword
     * \return supported probability distribution function keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const std::string& aKey) const
    {
        return (XMLGen::return_supported_value(aKey, mKeys));
    }
};
// struct ValidStatisticalDistributionKeys
//
struct ValidLoadKeys
{
    /*!<
     * \brief Valid plato input deck load keywords.
     **/
    std::vector<std::string> mKeys = {"traction", "uniform_surface_flux", "force", "pressure", "uniform_thermal_source"};
};

// struct ValidEssentialBoundaryConditionsKeys
struct ValidEssentialBoundaryConditionsKeys
{
private:
    /*!<
     * \brief Valid plato input deck essential boundary condition keywords.
     **/
    std::vector<std::string> mKeys = {"rigid", "fixed", "zero_value", "fixed_value", "insulated"};

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return supported essential boundary condition keyword.
     * \param [in] aKey input file keyword
     * \return supported essential boundary condition keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const std::string& aKey) const
    {
        return (XMLGen::return_supported_value(aKey, mKeys));
    }
};
// struct ValidEssentialBoundaryConditionsKeys

// struct ValidEssentialBoundaryConditionBlockTitleKeys
struct ValidEssentialBoundaryConditionBlockTitleKeys
{
private:
    /*!<
     * \brief Valid plato input deck essential boundary condition block keyword/title map.
     **/
    std::unordered_map<std::string, std::string> mKeys =
    {
        {"steady_state_mechanics","Essential Boundary Conditions"}, 
        {"transient_mechanics","Displacement Boundary Conditions"}, 
        {"steady_state_thermal","Essential Boundary Conditions"}, 
        {"transient_thermal","Essential Boundary Conditions"}, 
        {"steady_state_electrical","Essential Boundary Conditions"}, 
        {"steady_state_thermomechanics","Essential Boundary Conditions"},
        {"transient_thermomechanics","Essential Boundary Conditions"},
        {"steady_state_electromechanics","Essential Boundary Conditions"},
        {"plasticity","Essential Boundary Conditions"}, 
        {"thermoplasticity","Essential Boundary Conditions"}
    };
public:
    /******************************************************************************//**
     * \fn value
     * \brief Return supported criterion keyword.
     * \param [in] aKey input file keyword
     * \return supported criterion keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const std::string& aKey) const
    {
        return (XMLGen::return_supported_value(aKey, mKeys));
    }
};
// struct ValidEssentialBoundaryConditionBlockTitleKeys

struct ValidOutputToLayoutKeys
{
private:
    /*!<
     * \brief Valid plato main output keywords. \n
     *  Map from output keyword to data layout, i.e. map<output_key,data_layout>.
     **/
    std::unordered_map<std::string, std::string> mKeys =
    {
        {"vonmises","element field"}, 
        {"dispx","nodal field"}, 
        {"dispy","nodal field"}, 
        {"dispz","nodal field"},
        {"temperature","nodal field"}, 
        {"accumulated_plastic_strain","element field"}, 
        {"potential","nodal field"},
        {"objective_gradient","nodal field"}, 
        {"constraint_gradient","nodal field"}, 
        {"topology","nodal field"},
        {"control","nodal field"}, 
        {"cauchy_stress","element field"}, 
        {"deviatoric_stress","element field"},
        {"plastic_multiplier_increment","element field"}, 
        {"elastic_strain","element field"},
        {"plastic_strain","element field"}, 
        {"backstress","element field"}, 
        {"principal_stresses","element field"},
        {"stress","element field"}, 
        {"strain","element field"}
    };

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return supported output quantity of interest (QoI) data layout keyword.
     * \param [in] aKey input file keyword
     * \return supported output data layout keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const std::string& aKey) const
    {
        return (XMLGen::return_supported_value(aKey, mKeys));
    }

    /******************************************************************************//**
     * \fn key
     * \brief Return supported output quantity of interest (QoI) key.
     * \param [in] aKey input file keyword
     * \return supported output data layout key. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string key(const std::string& aKey) const
    {
        auto tLowerKey = XMLGen::to_lower(aKey);
        auto tItr = mKeys.find(tLowerKey);
        if(tItr == mKeys.end())
        {
            return ("");
        }
        return tItr->first;
    }
};
// struct ValidOutputToLayoutKeys

struct ValidPhysicsKeys
{
private:
     /******************************************************************************//**
     * \brief Map from valid simulation usecase to principal material state. Fluid 
     *  usecases can have both fluid and solid material states in a single run. 
     *  However, the principal material state for a fluids application is the fluid state.
     **********************************************************************************/
    std::unordered_map<std::string, std::string> mKeys =
    { 
        {"steady_state_mechanics", "solid"},
        {"transient_mechanics", "solid"}, 
        {"steady_state_thermal", "solid"}, 
        {"transient_thermal", "solid"}, 
        {"steady_state_electrical", "solid"}, 
        {"steady_state_thermomechanics", "solid"},
        {"transient_thermomechanics", "solid"},
        {"steady_state_electromechanics", "solid"},
        {"plasticity", "solid"},
        {"thermoplasticity", "solid"},
        {"frequency_response_function", "solid"},
        {"modal_response", "solid"},
        {"steady_state_incompressible_fluids", "fluid"},
        {"electromagnetics", "electromagnetics"}
    };

public:
    /******************************************************************************//**
     * \fn physics
     * \brief Return supported simulation physics.
     * \param [in] aKey keyword
     * \return supported simulation physics keyword, if not supported, return empty string.
    **********************************************************************************/
    std::string physics(const std::string& aKey) const
    {
        auto tItr = mKeys.find(aKey);
        if(tItr == mKeys.end())
        {
            return ("");
        }
        return tItr->first;
    }

    /******************************************************************************//**
     * \fn material_state
     * \brief Return principal material state given a supported simulation usecase.
     * \param [in] aKey supported simulation usecase
     * \return principal material state, if simulation usecase is not supported, return empty string.
    **********************************************************************************/
    std::string material_state(const std::string& aKey) const
    {
        auto tItr = mKeys.find(aKey);
        if(tItr == mKeys.end())
        {
            return ("");
        }
        return tItr->second;
    }
};
// struct ValidPhysicsKeys

struct ValidMaterialModelKeys
{
private:
    /*!<
     * \brief Valid plato input deck material model keywords \n
     **/
    std::vector<std::string> mKeys = 
    { 
        "isotropic_linear_elastic", 
        "orthotropic_linear_elastic", 
        "isotropic_linear_electroelastic", 
        "isotropic_linear_thermal",
        "isotropic_linear_thermoelastic",
        "j2_plasticity",
        "thermoplasticity",
        "forced_convection",
        "natural_convection",
        "laminar_flow",
        "electromagnetics"
    };

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return supported material model keyword.
     * \param [in] aKey input file keyword
     * \return supported material model keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const std::string& aKey) const
    {
        return (XMLGen::return_supported_value(aKey, mKeys));
    }
};
// struct ValidMaterialModelKeys

struct ValidConstraintTargetAbsoluteKeys
{
private:
    /*!<
     * \brief Valid plato input deck constraint absolute target keywords.
     **/
    std::vector<std::string> mKeys = {"volume absolute", "target absolute", "surface_area"};

public:
    /******************************************************************************//**
     * \fn keys
     * \brief Return list of valid keys.
     * \return list of valid keys
    **********************************************************************************/
    const std::vector<std::string> keys() const
    {
        return mKeys;
    }
};
// struct ValidConstraintTargetAbsoluteKeys

struct ValidConstraintTargetNormalizedKeys
{
private:
    /*!<
     * \brief Valid plato deck constraint normalized target keywords.
     **/
    std::vector<std::string> mKeys = {"volume fraction", "target normalized"};

public:
    /******************************************************************************//**
     * \fn keys
     * \brief Return list of valid keys.
     * \return list of valid keys
    **********************************************************************************/
    const std::vector<std::string> keys() const
    {
        return mKeys;
    }
};
// struct ValidConstraintTargetNormalizedKeys

struct ValidAxesKeys
{
private:
    /*!< map from dimension to axis, i.e. map<dimension, axis> */
    std::unordered_map<size_t, std::string> mKeys = { {0u, "x"}, {1u, "y"}, {2u, "z"} };

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return supported dimension axis keyword.
     * \param [in] aKey input file keyword
     * \return supported dimension axis keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const size_t aKey) const
    {
        auto tItr = mKeys.find(aKey);
        if(tItr == mKeys.end())
        {
            return ("");
        }
        return tItr->second;
    }
};
// struct ValidAxesKeys

struct ValidCodeKeys
{
private:
    /*!< valid plato input deck code keywords supported in plato */

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return code keyword supported by Plato Engine.
     * \param [in] aKey input file keyword
     * \return supported code keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const std::string& aKey) const
    {
        return (XMLGen::return_supported_value(aKey, mKeys));
    }
    std::vector<std::string> mKeys = {"plato_analyze", "sierra_tf", "sierra_sd", "lightmp", "platomain", "plato_esp", "xtk", "gemma"};
};
// struct ValidCodeKeys

struct ValidDiscretizationKeys
{
private:
    /*!< valid abstract design variables supported in plato, i.e. control variables discretization */
    std::vector<std::string> mKeys = {"density", "levelset"};

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return topology discretization keyword supported by Plato Engine.
     * \param [in] aKey input file keyword
     * \return supported topology discretization keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const std::string& aKey) const
    {
        return (XMLGen::return_supported_value(aKey, mKeys));
    }
};
// struct ValidLevelSetInitKeys

struct ValidLevelSetInitKeys
{
    /*!< valid level set initialization methods */
    std::vector<std::string> mKeys = {"primitives", "swiss_cheese"};
};
// struct ValidLevelSetInitKeys

struct ValidLayoutKeys
{
private:
    /*!<
     * valid operation field layouts \n
     * \brief map from light-input file key to Plato layout, i.e. map<light_input_file_key, plato_layout_key>
     **/
    std::unordered_map<std::string, std::string> mKeys =
        { {"element field", "Element Field"}, {"nodal field", "Nodal Field"}, {"global", "Global"} };

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return data layout keyword supported by Plato Engine.
     * \param [in] aKey input file keyword
     * \return supported data layout keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const std::string& aKey) const
    {
        return (XMLGen::return_supported_value(aKey, mKeys));
    }
};
// struct ValidLayoutKeys

struct ValidFilterKeys
{
private:
    /*!<
     * valid filters \n
     * \brief map from light-input file key to Plato main operation XML file key, i.e. map<light_input_file_key,plato_main_operation_file_key>
     **/
    std::unordered_map<std::string, std::string> mKeys = 
    { 
        {"identity", "Identity"},
        {"kernel", "Kernel"}, 
        {"kernel_then_heaviside", "KernelThenHeaviside"}, 
        {"kernel_then_tanh", "KernelThenTANH"} 
    };

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return filter keyword supported by Plato Engine.
     * \param [in] aKey input file keyword
     * \return supported filter keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const std::string& aKey) const
    {
        return (XMLGen::return_supported_value(aKey, mKeys));
    }
};
// struct ValidFilterKeys

struct ValidProjectionKeys
{
private:
    /*!<
     * valid filters \n
     * \brief map from light-input file key to Plato main operation XML file key, i.e. map<light_input_file_key,plato_main_operation_file_key>
     **/
    std::unordered_map<std::string, std::string> mKeys = 
    { 
        {"heaviside", "ProjectionHeaviside"}, 
        {"tanh", "ProjectionTANH"} 
    };

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return projection keyword supported by Plato Engine.
     * \param [in] aKey input file keyword
     * \return supported projection keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const std::string& aKey) const
    {
        return (XMLGen::return_supported_value(aKey, mKeys));
    }
};
// struct ValidProjectionKeys

struct ValidAnalyzeOutputKeys
{
private:
    /*!<
     * \brief Map from Plato Main input file output keywords to Plato Analyze plottable option output keywords. \n
     * The plottable feature is used by Plato Analyze to write output data into its global metadata map. Metadata \n
     * saved in Plato Analyze's metadata map can be accessed by Plato Main through the Plato Analyze application \n
     * layer during Multiple Program, Multiple Data (MPMD) runs. Similarly, plottable data can be saved by Plato \n
     * Analyze into an exodus file, foregoing output through Plato Engine, by invoking the Visualization operation \n
     * in Plato Analyze.
     **/
    std::unordered_map<std::string, std::string> mKeys =
    {
        {"vonmises", "Vonmises"}, 
        {"plastic_multiplier_increment", "plastic multiplier increment"}, 
        {"accumulated_plastic_strain", "accumulated plastic strain"},
        {"deviatoric_stress", "deviatoric stress"}, 
        {"elastic_strain", "elastic_strain"}, 
        {"plastic_strain", "plastic strain"}, 
        {"cauchy_stress", "cauchy stress"},
        {"backstress", "backstress"}, 
        {"stress", "stress"}, 
        {"strain", "strain"}
    };

public:
    /******************************************************************************//**
     * \fn value
     * \brief Return output quantity of interest (QoI) keyword supported by Plato Analyze.
     * \param [in] aKey input file keyword
     * \return supported output QoI keyword. If key is not supported, return an empty string.
    **********************************************************************************/
    std::string value(const std::string& aKey) const
    {
        return (XMLGen::return_supported_value(aKey, mKeys));
    }
};
// struct ValidAnalyzeOutputKeys

struct ValidPerformerOutputKeys
{
private:
    /*!<
     * \brief Map from Plato Main input file output keywords to performer codes output argument keywords. These \n
     * output argument keywords are recognized by the Plato::Application interface of the performer and are used \n
     * to extract data from the performer. Thus, Plato Main relies on these output argument names to extract data \n
     * from the performer, e.g. Plato Analyze. Plato Main relies on these output argument keywords to output the \n
     * quantities of interests associated with these keywords in the platomain.exo file.
     *
     *      i.e. map<code_keyword, pair<plato_main_output_keyword, code_output_keyword>. \n
     **/
    std::unordered_map<std::string, std::unordered_map<std::string, std::string> > mKeys =
    {
        { "plato_analyze", 
            { 
                {"vonmises", "Vonmises"}, 
                {"plastic_multiplier_increment", "plastic multiplier increment"},
                {"accumulated_plastic_strain", "accumulated plastic strain"},       
                {"deviatoric_stress", "deviatoric stress"},
                {"elastic_strain", "elastic_strain"}, 
                {"plastic_strain", "plastic strain"}, 
                {"cauchy_stress", "cauchy stress"},
                {"backstress", "backstress"}, 
                {"dispx", "displacement X"}, 
                {"dispy", "displacement Y"}, 
                {"dispz", "displacement Z"},
                {"principal_stresses", "principal stresses"}, 
                {"temperature", "temperature"} 
            }
        },
        { "sierra_sd", 
            { 
                {"vonmises", "vonmises"}, 
                {"dispx", "dispx"}, 
                {"dispy", "dispy"}, 
                {"dispz", "dispz"} 
            }
        }
    };

public:
    /******************************************************************************//**
     * \fn argument
     * \brief Return valid output argument name.
     * \param [in] aCode  code name
     * \param [in] aKey   output keyword
     * \return valid output argument name
    **********************************************************************************/
    std::string argument(const std::string& aCode, const std::string& aKey) const
    {
        if(aCode.empty())
        {
            THROWERR("Valid Performer Output Keys: input 'code' argument is empty.")
        }
        if(aKey.empty())
        {
            THROWERR("Valid Performer Output Keys: input 'key' argument is empty.")
        }
        auto tCodeItr = mKeys.find(aCode);
        if(tCodeItr == mKeys.end())
        {
            THROWERR("Valid Performer Output Keys: code '" + aCode + "' is not supported.")
        }
        auto tKeyItr = tCodeItr->second.find(aKey);
        if(tKeyItr == tCodeItr->second.end())
        {
            THROWERR("Valid Performer Output Keys: output keyword '" + aKey + "' is not supported by code '" + aCode + ".")
        }
        return (tKeyItr->second);
    }
};
struct ValidPhysicsLoadCombinations
{
    // Map physics->load->parent node name
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> mKeys =
    {
        {"steady_state_mechanics", 
            {
                {"traction", {"Natural Boundary Conditions"}},
                {"pressure", {"Natural Boundary Conditions"}}
            }
        },
        {"steady_state_thermal", 
            {
                {"uniform_surface_flux", {"Natural Boundary Conditions"}}
            }
        },
        { "steady_state_thermomechanics", 
            {
                {"uniform_surface_flux", {"Thermal Natural Boundary Conditions"}},
                {"traction", {"Mechanical Natural Boundary Conditions"}}, 
                {"pressure", {"Mechanical Natural Boundary Conditions"}} 
            }
        },
        {"transient_mechanics", 
            {
                {"traction", {"Natural Boundary Conditions"}},
                {"pressure", {"Natural Boundary Conditions"}}
            }
        },
        {"plasticity", 
            {
                {"traction", {"Natural Boundary Conditions","Mechanical Natural Boundary Conditions"}}
            }
        },
        {"thermoplasticity", 
            {
                {"uniform_surface_flux", {"Natural Boundary Conditions","Thermal Natural Boundary Conditions"}},
                {"traction", {"Natural Boundary Conditions","Mechanical Natural Boundary Conditions"}}
            }
        },
        {"steady_state_incompressible_fluids", 
            {
                {"uniform_thermal_source", {"Thermal Sources"}},
                {"traction", {"Momentum Natural Boundary Conditions"}},
                {"uniform_surface_flux", {"Thermal Natural Boundary Conditions"}}
            }
        }
    };
public:
    void get_parent_names
    (const std::string &aPhysics,
     std::set<std::vector<std::string>> &aParentNames)
    {
        auto tKeyItr = mKeys.find(aPhysics);
        if(tKeyItr == mKeys.end())
        {
            THROWERR("Valid Physics-To-Load Combinations: Couldn't find physics: " + aPhysics)
        }
        auto tLoadItr = tKeyItr->second.begin();
        while(tLoadItr != tKeyItr->second.end())
        {
            aParentNames.insert(tLoadItr->second);
            tLoadItr++;
        }
    }  

    std::vector<std::string> get_parent_load_node_names
    (const std::string &aPhysics,
     const std::string &aLoadType)
    {
        auto tKeyItr = mKeys.find(aPhysics);
        if(tKeyItr != mKeys.end())
        {
            auto tLoadItr = tKeyItr->second.find(aLoadType);
            if(tLoadItr != tKeyItr->second.end())
            {
                return tLoadItr->second;
            }
        }
        return {""};
    }
};

struct ValidMaterialPropertyKeys
{
private:
    /*!<
     * valid plato analyze material models and corresponding material property keywords \n
     * \brief map from material model to map from material property tag in plato input file to \n
     * pair of plato analyze input file material property key and its corresponding value type, i.e. \n
     *
     * map< material_model, map<plato_input_file_material_property_tag, pair<plato_analyze_input_file_material_property_tag, value_type>>>.
     *
     **/
    std::unordered_map<std::string, std::unordered_map<std::string, std::pair<std::string, std::string>>> mKeys =
    {
        { "isotropic_linear_elastic", 
            { 
                { "youngs_modulus", {"Youngs Modulus", "double"} },
                { "poissons_ratio", {"Poissons Ratio", "double"} }, 
                { "mass_density", {"Mass Density", "double"} } 
            }
        },

        { "orthotropic_linear_elastic", 
            { 
                { "youngs_modulus_x", {"Youngs Modulus X", "double"} },
                { "youngs_modulus_y", {"Youngs Modulus Y", "double"} }, 
                { "youngs_modulus_z", {"Youngs Modulus Z", "double"} },
                { "poissons_ratio_xy", {"Poissons Ratio XY", "double"} }, 
                { "poissons_ratio_xz", {"Poissons Ratio XZ", "double"} },
                { "poissons_ratio_yz", {"Poissons Ratio YZ", "double"} }, 
                { "shear_modulus_xy", {"Shear Modulus XY", "double"} },
                { "shear_modulus_xz", {"Shear Modulus XZ", "double"} }, 
                { "shear_modulus_yz", {"Shear Modulus YZ", "double"} },
                { "mass_density", {"Mass Density", "double"} } 
            }
        },

        { "isotropic_linear_electroelastic", 
            { 
                { "youngs_modulus", {"Youngs Modulus", "double"} },
                { "poissons_ratio", {"Poissons Ratio", "double"} }, 
                { "dielectric_permittivity_11", {"p11", "double"} },
                { "dielectric_permittivity_33", {"p33", "double"} }, 
                { "piezoelectric_coupling_15", {"e15", "double"} },
                { "piezoelectric_coupling_33", {"e33", "double"} }, 
                { "piezoelectric_coupling_31", {"e31", "double"} },
                { "thermal_expansivity", {"Alpha", "double"} } 
            }
        },

        { "isotropic_linear_thermal", 
            { 
                { "thermal_conductivity", { "Thermal Conductivity", "double" } },
                { "mass_density", {"Mass Density", "double"} }, 
                { "specific_heat", {"Specific Heat", "double"} } 
            }
        },

        { "isotropic_linear_thermoelastic", 
            { 
                { "thermal_conductivity", { "Thermal Conductivity", "double" } },
                { "youngs_modulus", {"Youngs Modulus", "double"} }, 
                { "poissons_ratio", {"Poissons Ratio", "double"} },
                { "thermal_expansivity", { "Thermal Expansivity", "double" } }, 
                { "reference_temperature", { "Reference Temperature", "double" } },
                { "mass_density", {"Mass Density", "double"} } 
            }
        },

        { "j2_plasticity",
            {
                { "youngs_modulus", {"Youngs Modulus", "double"} },
                { "poissons_ratio", {"Poissons Ratio", "double"} },
                { "mass_density", {"Mass Density", "double"} }, 
                { "hardening_modulus_isotropic", { "Hardening Modulus Isotropic", "double" } },
                { "hardening_modulus_kinematic", { "Hardening Modulus Kinematic", "double" } },
                { "initial_yield_stress", {"Initial Yield Stress", "double"} },
                { "elastic_properties_penalty_exponent", {"Elastic Properties Penalty Exponent", "double"} },
                { "elastic_properties_minimum_ersatz", {"Elastic Properties Minimum Ersatz", "double"} },
                { "plastic_properties_penalty_exponent", {"Plastic Properties Penalty Exponent", "double"} },
                { "plastic_properties_minimum_ersatz", {"Plastic Properties Minimum Ersatz", "double"} }
            }
        },

        { "thermoplasticity",
            {
                { "youngs_modulus", {"Youngs Modulus", "double"} },
                { "poissons_ratio", {"Poissons Ratio", "double"} },
                { "thermal_conductivity", { "Thermal Conductivity", "double" } },
                { "thermal_expansivity", { "Thermal Expansivity", "double" } }, 
                { "reference_temperature", { "Reference Temperature", "double" } },
                { "hardening_modulus_isotropic", { "Hardening Modulus Isotropic", "double" } },
                { "hardening_modulus_kinematic", { "Hardening Modulus Kinematic", "double" } },
                { "initial_yield_stress", {"Initial Yield Stress", "double"} },
                { "elastic_properties_penalty_exponent", {"Elastic Properties Penalty Exponent", "double"} },
                { "elastic_properties_minimum_ersatz", {"Elastic Properties Minimum Ersatz", "double"} },
                { "plastic_properties_penalty_exponent", {"Plastic Properties Penalty Exponent", "double"} },
                { "plastic_properties_minimum_ersatz", {"Plastic Properties Minimum Ersatz", "double"} }
            }
        },

        { "forced_convection",
            {
                { "darcy_number", { "Darcy Number", "double" } },
                { "prandtl_number", { "Prandtl Number", "double" } },
                { "reynolds_number", { "Reynolds Number", "double"} },
                { "thermal_diffusivity", { "Thermal Diffusivity", "double" } },
                { "kinematic_viscocity", { "Kinematic Viscocity", "double" } },
                { "thermal_conductivity", { "Thermal Conductivity", "double"} },
                { "impermeability_number", { "Impermeability Number", "double"} },
                { "temperature_difference", { "Temperature Difference", "double" } },
                { "characteristic_length", { "Characteristic Length", "double" } },            }
        },

        { "natural_convection",
        
            {
                { "impermeability_number", {"Impermeability Number", "double"} },
                { "darcy_number", { "Darcy Number", "double" } }, 
                { "prandtl_number", { "Prandtl Number", "double" } }, 
                { "thermal_diffusivity", { "Thermal Diffusivity", "double" } },
                { "kinematic_viscocity", { "Kinematic Viscocity", "double" } },
                { "temperature_difference", { "Temperature Difference", "double" } },
                { "characteristic_length", { "Characteristic Length", "double" } },
                { "grashof_number", { "Grashof Number", "Array(double)" } },
                { "richardson_number", {"Richardson Number", "Array(double)"} },
                { "rayleigh_number", {"Rayleigh Number", "Array(double)"} },
                { "thermal_conductivity", {"Thermal Conductivity", "double"} }
            }
        },

        { "laminar_flow",
            {
                { "darcy_number", { "Darcy Number", "double" } }, 
                { "reynolds_number", {"Reynolds Number", "double"} },
                { "impermeability_number", {"Impermeability Number", "double"} }
            }
        },

        { "electromagnetics",
            {
                { "conductivity", { "Conductivity", "double" } }
            }
        }
    };

public:
    /******************************************************************************//**
     * \fn tag
     * \brief Return valid Plato Analyze material property tag.
     * \param [in] aMaterialModelTag material model tag
     * \param [in] aPropertyTag      material property tag
     * \return material property tag recognized by Plato Analyze
    **********************************************************************************/
    std::string tag(const std::string& aMaterialModelTag, const std::string& aPropertyTag) const
    {
        auto tLowerMaterialModelTag = XMLGen::to_lower(aMaterialModelTag);
        auto tMaterialModelItr = mKeys.find(tLowerMaterialModelTag);
        if (tMaterialModelItr == mKeys.end())
        {
            THROWERR("Valid Analyze Material Property Keys: Material model '" + tLowerMaterialModelTag + "' is not supported in Plato Analyze.")
        }

        auto tLowerPropertyTag = XMLGen::to_lower(aPropertyTag);
        auto tItr = tMaterialModelItr->second.find(tLowerPropertyTag);
        if (tItr == tMaterialModelItr->second.end())
        {
            THROWERR(std::string("Append Material Properties To Plato Analyze Material Model: Material property with tag '")
                + tLowerPropertyTag + "' is not supported in Plato Analyze by '" + tLowerMaterialModelTag + "' material model.")
        }

        return tItr->second.first;
    }

    /******************************************************************************//**
     * \fn type
     * \brief Return material property data type, e.g. double, int, etc.
     * \param [in] aMaterialModelTag material model tag
     * \param [in] aPropertyTag      material property tag
     * \return material property data type
    **********************************************************************************/
    std::string type(const std::string& aMaterialModelTag, const std::string& aPropertyTag) const
    {
        auto tLowerMaterialModelTag = XMLGen::to_lower(aMaterialModelTag);
        auto tMaterialModelItr = mKeys.find(tLowerMaterialModelTag);
        if (tMaterialModelItr == mKeys.end())
        {
            THROWERR("Valid Analyze Material Property Keys: Material model '" + tLowerMaterialModelTag + "' is not supported in Plato Analyze.")
        }

        auto tLowerPropertyTag = XMLGen::to_lower(aPropertyTag);
        auto tItr = tMaterialModelItr->second.find(tLowerPropertyTag);
        if (tItr == tMaterialModelItr->second.end())
        {
            THROWERR(std::string("Append Material Properties To Plato Analyze Material Model: Material property with tag '")
                + tLowerPropertyTag + "' is not supported in Plato Analyze by '" + tLowerMaterialModelTag + "' material model.")
        }

        return tItr->second.second;
    }

    /******************************************************************************//**
     * \fn pair
     * \brief Return material property tag to material property data type pair, e.g. \n
     *     std::pair<material property tag, material property data>
     * \param [in] aMaterialModelTag material model tag
     * \param [in] aPropertyTag      material property tag
     * \return material property tag to material property data type pair
    **********************************************************************************/
    std::pair<std::string, std::string> pair(const std::string& aMaterialModelTag, const std::string& aPropertyTag) const
    {
        auto tLowerMaterialModelTag = XMLGen::to_lower(aMaterialModelTag);
        auto tMaterialModelItr = mKeys.find(tLowerMaterialModelTag);
        if (tMaterialModelItr == mKeys.end())
        {
            THROWERR("Valid Analyze Material Property Keys: Material model '" + tLowerMaterialModelTag + "' is not supported in Plato Analyze.")
        }

        auto tLowerPropertyTag = XMLGen::to_lower(aPropertyTag);
        auto tItr = tMaterialModelItr->second.find(tLowerPropertyTag);
        if (tItr == tMaterialModelItr->second.end())
        {
            THROWERR(std::string("Append Material Properties To Plato Analyze Material Model: Material property with tag '")
                + tLowerPropertyTag + "' is not supported in Plato Analyze by '" + tLowerMaterialModelTag + "' material model.")
        }

        return tItr->second;
    }

    /******************************************************************************//**
     * \fn properties
     * \brief Return supported material property tags for this material model.
     * \param [in] aMaterialModelTag material model tag
     * \return supported material properties
    **********************************************************************************/
    std::vector<std::string> properties(const std::string& aMaterialModelTag) const
    {
        auto tLowerMaterialModelTag = XMLGen::to_lower(aMaterialModelTag);
        auto tMaterialModelItr = mKeys.find(tLowerMaterialModelTag);
        if (tMaterialModelItr == mKeys.end())
        {
            THROWERR("Valid Analyze Material Property Keys: Material model '" + tLowerMaterialModelTag + "' is not supported in Plato Analyze.")
        }

        std::vector<std::string> tOutput;
        for(auto& tPair : tMaterialModelItr->second)
        {
            tOutput.push_back(tPair.first);
        }
        return tOutput;
    }
};
// struct ValidMaterialPropertyKeys

struct ValidAnalyzePhysicsKeys
{
private:
    /*!<
     * valid plato analyze physics and corresponding PDE keywords \n
     * \brief map from physics keyword to pair of plato analyze physics and \n
     * partial differential equations (PDE) keywords, i.e. \n
     *
     * map< plato_main_physics_key, pair<plato_analyze_physics_key, plato_analyze_pde_key> >.
     *
     **/
    std::unordered_map<std::string, std::pair<std::string,std::string>> mKeys =
        {
            { "steady_state_mechanics", {"Mechanical", "Elliptic"} },
            { "transient_mechanics", {"Mechanical", "Hyperbolic"} },
            { "steady_state_incompressible_fluids", {"Incompressible Fluids", "Hyperbolic"} },
            { "steady_state_thermal", {"Thermal", "Elliptic"} },
            { "transient_thermal", {"Thermal", "Parabolic"} },
            { "steady_state_electrical", {"Electrical", "Elliptic"} },
            { "steady_state_thermomechanics", {"Thermomechanical", "Elliptic"} },
            { "transient_thermomechanics", {"Thermomechanical", "Parabolic"} },
            { "steady_state_electromechanics", {"Electromechanical", "Elliptic"} },
            { "plasticity", {"Plasticity", "Elliptic"} },
            { "thermoplasticity", {"Thermoplasticity", "Elliptic"} },
            { "stabilized_mechanics", {"Stabilized Mechanics", "Elliptic"} }
        };

public:
    std::string pde(const std::string& aPhysicsTag) const
    {
        auto tLowerPhysics = XMLGen::to_lower(aPhysicsTag);
        auto tPhysicsItr = mKeys.find(tLowerPhysics);
        if (tPhysicsItr == mKeys.end())
        {
            THROWERR(std::string("Valid Analyze Physics Keys: Physics '") + tLowerPhysics + "' is not supported in Plato Analyze.")
        }
        return (tPhysicsItr->second.second);
    }

    std::string physics(const std::string& aPhysicsTag) const
    {
        auto tLowerPhysics = XMLGen::to_lower(aPhysicsTag);
        auto tPhysicsItr = mKeys.find(tLowerPhysics);
        if (tPhysicsItr == mKeys.end())
        {
            THROWERR(std::string("Valid Analyze Physics Keys: Physics '") + tLowerPhysics + "' is not supported in Plato Analyze.")
        }
        return (tPhysicsItr->second.first);
    }
};
// struct ValidAnalyzePhysicsKeys

struct ValidAnalyzeCriteriaKeys
{
    /*!<
     * valid plato analyze optimization criteria \n
     * \brief map from plato main objective type key to pair of plato analyze criterion \n
     * key and self-adjoint flag, i.e. \n
     *
     * map< plato_main_objective_type_key, pair<plato_analyze_criterion_key, plato_analyze_self_adjoint_key> >.
     *
     **/
    std::unordered_map<std::string, std::pair<std::string, bool>> mKeys =
    {
        { "composite", { "Composite", false } },
        { "volume", { "Volume", false } },
        { "volume_average", { "Volume Average Criterion", false } },
        { "elastic_work", { "Elastic Work", false } },
        { "plastic_work", { "Plastic Work", false } },
        { "total_work", { "Total Work", false } },
        { "thermoplasticity_thermal_energy", { "Thermal Energy", false } },
        { "mechanical_compliance", { "Internal Elastic Energy", true } },
        { "local_stress", { "Stress Constraint Quadratic", false } },
        { "stress_p-norm", { "Stress P-Norm", false } },
        { "effective_energy", { "Effective Energy", true } },
        { "stress_constraint", { "Stress Constraint", false } },
        { "stress_constraint_general", { "Stress Constraint General", false } },
        { "stress_constraint_quadratic", { "Stress Constraint Quadratic", false } },
        { "stress_and_mass", { "Stress Constraint General", false } },
        { "thermal_compliance", { "Internal Thermal Energy", false } },
        { "flux_p-norm", { "Flux P-Norm", false } },
        { "thermomechanical_compliance", { "Internal Thermoelastic Energy", false } },
        { "mean_surface_temperature", { "Mean Surface Temperature", false } },
        { "mean_surface_pressure", { "Mean Surface Pressure", false } },
        { "mean_temperature", { "Mean Temperature", false } },
        { "flow_rate", { "Flow Rate", false } },
        { "maximize_fluid_thermal_flux", { "Thermal Flux", false } },
        { "fluid_thermal_compliance", { "Thermal Compliance", false } },
        { "mass_properties", { "Mass Properties", false } },
        { "displacement", { "Solution", false } }
    };
};
// ValidAnalyzeCriteriaKeys

struct ValidAnalyzeCriteriaIsLinearKeys
{
    /*!<
     * valid plato analyze optimization criteria \n
     * \brief map from plato main objective type key to bool saying whether the criteria is linear \n
     *
     **/
    std::unordered_map<std::string, std::string> mKeys =
    {
        { "volume", "true" },
        { "mass_properties", "true" }
    };
};
// ValidAnalyzeCriteriaIsLinearKeys

struct ValidSpatialDimsKeys
{
    /*!<
     * \brief Valid plato problem spatial dimensions.
     **/
    std::vector<std::string> mKeys = { "3", "2" };
};
// struct ValidSpatialDimsKeys

/******************************************************************************/ /**
* \struct ValidEssentialBoundaryConditionBlockNames
* \brief Maps Plato Analyze physics to the corresponding Essential Boundary Condition
*        block name based on the degree of freedom key.
**********************************************************************************/
struct ValidEssentialBoundaryConditionBlockNames
{
private:
    /*!< map from physics to the essential boundary condition block name used in the Plato Analyze input deck */
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> mMap =
        {
            {"steady_state_incompressible_fluids", {{"velx", "Velocity Essential Boundary Conditions"}, {"vely", "Velocity Essential Boundary Conditions"}, {"velz", "Velocity Essential Boundary Conditions"}, {"press", "Pressure Essential Boundary Conditions"}, {"temp", "Temperature Essential Boundary Conditions"}}},
            {"steady_state_mechanics", {{"dispx", "Essential Boundary Conditions"}, {"dispy", "Essential Boundary Conditions"}, {"dispz", "Essential Boundary Conditions"}}},
            {"transient_mechanics", {{"dispx", "Displacement Boundary Conditions"}, {"dispy", "Displacement Boundary Conditions"}, {"dispz", "Displacement Boundary Conditions"}}},
            {"steady_state_thermal", {{"temp", "Essential Boundary Conditions"}}},
            {"transient_thermal", {{"temp", "Essential Boundary Conditions"}}},
            {"steady_state_electrical", {{"potential", "Essential Boundary Conditions"}}},
            {"steady_state_thermomechanics", {{"dispx", "Essential Boundary Conditions"}, {"dispy", "Essential Boundary Conditions"}, {"dispz", "Essential Boundary Conditions"}, {"temp", "Essential Boundary Conditions"}}},
            {"transient_thermomechanics", {{"dispx", "Essential Boundary Conditions"}, {"dispy", "Essential Boundary Conditions"}, {"dispz", "Essential Boundary Conditions"}, {"temp", "Essential Boundary Conditions"}}},
            {"steady_state_electromechanics", {{"dispx", "Essential Boundary Conditions"}, {"dispy", "Essential Boundary Conditions"}, {"dispz", "Essential Boundary Conditions"}, {"potential", "Essential Boundary Conditions"}}},
            {"plasticity", {{"dispx", "Essential Boundary Conditions"}, {"dispy", "Essential Boundary Conditions"}, {"dispz", "Essential Boundary Conditions"}}},
            {"thermoplasticity", {{"dispx", "Essential Boundary Conditions"}, {"dispy", "Essential Boundary Conditions"}, {"dispz", "Essential Boundary Conditions"}, {"temp", "Essential Boundary Conditions"}}},
        };

public:
    /******************************************************************************/ /**
        * \fn blockName
        * \brief Return supported essential boundary condition block name.
        * \param [in] aPhysics physics name/tag
        * \param [in] aDofName degree of freedom name/tag
        * \return supported essential boundary condition block name
        **********************************************************************************/
    std::string blockName(
        const std::string &aPhysics,
        const std::string &aDofName) const
    {
        auto tItr1 = mMap.find(aPhysics);
        if (tItr1 == mMap.end())
        {
            THROWERR(std::string("Physics key with tag '") + aPhysics + "' is not supported.")
        }
        auto tItr2 = tItr1->second.find(aDofName);
        if (tItr2 == tItr1->second.end())
        {
            THROWERR(std::string("Degree of freedom (dof) key with tag '") + aDofName + "' is not a supported dof in '" + aPhysics + "' physics.")
        }
        return (tItr2->second);
    }
};
// struct ValidEssentialBoundaryConditionBlockNames

struct ValidDofsKeys
{
private:
    /*!< map from physics to map from degree of freedom name to degree of freedom index, i.e. \n
     *
     * map<physics, map<dof_name, dof_index>>
     * */
    std::unordered_map<std::string, std::unordered_map<std::string,std::string>> mKeys =
        {
            {"steady_state_mechanics", { {"dispx", "0"}, {"dispy", "1"}, {"dispz", "2"} } },
            {"transient_mechanics", { {"dispx", "0"}, {"dispy", "1"}, {"dispz", "2"} } },
            {"steady_state_thermal", { {"temp", "0"} } }, 
            {"transient_thermal", { {"temp", "0"} } }, 
            {"steady_state_electrical", { {"potential", "0"} } },
            {"steady_state_thermomechanics", { {"dispx", "0"}, {"dispy", "1"}, {"dispz", "2"}, {"temp", "3"} } },
            {"transient_thermomechanics", { {"dispx", "0"}, {"dispy", "1"}, {"dispz", "2"}, {"temp", "3"} } },
            {"steady_state_electromechanics", { {"dispx", "0"}, {"dispy", "1"}, {"dispz", "2"}, {"potential", "3"} } },
            {"steady_state_incompressible_fluids", { {"velx", "0"}, {"vely", "1"}, {"velz", "2"}, {"press", "0"}, {"temp", "0"} } },
            {"plasticity", { {"dispx", "0"}, {"dispy", "1"}, {"dispz", "2"} } },
            {"thermoplasticity", { {"dispx", "0"}, {"dispy", "1"}, {"dispz", "2"}, {"temp", "3"} } }
        };

public:
    /******************************************************************************//**
     * \fn dof
     * \brief Return degree of freedom integer.
     * \param [in] aPhysics physics name
     * \param [in] aDofName degree of freedom name
     * \return degree of freedom integer
    **********************************************************************************/
    std::string dof(const std::string& aPhysics, const std::string& aDofName) const
    {
        auto tLowerPhysics = XMLGen::to_lower(aPhysics);
        auto tDofsKeysItr = mKeys.find(tLowerPhysics);
        if(tDofsKeysItr == mKeys.end())
        {
            THROWERR(std::string("Valid Dofs Keys: ") + "Physics '" + tLowerPhysics + "' is not supported in Plato Analyze.")
        }

        auto tLowerDof = XMLGen::to_lower(aDofName);
        auto tDofNameItr = tDofsKeysItr->second.find(tLowerDof);
        if(tDofNameItr == tDofsKeysItr->second.end())
        {
            THROWERR(std::string("Valid Dofs Keys: ") + "Degree of Freedom tag/key '" + tLowerDof + "' is not supported for physics '" + tLowerPhysics + "'.")
        }

        return tDofNameItr->second;
    }

    /******************************************************************************//**
     * \fn names
     * \brief Return list of supported degrees of freedom names.
     * \param [in] aPhysics physics name
     * \return degree of freedom names
    **********************************************************************************/
    std::vector<std::string> names(const std::string& aPhysics) const
    {
        auto tLowerPhysics = XMLGen::to_lower(aPhysics);
        auto tDofsKeysItr = mKeys.find(tLowerPhysics);
        if(tDofsKeysItr == mKeys.end())
        {
            THROWERR(std::string("Valid Dofs Keys: ") + "Physics '" + tLowerPhysics + "' is not supported in Plato Analyze.")
        }

        std::vector<std::string> tOutput;
        for(auto& tPair : tDofsKeysItr->second)
        {
            tOutput.push_back(tPair.first);
        }

        return tOutput;
    }
};
// struct ValidDofsKeys

struct ValidOptimizationParameterKeys
{
    /*!<
     * valid plato optimization parameters \n
     * \brief vector of valid optimization parameters \n
     **/
    std::unordered_set<std::string> mKeys =
    {
     "verbose",
     "descriptors",
     "lower_bounds",
     "upper_bounds",
     "discretization",
     "enforce_bounds",
     "number_refines",
     "max_iterations",
     "filter_in_engine",
     "num_shape_design_variables",
     "symmetry_plane_normal",
     "symmetry_plane_origin",
     "mesh_map_filter_radius",
     "filter_before_symmetry_enforcement",
     "mesh_map_search_tolerance",
     "initial_guess_file_name",
     "initial_guess_field_name",
     "normalize_in_aggregator",
     "output_method",
     "output_frequency",
     "initial_density_value",
     "restart_iteration",
     "csm_file",
     "create_levelset_spheres",
     "levelset_material_box_min",
     "levelset_material_box_max",
     "levelset_sphere_radius",
     "levelset_sphere_packing_factor",
     "levelset_initialization_method",
     "fixed_block_ids",
     "fixed_sideset_ids",
     "fixed_nodeset_ids",
     "fixed_block_domain_values",
     "fixed_block_boundary_values",
     "fixed_block_material_states",
     "levelset_nodesets",
     "number_prune_and_refine_processors",
     "prune_and_refine_path",
     "number_buffer_layers",
     "prune_mesh",
     "prune_threshold",
     "optimization_algorithm",
     "check_gradient",
     "check_hessian",
     "filter_type",
     "filter_service",
     "projection_type",
     "filter_power",
     "objective_number_standard_deviations",
     "filter_radius_scale",
     "filter_radius_absolute",
     "symmetry_plane_location_names",
     "boundary_sticking_penalty",
     "hessian_type",
     "limited_memory_storage",
     "problem_update_frequency",
     "filter_heaviside_min",
     "filter_heaviside_update",
     "filter_heaviside_max",
     "filter_heaviside_scale",
     "filter_projection_start_iteration",
     "filter_projection_update_interval",
     "filter_use_additive_continuation",
     "write_restart_file",
     "optimization_type",
     "filter_type_identity_generator_name",
     "filter_type_kernel_generator_name",
     "filter_type_kernel_then_heaviside_generator_name",
     "filter_type_kernel_then_tanh_generator_name",
     "amgx_solver_tolerance",
     "amgx_max_iterations",
     "amgx_solver_type",
     "amgx_print_solver_stats",
     "reset_algorithm_on_update",
     "rol_subproblem_model",
     "rol_lin_more_cauchy_initial_step_size",
     "rol_initial_trust_region_radius",
     "rol_gradient_tolerance",
     "rol_constraint_tolerance",
     "rol_step_tolerance",
     "dakota_workflow",
     "concurrent_evaluations",
     "mdps_partitions",
     "mdps_response_functions",
     "sbgo_max_iterations",
     "moga_population_size",
     "moga_niching_distance",
     "moga_max_function_evaluations",
     "sbgo_surrogate_output_name",
     "num_sampling_method_samples",
     "rol_gradient_check_perturbation_scale",
     "rol_gradient_check_steps",
     "rol_gradient_check_random_seed",
     "esp_workflow",
     "mesh_morph"
    };
};

/******************************************************************************/ /**
* \struct ValidHeatTransferMechanisms
* \brief Set of valid Plato Analyze's heat transfer mechanisms.
**********************************************************************************/
struct ValidHeatTransferMechanisms
{
    std::unordered_set<std::string> mKeys = { "none", "natural", "forced", "mixed" };
};

struct ValidRunParameterKeys
{
    std::vector<std::string> mKeys = 
    {
        "type", 
        "command",
        "service",
        "criterion"
    };
};

struct ValidRunTypes
{
    std::vector<std::string> mKeys = 
    {
        "modal_analysis" 
    };
};

}
// namespace XMLGen
