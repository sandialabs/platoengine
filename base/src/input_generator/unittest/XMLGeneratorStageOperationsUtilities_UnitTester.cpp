/*
 * XMLGeneratorStageOperationsUtilities_UnitTester.cpp
 *
 *  Created on: Jul 6, 2022
 */

#include <gtest/gtest.h>

#include "XMLGenerator_UnitTester_Tools.hpp"
#include "XMLGeneratorUtilities.hpp"
#include "XMLGeneratorStagesOperationsUtilities.hpp"

namespace PlatoTestXMLGenerator
{

TEST(PlatoTestXMLGenerator, StageOperation_get_objective_value_operation_name_SierraTF)
{
    XMLGen::Service tService;
    tService.id("2");
    tService.code("sierra_tf");
    std::string tString = XMLGen::get_objective_value_operation_name(tService);
    ASSERT_TRUE(tString == "Compute Criterion");
}

TEST(PlatoTestXMLGenerator, StageOperation_get_objective_value_operation_output_name_SierraTF)
{
    XMLGen::Service tService;
    tService.id("2");
    tService.code("sierra_tf");
    std::string tString = XMLGen::get_objective_value_operation_output_name(tService);
    ASSERT_TRUE(tString == "Criterion");
}

TEST(PlatoTestXMLGenerator, StageOperation_get_objective_gradient_operation_name_SierraTF)
{
    XMLGen::Service tService;
    tService.id("2");
    tService.code("sierra_tf");
    std::string tString = XMLGen::get_objective_gradient_operation_name(tService);
    ASSERT_TRUE(tString == "Compute Criterion Gradient");
}

TEST(PlatoTestXMLGenerator, StageOperation_append_compute_criterion_sensitivity_operation_SierraTF)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tMetaData;
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("esp_workflow", "aflr4_aflr3");
    tMetaData.set(tOptimizationParameters);
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("sierra_tf");
    tMetaData.append(tService);
    XMLGen::append_compute_criterion_sensitivity_operation(tMetaData, tService, "CriterionIdentifierString", tDocument);
    tDocument.save_file("xml.txt", " ");

    auto tReadData = XMLGen::read_data_from_file("xml.txt");
    std::string tGoldString = std::string("<?xmlversion=\"1.0\"?><Operation><Name>") +
        std::string("ChainRule</Name>") +
        std::string("<PerformerName>platomain_1</PerformerName><Forvar=\"I\"in=\"Parameters\">") +
        std::string("<Input><ArgumentName>ParameterSensitivity{I}</ArgumentName>") +
        std::string("<SharedDataName>ParameterSensitivity{I}</SharedDataName>") +
        std::string("</Input></For><Input><ArgumentName>DFDX</ArgumentName><SharedDataName>") +
        std::string("CriterionGradientX-CriterionIdentifierString</SharedDataName></Input><Output><ArgumentName>FullGradient") +
        std::string("</ArgumentName><SharedDataName>CriterionGradient-CriterionIdentifierString</SharedDataName>") +
        std::string("</Output></Operation>");

    ASSERT_STREQ(tGoldString.c_str(), tReadData.str().c_str());
    Plato::system("rm xml.txt");
}

TEST(PlatoTestXMLGenerator, StageOperation_append_compute_criterion_sensitivity_operation_SierraTF_egads_tetgen_workflow)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tMetaData;
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("esp_workflow", "egads_tetgen");
    tMetaData.set(tOptimizationParameters);
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("sierra_tf");
    tMetaData.append(tService);
    XMLGen::append_compute_criterion_sensitivity_operation(tMetaData, tService, "CriterionIdentifierString", tDocument);
    tDocument.save_file("xml.txt", " ");

    auto tReadData = XMLGen::read_data_from_file("xml.txt");
    std::string tGoldString = std::string("<?xmlversion=\"1.0\"?><Operation><Name>") +
        std::string("ChainRule</Name>") +
        std::string("<PerformerName>platomain_1</PerformerName><Forvar=\"I\"in=\"Parameters\">") +
        std::string("<Input><ArgumentName>ParameterSensitivity{I}</ArgumentName>") +
        std::string("<SharedDataName>ParameterSensitivity{I}</SharedDataName>") +
        std::string("</Input></For><Input><ArgumentName>DFDX</ArgumentName><SharedDataName>") +
        std::string("CriterionGradientX-CriterionIdentifierString</SharedDataName></Input><Output><ArgumentName>FullGradient") +
        std::string("</ArgumentName><SharedDataName>CriterionGradient-CriterionIdentifierString</SharedDataName>") +
        std::string("</Output></Operation>");

    ASSERT_STREQ(tGoldString.c_str(), tReadData.str().c_str());
    Plato::system("rm xml.txt");
}

TEST(PlatoTestXMLGenerator, StageOperation_append_compute_criterion_sensitivity_operation_SierraTF_unknown_esp_workflow)
{
    pugi::xml_document tDocument;
    XMLGen::InputData tMetaData;
    XMLGen::OptimizationParameters tOptimizationParameters;
    tOptimizationParameters.append("esp_workflow", "unknown_workflow");
    tOptimizationParameters.optimizationType(XMLGen::OT_SHAPE);
    tMetaData.set(tOptimizationParameters);
    XMLGen::Service tService;
    tService.id("1");
    tService.code("platomain");
    tMetaData.append(tService);
    tService.id("2");
    tService.code("sierra_tf");
    tMetaData.append(tService);
    EXPECT_THROW(XMLGen::append_compute_criterion_sensitivity_operation(tMetaData, tService, "CriterionIdentifierString", tDocument), std::exception);
}

}
// namespace PlatoTestXMLGenerator
