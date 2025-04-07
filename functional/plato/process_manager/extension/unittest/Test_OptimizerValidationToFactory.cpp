#include <gtest/gtest.h>

#include "plato/input_parser/InputParser.hpp"
#include "plato/process_manager/extension/ROLUtilities.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"

namespace plato::process_manager::extension::unittest
{
namespace
{
auto rol_parameter_list(const library::ValidatedInput& aData)
    -> std::pair<third_party_integration::rol::OptimizationParameters, input_parser::rol_optimization>
{
    const auto tProcessManagerData = aData.processManagers();
    EXPECT_EQ(tProcessManagerData.rawInput().size(), 1);
    using ValidatedOptimizationParameters = core::ValidatedInputTypeWrapper<input_parser::rol_optimization>;
    EXPECT_TRUE(std::holds_alternative<ValidatedOptimizationParameters>(tProcessManagerData.rawInput().front()));
    const auto& tOptimizationParameters =
        std::get<ValidatedOptimizationParameters>(tProcessManagerData.rawInput().front());
    return {make_optimization_parameters(tOptimizationParameters), tOptimizationParameters.rawInput()};
}
}  // namespace

TEST(OptimizerFactory, ParlistGenerationFromInput)
{
    const std::string tOutputFileName = "output.xml";
    const std::string tInput = plato::test_utilities::create_valid_brick_shape_geometry_string() +
                               plato::test_utilities::create_valid_example_objective_string() +
                               R"(
                                  begin rol_optimization
                                    step_tolerance 10
                                    gradient_tolerance 100.0
                                    max_iterations 10
                                    approximate_hessian true
                                    verbose_output true
                                    initial_search_radius 2
                                    export_settings_file_name )" +
                               tOutputFileName + "\nend";

    const library::ValidatedInput tData{library::make_validated_input(input_parser::parse_input(tInput))};
    const auto [tParameters, tOptimizationParameters] = rol_parameter_list(tData);
    const auto tParlist = tParameters.parameters();
    EXPECT_EQ(tParlist.sublist("Status Test").get<int>("Iteration Limit"),
              tOptimizationParameters.max_iterations.value());
    EXPECT_EQ(tParlist.sublist("Status Test").get<double>("Gradient Tolerance"),
              tOptimizationParameters.gradient_tolerance.value());
    EXPECT_EQ(tParlist.sublist("Status Test").get<double>("Step Tolerance"),
              tOptimizationParameters.step_tolerance.value());
    EXPECT_EQ(tParlist.sublist("Step").sublist("Trust Region").get<double>("Initial Radius"),
              tOptimizationParameters.initial_search_radius.value());
    EXPECT_EQ(tParlist.sublist("General").get<bool>("Inexact Hessian-Times-A-Vector"),
              tOptimizationParameters.approximate_hessian.value());
    constexpr int tVerboseExtreme = 4;
    EXPECT_EQ(tParlist.sublist("General").get<int>("Output Level"), tVerboseExtreme);

    tParameters.writeParameters(tOptimizationParameters.export_settings_file_name.value().mToken);
    test_utilities::test_for_existence_and_remove({tOutputFileName}, TEST_CONTEXT("Exported rol inputs file"));
}

TEST(OptimizerFactory, ParlistGenerationFromFile)
{
    const std::string kFileName = "test.xml";

    ROL::ParameterList tParameterListOnDisk("ROL");
    tParameterListOnDisk.sublist("Status Test").set<int>("Iteration Limit", 42);
    tParameterListOnDisk.sublist("Status Test").set<double>("Gradient Tolerance", 0.5);
    tParameterListOnDisk.sublist("Status Test").set<double>("Step Tolerance", 0.25);
    Teuchos::writeParameterListToXmlFile(tParameterListOnDisk, kFileName);

    const std::string tInput = plato::test_utilities::create_valid_brick_shape_geometry_string() +
                               plato::test_utilities::create_valid_example_objective_string() +
                               "begin rol_optimization"
                               " input_file_name" +
                               kFileName + " step_tolerance 10" + " end";

    const library::ValidatedInput tData{library::make_validated_input(input_parser::parse_input(tInput))};
    const auto [tParameters, tOptimizationParameters] = rol_parameter_list(tData);
    const auto tParameterListFromFactory = tParameters.parameters();
    EXPECT_EQ(tParameterListFromFactory.sublist("Status Test").get<int>("Iteration Limit"),
              tParameterListOnDisk.sublist("Status Test").get<int>("Iteration Limit"));
    EXPECT_EQ(tParameterListFromFactory.sublist("Status Test").get<double>("Gradient Tolerance"),
              tParameterListOnDisk.sublist("Status Test").get<double>("Gradient Tolerance"));
    EXPECT_EQ(tParameterListFromFactory.sublist("Status Test").get<double>("Step Tolerance"), 10.0);

    std::filesystem::remove(kFileName);
}

}  // namespace plato::process_manager::extension::unittest
