/*
 * XMLGeneratorAnalyzeAppendCriterionFunctionInterface.hpp
 *
 *  Created on: Jun 15, 2020
 */

#pragma once

#include "XMLGeneratorAnalyzeFunctionMapTypes.hpp"
#include "XMLGeneratorAnalyzeCriterionUtilities.hpp"

namespace XMLGen
{

namespace Private
{

template<typename CriterionT>
void append_incompressible_fluids_scalar_functions
(XMLGen::Analyze::CriterionFuncMap & aMap)
{
    // mean surface pressure
    auto tFuncIndex = std::type_index(typeid(XMLGen::Private::append_surface_scalar_function_criterion<CriterionT>));
    aMap.insert(std::make_pair("mean_surface_pressure",
        std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_surface_scalar_function_criterion<CriterionT>, tFuncIndex)));

    // mean surface temperature
    tFuncIndex = std::type_index(typeid(XMLGen::Private::append_surface_scalar_function_criterion<CriterionT>));
    aMap.insert(std::make_pair("mean_surface_temperature",
        std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_surface_scalar_function_criterion<CriterionT>, tFuncIndex)));

    // maximize fluid thermal flux
    tFuncIndex = std::type_index(typeid(XMLGen::Private::append_surface_scalar_function_criterion<CriterionT>));
    aMap.insert(std::make_pair("maximize_fluid_thermal_flux", 
        std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_surface_scalar_function_criterion<CriterionT>, tFuncIndex)));

    // fluid thermal compliance
    tFuncIndex = std::type_index(typeid(XMLGen::Private::append_scalar_function_criterion<CriterionT>));
    aMap.insert(std::make_pair("fluid_thermal_compliance", 
        std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_scalar_function_criterion<CriterionT>, tFuncIndex)));
    
    // mean temperature
    tFuncIndex = std::type_index(typeid(XMLGen::Private::append_scalar_function_criterion<CriterionT>));
    aMap.insert(std::make_pair("mean_temperature", 
        std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_scalar_function_criterion<CriterionT>, tFuncIndex)));
}
// function append_incompressible_fluids_scalar_functions

}
// namespace Private

/******************************************************************************//**
 * \tparam CriterionType criterion metadata type (e.g. objective or constraint)
 * 
 * \struct The goal of this C++ struct is to provide an interface for the \n
 * functions used to append design criteria paraemters to plato_analyze_input_deck.xml. \n
 * This interface reduces cyclomatic complexity due to having multiple design \n
 * criteria in Plato Analyze.
**********************************************************************************/
template<typename CriterionType>
struct AppendCriterionParameters
{
private:
    /*!< map from design criterion category to function used to append design criterion and respective parameters */
    XMLGen::Analyze::CriterionFuncMap mMap;

    /******************************************************************************//**
     * \fn insert
     * \brief Insert functions used to append design criterion parameters to design \n
     *   criterion function map.
     **********************************************************************************/
    void insert()
    {
        // volume
        auto tFuncIndex = std::type_index(typeid(XMLGen::Private::append_scalar_function_criterion<CriterionType>));
        mMap.insert(std::make_pair("volume",
          std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_scalar_function_criterion<CriterionType>, tFuncIndex)));

