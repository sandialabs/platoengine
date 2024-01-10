/*
 * Plato_Test_FreeFunctions.cpp
 *
 *  Created on: Jul 27, 2021
 */

#include <gtest/gtest.h>
#include "Plato_Macros.hpp"
#include "Plato_FreeFunctions.hpp"

TEST(PlatoTest, FreeFunctions_tokenize)
{
    // test 1
    auto tString = std::string("objective_value_1");
    auto tTokens = Plato::tokenize(tString);
    std::vector<std::string> tGold = { "objective", "value", "1" };
    for(auto& Token : tTokens)
    {
        auto tIndex = &Token - &tTokens[0];
        EXPECT_STREQ(tGold[tIndex].c_str(), Token.c_str());
    }

    // test 2
    tString = std::string("inequality_constraint_gradient_1");
    tTokens = Plato::tokenize(tString);
    tGold = { "inequality", "constraint", "gradient", "1" };
    for(auto& Token : tTokens)
    {
        auto tIndex = &Token - &tTokens[0];
        EXPECT_STREQ(tGold[tIndex].c_str(), Token.c_str());
    }

    // test 3
    tString = std::string("equality constraint gradient 1");
    tTokens = Plato::tokenize(tString, ' ');
    tGold = { "equality", "constraint", "gradient", "1" };
    for(auto& Token : tTokens)
    {
        auto tIndex = &Token - &tTokens[0];
        EXPECT_STREQ(tGold[tIndex].c_str(), Token.c_str());
    }
}

