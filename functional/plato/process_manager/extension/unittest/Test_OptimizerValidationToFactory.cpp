#include <gtest/gtest.h>

#include "plato/input_parser/InputParser.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/third_party_integration/rol/OptimizerFactory.hpp"

namespace plato::process_manager::extension::unittest
{
namespace
{
auto rol_parameter_list(const library::ValidatedInput& aData)
    -> std::pair<ROL::ParameterList, input_parser::rol_optimization>
{
    const auto tProcessManagerData = aData.processManagers();
    EXPECT_EQ(tProcessManagerData.rawInput().size(), 1);
    using ValidatedOptimizationParameters = core::ValidatedInputTypeWrapper<input_parser::rol_optimization>;
    EXPECT_TRUE(std::holds_alternative<ValidatedOptimizationParameters>(tProcessManagerData.rawInput().front()));
    const auto& tOptimizationParameters =
        std::get<ValidatedOptimizationParameters>(tProcessManagerData.rawInput().front());
    return {plato::third_party_integration::rol::rol_parameter_list(tOptimizationParameters),
            tOptimizationParameters.rawInput()};
}

}  // namespace
TEST(OptimizerFactory, ParlistGenerationFromInput)
{
    const std::string tInput = plato::test_utilities::create_valid_brick_shape_geometry_string() +
                               plato::test_utilities::create_valid_example_objective_string() +
                               R"(
                                  begin rol_optimization
                                    step_tolerance 10
                                    gradient_tolerance 100.0
                                    max_iterations 10
                                  end
                              )";

    const library::ValidatedInput tData{library::make_validated_input(input_parser::parse_input(tInput))};
    const auto [tParlist, tOptimizationParameters] = rol_parameter_list(tData);
    EXPECT_EQ(tParlist.sublist("Status Test").get<int>("Iteration Limit"),
              tOptimizationParameters.max_iterations.value());
    EXPECT_EQ(tParlist.sublist("Status Test").get<double>("Gradient Tolerance"),
              tOptimizationParameters.gradient_tolerance.value());
    EXPECT_EQ(tParlist.sublist("Status Test").get<double>("Step Tolerance"),
              tOptimizationParameters.step_tolerance.value());
}

TEST(OptimizerFactory, ParlistGenerationFromFile)
{
    const std::string kFileName = "test.xml";

    ROL::ParameterList tParameterListToWrite;
    tParameterListToWrite.sublist("Status Test").set<int>("Iteration Limit", 42);
    tParameterListToWrite.sublist("Status Test").set<double>("Gradient Tolerance", 0.5);
    tParameterListToWrite.sublist("Status Test").set<double>("Step Tolerance", 0.25);
    Teuchos::writeParameterListToXmlFile(tParameterListToWrite, kFileName);

    const std::string tInput = plato::test_utilities::create_valid_brick_shape_geometry_string() +
                               plato::test_utilities::create_valid_example_objective_string() +
                               "begin rol_optimization"
                               " input_file_name" +
                               kFileName + " step_tolerance 10" + " end";

    const library::ValidatedInput tData{library::make_validated_input(input_parser::parse_input(tInput))};
    const auto [tParameterListFromDisk, tOptimizationParameters] = rol_parameter_list(tData);
    EXPECT_EQ(tParameterListFromDisk.sublist("Status Test").get<int>("Iteration Limit"),
              tParameterListToWrite.sublist("Status Test").get<int>("Iteration Limit"));
    EXPECT_EQ(tParameterListFromDisk.sublist("Status Test").get<double>("Gradient Tolerance"),
              tParameterListToWrite.sublist("Status Test").get<double>("Gradient Tolerance"));
    EXPECT_EQ(tParameterListFromDisk.sublist("Status Test").get<double>("Step Tolerance"),
              tParameterListToWrite.sublist("Status Test").get<double>("Step Tolerance"));

    std::filesystem::remove(kFileName);
}

}  // namespace plato::process_manager::extension::unittest
