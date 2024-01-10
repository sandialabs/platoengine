#include <gtest/gtest.h>

#include "XMLGeneratorDataStruct.hpp"
#include "XMLGenerator_UnitTester_Tools.hpp"
#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorSierraSDOperationsFileUtilities.hpp"

namespace PlatoTestXMLGenerator {

TEST(PlatoTestXMLGenerator, SDOperationsFile_append_internal_energy_gradient_operation_shape_aflr4_aflr3)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tMetaData;
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.optimizationType(XMLGen::OT_SHAPE);
    tOptimizationParameters.append("esp_workflow", "aflr4_aflr3");
    tMetaData.set(tOptimizationParameters);
    XMLGen::append_internal_energy_gradient_operation_shape(tMetaData, tDocument);
    tDocument.save_file("xml.txt", " ");

    auto tReadData = XMLGen::read_data_from_file("xml.txt");
    std::string tGoldString = std::string("<?xmlversion=\"1.0\"?><Operation><Function>") +
        std::string("ComputeCriterionGradient</Function><Name>") +
        std::string("ComputeGradient</Name><Output><ArgumentName>DFDX") +
        std::string("</ArgumentName></Output></Operation>");

    ASSERT_STREQ(tGoldString.c_str(), tReadData.str().c_str());
    Plato::system("rm xml.txt");
}

TEST(PlatoTestXMLGenerator, SDOperationsFile_append_internal_energy_gradient_operation_shape_egads_tetgen)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tMetaData;
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.optimizationType(XMLGen::OT_SHAPE);
    tOptimizationParameters.append("esp_workflow", "egads_tetgen");
    tMetaData.set(tOptimizationParameters);
    XMLGen::append_internal_energy_gradient_operation_shape(tMetaData, tDocument);
    tDocument.save_file("xml.txt", " ");

    auto tReadData = XMLGen::read_data_from_file("xml.txt");
    std::string tGoldString = std::string("<?xmlversion=\"1.0\"?><Operation><Function>") +
        std::string("ComputeCriterionGradient</Function><Name>") +
        std::string("ComputeGradient</Name><Output><ArgumentName>DFDX") +
        std::string("</ArgumentName></Output></Operation>");

    ASSERT_STREQ(tGoldString.c_str(), tReadData.str().c_str());
    Plato::system("rm xml.txt");
}

TEST(PlatoTestXMLGenerator, SDOperationsFile_append_internal_energy_gradient_operation_topology)
{
    pugi::xml_document tDocument;
    XMLGen::Scenario tScenario;
    XMLGen::append_internal_energy_gradient_operation_topology(tScenario, tDocument);
    tDocument.save_file("xml.txt", " ");

    auto tReadData = XMLGen::read_data_from_file("xml.txt");
    std::string tGoldString = std::string("<?xmlversion=\"1.0\"?><Operation><Function>") +
        std::string("ComputeCriterionGradient</Function><Name>") +
        std::string("ComputeGradient</Name><PenaltyModel>SIMP</PenaltyModel>") +
        std::string("<Topology><Name>Topology</Name></Topology><SIMP><MinimumValue>0.001") +
        std::string("</MinimumValue></SIMP><OutputGradient><Name>InternalEnergyGradient") +
        std::string("</Name></OutputGradient></Operation>");

    ASSERT_STREQ(tGoldString.c_str(), tReadData.str().c_str());
    Plato::system("rm xml.txt");
}


} // namespace PlatoTestXMLGenerator
