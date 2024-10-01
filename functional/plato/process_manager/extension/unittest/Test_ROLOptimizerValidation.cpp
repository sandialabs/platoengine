#include <gtest/gtest.h>

#include <cmath>
#include <functional>

#include "plato/core/ValidationRegistration.hpp"
#include "plato/process_manager/extension/ROLOptimization.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::process_manager::extension::unittest
{
TEST(ROLOptimizerValidation, ValidateMaxIterations)
{
    input_parser::rol_optimization tOptimizationParameters;
    EXPECT_TRUE(detail::validate_rol_max_iterations(tOptimizationParameters).has_value());
    tOptimizationParameters.input_file_name = input_parser::FileName{"filler"};
    EXPECT_FALSE(detail::validate_rol_max_iterations(tOptimizationParameters).has_value());

    tOptimizationParameters.max_iterations = 0;
    EXPECT_TRUE(detail::validate_rol_max_iterations(tOptimizationParameters).has_value());

    tOptimizationParameters.input_file_name = boost::none;
    EXPECT_TRUE(detail::validate_rol_max_iterations(tOptimizationParameters).has_value());

    tOptimizationParameters.max_iterations = 1.0;
    EXPECT_FALSE(detail::validate_rol_max_iterations(tOptimizationParameters).has_value());
    tOptimizationParameters.max_iterations = 100.0;
    EXPECT_FALSE(detail::validate_rol_max_iterations(tOptimizationParameters).has_value());
}

TEST(ROLOptimizerValidation, ValidateStepTolerance)
{
    input_parser::rol_optimization tOptimizationParameters;
    EXPECT_TRUE(detail::validate_step_tolerance(tOptimizationParameters).has_value());  // Empty
    tOptimizationParameters.input_file_name = input_parser::FileName{"filler"};
    EXPECT_FALSE(
        detail::validate_step_tolerance(tOptimizationParameters).has_value());  // external file trumps missing others

    tOptimizationParameters.step_tolerance = -1.0;
    EXPECT_TRUE(detail::validate_step_tolerance(tOptimizationParameters)
                    .has_value());  // external file will be overwritten by bad entry
    tOptimizationParameters.input_file_name = boost::none;
    EXPECT_TRUE(detail::validate_step_tolerance(tOptimizationParameters).has_value());  // bad entry

    tOptimizationParameters.step_tolerance = std::nextafter(0.0, 1.0);
    EXPECT_FALSE(detail::validate_step_tolerance(tOptimizationParameters).has_value());  // good entry
    tOptimizationParameters.step_tolerance = 1e-8;
    EXPECT_FALSE(detail::validate_step_tolerance(tOptimizationParameters).has_value());
}

TEST(ROLOptimizerValidation, ValidateGradientTolerance)
{
    input_parser::rol_optimization tOptimizationParameters;
    EXPECT_TRUE(detail::validate_gradient_tolerance(tOptimizationParameters).has_value());
    tOptimizationParameters.input_file_name = input_parser::FileName{"filler"};
    EXPECT_FALSE(detail::validate_gradient_tolerance(tOptimizationParameters).has_value());

    tOptimizationParameters.gradient_tolerance = -1.0;
    EXPECT_TRUE(detail::validate_gradient_tolerance(tOptimizationParameters).has_value());

    tOptimizationParameters.input_file_name = boost::none;
    EXPECT_TRUE(detail::validate_gradient_tolerance(tOptimizationParameters).has_value());

    tOptimizationParameters.gradient_tolerance = std::nextafter(0.0, 1.0);
    EXPECT_FALSE(detail::validate_gradient_tolerance(tOptimizationParameters).has_value());
    tOptimizationParameters.gradient_tolerance = 1e-8;
    EXPECT_FALSE(detail::validate_gradient_tolerance(tOptimizationParameters).has_value());
}

TEST(ROLOptimizerValidation, ErrorMessagesValidOptimizationParameters)
{
    input_parser::rol_optimization tOptimizationParameters =
        plato::test_utilities::create_valid_example_rol_optimization();

    const auto tMessages = core::validate(tOptimizationParameters, std::vector<std::string>{});
    EXPECT_EQ(tMessages.size(), 0u);
}

TEST(ROLOptimizerValidation, ErrorMessagesInvalidOptimizationParameters)
{
    input_parser::rol_optimization tOptimizationParameters =
        plato::test_utilities::create_valid_example_rol_optimization();
    tOptimizationParameters.gradient_tolerance = -1;
    tOptimizationParameters.max_iterations = 0;
    tOptimizationParameters.step_tolerance = boost::none;

    std::vector<std::string> tMessages;
    tMessages = core::validate(tOptimizationParameters, std::move(tMessages));
    EXPECT_EQ(tMessages.size(), 3u);
}
}  // namespace plato::process_manager::extension::unittest
