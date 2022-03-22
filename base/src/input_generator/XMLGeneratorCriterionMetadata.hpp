/*
 * XMLGeneratorCriterionMetadata.hpp
 *
 *  Created on: May 14, 2020
 */

#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <map>

#include "Plato_FreeFunctions.hpp"
#include "XMLG_Macros.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \struct Criterion
 * \brief Criterion metadata for Plato problems.
**********************************************************************************/
struct Criterion
{
private:
    std::string mID; /*!< criterion identification number */
    std::string mType;  /*!< criterion type */
    std::unordered_map<std::string, std::vector<std::string> > mMetaData; /*!< Scenario metadata, map< tag, vector<values> > */
    std::vector<std::string> mCriterionWeights;
    std::vector<std::string> mCriterionIDs;
    std::vector<std::string> mDisplacementDirection;
    std::vector<std::string> mTargetSolutionVector;
    std::vector<std::string> mModesToExclude;
    std::vector<std::string> mMatchNodesetIDs;
    std::map<std::string, std::pair<double,double>> mMassProperties;

    std::string mReport;

public:
    /******************************************************************************//**
     * \fn report
     * \brief Return accumulated report, which will be outputed in the plato report text file.
     * \return report
    **********************************************************************************/
    std::string report() const
    {
        return mReport;
    }

    /******************************************************************************//**
     * \fn report
     * \brief Append input report to criteria report.
     * \param [in] aInput message to be appended to criterion report.
    **********************************************************************************/
    void report(const std::string& aInput) const
    {
        mReport + "\nPlato Input Deck Report: " + aInput;
    }

    /******************************************************************************//**
     * \fn id
     * \brief Return criterion identification number.
     * \return identification number
    **********************************************************************************/
    std::string id() const
    {
        return mID;
    }

    /******************************************************************************//**
     * \fn id
     * \brief Set criterion identification number.
     * \param [in] aID identification number
    **********************************************************************************/
    void id(const std::string& aID)
    {
        mID = aID;
    }

    /******************************************************************************//**
     * \fn type
     * \brief Return criterion type.
     * \return type
    **********************************************************************************/
    std::string type() const
    {
        return mType;
    }

    /******************************************************************************//**
     * \fn type
     * \brief Set criterion type.
     * \param [in] aType type
    **********************************************************************************/
    void type(const std::string& aType)
    {
        mType = aType;
    }

    /******************************************************************************//**
     * \fn value
     * \brief If criterion metadata is defined, return its value; else, return an empty string.
     * \param [in]  aTag     parameter tag
     * \param [in] aPosition parameter position in array
     * \return parameter value
    **********************************************************************************/
    std::string value(const std::string& aTag, size_t aPosition = 0) const
    {
        auto tTag = Plato::tolower(aTag);
        auto tItr = mMetaData.find(tTag);
        if(tItr == mMetaData.end())
        {
            return "";
        }
        return tItr->second[aPosition];
    }

    /******************************************************************************//**
     * \fn values
     * \brief Return values.
     * \param [in] aTag parameter tag
     * \return parameter values, return empty array if tag is not defined.
    **********************************************************************************/
    std::vector<std::string> values(const std::string& aTag) const
    {
        auto tTag = Plato::tolower(aTag);
        auto tItr = mMetaData.find(tTag);
        return ( tItr == mMetaData.end() ? std::vector<std::string>() : tItr->second );
    }

    /******************************************************************************//**
     * \fn append
     * \brief Append parameter to metadata.
     * \param [in] aTag   parameter tag
     * \param [in] aValue parameter value
    **********************************************************************************/
    void append(const std::string& aTag, const std::string& aValue)
    {
        if (aTag.empty())
        {
            THROWERR(std::string("XML Generator Scenario Metadata: Parameter with tag '") + aTag + "' is empty.")
        }
        auto tTag = Plato::tolower(aTag);
        mMetaData[aTag].push_back(aValue);
    }

    /******************************************************************************//**
     * \fn materialPenaltyExponent
     * \brief Set string value for keyword 'material_penalty_exponent'.
     * \param [in] aInput string value
     **********************************************************************************/
    void materialPenaltyExponent(const std::string& aInput)
    {
        this->append("material_penalty_exponent", aInput);
    }

    /******************************************************************************//**
     * \fn materialPenaltyExponent
     * \brief Return string value for keyword 'material_penalty_exponent'.
     * \return value
     **********************************************************************************/
    std::string materialPenaltyExponent() const
    {
        return (this->value("material_penalty_exponent"));
    }

    /******************************************************************************//**
     * \fn minErsatzMaterialConstant
     * \brief Set string value for keyword 'minimum_ersatz_material_value'.
     * \param [in] aInput string value
     **********************************************************************************/
    void minErsatzMaterialConstant(const std::string& aInput)
    {
        this->append("minimum_ersatz_material_value", aInput);
    }

