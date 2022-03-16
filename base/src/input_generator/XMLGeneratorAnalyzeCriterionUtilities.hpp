/*
 * XMLGeneratorAnalyzeCriterionUtilities.hpp
 *
 *  Created on: Jun 15, 2020
 */

#pragma once

#include <sstream>
#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorValidInputKeys.hpp"

namespace XMLGen
{

namespace Private
{

/******************************************************************************//**
 * \fn append_block_list
 * \tparam CriterionT criterion metadata type
 * \brief Append criterion node to paren pugi xml node.
 * \param [in] aCriterion  criterion metadata
 * \param [in] aParentNode pugi::xml metadata 
 * \return parent pugi xml node with criterion input information
 **********************************************************************************/
template<typename CriterionT>
pugi::xml_node append_criterion_node(const CriterionT& aCriterion, pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn append_block_list
 * \tparam CriterionT criterion metadata type
 * \brief RAppend element block list to pugi xml node.
 * \param [in]  aCriterion  criterion metadata
 * \param [out] aParentNode pugi::xml metadata 
 **********************************************************************************/
template<typename CriterionT>
void append_block_list(const CriterionT& aCriterion, pugi::xml_node& aParentNode);

/******************************************************************************//**
 * \fn is_criterion_supported_in_plato_analyze
 * \tparam CriterionT criterion metadata type
 * \brief Return Plato Analyze's criterion keyword if criterion is supported in Plato Analyze.
 * \param [in] aCriterion criterion metadata
 **********************************************************************************/
template<typename CriterionT>
std::string is_criterion_supported_in_plato_analyze(const CriterionT& aCriterion);

/******************************************************************************//**
 * \fn is_criterion_linear
 * \tparam Criterion criterion metadata
 * \brief Return Plato Analyze's criterion linearity type 
 * \param [in] aCriterion criterion metadata
 **********************************************************************************/
template<typename Criterion>
std::string is_criterion_linear
(const Criterion& aCriterion)
{
    XMLGen::ValidAnalyzeCriteriaIsLinearKeys tValidKeys;
    auto tLowerCriterion = Plato::tolower(aCriterion.type());
    auto tItr = tValidKeys.mKeys.find(tLowerCriterion);
    if (tItr == tValidKeys.mKeys.end())
    {
        return "false";
    }
    return tItr->second;
}

/******************************************************************************//**
 * \fn append_simp_penalty_function
 * \tparam MetaData criterion metadata
 * \brief Append SIMP penalty model parameters to criterion parameter list.
 * \param [in] aCriterion criterion metadata
 * \param [in/out] aParentNode  pugi::xml_node
 **********************************************************************************/
template<typename MetaData>
void append_simp_penalty_function
(const MetaData& aMetadata,
 pugi::xml_node& aParentNode)
{
    auto tPenaltyFunction = aParentNode.append_child("ParameterList");
    XMLGen::append_attributes({"name"}, {"Penalty Function"}, tPenaltyFunction);

    std::vector<std::string> tKeys = {"name", "type", "value"};
    std::vector<std::string> tValues = {"Type", "string", "SIMP"};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tPenaltyFunction);
    tValues = {"Exponent", "double", aMetadata.materialPenaltyExponent()};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tPenaltyFunction);
    auto tPropertyValue = XMLGen::set_value_keyword_to_ignore_if_empty(aMetadata.minErsatzMaterialConstant());
    tValues = {"Minimum Value", "double", tPropertyValue};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tPenaltyFunction);
}

/******************************************************************************//**
 * \fn append_no_penalty_function
 * \tparam MetaData criterion metadata
 * \brief Append no penalty model parameters to criterion parameter list.
 * \param [in] aCriterion criterion metadata
 * \param [in/out] aParentNode  pugi::xml_node
 **********************************************************************************/
template<typename MetaData>
void append_no_penalty_function
(const MetaData& aMetadata,
 pugi::xml_node& aParentNode)
{
    auto tPenaltyFunction = aParentNode.append_child("ParameterList");
    XMLGen::append_attributes({"name"}, {"Penalty Function"}, tPenaltyFunction);

    std::vector<std::string> tKeys = {"name", "type", "value"};
    std::vector<std::string> tValues = {"Type", "string", "NoPenalty"};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tPenaltyFunction);
}

