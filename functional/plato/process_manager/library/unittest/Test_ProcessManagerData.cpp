#include <gtest/gtest.h>

#include <filesystem>
#include <format>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/geometry/library/GeometryFactory.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::process_manager::library::unittest
{
namespace
{
constexpr auto kConstantFunctionName = std::string_view{"return_constant_function"};

constexpr auto kConstantTestFunctionValue{42.0};

auto make_constant_test_function() -> criteria::library::CriterionFunction
{
    return core::make_function_with_first_derivative(
        [](const analysis::AnalysisDomainMesh& /*aMesh*/) { return kConstantTestFunctionValue; },
        [](const analysis::AnalysisDomainMesh&) { return linear_algebra::DynamicVector<double>{1.0}; });
}

using Registration = criteria::library::CriterionRegistration<criteria::library::Parallelization::kSerial,
                                                              criteria::library::FunctionDimension::kScalar>;

const auto kConfiguration = services::CriterionConfiguration{.mName = std::string{kConstantFunctionName},
                                                             .mIsParallelized = false,
                                                             .mIsScalar = true,
                                                             .mFunctionName = "",
                                                             .mVectorComponents = std::nullopt};

[[maybe_unused]] static auto kConstantFunctionRegistration = Registration{
    criteria::library::builtin_criterion_registration_name(kConstantFunctionName),
    [](const criteria::library::CriterionInput&)
    { return criteria::library::FunctionWithConfiguration{make_constant_test_function(), kConfiguration}; }};

std::string test_objective_string(const std::string& aName, const bool aIsActive, const bool aIsNormalized)
{
    return std::format(
        "begin objective {}\n"
        "   app platoengine\n"
        "   criterion {}\n"
        "   number_of_processors 1\n"
        "   aggregation_weight 1.0\n"
        "   active {}\n"
        "   normalize_by_initial_value {}\n"
        "end",
        aName, kConstantFunctionName, aIsActive, aIsNormalized);
}
}  // namespace

TEST(ProcessManagerData, ParsePlatoProblemEvaluateObjective)
{
    const std::string tInput = test_utilities::create_valid_brick_shape_geometry_string() +
                               test_utilities::create_valid_example_objective_string() +
                               test_utilities::create_valid_example_rol_optimization_string();

    const auto tData = input_validation::parse_and_validate_string(tInput);
    ASSERT_TRUE(tData.hasValue());
    const auto tProblem = make_process_manager_data(tData.value());
    const auto tGeometry =
        geometry::library::make_geometry_data(tData.value().get<components::ComponentType::kGeometry>());

    // Test Geometry
    const auto tBoundingBox = linear_algebra::DynamicVector{0.0, 0.0, 0.0, 1.0, 1.0, 1.0};
    const analysis::AnalysisDomainMesh tGeomProxy =
        tGeometry.mCompute.evaluate<core::evaluation::kFunction>(tBoundingBox);
    EXPECT_TRUE(std::filesystem::exists(tGeomProxy.mFileName));

    // Test Objective
    const auto tObjective = criteria::library::make_aggregate_objective_function(
        tData.value().get<components::ComponentType::kObjective>(), tGeomProxy);
    EXPECT_EQ(tObjective.evaluate<core::evaluation::kFunction>(tGeomProxy),
              tProblem.mObjective.evaluate<core::evaluation::kFunction>(tGeomProxy));
}

TEST(ProcessManagerData, ParseAndValidateInvalidInput)
{
    const auto tData = input_validation::parse_and_validate_string("");
    EXPECT_TRUE(tData.hasError());
}

TEST(ProcessManagerData, ParsePlatoProblemEvaluateObjectivesWithNormalization)
{
    const auto tObjectiveString = test_objective_string(/*aName=*/"test1", /*aIsActive=*/true, /*aIsNormalized=*/false);
    const auto tNormalizedObjectiveString =
        test_objective_string(/*aName=*/"test2", /*aIsActive=*/true, /*aIsNormalized=*/true);
    const auto tInactiveObjectiveString =
        test_objective_string(/*aName=*/"test3", /*aIsActive=*/false, /*aIsNormalized=*/false);

    const std::string tInput = test_utilities::create_valid_brick_shape_geometry_string() +
                               test_utilities::create_valid_example_rol_optimization_string() + tObjectiveString +
                               tNormalizedObjectiveString + tInactiveObjectiveString;

    const auto tData = input_validation::parse_and_validate_string(tInput);
    ASSERT_TRUE(tData.hasValue());
    const auto tProblem = make_process_manager_data(tData.value());
    const auto tGeometry =
        geometry::library::make_geometry_data(tData.value().get<components::ComponentType::kGeometry>());

    const analysis::AnalysisDomainMesh tGeomProxy =
        tGeometry.mCompute.evaluate<core::evaluation::kFunction>(tGeometry.mInitialGuess);
    EXPECT_TRUE(std::filesystem::exists(tGeomProxy.mFileName));

    constexpr auto tGoldValue{kConstantTestFunctionValue +
                              1.0};  // non-normalized objective value is kConstantTestFunctionValue, normalized is 1
    EXPECT_EQ(tProblem.mObjective.evaluate<core::evaluation::kFunction>(tGeomProxy), tGoldValue);
}

}  // namespace plato::process_manager::library::unittest
