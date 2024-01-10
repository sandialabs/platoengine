 /*
 * SharedData_UnitTester.cpp
 *
 *  Created on: April 12, 2022
 */
#include <gtest/gtest.h>

#include "pugixml.hpp"
#include "XMLGenerator_UnitTester_Tools.hpp"
#include "SharedData.hpp"
#include "Performer.hpp"

namespace PlatoTestXMLGenerator
{

TEST(PlatoTestXMLGenerator, WriteDesignParametersSharedDataGlobalWithTag)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,2);
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain_1","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};
    std::shared_ptr<director::SharedData> tSharedData = std::make_shared<director::SharedDataGlobal>("design_parameters","3",tPerformerMain,tUserPerformers,2);
    
    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tSharedData->write_interface(tDocument));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("SharedData");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("SharedData", tOperation.name());
    std::vector<std::string> tKeys = {
        "Name", 
        "Type", 
        "Layout",
        "Size",
        "OwnerName",
        "UserName",
        "UserName"};
    
    std::vector<std::string> tValues = {
        "design_parameters_{E}", 
        "Scalar", 
        "Global",
        "3",
        "platomain_1",
        "platomain_1",
        "plato_services_{E}"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    tOperation = tOperation.next_sibling("SharedData");
    ASSERT_TRUE(tOperation.empty());
}

TEST(PlatoTestXMLGenerator, WriteDesignParametersSharedDataGlobal)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,2);
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain_1","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};
    
    std::shared_ptr<director::SharedData> tSharedData = std::make_shared<director::SharedDataGlobal>("design_parameters","3",tPerformerMain,tUserPerformers,2);
    
    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tSharedData->write_interface(tDocument,"2"));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("SharedData");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("SharedData", tOperation.name());
    std::vector<std::string> tKeys = {
        "Name", 
        "Type", 
        "Layout",
        "Size",
        "OwnerName",
        "UserName",
        "UserName"};
    
    std::vector<std::string> tValues = {
        "design_parameters_2", 
        "Scalar", 
        "Global",
        "3",
        "platomain_1",
        "platomain_1",
        "plato_services_2"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    tOperation = tOperation.next_sibling("SharedData");
    ASSERT_TRUE(tOperation.empty());
}

TEST(PlatoTestXMLGenerator, WriteDesignParametersSharedDataGlobalNoConcurrency)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,0);
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain_1","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};
    
    std::shared_ptr<director::SharedData> tSharedData = std::make_shared<director::SharedDataGlobal>("design_parameters","3",tPerformerMain,tUserPerformers);
    
    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tSharedData->write_interface(tDocument));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("SharedData");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("SharedData", tOperation.name());
    std::vector<std::string> tKeys = {
        "Name", 
        "Type", 
        "Layout",
        "Size",
        "OwnerName",
        "UserName",
        "UserName"};
    
    std::vector<std::string> tValues = {
        "design_parameters", 
        "Scalar", 
        "Global",
        "3",
        "platomain_1",
        "platomain_1",
        "plato_services"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    tOperation = tOperation.next_sibling("SharedData");
    ASSERT_TRUE(tOperation.empty());
}

TEST(PlatoTestXMLGenerator, WriteDesignParametersSharedDataNodalFieldNoConcurrency)
{
    std::shared_ptr<director::Performer> tPerformer = std::make_shared<director::Performer>("plato_services","plato_services",1,16,0);
    std::shared_ptr<director::Performer> tPerformerMain = std::make_shared<director::Performer>("platomain_1","platomain");
    std::vector<std::shared_ptr<director::Performer>> tUserPerformers = {tPerformerMain,tPerformer};
    
    std::shared_ptr<director::SharedData> tSharedData = std::make_shared<director::SharedDataNodalField>("design_parameters",tPerformerMain,tUserPerformers);
    
    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tSharedData->write_interface(tDocument));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tOperation = tDocument.child("SharedData");
    ASSERT_FALSE(tOperation.empty());
    ASSERT_STREQ("SharedData", tOperation.name());
    std::vector<std::string> tKeys = {
        "Name", 
        "Type", 
        "Layout",
        "OwnerName",
        "UserName",
        "UserName"};
    
    std::vector<std::string> tValues = {
        "design_parameters", 
        "Scalar", 
        "Nodal Field",
        "platomain_1",
        "platomain_1",
        "plato_services"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tOperation);

    tOperation = tOperation.next_sibling("SharedData");
    ASSERT_TRUE(tOperation.empty());
}

}