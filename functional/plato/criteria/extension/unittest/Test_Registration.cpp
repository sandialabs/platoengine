#include <gtest/gtest.h>

#include "plato/core/Function.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::criteria::extension::unittest
{

TEST(CriterionRegistration, NodalSum)
{
    const std::string tNodalSumName =
        input_parser::kCodeOptionsTable.toString(input_parser::CodeOptions::kNodalSum).value();
    EXPECT_TRUE(library::is_criterion_function_registered(tNodalSumName));
}

TEST(CriterionRegistration, Volume)
{
    const std::string tVolumeName =
        input_parser::kCodeOptionsTable.toString(input_parser::CodeOptions::kVolume).value();
    EXPECT_TRUE(library::is_criterion_function_registered(tVolumeName));
}

TEST(CriterionRegistration, VolumeFraction)
{
    const std::string tVolumeFractionName =
        input_parser::kCodeOptionsTable.toString(input_parser::CodeOptions::kVolumeFraction).value();
    EXPECT_TRUE(library::is_criterion_function_registered(tVolumeFractionName));
}

TEST(CriterionRegistration, CustomApp)
{
    const std::string tCustomAppName =
        input_parser::kCodeOptionsTable.toString(input_parser::CodeOptions::kCustomApp).value();
    EXPECT_TRUE(plato::criteria::library::is_criterion_function_registered(tCustomAppName));
}

TEST(CriterionRegistration, ParallelCustomApp)
{
    const std::string tCustomAppName =
        input_parser::kCodeOptionsTable.toString(input_parser::CodeOptions::kCustomApp).value();
    EXPECT_TRUE(plato::criteria::library::is_parallel_criterion_function_registered(tCustomAppName));
}

}  // namespace plato::criteria::extension::unittest