template<typename CriterionT>
std::vector<std::string> set_conductivity_ratios(const CriterionT& aCriterion)
{
    auto tConductivityRatios = aCriterion.values("conductivity_ratios");
    if( tConductivityRatios.empty() )
    {
        std::cout << "\n" << std::flush;
        THROWERR(std::string("Error creating surface scalar function of type '") + aCriterion.type() + "' with criterion id '" 
            + aCriterion.id() + "'. List of conductivity ratios (i.e. 'conductivity_ratios' keyword) is empty. "
            + "Conductivity ratios must be defined.")
    }

    if( tConductivityRatios.size() <= 1 && tConductivityRatios[0].empty() )
    {
        auto tLocationNames = aCriterion.values("location_name");
        tConductivityRatios.resize(tLocationNames.size());
        std::fill(tConductivityRatios.begin(), tConductivityRatios.end(), "1.0");
    }

    return tConductivityRatios;
}

/******************************************************************************//**
 * \fn check_criterion_location_name_list
 * \tparam CriterionT criterion metadata
 * \brief Check if 'location_name' keyword is defined, if not, thorw error to console.
 * \param [in]  aCriterion   criterion metadata
 **********************************************************************************/
template<typename CriterionT>
void check_criterion_location_name_list
(const CriterionT& aCriterion)
{
    auto tEntitySetNames = aCriterion.values("location_name");
    if (!tEntitySetNames.empty())
    {
        if (tEntitySetNames.size() <= 1 && tEntitySetNames[0].empty())
        {
            std::cout << "\n" << std::flush;
            THROWERR(std::string("Surface scalar function of type '") + aCriterion.type() + "' with criterion id '" + aCriterion.id() 
                + "' was requested but the location name (i.e. sideset name where the criterion will be evaluated) is not define in the "
                + "criterion block. User must defined the location name by setting the 'location_name' keyword.")
        }
    }
    else
    {
            std::cout << "\n" << std::flush;
            THROWERR(std::string("Surface scalar function of type '") + aCriterion.type() + "' with criterion id '" + aCriterion.id() 
                + "' was requested but the location name (i.e. sideset name where the criterion will be evaluated) is not define in the "
                + "criterion block. User must defined the location name by setting the 'location_name' keyword.")
    }
}

template<typename CriterionT>
void append_conductivity_ratios
(const CriterionT& aCriterion,
 pugi::xml_node& aParentNode)
{
    auto tLowerType = Plato::tolower(aCriterion.type());
    if( tLowerType == "maximize_fluid_thermal_flux" )
    {
        XMLGen::Private::check_criterion_location_name_list(aCriterion);
        auto tConductivityRatios = XMLGen::Private::set_conductivity_ratios(aCriterion);
        XMLGen::negate_scalar_values(tConductivityRatios);
        auto tConductivityRatiosList = XMLGen::transform_tokens_for_plato_analyze_input_deck(tConductivityRatios);
        std::vector<std::string> tKeys = {"name", "type", "value"};
        std::vector<std::string> tValues = {"Conductivity Ratios", "Array(double)", tConductivityRatiosList};
        XMLGen::append_parameter_plus_attributes(tKeys, tValues, aParentNode);
    }
}

/******************************************************************************//**
 * \fn append_surface_scalar_function_criterion
 * \tparam CriterionT criterion metadata
 * \brief Append surface scalar function to criterion parameter list. Surface scalar 
*         functions are integrated along surfaces. 
 * \param [in]  aCriterion   criterion metadata
 * \param [out] aParentNode  pugi::xml_node
 **********************************************************************************/
