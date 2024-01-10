/*
 * XMLGeneratorSierraTFInputDeckUtilities.cpp
 *
 *  Created on: July 6, 2022
 */

#include <algorithm>
#include <fstream>
#include <ostream>
#include <sstream>
#include <sys/stat.h>

#include "XMLGeneratorUtilities.hpp"
//#include "XMLGeneratorSierraTFUtilities.hpp"
//#include "XMLGeneratorCriterionMetadata.hpp"
//#include "XMLGeneratorInterfaceFileUtilities.hpp"
#include "XMLGeneratorSierraTFInputDeckUtilities.hpp"

namespace XMLGen
{

void write_sierra_tf_inverse_input_file
(const XMLGen::InputData& aXMLMetaData)
{
    pugi::xml_document doc;
    pugi::xml_node n1, n2, n3;
    pugi::xml_attribute a1;
    // There should only be one criterion by the time we get to here.
    XMLGen::Criterion tCriterion = aXMLMetaData.criteria()[0];
    std::string tVariableToMatch = tCriterion.temperature_field_name(); 
    std::vector<std::string> tMatchNodesetIDs = tCriterion.matchNodesetIDs();
    std::vector<std::string> tSearchNodesetIDs = tCriterion.searchNodesetIDs();
    std::string tRefDataFile = tCriterion.ref_data_file();
    std::string tMeshFile = aXMLMetaData.mesh.name;

    // Version entry
    n1 = doc.append_child("ParameterList");
    n1.append_attribute("name") = "Inputs";

    addNTVParameter(n1, "checkReducedGradient", "bool", "true");
    addNTVParameter(n1, "finiteDifferenceScale", "double", "1e-3");
    addNTVParameter(n1, "variableToMatch", "string", tVariableToMatch);
    n2 = n1.append_child("ParameterList");
    n2.append_attribute("name") = "Problem Data";
    addNTVParameter(n2, "Data File", "string", "aria_gold_data.txt");
    n2 = n1.append_child("ParameterList");
    n2.append_attribute("name") = "Plato";
    addNTVParameter(n2, "Exodus Gold Data File", "string", tRefDataFile);
    addNTVParameter(n2, "Mesh File", "string", tMeshFile);
    addNTVParameter(n2, "State Variable", "string", tVariableToMatch);
    std::string tBuffer = "{";
    for(size_t i=0; i<tMatchNodesetIDs.size(); ++i)
    {
        if(i!=0)
            tBuffer += ", ";
        tBuffer += tMatchNodesetIDs[i];
    }
    tBuffer += "}";
    addNTVParameter(n2, "Nodesets To Match", "string", tBuffer);
    tBuffer = "{";
    for(size_t i=0; i<tSearchNodesetIDs.size(); ++i)
    {
        if(i!=0)
            tBuffer += ", ";
        tBuffer += tSearchNodesetIDs[i];
    }
    tBuffer += "}";
    addNTVParameter(n2, "Nodesets To Search", "string", tBuffer);

    // Write the file to disk
    doc.save_file("inverseInput.xml", "  ");
}

}
// namespace XMLGen