    /******************************************************************************//**
     * \fn minErsatzMaterialConstant
     * \brief Return string value for keyword 'minimum_ersatz_material_value'.
     * \return value
     **********************************************************************************/
    std::string minErsatzMaterialConstant() const
    {
        return (this->value("minimum_ersatz_material_value"));
    }

    /******************************************************************************//**
     * \fn pnormExponent
     * \brief Set string value for keyword 'pnorm_exponent'.
     * \param [in] aInput string value
     **********************************************************************************/
    void pnormExponent(const std::string& aInput)
    {
        this->append("stress_p_norm_exponent", aInput);
    }

    /******************************************************************************//**
     * \fn pnormExponent
     * \brief Return string value for keyword 'pnorm_exponent'.
     * \return value
     **********************************************************************************/
    std::string pnormExponent() const
    {
        return (this->value("stress_p_norm_exponent"));
    }

    /******************************************************************************//**
     * \fn mechanical_weighting_factor
     * \brief Set string value for keyword 'mechanical_weighting_factor'.
     * \param [in] aInput string value
     **********************************************************************************/
    void mechanicalWeightingFactor(const std::string& aInput)
    {
        this->append("mechanical_weighting_factor", aInput);
    }

    /******************************************************************************//**
     * \fn mechanical_weighting_factor
     * \brief Return string value for keyword 'mechanical_weighting_factor'.
     * \return value
     **********************************************************************************/
    std::string mechanicalWeightingFactor() const
    {
        return (this->value("mechanical_weighting_factor"));
    }

    /******************************************************************************//**
     * \fn thermalWeightingFactor
     * \brief Set string value for keyword 'thermal_weighting_factor'.
     * \param [in] aInput string value
     **********************************************************************************/
    void thermalWeightingFactor(const std::string& aInput)
    {
        this->append("thermal_weighting_factor", aInput);
    }

    /******************************************************************************//**
     * \fn thermalWeightingFactor
     * \brief Return string value for keyword 'thermal_weighting_factor'.
     * \return value
     **********************************************************************************/
    std::string thermalWeightingFactor() const
    {
        return (this->value("thermal_weighting_factor"));
    }

    /******************************************************************************//**
     * \fn scmmPenaltyExpansionMultiplier
     * \brief Set string value for keyword 'scmm_penalty_expansion_multiplier'.
     * \param [in] aInput string value
     **********************************************************************************/
    void scmmPenaltyExpansionMultiplier(const std::string& aInput)
    {
        this->append("scmm_penalty_expansion_multiplier", aInput);
    }

    /******************************************************************************//**
     * \fn scmmPenaltyExpansionMultiplier
     * \brief Return string value for keyword 'scmm_penalty_expansion_multiplier'.
     * \return value
     **********************************************************************************/
    std::string scmmPenaltyExpansionMultiplier() const
    {
        return (this->value("scmm_penalty_expansion_multiplier"));
    }

    /******************************************************************************//**
     * \fn scmmPenaltyUpperBound
     * \brief Set string value for keyword 'scmm_penalty_upper_bound'.
     * \param [in] aInput string value
     **********************************************************************************/
    void scmmPenaltyUpperBound(const std::string& aInput)
    {
        this->append("scmm_penalty_upper_bound", aInput);
    }

    /******************************************************************************//**
     * \fn scmmPenaltyUpperBound
     * \brief Return string value for keyword 'scmm_penalty_upper_bound'.
     * \return value
     **********************************************************************************/
    std::string scmmPenaltyUpperBound() const
    {
        return (this->value("scmm_penalty_upper_bound"));
    }

    /******************************************************************************//**
     * \fn scmmInitialPenalty
     * \brief Set string value for keyword 'scmm_initial_penalty'.
     * \param [in] aInput string value
     **********************************************************************************/
    void scmmInitialPenalty(const std::string& aInput)
    {
        this->append("scmm_initial_penalty", aInput);
    }

    /******************************************************************************//**
     * \fn scmmInitialPenalty
     * \brief Return string value for keyword 'scmm_initial_penalty'.
     * \return value
     **********************************************************************************/
    std::string scmmInitialPenalty() const
    {
        return (this->value("scmm_initial_penalty"));
    }

    /******************************************************************************//**
     * \fn scmmStressWeight
     * \brief Set string value for keyword 'scmm_stress_weight'.
     * \param [in] aInput string value
     **********************************************************************************/
    void scmmStressWeight(const std::string& aInput)
    {
        this->append("scmm_stress_weight", aInput);
    }

