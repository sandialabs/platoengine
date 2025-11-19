#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/ConstraintValidation.hpp"
#include "plato/criteria/library/test_utilities/TestAppConfigurations.hpp"
#include "plato/services/AppConfiguration.hpp"
#include "plato/services/AppConfigurationUtilities.hpp"
#include "plato/test_utilities/TestDirectorySetupTeardown.hpp"

namespace plato::criteria::library::unittest
{
TEST(ConstraintValidation, ValidateConstraintValue)
{
    namespace pfcd = plato::criteria::library::detail;
    // Valid, only constraint_value is set
    {
        auto tConstraint = input_parser::constraint{};
        tConstraint.constraint_value = 1.0;  // has only 1 : valid
        EXPECT_FALSE(pfcd::validate_constraint_value(tConstraint).has_value());
    }
    // Valid, only constraint_value_list is set
    {
        auto tConstraint = input_parser::constraint{};
        tConstraint.constraint_value_list = ConstraintValueList{{input_parser::ComponentAndTarget{}}};
        EXPECT_FALSE(pfcd::validate_constraint_value(tConstraint).has_value());
    }
    // Invalid, no constraint targets
    {
        auto tConstraint = input_parser::constraint{};
        EXPECT_TRUE(pfcd::validate_constraint_value(tConstraint).has_value());
    }
    // Invalid, both constraint target types are set (constraint_value and constraint_value_list)
    {
        auto tConstraint = input_parser::constraint{};
        tConstraint.constraint_value = 1.0;
        tConstraint.constraint_value_list = ConstraintValueList{{input_parser::ComponentAndTarget{}}};
        EXPECT_TRUE(pfcd::validate_constraint_value(tConstraint).has_value());
    }
}

TEST(ConstraintValidation, ValidateConstraintType)
{
    namespace pfcd = plato::criteria::library::detail;

    auto tConstraint = input_parser::constraint{};
    EXPECT_TRUE(pfcd::validate_constraint_type(tConstraint).has_value());

    tConstraint.constraint_type = input_parser::ConstraintTypes::kEqualTo;
    EXPECT_FALSE(pfcd::validate_constraint_type(tConstraint).has_value());

    tConstraint.constraint_type = input_parser::ConstraintTypes::kLessThan;
    EXPECT_FALSE(pfcd::validate_constraint_type(tConstraint).has_value());

    tConstraint.constraint_type = input_parser::ConstraintTypes::kGreaterThan;
    EXPECT_FALSE(pfcd::validate_constraint_type(tConstraint).has_value());
}

TEST(ConstraintValidation, NumberOfProcessors)
{
    auto tConstraint = input_parser::constraint{};
    EXPECT_FALSE(detail::validate_constraint_number_of_processors(tConstraint).has_value());
    tConstraint.number_of_processors = 42u;  // Only one is valid
    EXPECT_TRUE(detail::validate_constraint_number_of_processors(tConstraint).has_value());
    tConstraint.number_of_processors = 1u;
    EXPECT_FALSE(detail::validate_constraint_number_of_processors(tConstraint).has_value());
}

TEST(ConstraintValidation, ConstraintComponentTargets)
{
    const auto [tTestDirectorySetupTeardown, tTestAppName] = test_utilities::test_configurations();
    const auto tAppConfigurations = services::app_configurations({tTestDirectorySetupTeardown.directory()});

    const auto tBaseConstraint = [mTestAppName = tTestAppName]()
    {
        auto tConstraint = input_parser::constraint{};
        tConstraint.app = input_parser::AppName{std::string{mTestAppName}};
        return tConstraint;
    }();
    const auto tConstraintList = ConstraintValueList{
        {input_parser::ComponentAndTarget{/*.component=*/input_parser::IdentifierString{"america"}, /*.target=*/42.0}}};
    // Scalar criterion, no components
    {
        auto tConstraint = tBaseConstraint;
        tConstraint.criterion = input_parser::CriterionName{"scalar"};
        const auto tErrorMessage = detail::validate_constraint_component_targets(tConstraint, tAppConfigurations);
        EXPECT_FALSE(tErrorMessage.has_value());
    }
    // Scalar criterion, with components specified
    {
        auto tConstraint = tBaseConstraint;
        tConstraint.criterion = input_parser::CriterionName{"scalar"};
        tConstraint.constraint_value_list = tConstraintList;

        const auto tErrorMessage = detail::validate_constraint_component_targets(tConstraint, tAppConfigurations);
        EXPECT_TRUE(tErrorMessage.has_value());
    }
    // Vector criterion but no components, with components specified
    {
        auto tConstraint = tBaseConstraint;
        tConstraint.criterion = input_parser::CriterionName{"vector-no-components"};
        tConstraint.constraint_value_list = tConstraintList;

        const auto tErrorMessage = detail::validate_constraint_component_targets(tConstraint, tAppConfigurations);
        EXPECT_TRUE(tErrorMessage.has_value());
    }
    // Vector criterion with components, and with components specified
    {
        auto tConstraint = tBaseConstraint;
        tConstraint.criterion = input_parser::CriterionName{"vector-with-components"};
        tConstraint.constraint_value_list = tConstraintList;

        const auto tErrorMessage = detail::validate_constraint_component_targets(tConstraint, tAppConfigurations);
        EXPECT_FALSE(tErrorMessage.has_value()) << tErrorMessage.value();
    }
    // Vector criterion with components, and with no components specified
    {
        auto tConstraint = tBaseConstraint;
        tConstraint.criterion = input_parser::CriterionName{"vector-with-components"};

        const auto tErrorMessage = detail::validate_constraint_component_targets(tConstraint, tAppConfigurations);
        EXPECT_TRUE(tErrorMessage.has_value());
    }
}

TEST(ConstraintValidation, ConstraintComponentNames)
{
    const auto [tTestDirectorySetupTeardown, tTestAppName] = test_utilities::test_configurations();
    const auto tAppConfigurations = services::app_configurations({tTestDirectorySetupTeardown.directory()});

    const auto tBaseConstraint = [mTestAppName = tTestAppName]()
    {
        auto tConstraint = input_parser::constraint{};
        tConstraint.app = input_parser::AppName{std::string{mTestAppName}};
        tConstraint.criterion = input_parser::CriterionName{"vector-with-components"};
        return tConstraint;
    }();

    // All components match
    {
        auto tConstraint = tBaseConstraint;
        tConstraint.constraint_value_list = ConstraintValueList{{input_parser::ComponentAndTarget{
            /*.component=*/input_parser::IdentifierString{"europe"}, /*.target=*/42.0}}};
        EXPECT_FALSE(detail::validate_constraint_component_names(tConstraint, tAppConfigurations).has_value());
    }
    // Misspelled component
    {
        auto tConstraint = tBaseConstraint;

        tConstraint.constraint_value_list = ConstraintValueList{{input_parser::ComponentAndTarget{
            /*.component=*/input_parser::IdentifierString{"murica"}, /*.target=*/42.0}}};
        EXPECT_TRUE(detail::validate_constraint_component_names(tConstraint, tAppConfigurations).has_value());
    }
}

}  // namespace plato::criteria::library::unittest