        // maximize stiffness
        tFuncIndex = std::type_index(typeid(XMLGen::Private::append_scalar_function_criterion<CriterionType>));
        mMap.insert(std::make_pair("mechanical_compliance",
          std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_scalar_function_criterion<CriterionType>, tFuncIndex)));

        // stress_and_mass (same as stress_constraint_general)
        tFuncIndex = std::type_index(typeid(XMLGen::Private::append_stress_constrained_mass_minimization_criterion));
        mMap.insert(std::make_pair("stress_and_mass",
          std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_stress_constrained_mass_minimization_criterion, tFuncIndex)));

        // stress_constraint_general
        tFuncIndex = std::type_index(typeid(XMLGen::Private::append_stress_constrained_mass_minimization_criterion));
        mMap.insert(std::make_pair("stress_constraint_general",
          std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_stress_constrained_mass_minimization_criterion, tFuncIndex)));
        
        // stress_constraint_quadratic
        tFuncIndex = std::type_index(typeid(XMLGen::Private::append_stress_constrained_mass_minimization_criterion));
        mMap.insert(std::make_pair("stress_constraint_quadratic",
          std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_stress_constraint_quadratic_criterion, tFuncIndex)));

        // stress p-norm
        tFuncIndex = std::type_index(typeid(XMLGen::Private::append_pnorm_criterion<CriterionType>));
        mMap.insert(std::make_pair("stress_p-norm",
          std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_pnorm_criterion<CriterionType>, tFuncIndex)));

        // effective energy
        tFuncIndex = std::type_index(typeid(XMLGen::Private::append_scalar_function_criterion<CriterionType>));
        mMap.insert(std::make_pair("effective_energy",
          std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_scalar_function_criterion<CriterionType>, tFuncIndex)));

        // maximize heat conduction
        tFuncIndex = std::type_index(typeid(XMLGen::Private::append_scalar_function_criterion<CriterionType>));
        mMap.insert(std::make_pair("thermal_compliance",
          std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_scalar_function_criterion<CriterionType>, tFuncIndex)));

        // thermo-elastic energy
        tFuncIndex = std::type_index(typeid(XMLGen::Private::append_scalar_function_criterion<CriterionType>));
        mMap.insert(std::make_pair("thermomechanical_compliance",
          std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_thermomechanical_compliance_criterion<CriterionType>, tFuncIndex)));

        // electro-elastic energy
        tFuncIndex = std::type_index(typeid(XMLGen::Private::append_scalar_function_criterion<CriterionType>));
        mMap.insert(std::make_pair("electromechanical_compliance",
          std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_scalar_function_criterion<CriterionType>, tFuncIndex)));

        // flux p-norm
        tFuncIndex = std::type_index(typeid(XMLGen::Private::append_pnorm_criterion<CriterionType>));
        mMap.insert(std::make_pair("flux_p-norm",
          std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_pnorm_criterion<CriterionType>, tFuncIndex)));
        
        // elastic_work
        tFuncIndex = std::type_index(typeid(XMLGen::Private::append_scalar_function_criterion<CriterionType>));
        mMap.insert(std::make_pair("elastic_work",
          std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_scalar_function_criterion<CriterionType>, tFuncIndex)));

        // plastic_work
        tFuncIndex = std::type_index(typeid(XMLGen::Private::append_scalar_function_criterion<CriterionType>));
        mMap.insert(std::make_pair("plastic_work",
          std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_scalar_function_criterion<CriterionType>, tFuncIndex)));
        
        // total_work
        tFuncIndex = std::type_index(typeid(XMLGen::Private::append_scalar_function_criterion<CriterionType>));
        mMap.insert(std::make_pair("total_work",
          std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_scalar_function_criterion<CriterionType>, tFuncIndex)));
        
        // thermoplasticity_thermal_energy
        tFuncIndex = std::type_index(typeid(XMLGen::Private::append_scalar_function_criterion<CriterionType>));
        mMap.insert(std::make_pair("thermoplasticity_thermal_energy",
          std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_scalar_function_criterion<CriterionType>, tFuncIndex)));
        
        // volume_average
        tFuncIndex = std::type_index(typeid(XMLGen::Private::append_volume_average_criterion<CriterionType>));
        mMap.insert(std::make_pair("volume_average",
          std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_volume_average_criterion<CriterionType>, tFuncIndex)));

        // mass_properties
        tFuncIndex = std::type_index(typeid(XMLGen::Private::append_mass_properties_criterion<CriterionType>));
        mMap.insert(std::make_pair("mass_properties",
          std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_mass_properties_criterion<CriterionType>, tFuncIndex)));

        // displacement
        tFuncIndex = std::type_index(typeid(XMLGen::Private::append_displacement_criterion<CriterionType>));
        mMap.insert(std::make_pair("displacement",
          std::make_pair((XMLGen::Analyze::CriterionFunc)XMLGen::Private::append_displacement_criterion<CriterionType>, tFuncIndex)));

        // incompressible fluids
        XMLGen::Private::append_incompressible_fluids_scalar_functions<CriterionType>(mMap);
    }

public:
    /******************************************************************************//**
     * \fn AppendCriterionParameters
     * \brief Default constructor
    **********************************************************************************/
    AppendCriterionParameters()
    {
        this->insert();
    }

    /******************************************************************************//**
     * \fn call
     * \tparam CriterionType criterion function metadata C++ structure type
     * \brief Append criterion function parameters to plato_analyze_input_deck.xml file.
     * \param [in]  aCriterion   criterion metadata
     * \param [out] aParentNode  pugi::xml_node
    **********************************************************************************/
    pugi::xml_node call(const CriterionType& aCriterion, pugi::xml_node &aParentNode) const
    {
/* Code is not a member of Criterion and we should never get in here except for plato_analyze if
 * things are being called correctly
        auto tLowerPerformer = Plato::tolower(aCriterion.code());
        if(tLowerPerformer.compare("plato_analyze") != 0)
        {
            return;
        }
*/

        auto tLowerCategory = Plato::tolower(aCriterion.type());
        auto tMapItr = mMap.find(tLowerCategory);
        if(tMapItr == mMap.end())
        {
            THROWERR(std::string("Criterion Function Interface: Did not find criterion function with tag '") + tLowerCategory + "' in list.")
        }
        auto tTypeCastedFunc = reinterpret_cast<pugi::xml_node(*)(const CriterionType&, pugi::xml_node&)>(tMapItr->second.first);
        if(tMapItr->second.second == std::type_index(typeid(tTypeCastedFunc)))
        {
            THROWERR(std::string("Criterion Function Interface: Reinterpret cast for criterion function with tag '") + tLowerCategory + "' failed.")
        }
        return tTypeCastedFunc(aCriterion, aParentNode);
    }
};
// struct AppendCriterionParameters

}
// namespace XMLGen
