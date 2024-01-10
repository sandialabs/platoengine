 /*
 * FileObject_UnitTester.cpp
 *
 *  Created on: April 12, 2022
 */
#include <gtest/gtest.h>

#include "pugixml.hpp"
#include "XMLGenerator_UnitTester_Tools.hpp"
#include "FileObject.hpp"
#include "XMLGeneratorUtilities.hpp"

namespace PlatoTestXMLGenerator
{

TEST(FileObjectTest, FileObjectFunctionsNoConcurrency)
{
    director::FileObject tFileObject("name");
    ASSERT_NO_THROW(tFileObject.name());
    ASSERT_STREQ("name", tFileObject.name().c_str());
    ASSERT_STREQ("name", tFileObject.name("").c_str());
    ASSERT_STREQ("name", tFileObject.name("1").c_str());

    ASSERT_EQ(0,tFileObject.evaluations());

    ASSERT_STREQ("", tFileObject.tag().c_str());
    ASSERT_STREQ("", tFileObject.tag("").c_str());
    ASSERT_STREQ("", tFileObject.tag("1").c_str());

    pugi::xml_document tDocument;
    auto tForNode = tFileObject.forNode(tDocument,std::string("Parameters"));
    XMLGen::addChild(tForNode, "File", "test");

    auto tempForNode = tDocument.child("For");
    ASSERT_TRUE(tempForNode.empty());

    PlatoTestXMLGenerator::test_children({"File"}, {"test"}, tDocument);
}

TEST(FileObjectTest, FileObjectFunctionsWithConcurrency)
{
    director::FileObject tFileObject("name",3);
    ASSERT_NO_THROW(tFileObject.name());
    ASSERT_STREQ("name_{E}", tFileObject.name().c_str());
    ASSERT_STREQ("name_{E}", tFileObject.name("").c_str());
    ASSERT_STREQ("name_1", tFileObject.name("1").c_str());
    ASSERT_EQ(3,tFileObject.evaluations());

    ASSERT_STREQ("{E}", tFileObject.tag().c_str());
    ASSERT_STREQ("{E}", tFileObject.tag("").c_str());
    ASSERT_STREQ("1", tFileObject.tag("1").c_str());
    ASSERT_STREQ("I", tFileObject.tag("I").c_str());
    ASSERT_STREQ("{I}", tFileObject.tag("{I}").c_str());

    pugi::xml_document tDocument;
    auto tForNode = tFileObject.forNode(tDocument,std::string("Parameters"));
    XMLGen::addChild(tForNode, "File", "test");

    auto tOperation = tDocument.child("Operation");
    ASSERT_TRUE(tOperation.empty());
    auto tempForNode = tDocument.child("For");
    ASSERT_FALSE(tempForNode.empty());

    ASSERT_STREQ("For", tempForNode.name());
    auto tFileNode = tempForNode.child("File");
    ASSERT_FALSE(tFileNode.empty());
    ASSERT_STREQ("File", tFileNode.name());
    
    tempForNode = tempForNode.next_sibling("For");
    ASSERT_TRUE(tempForNode.empty());
}

}