/*
 * XMLGeneratorAnalyzeAssemblyFunctionInterface.cpp
 *
 *  Created on: Jun 7, 2021
 */

#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorPlatoAnalyzeInputFileUtilities.hpp"
#include "XMLGeneratorAnalyzeAssemblyFunctionInterface.hpp"

namespace XMLGen
{

namespace Private
{

std::string return_assembly_offset_vector_values
(const XMLGen::Assembly& aAssembly)
{
    std::vector<std::string> tTokens;
    for(auto& tToken : aAssembly.offset())
    {
        tTokens.push_back(tToken);
    }
    auto tOutput = XMLGen::transform_tokens_for_plato_analyze_input_deck(tTokens);
    return tOutput;
}
// function return_assembly_offset_vector_values

void append_tied_assembly_to_plato_problem
(const XMLGen::Assembly& aAssembly,
 pugi::xml_node &aParentNode)
{
    std::vector<std::string> tKeys = {"name", "type", "value"};

    std::string tAssemblyName = "Tied Assembly " + aAssembly.id() + " Using PBC Multipoint Constraint";
    auto tAssemblyParentNode = aParentNode.append_child("ParameterList");
    XMLGen::append_attributes({"name"}, {tAssemblyName}, tAssemblyParentNode);

    std::vector<std::string> tValues = {"Type", "string", "PBC"};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tAssemblyParentNode);

    tValues = {"Child", "string", aAssembly.value("child_nodeset")};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tAssemblyParentNode);

    std::string tBlockName = "Block " + aAssembly.value("parent_block");
    tValues = {"Parent", "string", tBlockName};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tAssemblyParentNode);

    auto tVecValues = XMLGen::Private::return_assembly_offset_vector_values(aAssembly);
    tValues = {"Vector", "Array(double)", tVecValues};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tAssemblyParentNode);

    tValues = {"Value", "double", aAssembly.value("rhs_value")};
    XMLGen::append_parameter_plus_attributes(tKeys, tValues, tAssemblyParentNode);

}
// function append_tied_assembly_to_plato_problem

}
// namespace Private

}
// namespace XMLGen

namespace XMLGen
{

void AppendAssembly::insert()
{
    // tied
    auto tFuncIndex = std::type_index(typeid(XMLGen::Private::append_tied_assembly_to_plato_problem));
    mMap.insert(std::make_pair("tied",
      std::make_pair((XMLGen::Analyze::AssemblyFunc)XMLGen::Private::append_tied_assembly_to_plato_problem, tFuncIndex)));

}

void AppendAssembly::call
(const XMLGen::Assembly& aAssembly,
 pugi::xml_node& aParentNode) const
{
    auto tCategory = Plato::tolower(aAssembly.value("type"));
    auto tMapItr = mMap.find(tCategory);
    if(tMapItr == mMap.end())
    {
        THROWERR(std::string("Assembly Function Interface: Did not find assembly function with tag '")
            + tCategory + "' in list. Assembly '" + tCategory + "' is not supported in Plato Analyze.")
    }
    auto tTypeCastedFunc = reinterpret_cast<void(*)(const XMLGen::Assembly&, pugi::xml_node&)>(tMapItr->second.first);
    if(tMapItr->second.second == std::type_index(typeid(tTypeCastedFunc)))
    {
        THROWERR(std::string("Assembly Interface: Reinterpret cast for assembly function with tag '")
            + tCategory + "' failed.")
    }
    tTypeCastedFunc(aAssembly, aParentNode);
}

}
// namespace XMLGen
