/*
 * XMLGeneratorParseCriteria.cpp
 *
 *  Created on: Jun 23, 2020
 */

#include <algorithm>
#include <sstream>

#include "XMLG_Macros.hpp"
#include "XMLGeneratorCriterionMetadata.hpp"
#include "XMLGeneratorParseCriteria.hpp"
#include "XMLGeneratorValidInputKeys.hpp"

namespace XMLGen
{

void ParseCriteria::setTags(XMLGen::Criterion& aCriterion)
{
    XMLGen::ValidCriterionParameterKeys tValidKeys;
    for(auto& tTag : mTags)
    {
        auto tLowerKey = XMLGen::to_lower(tTag.first);
        auto tItr = std::find(tValidKeys.mKeys.begin(), tValidKeys.mKeys.end(), tLowerKey);
        if(tItr == tValidKeys.mKeys.end())
        {
            THROWERR(std::string("Check Keyword: keyword '") + tTag.first + std::string("' is not supported"))
        }

        if(tTag.second.first.second.empty())
        {
            auto tDefaultValue = tTag.second.second;
            aCriterion.append(tTag.first, tDefaultValue);
        }
        else
        {
            auto tInputValue = tTag.second.first.second;
            aCriterion.append(tTag.first, tInputValue);
        }
    }
}

std::string check_criterion_type_key
(const std::string& aKeyword)
{
    XMLGen::ValidCriterionTypeKeys tValidKeys;
    auto tValue = tValidKeys.value(aKeyword);
    if(tValue.empty())
    {
        auto tMsg = std::string("Input criterion keyword '") + aKeyword + "' is not defined in the list of supported Plato criterion. Supported Plato criterion options are: ";
        const auto& tKeys = tValidKeys.list();
        for(const auto& tKey : tKeys)
        {
            size_t tIndex = &tKey - &tKeys[0];
            auto tDelimiter = (tIndex + 1u) == tKeys.size() ? ".\n" : ", ";
            tMsg = tMsg + tKey + tDelimiter;
        }
        THROWERR(tMsg)
    }
    return tValue;
}

void ParseCriteria::insertTag(std::string keyword, std::string defaultValue)
{
    mTags.insert({keyword, { { {keyword}, ""}, defaultValue } });
}

void ParseCriteria::allocate()
{
    mTags.clear();

    // core properties
    this->insertTag("type");
    this->insertTag("normalize", "false");
    this->insertTag("normalization_value", "1.0");
    this->insertTag("stress_limit");
    this->insertTag("scmm_initial_penalty");
    this->insertTag("scmm_penalty_expansion_multiplier");
    this->insertTag("scmm_constraint_exponent");
    this->insertTag("scmm_penalty_upper_bound");
    this->insertTag("scmm_stress_weight", "1.0");
    this->insertTag("scmm_mass_weight", "1.0");

    this->insertTag("stress_p_norm_exponent", "6.0");
    this->insertTag("stress_p_norm_measure");
    this->insertTag("stress_p_norm_volume_scaling");
    this->insertTag("mechanical_weighting_factor", "1.0");
    this->insertTag("thermal_weighting_factor", "1.0");
    this->insertTag("local_measure", "vonmises");
    this->insertTag("spatial_weighting_function", "1.0"); // function of x, y, and z
    this->insertTag("material_penalty_model", "simp");
    this->insertTag("material_penalty_exponent", "3.0");
    this->insertTag("minimum_ersatz_material_value");
    this->insertTag("criterion_ids");
    this->insertTag("criterion_weights");
    this->insertTag("location_name");
    this->insertTag("location_type");
    this->insertTag("conductivity_ratios");
    this->insertTag("displacement_direction");
    this->insertTag("measure_magnitude", "false");
    this->insertTag("target", "0.0");
    this->insertTag("target_solution_vector");
    this->insertTag("target_magnitude");
    this->insertTag("target_solution");

    this->insertTag("block");

    this->insertTag("mass");
    this->insertTag("cgx");
    this->insertTag("cgy");
    this->insertTag("cgz");
    this->insertTag("ixx");
    this->insertTag("iyy");
    this->insertTag("izz");
    this->insertTag("iyz");
    this->insertTag("ixz");
    this->insertTag("ixy");

    /* These are all related to stress-constrained mass minimization problems with Sierra/SD */
    this->insertTag("volume_misfit_target");
    this->insertTag("relative_stress_limit");
    this->insertTag("relaxed_stress_ramp_factor");
    this->insertTag("limit_power_min");
    this->insertTag("limit_power_max");
    this->insertTag("limit_power_feasible_bias");
    this->insertTag("limit_power_feasible_slope");
    this->insertTag("limit_power_infeasible_bias");
    this->insertTag("limit_power_infeasible_slope");
    this->insertTag("limit_reset_subfrequency");
    this->insertTag("limit_reset_count");
    this->insertTag("inequality_allowable_feasibility_upper");
    this->insertTag("inequality_feasibility_scale");
    this->insertTag("inequality_infeasibility_scale");
    this->insertTag("stress_inequality_power");
    this->insertTag("stress_favor_final");
    this->insertTag("stress_favor_updates");
    this->insertTag("volume_penalty_power");
    this->insertTag("volume_penalty_divisor");
    this->insertTag("volume_penalty_bias");
    this->insertTag("surface_area_sideset_id");

    // Sierra/SD modal objectives
    this->insertTag("num_modes_compute", "30");
    this->insertTag("modes_to_exclude");
    this->insertTag("eigen_solver_shift", "-1e6");
    this->insertTag("camp_solver_tol", "1e-6");
    this->insertTag("camp_max_iter", "1000");
    this->insertTag("ref_data_file");
    this->insertTag("match_nodesets");

    // Sierra/TF objectives
    this->insertTag("search_nodesets");
    this->insertTag("temperature_field_name");

    // system call criterion
    this->insertTag("data_file");
    this->insertTag("data_group");
    this->insertTag("data_extraction_operation");
}

void ParseCriteria::setModesToExclude(XMLGen::Criterion &aMetadata)
{
    auto tItr = mTags.find("modes_to_exclude");
    std::string tValues = tItr->second.first.second;
    if (tItr != mTags.end() && !tValues.empty()) {
        std::vector<std::string> tModes;
        char tValuesBuffer[10000];
        strcpy(tValuesBuffer, tValues.c_str());
        XMLGen::parse_tokens(tValuesBuffer, tModes);
        aMetadata.modesToExclude(tModes);
    }
}

void ParseCriteria::setSearchNodesetIDs(XMLGen::Criterion &aMetadata)
{
    auto tItr = mTags.find("search_nodesets");
    std::string tValues = tItr->second.first.second;
    if (tItr != mTags.end() && !tValues.empty())
    {
        std::vector<std::string> tNodesetIDs;
        char tValuesBuffer[10000];
        strcpy(tValuesBuffer, tValues.c_str());
        XMLGen::parse_tokens(tValuesBuffer, tNodesetIDs);
        aMetadata.setSearchNodesetIDs(tNodesetIDs);
    }
}

void ParseCriteria::setMatchNodesetIDs(XMLGen::Criterion &aMetadata)
{
    auto tItr = mTags.find("match_nodesets");
    std::string tValues = tItr->second.first.second;
    if (tItr != mTags.end() && !tValues.empty())
    {
        std::vector<std::string> tNodesetIDs;
        char tValuesBuffer[10000];
        strcpy(tValuesBuffer, tValues.c_str());
        XMLGen::parse_tokens(tValuesBuffer, tNodesetIDs);
        aMetadata.setMatchNodesetIDs(tNodesetIDs);
    }
}


void ParseCriteria::setCriterionWeights(XMLGen::Criterion &aMetadata)
{
    if(aMetadata.type() == "composite")
    {
        auto tItr = mTags.find("criterion_weights");
        std::string tValues = tItr->second.first.second;
        if (tItr != mTags.end() && !tValues.empty())
        {
            std::vector<std::string> tWeights;
            char tValuesBuffer[10000];
            strcpy(tValuesBuffer, tValues.c_str());
            XMLGen::parse_tokens(tValuesBuffer, tWeights);
            aMetadata.criterionWeights(tWeights);
        }
        else
        {
            THROWERR("Criterion Weights ('criterion_weights' keyword) are not defined for composite criterion with criterion block id '" + aMetadata.id() + "'.");
        }
    }
}

void ParseCriteria::setCriterionIDs(XMLGen::Criterion &aMetadata)
{
    if(aMetadata.type() == "composite")
    {
        auto tItr = mTags.find("criterion_ids");
        std::string tValues = tItr->second.first.second;
        if (tItr != mTags.end() && !tValues.empty())
        {
            std::vector<std::string> tIDs;
            char tValuesBuffer[10000];
            strcpy(tValuesBuffer, tValues.c_str());
            XMLGen::parse_tokens(tValuesBuffer, tIDs);
            aMetadata.criterionIDs(tIDs);
        }
        else
        {
            THROWERR("Criterion IDs ('criterion_ids' keyword) are not defined for composite criterion with criterion block id '" + aMetadata.id() + "'.");
        }
    }
}

void ParseCriteria::setVolumeBasedCriterionBlock(XMLGen::Criterion &aMetadata)
{
    auto tItr = mTags.find("block");
    std::string tValues = tItr->second.first.second;
    if (tItr != mTags.end() && !tValues.empty())
    {
        if(aMetadata.type() != "volume_average_von_mises")
            THROWERR("ParseCriteria: Criterion computation in block is only supported for volume_average_von_mises type");

        std::vector<std::string> tBlocks;
        char tValuesBuffer[10000];
        strcpy(tValuesBuffer, tValues.c_str());
        XMLGen::parse_tokens(tValuesBuffer, tBlocks);

        if(tBlocks.size() > 1)
            THROWERR("ParseCriteria: Only one block may be specified for criterion computation in block");

        aMetadata.block(tBlocks[0]);
    }
}

void ParseCriteria::setTargetSolutionVector(XMLGen::Criterion &aMetadata)
{
    if(aMetadata.type() == "displacement")
    {
        auto tItr = mTags.find("target_solution_vector");
        std::string tValues = tItr->second.first.second;
        if (tItr != mTags.end() && !tValues.empty())
        {
            std::vector<std::string> tDirection;
            char tValuesBuffer[10000];
            strcpy(tValuesBuffer, tValues.c_str());
            XMLGen::parse_tokens(tValuesBuffer, tDirection);
            aMetadata.targetSolutionVector(tDirection);
        }
    }
}

void ParseCriteria::setDisplacementDirection(XMLGen::Criterion &aMetadata)
{
    if(aMetadata.type() == "displacement")
    {
        auto tItr = mTags.find("displacement_direction");
        std::string tValues = tItr->second.first.second;
        if (tItr != mTags.end() && !tValues.empty())
        {
            std::vector<std::string> tDirection;
            char tValuesBuffer[10000];
            strcpy(tValuesBuffer, tValues.c_str());
            XMLGen::parse_tokens(tValuesBuffer, tDirection);
            aMetadata.displacementDirection(tDirection);
        }
        else
        {
            THROWERR("Displacement direction ('displacement_direction' keyword) is not defined for displacement criterion with criterion block id '" + aMetadata.id() + "'.");
        }
    }
}

void ParseCriteria::errorCheckDisplacementCriterion(XMLGen::Criterion &aMetadata)
{
    if(aMetadata.type() == "displacement")
    {
        if(aMetadata.location_type().empty())
        {
            THROWERR("Displacement criterion must have 'location_type' option set.");
        }
        if(aMetadata.location_name().empty())
        {
            THROWERR("Displacement criterion must have 'location_name' option set.");
        }
        if(aMetadata.displacementDirection().size() == 0)
        {
            THROWERR("Displacement criterion must have 'displacement_direction' option set.");
        }
    }
}

void ParseCriteria::setCriterionType(XMLGen::Criterion& aMetadata)
{
    auto tItr = mTags.find("type");
    if (tItr->second.first.second.empty())
    {
        THROWERR("Parse Criteria: Criterion type is not defined.")
    }
    else
    {
        auto tValidCriterionType = XMLGen::check_criterion_type_key(tItr->second.first.second);
        aMetadata.type(tValidCriterionType);
    }
}

void ParseCriteria::setMetadata(XMLGen::Criterion& aMetadata)
{
    this->setCriterionType(aMetadata);
    this->setCriterionIDs(aMetadata);
    this->setVolumeBasedCriterionBlock(aMetadata);
    this->setCriterionWeights(aMetadata);
    this->setMassProperties(aMetadata);
    this->checkVolumePenaltyExponent(aMetadata);
    this->setStressPNormOptions(aMetadata);
    this->setDisplacementDirection(aMetadata);
    this->setTargetSolutionVector(aMetadata);
    setModesToExclude(aMetadata);
    setMatchNodesetIDs(aMetadata);
    setSearchNodesetIDs(aMetadata);
    this->setTags(aMetadata);
    this->errorCheckDisplacementCriterion(aMetadata);
}

void ParseCriteria::checkUniqueIDs()
{
    std::vector<std::string> tIDs;
    for(auto& tCriterion : mData)
    {
        tIDs.push_back(tCriterion.id());
    }

    if(!XMLGen::unique(tIDs))
    {
        THROWERR("Parse Criteria: Criterion block identification numbers, i.e. IDs, are not unique.  Criterion block IDs must be unique.")
    }
}

void ParseCriteria::setMassProperties(XMLGen::Criterion& aCriterion)
{
    if(aCriterion.type() == "mass_properties")
    {
        const std::vector<std::string> massProperties = {"Mass", "CGx", "CGy", "CGz", "Ixx", "Iyy", "Izz", "Ixz", "Iyz", "Ixy"};

        for(auto &property : massProperties) {
            auto tItr = mTags.find(XMLGen::to_lower(property));

            if (tItr != mTags.end()) {
                std::string tValues = tItr->second.first.second;
                if (tValues.empty()) continue;

                std::vector<std::string> tTokens;
                char tValuesBuffer[10000];
                strcpy(tValuesBuffer, tValues.c_str());
                XMLGen::parse_tokens(tValuesBuffer, tTokens);

                if (tTokens[1] != "weight" || tTokens.size() != 3) {
                    std::stringstream errorstream;
                    errorstream << "Parse Criteria: expected syntax for mass properties is '"
                        << property << " [propertyValue] weight [weightValue]'" << std::endl; 
                    THROWERR(errorstream.str());
                }

                std::istringstream goldStream(tTokens[0]);
                double goldValue;
                goldStream >> goldValue;

                std::istringstream weightStream(tTokens[2]);
                double weight;
                weightStream >> weight;

                aCriterion.setMassProperty(property, goldValue, weight);
            }
        }
    }
}

void ParseCriteria::checkVolumePenaltyExponent(XMLGen::Criterion& aCriterion)
{
    if(aCriterion.type() == "volume")
    {
        auto tItr = mTags.find("material_penalty_exponent");

        // If the user did not specify an exponent for the volume
        // criterion set it to 1.0 (default was 3.0 which won't 
        // result in the correct volume calculation).
        if (tItr->second.first.second.empty())
        {
            aCriterion.materialPenaltyExponent("1.0");
        }
    }
}

void ParseCriteria::setStressPNormOptions(XMLGen::Criterion &aMetadata)
{
    if(aMetadata.type() == "stress_p-norm")
    {
        auto tMeasureItr = mTags.find("stress_p_norm_measure");
        std::string tMeasure = tMeasureItr->second.first.second;
        bool pNormMeasureSpecified = (tMeasureItr != mTags.end() && !tMeasure.empty());

        auto tScalingItr = mTags.find("stress_p_norm_volume_scaling");
        std::string tVolumeScaling = tScalingItr->second.first.second;
        bool pNormVolumeScalingSpecified = (tScalingItr != mTags.end() && !tVolumeScaling.empty());

        if (!pNormMeasureSpecified && pNormVolumeScalingSpecified)
            THROWERR(std::string("Stress p-norm volume scaling requires stress p-norm measure to be specified. Use keyword stress_p_norm_measure to do so."))

        if (pNormMeasureSpecified)
        {
            XMLGen::ValidPNormMeasureKeys tValidPNormMeasureKeys;
            auto tValidMeasure = tValidPNormMeasureKeys.value(tMeasure);
            if (!tValidMeasure.empty())
                aMetadata.pnormMeasure(tValidMeasure);
            else
                THROWERR(std::string("Stress p-norm measure: '") + tMeasure + std::string("' is not supported. Currently supported are: 'vonmises'"))
        }

        if (pNormVolumeScalingSpecified)
        {
            XMLGen::ValidBoolKeys tValidBoolKeys;
            auto tValidBool = tValidBoolKeys.value(tVolumeScaling);
            if (!tValidBool.empty())
                aMetadata.pnormVolumeScaling(tValidBool);
            else
                THROWERR(std::string("Stress p-norm volume scaling: '") + tVolumeScaling + std::string("' is not a bool. Must specify 'true' or 'false'."))
        }
    }
}

std::vector<XMLGen::Criterion> ParseCriteria::data() const
{
    return mData;
}

void ParseCriteria::parse(std::istream &aInputFile)
{
    mData.clear();
    this->allocate();
    constexpr int MAX_CHARS_PER_LINE = 10000;
    std::vector<char> tBuffer(MAX_CHARS_PER_LINE);
    while (!aInputFile.eof())
    {
        // read an entire line into memory
        std::vector<std::string> tTokens;
        aInputFile.getline(tBuffer.data(), MAX_CHARS_PER_LINE);
        XMLGen::parse_tokens(tBuffer.data(), tTokens);
        XMLGen::to_lower(tTokens);

        std::string tCriterionBlockID;
        if (XMLGen::parse_single_value(tTokens, { "begin", "criterion" }, tCriterionBlockID))
        {
            XMLGen::Criterion tMetadata;
            if(tCriterionBlockID.empty())
            {
                THROWERR(std::string("Parse Criteria: criterion identification number is empty. ")
                    + "A unique criterion identification number must be assigned to each criterion block.")
            }
            tMetadata.id(tCriterionBlockID);
            XMLGen::erase_tag_values(mTags);
            XMLGen::parse_input_metadata( { "end", "criterion" }, aInputFile, mTags);
            this->setMetadata(tMetadata);
            mData.push_back(tMetadata);
        }
    }
    this->checkUniqueIDs();
}

}
// namespace XMLGen