template<typename CriterionT>
pugi::xml_node append_surface_scalar_function_criterion
(const CriterionT& aCriterion,
 pugi::xml_node&   aParentNode)
{
    auto tDesignCriterionName = XMLGen::Private::is_criterion_supported_in_plato_analyze(aCriterion);
    auto tCriterionNode = append_criterion_node(aCriterion, aParentNode);

    std::vector<std::string> tKeys, tValues;

    tKeys = {"name", "type", "value"}; tValues = {"Type", "string", "Scalar Function"};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tCriterionNode);

    tValues = {"Scalar Function Type", "string", tDesignCriterionName};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tCriterionNode);

    XMLGen::Private::check_criterion_location_name_list(aCriterion);
    auto tEntitySetNames = aCriterion.values("location_name");
    auto tEntitySetList = XMLGen::transform_tokens_for_plato_analyze_input_deck(tEntitySetNames);
    tValues = {"Sides", "Array(string)", tEntitySetList};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tCriterionNode);

    XMLGen::Private::append_conductivity_ratios(aCriterion, tCriterionNode);

    return tCriterionNode;
}

/******************************************************************************//**
 * \fn append_scalar_function_criterion
 * \tparam Criterion criterion metadata
 * \brief Append scalar function parameters to criterion parameter list.
 * \param [in] aCriterion criterion metadata
 * \param [in/out] aParentNode  pugi::xml_node
 **********************************************************************************/
template<typename Criterion>
pugi::xml_node append_scalar_function_criterion
(const Criterion& aCriterion,
 pugi::xml_node& aParentNode)
{
    auto tDesignCriterionName = XMLGen::Private::is_criterion_supported_in_plato_analyze(aCriterion);
    auto tCriterionLinearFlag = XMLGen::Private::is_criterion_linear(aCriterion);

    auto tScalarFunction = append_criterion_node(aCriterion, aParentNode);

    std::vector<std::string> tKeys, tValues;

    tKeys = {"name", "type", "value"}; tValues = {"Type", "string", "Scalar Function"};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tScalarFunction);

    if(tCriterionLinearFlag == "true")
    {
        tKeys = {"name", "type", "value"}; tValues = {"Linear", "bool", "true"};
        XMLGen::append_parameter_plus_attributes(tKeys, tValues, tScalarFunction);
    }

    XMLGen::Private::append_block_list(aCriterion, tScalarFunction);
    
    tValues = {"Scalar Function Type", "string", tDesignCriterionName};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tScalarFunction);

    std::string tPenaltyMethod = aCriterion.value("material_penalty_model");
    if(tPenaltyMethod == "none")
    {
         XMLGen::Private::append_no_penalty_function(aCriterion, tScalarFunction);
    }
    else
    {
        XMLGen::Private::append_simp_penalty_function(aCriterion, tScalarFunction);
    }

    return tScalarFunction;
}

/******************************************************************************//**
 * \fn append_pnorm_criterion
 * \tparam Criterion criterion metadata
 * \brief Append p-norm function parameters to criterion parameter list.
 * \param [in] aCriterion criterion metadata
 * \param [in/out] aParentNode  pugi::xml_node
**********************************************************************************/
template<typename Criterion>
pugi::xml_node append_pnorm_criterion
(const Criterion& aCriterion,
 pugi::xml_node& aParentNode)
{
    auto tCriterion = XMLGen::Private::append_scalar_function_criterion(aCriterion, aParentNode);
 //   auto tCriterion = aParentNode.child("ParameterList");
    if(tCriterion.empty())
    {
        THROWERR("Append P-Norm Criterion: Criterion parameter list is empty. Most likely, "
            + "there was an error appending the scalar function criterion.")
    }
    std::vector<std::string> tKeys = {"name", "type", "value"};
    std::vector<std::string> tValues = {"Exponent", "double", aCriterion.pnormExponent()};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tCriterion);
    return tCriterion;
}

/******************************************************************************//**
 * \fn append_thermomechanical_compliance_criterion
 * \tparam Criterion criterion metadata
 * \brief Append thermomechanical compliance function parameters to criterion parameter list.
 * \param [in] aCriterion criterion metadata
 * \param [in/out] aParentNode  pugi::xml_node
**********************************************************************************/
template<typename Criterion>
pugi::xml_node append_thermomechanical_compliance_criterion
(const Criterion& aCriterion,
 pugi::xml_node& aParentNode)
{
    auto tCriterion = XMLGen::Private::append_scalar_function_criterion(aCriterion, aParentNode);
    if(tCriterion.empty())
    {
        THROWERR("Append Thermomechanical Compliance Criterion: Criterion parameter list is empty. Most likely, "
            + "there was an error appending the scalar function criterion.")
    }

    std::vector<std::string> tKeys = {"name", "type", "value"};
    std::vector<std::string> tValues = {"Mechanical Weighting Factor", "double", aCriterion.mechanicalWeightingFactor()};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tCriterion);

    tValues = {"Thermal Weighting Factor", "double", aCriterion.thermalWeightingFactor()};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tCriterion);

    return tCriterion;
}

