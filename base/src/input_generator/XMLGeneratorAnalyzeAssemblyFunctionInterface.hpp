/*
 * XMLGeneratorAnalyzeAssemblyFunctionInterface.hpp
 *
 *  Created on: Jun 7, 2021
 */

#pragma once

#include "pugixml.hpp"

#include "XMLGeneratorDataStruct.hpp"
#include "XMLGeneratorAnalyzeFunctionMapTypes.hpp"

namespace XMLGen
{

/******************************************************************************//**
 * \struct The goal of this C++ struct is to provide an interface for the \n
 * functions used to define assembly names for plato_analyze_input_deck.xml. 
**********************************************************************************/
struct AppendAssembly
{
private:
    /*!< map from assembly category to function used to append its properties */
    XMLGen::Analyze::AssemblyFuncMap mMap;

    /******************************************************************************//**
     * \fn insert
     * \brief Insert assembly functions to map.
     **********************************************************************************/
    void insert();

public:
    /******************************************************************************//**
     * \fn AppendAssembly
     * \brief Default constructor
    **********************************************************************************/
    AppendAssembly() { this->insert(); }

    /******************************************************************************//**
     * \fn call
     * \brief Append assembly parameters to plato_analyze_input_deck.xml.
     * \param [in]     aAssembly    assembly metadata
     * \param [in/out] aParentNode  pugi::xml_node
    **********************************************************************************/
    void call(const XMLGen::Assembly& aAssembly, pugi::xml_node& aParentNode) const;
};
// struct AppendAssembly

}
// namespace XMLGen
