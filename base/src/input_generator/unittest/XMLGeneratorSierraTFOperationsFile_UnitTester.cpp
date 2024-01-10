#include <gtest/gtest.h>

#include "XMLGeneratorDataStruct.hpp"
#include "XMLGenerator_UnitTester_Tools.hpp"
#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorSierraTFOperationsFileUtilities.hpp"

namespace PlatoTestXMLGenerator {

XMLGen::InputData setUpMetaDataForExistingTFInputDeck() {
    XMLGen::InputData aXMLMetaData;

    XMLGen::Service tService;
    tService.id("1");
    tService.code("sierra_tf");
    aXMLMetaData.append(tService);

    XMLGen::Scenario tScenario;
    tScenario.id("1");
    tScenario.physics("steady_state_thermal");
    tScenario.append("existing_input_deck", "dummy_deck.i");
    aXMLMetaData.append(tScenario);

    XMLGen::Criterion tCriterion;
    tCriterion.id("1");
    tCriterion.type("temperature_matching");
    std::vector<std::string> tNodesetIDs = {"4"};
    tCriterion.setSearchNodesetIDs(tNodesetIDs);
    tNodesetIDs = {"1", "2", "3"};
    tCriterion.setMatchNodesetIDs(tNodesetIDs);
    tCriterion.append("ref_data_file", "gold.e");
    tCriterion.append("temperature_field_name", "T");
    aXMLMetaData.append(tCriterion);

    XMLGen::Objective tObjective;
    tObjective.serviceIDs.push_back("1");
    tObjective.criteriaIDs.push_back("1");
    tObjective.scenarioIDs.push_back("1");
    tObjective.multi_load_case = "false";
    aXMLMetaData.objective = tObjective;

    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.optimizationType(XMLGen::OT_SHAPE);
    tOptimizationParameters.append("esp_workflow", "aflr4_aflr3");
    aXMLMetaData.set(tOptimizationParameters);

    return aXMLMetaData;
}

TEST(PlatoTestXMLGenerator, TF_WriteOperationsFile)
{
    XMLGen::InputData aXMLMetaData = setUpMetaDataForExistingTFInputDeck();
    ASSERT_NO_THROW(XMLGenSierraTF::write_sierra_tf_operation_xml_file(aXMLMetaData));

    auto tReadData = XMLGen::read_data_from_file("sierra_tf_1_operations.xml");
    auto tGold = std::string("<?xmlversion=\"1.0\"?>") + 
        std::string("<includefilename=\"defines.xml\"/>") + 
        std::string("<Operation><Name>ComputeCriterion</Name>") + 
        std::string("<OutputValue><Name>Criterion</Name></OutputValue></Operation>") + 
        std::string("<Operation><Name>ComputeCriterionGradient</Name><Output>") +
        std::string("<ArgumentName>DFDX</ArgumentName></Output></Operation>");

    ASSERT_STREQ(tGold.c_str(), tReadData.str().c_str());
    Plato::system("rm -f sierra_tf_1_operations.xml");
}


} // namespace PlatoTestXMLGenerator