/******************************************************************************//**
 * \fn append_stress_constrained_mass_minimization_criterion_parameters
 * \brief Append stress constrained mass_minimization criterion input parameters \n
 * to criterion parameter list.
 * \param [in] aCriterion criterion metadata
 * \param [in/out] aParentNode  pugi::xml_node
 **********************************************************************************/
inline void append_stress_constrained_mass_minimization_criterion_parameters
(const XMLGen::Criterion& aCriterion,
 pugi::xml_node& aParentNode)
{
    // append stress constrained mass minimization scalar function parameters
    std::vector<std::string> tKeys = {"name", "type", "value"};
    std::vector<std::string> tValues = {"Stress Limit", "double", aCriterion.stressLimit()};
    XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, aParentNode);

    std::string tWeight = (aCriterion.scmmMassWeight() == "" ? std::string("1.0") : aCriterion.scmmMassWeight());
    tValues = {"Mass Criterion Weight", "double", tWeight};
    XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, aParentNode);

    tWeight = (aCriterion.scmmStressWeight() == "" ? std::string("1.0") : aCriterion.scmmStressWeight());
    tValues = {"Stress Criterion Weight", "double", tWeight};
    XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, aParentNode);

    tValues = {"Initial Penalty", "double", aCriterion.scmmInitialPenalty()};
    XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, aParentNode);

    tValues = {"Penalty Upper Bound", "double", aCriterion.scmmPenaltyUpperBound()};
    XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, aParentNode);

    tValues = {"Penalty Expansion Multiplier", "double", aCriterion.scmmPenaltyExpansionMultiplier()};
    XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, aParentNode);
}

/******************************************************************************//**
 * \fn append_stress_constrained_mass_minimization_criterion
 * \brief Append stress constrained mass_minimization criterion to criterion parameter list.
 * \param [in] aCriterion criterion metadata
 * \param [in/out] aParentNode  pugi::xml_node
**********************************************************************************/
inline pugi::xml_node append_stress_constrained_mass_minimization_criterion
(const XMLGen::Criterion& aCriterion,
 pugi::xml_node& aParentNode)
{
    auto tDesignCriterionName = XMLGen::Private::is_criterion_supported_in_plato_analyze(aCriterion);
    auto tObjective = append_criterion_node(aCriterion, aParentNode);

    std::vector<std::string> tKeys, tValues;

    // append stress constrained mass minimization scalar function
    tKeys = {"name", "type", "value"};
    tValues = {"Type", "string", "Scalar Function"};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);
    tValues = {"Scalar Function Type", "string", tDesignCriterionName};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);
    XMLGen::Private::append_stress_constrained_mass_minimization_criterion_parameters(aCriterion, tObjective);
    return tObjective;
}

/******************************************************************************//**
 * \fn append_stress_constraint_quadratic_criterion
 * \brief Append stress constraint quadratic input parameters \n
 * to criterion parameter list.
 * \param [in] aCriterion criterion metadata
 * \param [in/out] aParentNode  pugi::xml_node
 **********************************************************************************/
