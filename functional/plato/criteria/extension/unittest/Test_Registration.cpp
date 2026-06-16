#include <gtest/gtest.h>

#include "plato/criteria/extension/NodalSumObjective.hpp"
#include "plato/criteria/extension/VolumeCriterion.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/criteria/library/CriterionTraits.hpp"

namespace plato::criteria::extension::unittest
{

TEST(CriterionRegistration, NodalSum)
{
    EXPECT_TRUE(library::criterion_function_has_traits(
        library::builtin_criterion_registration_name(NodalSumObjective::mCriterionName),
        library::CriterionTraits{library::Parallelization::kSerial, library::FunctionDimension::kScalar}));

    EXPECT_TRUE(library::criterion_function_has_trait(
        library::builtin_criterion_registration_name(NodalSumObjective::mCriterionName),
        library::Parallelization::kSerial));
    EXPECT_TRUE(library::criterion_function_has_trait(
        library::builtin_criterion_registration_name(NodalSumObjective::mCriterionName),
        library::FunctionDimension::kScalar));

    EXPECT_FALSE(library::criterion_function_has_trait(
        library::builtin_criterion_registration_name(NodalSumObjective::mCriterionName),
        library::Parallelization::kParallel));
    EXPECT_FALSE(library::criterion_function_has_trait(
        library::builtin_criterion_registration_name(NodalSumObjective::mCriterionName),
        library::FunctionDimension::kVector));
}

TEST(CriterionRegistration, Volume)
{
    EXPECT_TRUE(library::criterion_function_has_traits(
        library::builtin_criterion_registration_name(VolumeCriterion::mVolumeCriterionName),
        library::CriterionTraits{library::Parallelization::kSerial, library::FunctionDimension::kScalar}));
}

TEST(CriterionRegistration, VolumeFraction)
{
    EXPECT_TRUE(library::criterion_function_has_traits(
        library::builtin_criterion_registration_name(VolumeCriterion::mVolumeFractionCriterionName),
        library::CriterionTraits{library::Parallelization::kSerial, library::FunctionDimension::kScalar}));
}

TEST(CriterionRegistration, BuiltinRegisterAppsList)
{
    const auto tRegisteredApps = library::registered_criteria_names();
    const auto tCriterionIsInRegisteredNames = [&tRegisteredApps](const std::string_view aCriterionName)
    {
        const auto tRegistrationName = library::builtin_criterion_registration_name(aCriterionName);
        return tRegisteredApps.count(tRegistrationName) == 1U;
    };

    EXPECT_TRUE(tCriterionIsInRegisteredNames(NodalSumObjective::mCriterionName));
    EXPECT_TRUE(tCriterionIsInRegisteredNames(VolumeCriterion::mVolumeCriterionName));
    EXPECT_TRUE(tCriterionIsInRegisteredNames(VolumeCriterion::mVolumeFractionCriterionName));
}

}  // namespace plato::criteria::extension::unittest