    /******************************************************************************//**
     * \fn scmmStressWeight
     * \brief Return string value for keyword 'scmm_stress_weight'.
     * \return value
     **********************************************************************************/
    std::string scmmStressWeight() const
    {
        return (this->value("scmm_stress_weight"));
    }

    /******************************************************************************//**
     * \fn scmmMassWeight
     * \brief Set string value for keyword 'scmm_mass_weight'.
     * \param [in] aInput string value
     **********************************************************************************/
    void scmmMassWeight(const std::string& aInput)
    {
        this->append("scmm_mass_weight", aInput);
    }

    /******************************************************************************//**
     * \fn scmmMassWeight
     * \brief Return string value for keyword 'scmm_mass_weight'.
     * \return value
     **********************************************************************************/
    std::string scmmMassWeight() const
    {
        return (this->value("scmm_mass_weight"));
    }

    /******************************************************************************//**
     * \fn stressLimit
     * \brief Set string value for keyword 'stress_limit'.
     * \param [in] aInput string value
     **********************************************************************************/
    void stressLimit(const std::string& aInput)
    {
        this->append("stress_limit", aInput);
    }

    /******************************************************************************//**
     * \fn stressLimit
     * \brief Return string value for keyword 'stress_limit'.
     * \return value
     **********************************************************************************/
    std::string stressLimit() const
    {
        return (this->value("stress_limit"));
    }

    /******************************************************************************//**
     * \fn spatial_weighting_function
     * \brief Return string value for keyword 'spatial_weighting_function'.
     * \return value
     **********************************************************************************/
    std::string spatialWeightingFunction() const
    {
        return (this->value("spatial_weighting_function"));
    }

    /******************************************************************************//**
     * \fn local_measure
     * \brief Return string value for keyword 'local_measure'.
     * \return value
     **********************************************************************************/
    std::string localMeasure() const
    {
        return (this->value("local_measure"));
    }

    /******************************************************************************//**
     * \fn criterionIDs
     * \brief Set ID strings for composite criteria
     * \param [in] aInput list of IDs
     **********************************************************************************/
    void criterionIDs(const std::vector<std::string>& aInput)
    {
        this->mCriterionIDs = aInput;
    }

    /******************************************************************************//**
     * \fn criterionIDs
     * \brief Return ID strings for composite criteria
     * \return value
     **********************************************************************************/
    std::vector<std::string> criterionIDs() const
    {
        return this->mCriterionIDs;
    }

    /******************************************************************************//**
     * \fn displacementDirection
     * \brief Cooordinates of the displacement dirction
     * \param [in] aInput list of coords
     **********************************************************************************/
    void displacementDirection(const std::vector<std::string>& aInput)
    {
        this->mDisplacementDirection = aInput;
    }

    /******************************************************************************//**
     * \fn displacementDirection
     * \brief Return cooordinates of the displacement dirction
     * \return value
     **********************************************************************************/
    std::vector<std::string> displacementDirection() const
    {
        return this->mDisplacementDirection;
    }

    /******************************************************************************//**
     * \fn targetSolutionVector
     * \brief User-specified solution vector
     * \param [in] aInput list of solution components
     **********************************************************************************/
    void targetSolutionVector(const std::vector<std::string>& aInput)
    {
        this->mTargetSolutionVector = aInput;
    }

    /******************************************************************************//**
     * \fn targetSolutionVector
     * \brief Return user-specified solution vector
     * \return value
     **********************************************************************************/
    std::vector<std::string> targetSolutionVector() const
    {
        return this->mTargetSolutionVector;
    }

    /******************************************************************************//**
     * \fn criterionWeights
     * \brief Set weight strings for composite criteria
     * \param [in] aInput list of IDs
     **********************************************************************************/
    void criterionWeights(const std::vector<std::string>& aInput)
    {
        this->mCriterionWeights = aInput;
    }

    /******************************************************************************//**
     * \fn criterionWeights
     * \brief Return ID strings for composite criteria
     * \return value
     **********************************************************************************/
    std::vector<std::string> criterionWeights() const
    {
        return this->mCriterionWeights;
    }

    /******************************************************************************//**
     * \fn modesToExclude
     * \brief Set list of modes to be excluded by modal inverse
     * \param [in] aInput list of IDs
     **********************************************************************************/
    void modesToExclude(const std::vector<std::string>& aInput)
    {
        this->mModesToExclude = aInput;
    }

    /******************************************************************************//**
     * \fn modesToExclude
     * \brief Return list of modes to be excluded by modal inverse
     * \return value
     **********************************************************************************/
    std::vector<std::string> modesToExclude() const
    {
        return this->mModesToExclude;
    }

    /******************************************************************************//**
     * \fn shapeSideset
     * \brief Set string value for Sierra/SD shape sideset
     * \param [in] aInput string value
    **********************************************************************************/
    void shapeSideset(const std::string& aInput) {
        this->append("shape_sideset", aInput);
    }