inline void append_stress_constraint_quadratic_criterion
(const XMLGen::Criterion& aCriterion,
 pugi::xml_node& aParentNode)
{
    auto tDesignCriterionName = XMLGen::Private::is_criterion_supported_in_plato_analyze(aCriterion);
    auto tObjective = append_criterion_node(aCriterion, aParentNode);

    std::vector<std::string> tKeys, tValues;

    tKeys = {"name", "type", "value"};
    tValues = {"Type", "string", "Scalar Function"};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);
    tValues = {"Scalar Function Type", "string", tDesignCriterionName};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);

    std::string tLocalMeasureType = aCriterion.localMeasure();
    tValues = {"Local Measure", "string", tLocalMeasureType};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);

    tKeys = {"name", "type", "value"};
    tValues = {"Local Measure Limit", "double", aCriterion.stressLimit()};
    XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);

    tValues = {"Initial Penalty", "double", aCriterion.scmmInitialPenalty()};
    XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);

    tValues = {"Penalty Upper Bound", "double", aCriterion.scmmPenaltyUpperBound()};
    XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);

    tValues = {"Penalty Expansion Multiplier", "double", aCriterion.scmmPenaltyExpansionMultiplier()};
    XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);

    tValues = {"SIMP Penalty", "double", aCriterion.materialPenaltyExponent()};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);

    auto tPropertyValue = XMLGen::set_value_keyword_to_ignore_if_empty(aCriterion.minErsatzMaterialConstant());
    tValues = {"Min. Ersatz Material", "double", tPropertyValue};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);
}

/******************************************************************************//**
 * \fn append_volume_average_criterion
 * \brief Append volume average criterion to criterion parameter list.
 * \tparam Criterion criterion metadata
 * \param [in] aCriterion criterion metadata
 * \param [in/out] aParentNode  pugi::xml_node
**********************************************************************************/
template<typename Criterion>
pugi::xml_node append_volume_average_criterion
(const Criterion& aCriterion,
 pugi::xml_node& aParentNode)
{
    XMLGen::Private::is_criterion_supported_in_plato_analyze(aCriterion);
    auto tObjective = append_criterion_node(aCriterion, aParentNode);

    std::vector<std::string> tKeys, tValues;

    tKeys = {"name", "type", "value"};
    tValues = {"Type", "string", "Volume Average Criterion"};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);

    std::string tLocalMeasureType = aCriterion.localMeasure();
    tValues = {"Local Measure", "string", tLocalMeasureType};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);

    std::string tSpatialWeightingFunction = aCriterion.spatialWeightingFunction();
    tValues = {"Function", "string", tSpatialWeightingFunction};
    XMLGen::set_value_keyword_to_ignore_if_empty(tValues);
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);
    
    std::cout<<aCriterion.value("material_penalty_model")<<std::endl;
    std::string tPenaltyMethod = aCriterion.value("material_penalty_model");
    if(tPenaltyMethod == "none")
    {
         XMLGen::Private::append_no_penalty_function(aCriterion, tObjective);
    }
    else
    {
        XMLGen::Private::append_simp_penalty_function(aCriterion, tObjective);
    }
    return tObjective;
}

/******************************************************************************//**
 * \fn append_mass_properties_criterion
 * \brief Append mass properties criterion to criterion parameter list.
 * \tparam Criterion criterion metadata
 * \param [in] aCriterion criterion metadata
 * \param [in/out] aParentNode  pugi::xml_node
**********************************************************************************/
template<typename Criterion>
pugi::xml_node append_mass_properties_criterion
(const Criterion& aCriterion,
 pugi::xml_node& aParentNode)
{
    XMLGen::Private::is_criterion_supported_in_plato_analyze(aCriterion);
    auto tObjective = append_criterion_node(aCriterion, aParentNode);

    std::vector<std::string> tKeys, tValues;

    tKeys = {"name", "type", "value"}; 
    tValues = {"Linear", "bool", "true"};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);
    tKeys = {"name", "type", "value"};
    tValues = {"Type", "string", "Mass Properties"};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);
    std::stringstream properties, goldValues, weights;
    properties << "{";
    goldValues << "{";
    weights << "{";
    const auto &p = aCriterion.getMassProperties();
    for(auto pi=p.begin();pi!=p.end();pi++) {
        properties << (*pi).first;
        goldValues << (*pi).second.first;
        weights << (*pi).second.second;
        if (pi != --p.end()) {
            properties << ",";
            goldValues << ",";
            weights << ",";
        }
    }
    properties << "}";
    goldValues << "}";
    weights << "}";

    tValues = {"Properties", "Array(string)", properties.str()};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);

    tValues = {"Gold Values", "Array(double)", goldValues.str()};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);

    tValues = {"Weights", "Array(double)", weights.str()};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);

    return tObjective;
}

