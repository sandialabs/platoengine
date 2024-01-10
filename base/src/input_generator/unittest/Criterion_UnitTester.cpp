 /*
 * Criterion_UnitTester.cpp
 *
 *  Created on: July 6, 2022
 */
#include <gtest/gtest.h>

#include "XMLGeneratorCriterionMetadata.hpp"

namespace PlatoTestXMLGenerator
{

TEST(CriterionTest, SetGetSearchNodesets)
{
    XMLGen::Criterion tCriterion;
    std::vector<std::string> tInputNodesetIDs = {"1", "3", "24", "44"};
    tCriterion.setSearchNodesetIDs(tInputNodesetIDs);
    std::vector<std::string> tOutputNodesetIDs = tCriterion.searchNodesetIDs();
    for(size_t i=0; i<tInputNodesetIDs.size(); ++i)
    {
        ASSERT_TRUE(tInputNodesetIDs[i] == tOutputNodesetIDs[i]);
    } 
}

} 
