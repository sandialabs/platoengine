 /*
 * Performer_UnitTester.cpp
 *
 *  Created on: April 12, 2022
 */
#include <gtest/gtest.h>

#include "pugixml.hpp"
#include "XMLGenerator_UnitTester_Tools.hpp"
#include "Performer.hpp"

namespace PlatoTestXMLGenerator
{

TEST(PerformerTest, WritePerformerNoConcurrencyNoOffset)
{
    director::Performer tPerformer("platomain_1", "platomain");

    ASSERT_EQ(1, tPerformer.numberRanks());
    ASSERT_EQ("0" , tPerformer.ID(""));
    ASSERT_EQ("0" , tPerformer.ID("13"));
    ASSERT_EQ("0" , tPerformer.ID("potato"));

    ASSERT_EQ(0, tPerformer.evaluations());

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tPerformer.write_interface(tDocument));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tPerformerNode = tDocument.child("Performer");
    ASSERT_FALSE(tPerformerNode.empty());
    ASSERT_STREQ("Performer", tPerformerNode.name());
    std::vector<std::string> tKeys = {
        "PerformerID",
        "Name", 
        "Code"};
    
    std::vector<std::string> tValues = {
        "0", 
        "platomain_1", 
        "platomain"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tPerformerNode);

    tPerformerNode = tPerformerNode.next_sibling("Performer");
    ASSERT_TRUE(tPerformerNode.empty());
}

TEST(PerformerTest, WritePerformerNoConcurrencyWithOffset)
{
    int tNumRanks = 1;
    int tOffset = 13;
    int tConcurrentEvaluations = 0;
    director::Performer tPerformer("name", "code", tOffset, tNumRanks, tConcurrentEvaluations);

    ASSERT_EQ(1, tPerformer.numberRanks());
    ASSERT_EQ("13" , tPerformer.ID(""));
    ASSERT_EQ("13" , tPerformer.ID("3"));
    ASSERT_EQ("13" , tPerformer.ID("potato"));

    ASSERT_EQ(0, tPerformer.evaluations());

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tPerformer.write_interface(tDocument));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tPerformerNode = tDocument.child("Performer");
    ASSERT_FALSE(tPerformerNode.empty());
    ASSERT_STREQ("Performer", tPerformerNode.name());
    std::vector<std::string> tKeys = {
        "PerformerID",
        "Name", 
        "Code"};
    
    std::vector<std::string> tValues = {
        "13", 
        "name", 
        "code"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tPerformerNode);

    tPerformerNode = tPerformerNode.next_sibling("Performer");
    ASSERT_TRUE(tPerformerNode.empty());
}

TEST(PerformerTest, WritePerformerWithConcurrencyNoOffset)
{
    int tNumRanks = 1;
    int tOffset = 0;
    int tConcurrentEvaluations = 2;
    director::Performer tPerformer("name", "code", tOffset, tNumRanks, tConcurrentEvaluations);

    ASSERT_EQ(tNumRanks, tPerformer.numberRanks());
    ASSERT_EQ("{E+0}" , tPerformer.ID(""));
    ASSERT_EQ("3" , tPerformer.ID("3"));

    ASSERT_EQ(tConcurrentEvaluations, tPerformer.evaluations());

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tPerformer.write_interface(tDocument,""));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tPerformerNode = tDocument.child("Performer");
    ASSERT_FALSE(tPerformerNode.empty());
    ASSERT_STREQ("Performer", tPerformerNode.name());
    std::vector<std::string> tKeys = {
        "PerformerID",
        "Name", 
        "Code"};
    
    std::vector<std::string> tValues = {
        "{E+0}", 
        "name_{E}", 
        "code"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tPerformerNode);

    tPerformerNode = tPerformerNode.next_sibling("Performer");
    ASSERT_TRUE(tPerformerNode.empty());
}

TEST(PerformerTest, WritePerformerWithConcurrencyAndOffset)
{
    int tNumRanks = 1;
    int tOffset = 1;
    int tConcurrentEvaluations = 3;
    director::Performer tPerformer("name", "code", tOffset, tNumRanks, tConcurrentEvaluations);

    ASSERT_EQ(tNumRanks , tPerformer.numberRanks());
    ASSERT_EQ("{E+1}" , tPerformer.ID(""));
    ASSERT_EQ("1" , tPerformer.ID("0"));

    ASSERT_EQ(tConcurrentEvaluations, tPerformer.evaluations());

    pugi::xml_document tDocument;
    ASSERT_NO_THROW(tPerformer.write_interface(tDocument,"2"));
    ASSERT_FALSE(tDocument.empty());

    // TEST RESULTS AGAINST GOLD VALUES
    auto tPerformerNode = tDocument.child("Performer");
    ASSERT_FALSE(tPerformerNode.empty());
    ASSERT_STREQ("Performer", tPerformerNode.name());
    std::vector<std::string> tKeys = {
        "PerformerID",
        "Name", 
        "Code"};
    
    std::vector<std::string> tValues = {
        "3", 
        "name_2", 
        "code"};
    PlatoTestXMLGenerator::test_children(tKeys, tValues, tPerformerNode);

    tPerformerNode = tPerformerNode.next_sibling("Performer");
    ASSERT_TRUE(tPerformerNode.empty());
}


TEST(PerformerTest, PerformerMpiRunLine)
{
    int tNumRanks = 16;
    int tOffset = 1;
    int tConcurrentEvaluations = 3;
    director::Performer tPerformer("name", "code", tOffset, tNumRanks, tConcurrentEvaluations);

    ASSERT_EQ(": -np 16 -x PLATO_PERFORMER_ID=2 \\\n" , tPerformer.return_mpirun("1"));

    tNumRanks = 4;
    tOffset = 3;
    tConcurrentEvaluations = 0;
    director::Performer tPerformer2("name", "code", tOffset, tNumRanks, tConcurrentEvaluations);

    ASSERT_EQ(": -np 4 -x PLATO_PERFORMER_ID=3 \\\n" , tPerformer2.return_mpirun(""));

    tNumRanks = 1;
    tOffset = 3;
    tConcurrentEvaluations = 3;
    director::Performer tPerformer3("name", "code", tOffset, tNumRanks, tConcurrentEvaluations);

    ASSERT_EQ(": -np 1 -x PLATO_PERFORMER_ID=5 \\\n" , tPerformer3.return_mpirun("2"));

}


}