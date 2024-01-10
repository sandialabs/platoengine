 /*
 * UnitTesterTools_UnitTester.cpp
 *
 *  Created on: April 12, 2022
 */
#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

#include "pugixml.hpp"
#include "XMLGenerator_UnitTester_Tools.hpp"
#include "XMLGeneratorUtilities.hpp"

namespace PlatoTestXMLGenerator
{
    using namespace XMLGen;

TEST(UnitTesterUtilities, TestChildrenABXY)
{
    pugi::xml_document tDocument;
    auto tOperationNode = tDocument.append_child("Operation");
    addChild(tOperationNode, "File", "test");
    addChild(tOperationNode, "File2", "test2");

    std::vector<std::string> tKeys = {
        "File", 
        "File2"};
    std::vector<std::string> tValues = {
        "test", 
        "test2"};
    PlatoTestXMLGenerator::test_children(tKeys,tValues, tOperationNode);
}


TEST(UnitTesterUtilities, TestChildrenAAXY)
{
    pugi::xml_document tDocument;
    auto tOperationNode = tDocument.append_child("Operation");
    addChild(tOperationNode, "File", "test");
    addChild(tOperationNode, "File", "test2");

    std::vector<std::string> tKeys = {
        "File", 
        "File"};
    std::vector<std::string> tValues = {
        "test", 
        "test2"};
    PlatoTestXMLGenerator::test_children(tKeys,tValues, tOperationNode);
}


TEST(UnitTesterUtilities, TestChildrenABXX)
{
    pugi::xml_document tDocument;
    auto tOperationNode = tDocument.append_child("Operation");
    addChild(tOperationNode, "File", "test");
    addChild(tOperationNode, "File2", "test");

    std::vector<std::string> tKeys = {
        "File", 
        "File2"};
    std::vector<std::string> tValues = {
        "test", 
        "test"};
    PlatoTestXMLGenerator::test_children(tKeys,tValues, tOperationNode);

}


TEST(UnitTesterUtilities, TestChildrenABAXYZ)
{
    pugi::xml_document tDocument;
    auto tOperationNode = tDocument.append_child("Operation");
    addChild(tOperationNode, "File", "test");
    addChild(tOperationNode, "File2", "test2");
    addChild(tOperationNode, "File", "test3");

    std::vector<std::string> tKeys = {
        "File", 
        "File2",
        "File"};
    std::vector<std::string> tValues = {
        "test", 
        "test2",
        "test3"};
    PlatoTestXMLGenerator::test_children(tKeys,tValues, tOperationNode);
}

TEST(UnitTesterUtilities, TestChildrenExpectFailToPassABXX)
{
    //Keys are flipped from gold
    pugi::xml_document tDocument;
    auto tOperationNode = tDocument.append_child("Operation");
    addChild(tOperationNode, "File", "test");
    addChild(tOperationNode, "File2", "test");

    std::vector<std::string> tKeys = {
        "File2", 
        "File"};
    std::vector<std::string> tValues = {
        "test", 
        "test"};
   
    unsigned int fails=0;
    do {
        ::testing::TestPartResultArray gtest_failures;
        ::testing::ScopedFakeTestPartResultReporter gtest_reporter(
        ::testing::ScopedFakeTestPartResultReporter::
        INTERCEPT_ONLY_CURRENT_THREAD, &gtest_failures);
        PlatoTestXMLGenerator::test_children(tKeys,tValues, tOperationNode);
        fails += gtest_failures.size();
        std::cout<<gtest_failures.size()<<std::endl;
    } while (::testing::internal::AlwaysFalse());

    ASSERT_GT(fails,0);
}

TEST(UnitTesterUtilities, TestChildrenExpectFailToPassAAXY)
{
    //Values are flipped from gold
    pugi::xml_document tDocument;
    auto tOperationNode = tDocument.append_child("Operation");
    addChild(tOperationNode, "File", "test2");
    addChild(tOperationNode, "File", "test");

    std::vector<std::string> tKeys = {
        "File", 
        "File"};
    std::vector<std::string> tValues = {
        "test", 
        "test2"};
       
    unsigned int fails=0;
    do {
        ::testing::TestPartResultArray gtest_failures;
        ::testing::ScopedFakeTestPartResultReporter gtest_reporter(
        ::testing::ScopedFakeTestPartResultReporter::
        INTERCEPT_ONLY_CURRENT_THREAD, &gtest_failures);
        PlatoTestXMLGenerator::test_children(tKeys,tValues, tOperationNode);
        fails += gtest_failures.size();
        std::cout<<gtest_failures.size()<<std::endl;
    } while (::testing::internal::AlwaysFalse());

    ASSERT_GT(fails,0);
}

TEST(UnitTesterUtilities, TestChildrenExpectFailToPassABAXYY)
{
    //Duplicate parent, different children
    pugi::xml_document tDocument;
    auto tOperationNode = tDocument.append_child("Operation");
    addChild(tOperationNode, "File", "test");
    addChild(tOperationNode, "File2", "test2");
    addChild(tOperationNode, "File", "test");

    std::vector<std::string> tKeys = {
        "File", 
        "File2",
        "File"};
    std::vector<std::string> tValues = {
        "test", 
        "test2",
        "test3"};
       
    unsigned int fails=0;
    do {
        ::testing::TestPartResultArray gtest_failures;
        ::testing::ScopedFakeTestPartResultReporter gtest_reporter(
        ::testing::ScopedFakeTestPartResultReporter::
        INTERCEPT_ONLY_CURRENT_THREAD, &gtest_failures);
        PlatoTestXMLGenerator::test_children(tKeys,tValues, tOperationNode);
        fails += gtest_failures.size();
        std::cout<<gtest_failures.size()<<std::endl;
    } while (::testing::internal::AlwaysFalse());

    ASSERT_GT(fails,0);
}


TEST(UnitTesterUtilities, TestChildrenExpectFailToPassABCXYX)
{
    //Unique parents, 1 duplicate child
    pugi::xml_document tDocument;
    auto tOperationNode = tDocument.append_child("Operation");
    addChild(tOperationNode, "File", "test");
    addChild(tOperationNode, "File2", "test2");
    addChild(tOperationNode, "File3", "test3");

    std::vector<std::string> tKeys = {
        "File", 
        "File2",
        "File3"};
    std::vector<std::string> tValues = {
        "test", 
        "test2",
        "test"};
     
    unsigned int fails=0;
    do {
        ::testing::TestPartResultArray gtest_failures;
        ::testing::ScopedFakeTestPartResultReporter gtest_reporter(
        ::testing::ScopedFakeTestPartResultReporter::
        INTERCEPT_ONLY_CURRENT_THREAD, &gtest_failures);
        PlatoTestXMLGenerator::test_children(tKeys,tValues, tOperationNode);
        fails += gtest_failures.size();
        std::cout<<gtest_failures.size()<<std::endl;
    } while (::testing::internal::AlwaysFalse());

    ASSERT_GT(fails,0);
}


TEST(UnitTesterUtilities, TestChildrenExpectFailToPassTooManyKeyValues)
{
    //Unique parents, 1 duplicate child
    pugi::xml_document tDocument;
    auto tOperationNode = tDocument.append_child("Operation");
    addChild(tOperationNode, "File", "test");
    addChild(tOperationNode, "File2", "test2");
    

    std::vector<std::string> tKeys = {
        "File", 
        "File2",
        "File3"};
    std::vector<std::string> tValues = {
        "test", 
        "test2",
        "test3"};
     
    unsigned int fails=0;
    do {
        ::testing::TestPartResultArray gtest_failures;
        ::testing::ScopedFakeTestPartResultReporter gtest_reporter(
        ::testing::ScopedFakeTestPartResultReporter::
        INTERCEPT_ONLY_CURRENT_THREAD, &gtest_failures);
        PlatoTestXMLGenerator::test_children(tKeys,tValues, tOperationNode);
        fails += gtest_failures.size();
        std::cout<<gtest_failures.size()<<std::endl;
    } while (::testing::internal::AlwaysFalse());

    ASSERT_GT(fails,0);
}


TEST(UnitTesterUtilities, TestChildrenExpectFailToPassTooFewKeyValues)
{
    //Unique parents, 1 duplicate child
    pugi::xml_document tDocument;
    auto tOperationNode = tDocument.append_child("Operation");
    addChild(tOperationNode, "File", "test");
    addChild(tOperationNode, "File2", "test2");
    
    std::vector<std::string> tKeys = {
        "File"};
    std::vector<std::string> tValues = {
        "test"};
     
    unsigned int fails=0;
    do {
        ::testing::TestPartResultArray gtest_failures;
        ::testing::ScopedFakeTestPartResultReporter gtest_reporter(
        ::testing::ScopedFakeTestPartResultReporter::
        INTERCEPT_ONLY_CURRENT_THREAD, &gtest_failures);
        PlatoTestXMLGenerator::test_children(tKeys,tValues, tOperationNode);
        fails += gtest_failures.size();
        std::cout<<gtest_failures.size()<<std::endl;
    } while (::testing::internal::AlwaysFalse());

    ASSERT_GT(fails,0);
}


TEST(UnitTesterUtilities, TestChildrenExpectFailToPassEmptyNode)
{
    
    pugi::xml_document tDocument;
    auto tOperationNode = tDocument.append_child("Operation");
    
    std::vector<std::string> tKeys = {
        "File"};
    std::vector<std::string> tValues = {
        "test"};
     
    unsigned int fails=0;
    do {
        ::testing::TestPartResultArray gtest_failures;
        ::testing::ScopedFakeTestPartResultReporter gtest_reporter(
        ::testing::ScopedFakeTestPartResultReporter::
        INTERCEPT_ONLY_CURRENT_THREAD, &gtest_failures);
        PlatoTestXMLGenerator::test_children(tKeys,tValues, tOperationNode);
        fails += gtest_failures.size();
        std::cout<<gtest_failures.size()<<std::endl;
    } while (::testing::internal::AlwaysFalse());

    ASSERT_GT(fails,0);
}
/// test attributes
TEST(UnitTesterUtilities, TestAttributesABXY)
{
    pugi::xml_document tDocument;
    auto tForNode = tDocument.append_child("For");
    tForNode.append_attribute("var") = "E";
    tForNode.append_attribute("in") = "list";

    std::vector<std::string> tKeys = {
        "var", 
        "in"};
    std::vector<std::string> tValues = {
        "E", 
        "list"};
    PlatoTestXMLGenerator::test_attributes(tKeys,tValues, tForNode);
}

TEST(UnitTesterUtilities, TestAttributesAAXY)
{
    pugi::xml_document tDocument;
    auto tForNode = tDocument.append_child("For");
    tForNode.append_attribute("var") = "E";
    tForNode.append_attribute("var") = "list";

    std::vector<std::string> tKeys = {
        "var", 
        "var"};
    std::vector<std::string> tValues = {
        "E", 
        "list"};
    PlatoTestXMLGenerator::test_attributes(tKeys,tValues, tForNode);
}

TEST(UnitTesterUtilities, TestAttributesABXX)
{
    pugi::xml_document tDocument;
    auto tForNode = tDocument.append_child("For");
    tForNode.append_attribute("var") = "E";
    tForNode.append_attribute("in") = "E";

    std::vector<std::string> tKeys = {
        "var", 
        "in"};
    std::vector<std::string> tValues = {
        "E", 
        "E"};
    PlatoTestXMLGenerator::test_attributes(tKeys,tValues, tForNode);
}

TEST(UnitTesterUtilities, TestAttributesAAXX)
{
    pugi::xml_document tDocument;
    auto tForNode = tDocument.append_child("For");
    tForNode.append_attribute("in") = "E";
    tForNode.append_attribute("in") = "E";

    std::vector<std::string> tKeys = {
        "in", 
        "in"};
    std::vector<std::string> tValues = {
        "E", 
        "E"};
    PlatoTestXMLGenerator::test_attributes(tKeys,tValues, tForNode);
}

TEST(UnitTesterUtilities, TestAttributesABAXYZ)
{
    pugi::xml_document tDocument;
    auto tForNode = tDocument.append_child("For");
    tForNode.append_attribute("param") = "E";
    tForNode.append_attribute("in") = "list";
    tForNode.append_attribute("param") = "step";

    std::vector<std::string> tKeys = {
        "param", 
        "in",
        "param"};
    std::vector<std::string> tValues = {
        "E", 
        "list",
        "step"};
    PlatoTestXMLGenerator::test_attributes(tKeys,tValues, tForNode);
}

TEST(UnitTesterUtilities, TestAttributesExpectFailToPassABXY)
{
    //Flipped order of children
    pugi::xml_document tDocument;
    auto tForNode = tDocument.append_child("For");
    tForNode.append_attribute("var") = "list";
    tForNode.append_attribute("in") = "E";

    std::vector<std::string> tKeys = {
        "var", 
        "in"};
    std::vector<std::string> tValues = {
        "E", 
        "list"};

    unsigned int fails=0;
    do {
        ::testing::TestPartResultArray gtest_failures;
        ::testing::ScopedFakeTestPartResultReporter gtest_reporter(
        ::testing::ScopedFakeTestPartResultReporter::
        INTERCEPT_ONLY_CURRENT_THREAD, &gtest_failures);
        PlatoTestXMLGenerator::test_attributes(tKeys,tValues, tForNode);
        fails += gtest_failures.size();
        std::cout<<gtest_failures.size()<<std::endl;
    } while (::testing::internal::AlwaysFalse());

    ASSERT_GT(fails,0);

}

TEST(UnitTesterUtilities, TestAttributesExpectFailToPassAAXY)
{
    //Flipped order of values repeated keys
    pugi::xml_document tDocument;
    auto tForNode = tDocument.append_child("For");
    tForNode.append_attribute("in") = "list";
    tForNode.append_attribute("in") = "E";

    std::vector<std::string> tKeys = {
        "in", 
        "in"};
    std::vector<std::string> tValues = {
        "E", 
        "list"};

    unsigned int fails=0;
    do {
        ::testing::TestPartResultArray gtest_failures;
        ::testing::ScopedFakeTestPartResultReporter gtest_reporter(
        ::testing::ScopedFakeTestPartResultReporter::
        INTERCEPT_ONLY_CURRENT_THREAD, &gtest_failures);
        PlatoTestXMLGenerator::test_attributes(tKeys,tValues, tForNode);
        fails += gtest_failures.size();
        std::cout<<gtest_failures.size()<<std::endl;
    } while (::testing::internal::AlwaysFalse());

    ASSERT_GT(fails,0);

}

TEST(UnitTesterUtilities, TestAttributesExpectFailToPassABXX)
{
    //Flipped order of keys repeated values 
    pugi::xml_document tDocument;
    auto tForNode = tDocument.append_child("For");
    tForNode.append_attribute("param") = "E";
    tForNode.append_attribute("in") = "E";

    std::vector<std::string> tKeys = {
        "in", 
        "param"};
    std::vector<std::string> tValues = {
        "E", 
        "E"};

    unsigned int fails=0;
    do {
        ::testing::TestPartResultArray gtest_failures;
        ::testing::ScopedFakeTestPartResultReporter gtest_reporter(
        ::testing::ScopedFakeTestPartResultReporter::
        INTERCEPT_ONLY_CURRENT_THREAD, &gtest_failures);
        PlatoTestXMLGenerator::test_attributes(tKeys,tValues, tForNode);
        fails += gtest_failures.size();
        std::cout<<gtest_failures.size()<<std::endl;
    } while (::testing::internal::AlwaysFalse());

    ASSERT_GT(fails,0);

}

TEST(UnitTesterUtilities, TestAttributesExpectFailToPassABAXYZ)
{
    //Repeated key, one bad value
    pugi::xml_document tDocument;
    auto tForNode = tDocument.append_child("For");
    tForNode.append_attribute("param") = "X";
    tForNode.append_attribute("in") = "Y";
    tForNode.append_attribute("param") = "X";

    std::vector<std::string> tKeys = {
        "param",
        "in", 
        "param"};
    std::vector<std::string> tValues = {
        "X", 
        "Y",
        "Z"};

    unsigned int fails=0;
    do {
        ::testing::TestPartResultArray gtest_failures;
        ::testing::ScopedFakeTestPartResultReporter gtest_reporter(
        ::testing::ScopedFakeTestPartResultReporter::
        INTERCEPT_ONLY_CURRENT_THREAD, &gtest_failures);
        PlatoTestXMLGenerator::test_attributes(tKeys,tValues, tForNode);
        fails += gtest_failures.size();
        std::cout<<gtest_failures.size()<<std::endl;
    } while (::testing::internal::AlwaysFalse());

    ASSERT_GT(fails,0);

}

TEST(UnitTesterUtilities, TestAttributesExpectFailToPassNoAttributes)
{
    //Repeated key, one bad value
    pugi::xml_document tDocument;
    auto tForNode = tDocument.append_child("For");
    
    std::vector<std::string> tKeys = {
        "param",
        "in"};
    std::vector<std::string> tValues = {
        "X", 
        "Y"};

    unsigned int fails=0;
    do {
        ::testing::TestPartResultArray gtest_failures;
        ::testing::ScopedFakeTestPartResultReporter gtest_reporter(
        ::testing::ScopedFakeTestPartResultReporter::
        INTERCEPT_ONLY_CURRENT_THREAD, &gtest_failures);
        PlatoTestXMLGenerator::test_attributes(tKeys,tValues, tForNode);
        fails += gtest_failures.size();
        std::cout<<gtest_failures.size()<<std::endl;
    } while (::testing::internal::AlwaysFalse());

    ASSERT_GT(fails,0);

}


TEST(UnitTesterUtilities, TestAttributesExpectFailToPassTooManyAttributes)
{
    //Repeated key, one bad value
    pugi::xml_document tDocument;
    auto tForNode = tDocument.append_child("For");
    tForNode.append_attribute("param") = "X";
    tForNode.append_attribute("in") = "Y";
    tForNode.append_attribute("param") = "X";

    std::vector<std::string> tKeys = {
        "param",
        "in"};
    std::vector<std::string> tValues = {
        "X", 
        "Y"};

    unsigned int fails=0;
    do {
        ::testing::TestPartResultArray gtest_failures;
        ::testing::ScopedFakeTestPartResultReporter gtest_reporter(
        ::testing::ScopedFakeTestPartResultReporter::
        INTERCEPT_ONLY_CURRENT_THREAD, &gtest_failures);
        PlatoTestXMLGenerator::test_attributes(tKeys,tValues, tForNode);
        fails += gtest_failures.size();
        std::cout<<gtest_failures.size()<<std::endl;
    } while (::testing::internal::AlwaysFalse());

    ASSERT_GT(fails,0);

}

TEST(UnitTesterUtilities, TestAttributesExpectFailToPassTooManyGoldPairs)
{
    //Repeated key, one bad value
    pugi::xml_document tDocument;
    auto tForNode = tDocument.append_child("For");
    tForNode.append_attribute("param") = "X";
    tForNode.append_attribute("in") = "Y";

    std::vector<std::string> tKeys = {
        "param",
        "in", 
        "param"};
    std::vector<std::string> tValues = {
        "X", 
        "Y",
        "Z"};

    unsigned int fails=0;
    do {
        ::testing::TestPartResultArray gtest_failures;
        ::testing::ScopedFakeTestPartResultReporter gtest_reporter(
        ::testing::ScopedFakeTestPartResultReporter::
        INTERCEPT_ONLY_CURRENT_THREAD, &gtest_failures);
        PlatoTestXMLGenerator::test_attributes(tKeys,tValues, tForNode);
        fails += gtest_failures.size();
        std::cout<<gtest_failures.size()<<std::endl;
    } while (::testing::internal::AlwaysFalse());

    ASSERT_GT(fails,0);

}


}
  