    /******************************************************************************//**
     * \fn shapeSideset
     * \brief Return string value Sierra/SD shape sideset
     * \return value
    **********************************************************************************/
    std::string shapeSideset() const {
        return (this->value("shape_sideset"));
    }

    /******************************************************************************//**
     * \fn ref_data_file
     * \brief Return string value ref_data_file
     * \return value
    **********************************************************************************/
    std::string ref_data_file() const {
        return this->value("ref_data_file");
    }

    /******************************************************************************//**
     * \fn block
     * \brief Set string value for block where criterion is computed
     * \param [in] aInput string value
    **********************************************************************************/
    void block(const std::string& aInput) {
        this->append("block", aInput);
    }

    /******************************************************************************//**
     * \fn block
     * \brief Return string value for block where criterion is computed
     * \return value
    **********************************************************************************/
    std::string block() const {
        return (this->value("block"));
    }

    /******************************************************************************//**
     * \fn matchNodesetIDs
     * \brief Return nodeset ids for matching frfs or eigen
     * \return mMatchNodesetIDs
    **********************************************************************************/
    std::vector<std::string> matchNodesetIDs() const {return mMatchNodesetIDs;};

    /******************************************************************************//**
     * \fn setMatchNodesetIDs
     * \brief Set nodeset ids for matching frfs or eigen
     * \param [in] input nodeset IDs 
    **********************************************************************************/
    void setMatchNodesetIDs(std::vector<std::string>& aNodesetIDs) {mMatchNodesetIDs = aNodesetIDs;};

    void setMassProperty(std::string property, double goldValue, double weight) {
        mMassProperties[property] = std::make_pair(goldValue, weight);
    }

    const std::map<std::string, std::pair<double,double>>& getMassProperties() const
    {
        return mMassProperties;
    }

    /* These are all related to stress-constrained mass minimization problems with Sierra/SD */
    std::string volume_misfit_target() const { return this->value("volume_misfit_target"); }
    std::string scmm_constraint_exponent() const { return this->value("scmm_constraint_exponent"); }
    std::string relative_stress_limit() const { return this->value("relative_stress_limit"); }
    std::string relaxed_stress_ramp_factor() const { return this->value("relaxed_stress_ramp_factor"); }
    std::string limit_power_min() const { return this->value("limit_power_min"); }
    std::string limit_power_max() const { return this->value("limit_power_max"); }
    std::string limit_power_feasible_bias() const { return this->value("limit_power_feasible_bias"); }
    std::string limit_power_feasible_slope() const { return this->value("limit_power_feasible_slope"); }
    std::string limit_power_infeasible_bias() const { return this->value("limit_power_infeasible_bias"); }
    std::string limit_power_infeasible_slope() const { return this->value("limit_power_infeasible_slope"); }
    std::string limit_reset_subfrequency() const { return this->value("limit_reset_subfrequency"); }
    std::string limit_reset_count() const { return this->value("limit_reset_count"); }
    std::string inequality_allowable_feasibility_upper() const { return this->value("inequality_allowable_feasibility_upper"); }
    std::string inequality_feasibility_scale() const { return this->value("inequality_feasibility_scale"); }
    std::string inequality_infeasibility_scale() const { return this->value("inequality_infeasibility_scale"); }
    std::string stress_inequality_power() const { return this->value("stress_inequality_power"); }
    std::string stress_favor_final() const { return this->value("stress_favor_final"); }
    std::string stress_favor_updates() const { return this->value("stress_favor_updates"); }
    std::string volume_penalty_power() const { return this->value("volume_penalty_power"); }
    std::string volume_penalty_divisor() const { return this->value("volume_penalty_divisor"); }
    std::string volume_penalty_bias() const { return this->value("volume_penalty_bias"); }
    std::string surface_area_sideset_id() const { return this->value("surface_area_sideset_id"); }
    std::string location_name() const { return this->value("location_name"); }
    std::string location_type() const { return this->value("location_type"); }
    std::string measure_magnitude() const { return this->value("measure_magnitude"); }

    // Sierra/SD modal objectives
    std::string num_modes_compute() const { return this->value("num_modes_compute"); }
    std::vector<std::string> modes_to_exclude() const { return mModesToExclude; }
    std::string eigen_solver_shift() const { return this->value("eigen_solver_shift"); }
    std::string camp_solver_tol() const { return this->value("camp_solver_tol"); }
    std::string camp_max_iter() const { return this->value("camp_max_iter"); }

    std::string target() const { return this->value("target"); }
    std::string target_magnitude() const { return this->value("target_magnitude"); }
    std::string target_solution() const { return this->value("target_solution"); }
};
// struct Criterion

}
// namespace XMLGen
