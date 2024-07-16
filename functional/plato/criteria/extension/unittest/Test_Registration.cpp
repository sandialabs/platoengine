#include <gtest/gtest.h>

#include "plato/core/Function.hpp"
#include "plato/criteria/extension/NodalSumObjective.hpp"
#include "plato/criteria/extension/VolumeCriterion.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::criteria::extension::unittest
{

TEST(CriterionRegistration, NodalSum)
{
    EXPECT_TRUE(library::is_criterion_function_registered(
        library::builtin_criterion_registration_name(NodalSumObjective::kCriterionName)));
}

TEST(CriterionRegistration, Volume)
{
    EXPECT_TRUE(library::is_criterion_function_registered(
        library::builtin_criterion_registration_name(VolumeCriterion::kVolumeCriterionName)));
}

TEST(CriterionRegistration, VolumeFraction)
{
    EXPECT_TRUE(library::is_criterion_function_registered(
        library::builtin_criterion_registration_name(VolumeCriterion::kVolumeFractionCriterionName)));
}

TEST(CriterionRegistration, BuiltinRegisterAppsList)
{
    const auto tRegisteredApps = library::registered_criteria_names();
    const auto tCriterionIsInRegisteredNames = [&tRegisteredApps](const std::string_view tCriterionName)
    {
        const auto tRegistrationName = library::builtin_criterion_registration_name(tCriterionName);
        return tRegisteredApps.count(tRegistrationName) == 1u;
    };

    EXPECT_TRUE(tCriterionIsInRegisteredNames(VolumeCriterion::kVolumeCriterionName));
    EXPECT_TRUE(tCriterionIsInRegisteredNames(VolumeCriterion::kVolumeCriterionName));
    EXPECT_TRUE(tCriterionIsInRegisteredNames(VolumeCriterion::kVolumeFractionCriterionName));
}

}  // namespace plato::criteria::extension::unittest