/******************************************************************************//**
 * \fn append_displacement_criterion
 * \brief Append displacement criterion to criterion parameter list.
 * \tparam Criterion criterion metadata
 * \param [in] aCriterion criterion metadata
 * \param [in/out] aParentNode  pugi::xml_node
**********************************************************************************/
template<typename Criterion>
pugi::xml_node append_displacement_criterion
(const Criterion& aCriterion,
 pugi::xml_node& aParentNode)
{
    XMLGen::Private::is_criterion_supported_in_plato_analyze(aCriterion);
    auto tObjective = append_criterion_node(aCriterion, aParentNode);

    std::vector<std::string> tKeys, tValues;

    tKeys = {"name", "type", "value"}; 
    tValues = {"Type", "string", "Solution"};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);
    std::vector<std::string> tDir = aCriterion.displacementDirection();
    tValues = {"Normal", "Array(double)", "{" + tDir[0] + "," + tDir[1] + "," + tDir[2] + "}"};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);
    tValues = {"Domain", "string", aCriterion.location_name()};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);
    tValues = {"Magnitude", "bool", aCriterion.measure_magnitude()};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);
    std::vector<std::string> tTargetSolutionVector = aCriterion.targetSolutionVector();
    if(tTargetSolutionVector.size() > 0)
    {
        std::string tTempString = "{";
        for(size_t i=0; i<tTargetSolutionVector.size(); ++i)
        {
            if(i>0)
            {
                tTempString += ",";
            }
            tTempString += tTargetSolutionVector[i];
        }
        tTempString += "}";
        tValues = {"TargetSolutionVector", "Array(double)", tTempString};
        XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);
    }
    if(aCriterion.target_magnitude() != "")
    {
        tValues = {"TargetMagnitude", "double", aCriterion.target_magnitude()};
        XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);
    }
    if(aCriterion.target_solution() != "")
    {
        tValues = {"TargetSolution", "double", aCriterion.target_solution()};
        XMLGen::append_parameter_plus_attributes(tKeys, tValues, tObjective);
    }

    return tObjective;
}

template<typename CriterionT>
pugi::xml_node append_criterion_node(const CriterionT& aCriterion, pugi::xml_node& aParentNode)
{
    std::string tName = std::string("my_") + Plato::tolower(aCriterion.type()) + "_criterion_id_" + aCriterion.id();
    pugi::xml_node tCriterionNode = aParentNode.append_child("ParameterList");
    std::vector<std::string> tKeys = {"name"};
    std::vector<std::string> tValues = {tName};
    XMLGen::append_attributes(tKeys, tValues, tCriterionNode);
    return tCriterionNode;
}
// function append_criterion_node

template<typename CriterionT>
void append_block_list(const CriterionT& aCriterion, pugi::xml_node& aParentNode)
{
    auto tLowerType = Plato::tolower(aCriterion.type());
    if( tLowerType == "volume" )
    {
        auto tElemBlockList = aCriterion.values("location_name");
        if( !tElemBlockList.empty() )
        {
            std::vector<std::string> tKeys = {"name", "type", "value"};
            auto tElemBlocksNames = XMLGen::transform_tokens_for_plato_analyze_input_deck(tElemBlockList);
            std::vector<std::string> tValues = {"Domains", "Array(string)", tElemBlocksNames};
            XMLGen::append_parameter_plus_attributes(tKeys, tValues, aParentNode);
        }
        else
        {
            aCriterion.report("All the element blocks will be considered in the volume evaluation.");
        }
    }
}
// function append_block_list

template<typename CriterionT>
std::string is_criterion_supported_in_plato_analyze(const CriterionT& aCriterion)
{
    XMLGen::ValidAnalyzeCriteriaKeys tValidKeys;
    auto tLowerCriterion = Plato::tolower(aCriterion.type());
    auto tItr = tValidKeys.mKeys.find(tLowerCriterion);
    if (tItr == tValidKeys.mKeys.end())
    {
        THROWERR(std::string("Is Criterion Supported in Plato Analyze: Criterion '")
            + tLowerCriterion + "' is not supported.")
    }
    return tItr->second.first;
}

}
// namespace Private

}
// namespace XMLGen
