#include <gtest/gtest.h>

#include <boost/none.hpp>
#include <cmath>

#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/process_manager/extension/ROLOptimization.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::process_manager::extension::unittest
{
TEST(ROLOptimizerValidation, ValidateMaxIterations)
{
    auto tOptimizationParameters = input_parser::rol_optimization{};
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

namespace
{
template <typename ValidationFunction>
void test_optional_generic_tolerance(input_parser::rol_optimization& aOptimizationParameters,
                                     boost::optional<double>& aField,
                                     const ValidationFunction& aValidationFunction,
                                     const test_utilities::TestContext& aTestContext)
{
    aField = boost::none;
    EXPECT_FALSE(aValidationFunction(aOptimizationParameters).has_value()) << aTestContext;
    aField = -1.0;
    EXPECT_TRUE(aValidationFunction(aOptimizationParameters).has_value()) << aTestContext;
    aField = .0;
    EXPECT_TRUE(aValidationFunction(aOptimizationParameters).has_value()) << aTestContext;
    aField = std::nextafter(0.0, 1.0);
    EXPECT_FALSE(aValidationFunction(aOptimizationParameters).has_value()) << aTestContext;
    aField = 1e-8;
    EXPECT_FALSE(aValidationFunction(aOptimizationParameters).has_value()) << aTestContext;
}

}  // namespace

TEST(ROLOptimizerValidation, ValidateAverageGradientTolerance)
{
    auto tOptimizationParameters = input_parser::rol_optimization{};
    test_optional_generic_tolerance(
        tOptimizationParameters, tOptimizationParameters.gradient_tolerance,
        [](const auto aInput) { return detail::validate_gradient_tolerance(aInput); },
        TEST_CONTEXT("Gradient tolerance tests"));
}

TEST(ROLOptimizerValidation, ValidateAverageStepTolerance)
{
    auto tOptimizationParameters = input_parser::rol_optimization{};
    test_optional_generic_tolerance(
        tOptimizationParameters, tOptimizationParameters.step_tolerance,
        [](const auto aInput) { return detail::validate_step_tolerance(aInput); },
        TEST_CONTEXT("Step tolerance tests"));
}

TEST(ROLOptimizerValidation, ValidateInitialSearchRadius)
{
    auto tOptimizationParameters = input_parser::rol_optimization{};
    EXPECT_FALSE(detail::validate_initial_search_radius(tOptimizationParameters).has_value())
        << "Optional parameter absent is valid";

    tOptimizationParameters.initial_search_radius = -1.0;
    EXPECT_TRUE(detail::validate_initial_search_radius(tOptimizationParameters).has_value())
        << "Initial search radius cannot be negative";
    tOptimizationParameters.initial_search_radius = 0;
    EXPECT_TRUE(detail::validate_initial_search_radius(tOptimizationParameters).has_value())
        << "Initial search radius cannot be zero";

    tOptimizationParameters.initial_search_radius = 1;
    EXPECT_FALSE(detail::validate_initial_search_radius(tOptimizationParameters).has_value())
        << "Initial search radius valid";
}

TEST(ROLOptimizerValidation, ValidateUniqueOutputName)
{
    auto tOptimizationParameters = input_parser::rol_optimization{};
    EXPECT_FALSE(detail::validate_unique_output_name(tOptimizationParameters).has_value())
        << "Optional output file name absent is valid";

    const auto tFileName = input_parser::FileName{"junk.xml"};
    const auto tOtherFileName = input_parser::FileName{"not-junk.xml"};
    tOptimizationParameters.input_file_name = tFileName;
    tOptimizationParameters.export_settings_file_name = tFileName;
    EXPECT_TRUE(detail::validate_unique_output_name(tOptimizationParameters).has_value())
        << "Output file name cannot match input file name";

    tOptimizationParameters.input_file_name = tOtherFileName;
    EXPECT_FALSE(detail::validate_unique_output_name(tOptimizationParameters).has_value()) << "Output file is valid";

    tOptimizationParameters.input_file_name = boost::none;
    EXPECT_FALSE(detail::validate_unique_output_name(tOptimizationParameters).has_value()) << "Output file is valid";
}

TEST(ROLOptimizerValidation, ErrorMessagesValidOptimizationParameters)
{
    const auto tOptimizationParameters = create_valid_example_rol_optimization_input();

    const auto tMessages = input_validation::validate(tOptimizationParameters, std::vector<std::string>{});
    EXPECT_EQ(tMessages.size(), 0U);
}

TEST(ROLOptimizerValidation, ErrorMessagesInvalidOptimizationParameters)
{
    auto tOptimizationParameters = create_valid_example_rol_optimization_input();
    tOptimizationParameters.gradient_tolerance = -1;
    tOptimizationParameters.max_iterations = 0;
    tOptimizationParameters.step_tolerance = boost::none;

    const auto tMessages = input_validation::validate(input_parser::InputBlockWrapper{tOptimizationParameters}, {});
    EXPECT_EQ(tMessages.size(), 2U);
}
}  // namespace plato::process_manager::extension::unittest
