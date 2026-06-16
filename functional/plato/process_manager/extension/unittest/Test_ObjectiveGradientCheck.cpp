#include <gtest/gtest.h>

#include <algorithm>
#include <boost/none.hpp>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>

#include "plato/components/ComponentType.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_parser/InputDefinitions.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/input_validation/ValidationRegistration.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/process_manager/extension/DesignVariableTypes.hpp"
#include "plato/process_manager/extension/DirectionVectorTypes.hpp"
#include "plato/process_manager/extension/ObjectiveGradientCheck.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/process_manager/library/ProcessManagerRegistration.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/test_utilities/ValidationTestUtilities.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::process_manager::extension::unittest
{
namespace
{
using ObjectiveGradientCheckFixture = third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;
[[nodiscard]] auto make_objective_gradient_check_input(const std::filesystem::path& aMeshFile,
                                                       const input_parser::DesignVariableTypes& aDesignVariableType,
                                                       const input_parser::DirectionVectorTypes& aDirectionVectorType)
    -> input_parser::objective_gradient_check
{
    constexpr auto tDefaultNumberOfSteps = 10U;
    constexpr auto tDefaultInitialDirectionMagnitude = 0.1;
    constexpr auto tDefaultStepSizeReduction = 0.1;
    constexpr auto tDefaultRandomSeed = 42U;
    return input_parser::objective_gradient_check{input_parser::FileName{aMeshFile},
                                                  input_parser::FileName{"output.txt"},
                                                  aDesignVariableType,
                                                  /*number_of_steps*/ tDefaultNumberOfSteps,
                                                  /*initial_direction_magnitude*/ tDefaultInitialDirectionMagnitude,
                                                  /*step_size_reduction_factor*/ tDefaultStepSizeReduction,
                                                  /*random_direction_seed*/ tDefaultRandomSeed,
                                                  aDirectionVectorType};
}

}  // namespace

TEST(ObjectiveGradientCheck, Registration)
{
    EXPECT_TRUE(library::is_process_manager_function_registered("objective_gradient_check"));
}

TEST(ObjectiveGradientCheck, DetailValidateOutputFileName)
{
    const auto tValidateOutputFileName = [](const auto& aInput) { return detail::validate_output_file_name(aInput); };
    auto tInput = input_parser::objective_gradient_check{};
    plato::test_utilities::expect_input_is_invalid(TEST_CONTEXT("No output file is an error"), tValidateOutputFileName,
                                                   tInput);
    tInput.output_file_name = input_parser::FileName{"dummy.out"};
    plato::test_utilities::expect_input_is_valid(TEST_CONTEXT("Output file specified is valid"),
                                                 tValidateOutputFileName, tInput);
}

TEST(ObjectiveGradientCheck, DetailValidateInputFileName)
{
    const auto tValidateInputMeshName = [](const auto& aInput) { return detail::validate_input_mesh_name(aInput); };
    auto tInput = input_parser::objective_gradient_check{};
    plato::test_utilities::expect_input_is_invalid(TEST_CONTEXT("No input file is an error"), tValidateInputMeshName,
                                                   tInput);
    const auto* const tFileName = "dummy.exo";
    tInput.input_mesh_name = input_parser::FileName{tFileName};
    plato::test_utilities::expect_input_is_invalid(
        TEST_CONTEXT("Output file specified but missing from disk is an error"), tValidateInputMeshName, tInput);
    auto tOutFile = std::ofstream(tFileName);
    plato::test_utilities::expect_input_is_valid(TEST_CONTEXT("Output file specified and on disk is valid."),
                                                 tValidateInputMeshName, tInput);
    std::filesystem::remove(tFileName);
}

TEST_F(ObjectiveGradientCheckFixture, Validation)
{
    const auto tInputBase = geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                            criteria::library::test_utilities::create_valid_example_objective_input();

    const auto tInput = tInputBase | input_parser::objective_gradient_check{};
    const auto tValidatedInput = input_validation::make_validated_input(tInput);
    EXPECT_TRUE(tValidatedInput.hasError());

    const auto tValidInput = tInputBase | make_objective_gradient_check_input(
                                              mMeshFilePath, input_parser::DesignVariableTypes::kElementDensity,
                                              input_parser::DirectionVectorTypes::kUniformNegative);
    const auto tValidatedValidInput = input_validation::make_validated_input(tValidInput);
    EXPECT_FALSE(tValidatedValidInput.hasError()) << tValidatedValidInput.error();
}

namespace
{

[[nodiscard]] auto validated_input(const input_parser::objective_gradient_check& aCriteriaGradientInput)
{
    const auto tInput = geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                        criteria::library::test_utilities::create_valid_example_objective_input() |
                        aCriteriaGradientInput;
    return input_validation::make_validated_input(tInput);
}

[[nodiscard]] auto validated_process_managers(const input_parser::objective_gradient_check& aCriteriaGradientInput)
{
    const auto tValidatedInput = validated_input(aCriteriaGradientInput);
    return tValidatedInput.value().get<components::ComponentType::kProcessManager>().rawInput().back();
}

void check_density_values(const std::vector<double>& aVector,
                          const std::size_t aGoldSize,
                          const plato::test_utilities::TestContext& aTestContext)
{
    ASSERT_EQ(aVector.size(), aGoldSize) << aTestContext;
    for (const auto& tDensity : aVector)
    {
        EXPECT_DOUBLE_EQ(tDensity, detail::default_starting_density()) << aTestContext;
    }
}

}  // namespace

TEST_F(ObjectiveGradientCheckFixture, DetailDesignVariablesOnCoordinates)
{
    const auto tOriginalCoordinates =
        detail::design_variables(mMeshFilePath, input_parser::DesignVariableTypes::kCoordinate);
    const auto tMeshCoordinates =
        third_party_integration::common::flatten(mesh::EntityRetrieval{mesh::Mesh{mMeshFilePath}}.nodalCoordinates());

    ASSERT_EQ(tMeshCoordinates.size(), tOriginalCoordinates.size());
    for (const auto& [tMeshOnDisk, tDesignVariables] : utilities::Zip(tMeshCoordinates, tOriginalCoordinates))
    {
        EXPECT_DOUBLE_EQ(tMeshOnDisk, tDesignVariables);
    }
}

TEST_F(ObjectiveGradientCheckFixture, DetailDesignVariablesOnElements)
{
    const auto tOriginalElementDensity =
        detail::design_variables(mMeshFilePath, input_parser::DesignVariableTypes::kElementDensity);
    const auto tNumberOfDesignDomainElements =
        mesh::EntityCounts{mesh::Mesh{mMeshFilePath}}.numberOfDesignDomainElements();
    check_density_values(tOriginalElementDensity, tNumberOfDesignDomainElements,
                         TEST_CONTEXT("Element centered densities."));
}

TEST_F(ObjectiveGradientCheckFixture, DetailDesignVariablesOnNodes)
{
    const auto tNodalDensity =
        detail::design_variables(mMeshFilePath, input_parser::DesignVariableTypes::kNodalDensity);
    const auto tNumberOfDesignDomainNodes = mesh::EntityCounts{mesh::Mesh{mMeshFilePath}}.numberOfDesignDomainNodes();
    check_density_values(tNodalDensity, tNumberOfDesignDomainNodes, TEST_CONTEXT("Node centered densities."));
}

TEST(ObjectiveGradientCheck, DetailPerturbation)
{
    constexpr auto tSize = std::size_t{5};
    const auto tSeed = detail::Seed{1U};
    {
        const auto tPerturbation = detail::perturbation(input_parser::DirectionVectorTypes::kRandom, tSize, tSeed);
        EXPECT_EQ(tPerturbation.size(), tSize) << "Random vector is the right size.";
    }
    {
        const auto tPerturbation =
            detail::perturbation(input_parser::DirectionVectorTypes::kUniformPositive, tSize, tSeed);
        EXPECT_EQ(tPerturbation.size(), tSize) << "Uniform positive vector is the right size.";
        const auto tSum = std::accumulate(tPerturbation.stdVector().begin(), tPerturbation.stdVector().end(), 0.0);
        EXPECT_EQ(tSum, tSize);
    }
    {
        const auto tPerturbation =
            detail::perturbation(input_parser::DirectionVectorTypes::kUniformNegative, tSize, tSeed);
        EXPECT_EQ(tPerturbation.size(), tSize) << "Uniform negative vector is the right size.";
        const auto tSum = std::accumulate(tPerturbation.stdVector().begin(), tPerturbation.stdVector().end(), 0.0);
        EXPECT_EQ(-1.0 * tSum, tSize);
    }
}

namespace
{
[[nodiscard]] auto count_file_lines(const std::filesystem::path& aFilePath) -> std::size_t
{
    auto tInFile = std::ifstream(aFilePath);
    return std::count(std::istreambuf_iterator<char>(tInFile), std::istreambuf_iterator<char>(), '\n');
}

[[nodiscard]] auto read_file_header(const std::filesystem::path& aFilePath) -> std::string
{
    auto tInFile = std::ifstream(aFilePath);
    std::string tFirstLine;
    std::getline(tInFile, tFirstLine);
    return tFirstLine;
}

[[nodiscard]] auto step_ratio_in_file(const std::filesystem::path& aFilePath) -> double
{
    auto tInFile = std::ifstream(aFilePath);
    std::string tLine;
    std::getline(tInFile, tLine);
    std::getline(tInFile, tLine);
    const double tFirstStep = std::stod(tLine);
    std::getline(tInFile, tLine);
    const double tSecondStep = std::stod(tLine);
    return tSecondStep / tFirstStep;
}

}  // namespace

TEST_F(ObjectiveGradientCheckFixture, RunCoordinates)
{
    const auto tInput = make_objective_gradient_check_input(
        mMeshFilePath, input_parser::DesignVariableTypes::kCoordinate, input_parser::DirectionVectorTypes::kRandom);
    const auto tProblem = library::make_process_manager_data(validated_input(tInput).value());
    const auto tValidatedCriteriaGradientInput = validated_process_managers(tInput);
    const auto tObjectiveGradientCheck = ObjectiveGradientCheck{tValidatedCriteriaGradientInput};
    tObjectiveGradientCheck.run(tProblem);

    ASSERT_TRUE(std::filesystem::exists(tInput.output_file_name.value().mToken));
    constexpr auto tNumberOfHeaderLines = 1U;
    EXPECT_EQ(count_file_lines(tInput.output_file_name.value().mToken),
              tInput.number_of_steps.value() + tNumberOfHeaderLines);
    const auto tHeaderStringGold =
        std::string{"Step                    Gradient*Direction      Finite Difference       Relative Error          "};
    EXPECT_EQ(read_file_header(tInput.output_file_name.value().mToken), tHeaderStringGold);
    EXPECT_DOUBLE_EQ(step_ratio_in_file(tInput.output_file_name.value().mToken),
                     tInput.step_size_reduction_factor.value());
    plato::test_utilities::test_for_existence_and_remove(
        {tInput.output_file_name.value().mToken}, TEST_CONTEXT("Run Criteria Gradient Check on Vector Variables"));
}

namespace
{

const auto kPlatoEngineQuadraticObjectiveInput =
    input_parser::objective{/*.name=*/std::string{"quadratic"},
                            /*.active=*/true,
                            /*.app=*/input_parser::AppName{std::string{input_parser::kBuiltinAppName}},
                            /*.criterion=*/input_parser::CriterionName{"quadratic"},
                            /*.number_of_processors=*/1U,
                            /*.input_files=*/boost::none,
                            /*.aggregation_weight=*/1.0,
                            /*.normalize_by_initial_value=*/false,
                            /*.objective_goal=*/boost::none};

void expect_first_data_row_matches_analytic_quadratic_objective(const std::filesystem::path& aFilePath,
                                                                const std::size_t aVectorSize,
                                                                const double aDirectionValue,
                                                                const plato::test_utilities::TestContext& aTestContext)
{
    std::ifstream tInFile(aFilePath);
    std::string tHeader;
    std::getline(tInFile, tHeader);
    double tStepFromFile = 0.0;
    double tAnalyticFromFile = 0.0;
    double tFiniteDifferenceFromFile = 0.0;
    double tRelativeErrorFromFile = 0.0;
    tInFile >> tStepFromFile >> tAnalyticFromFile >> tFiniteDifferenceFromFile >> tRelativeErrorFromFile;

    const auto tDensities = std::vector<double>(aVectorSize, detail::default_starting_density());
    const auto tDirection = std::vector<double>(aVectorSize, aDirectionValue);
    const double tAnalyticExpected = std::inner_product(tDensities.begin(), tDensities.end(), tDirection.begin(), 0.0);
    const double tDirectionNormSquare =
        std::inner_product(tDirection.begin(), tDirection.end(), tDirection.begin(), 0.0);
    constexpr auto tFirstStep = 0.1;
    const double tFiniteDifferenceExpected = tAnalyticExpected + 0.5 * tFirstStep * tDirectionNormSquare;
    const double tRelativeErrorExpected =
        std::abs(tAnalyticExpected - tFiniteDifferenceExpected) / std::abs(tAnalyticExpected);

    constexpr auto tTolerance = 1e-12;
    EXPECT_NEAR(tStepFromFile, tFirstStep, tTolerance) << aTestContext;
    EXPECT_NEAR(tAnalyticFromFile, tAnalyticExpected, tTolerance) << aTestContext;
    EXPECT_NEAR(tFiniteDifferenceFromFile, tFiniteDifferenceExpected, tTolerance) << aTestContext;
    EXPECT_NEAR(tRelativeErrorFromFile, tRelativeErrorExpected, tTolerance) << aTestContext;
}

void run_density_gradient_check(const input_parser::objective_gradient_check& aCriteriaGradientInput,
                                const plato::test_utilities::TestContext& aTestContext)
{
    const auto tFullInput = geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                            kPlatoEngineQuadraticObjectiveInput | aCriteriaGradientInput;
    const auto tValidatedInput = input_validation::make_validated_input(tFullInput);

    const auto tProblem = library::make_process_manager_data(tValidatedInput.value());
    const auto tValidatedCriteriaGradientInput =
        tValidatedInput.value().get<components::ComponentType::kProcessManager>().rawInput().back();
    const auto tObjectiveGradientCheck = ObjectiveGradientCheck{tValidatedCriteriaGradientInput};
    tObjectiveGradientCheck.run(tProblem);

    constexpr auto tElementDensitySize = 3U;
    constexpr auto tNodalCoordinateSize = 16U;
    expect_first_data_row_matches_analytic_quadratic_objective(
        aCriteriaGradientInput.output_file_name.value().mToken,
        aCriteriaGradientInput.design_variable_type == input_parser::DesignVariableTypes::kElementDensity
            ? tElementDensitySize
            : tNodalCoordinateSize,
        aCriteriaGradientInput.design_variable_type == input_parser::DesignVariableTypes::kElementDensity ? -1.0 : 1.0,
        aTestContext);
    plato::test_utilities::test_for_existence_and_remove({aCriteriaGradientInput.output_file_name.value().mToken},
                                                         aTestContext);
}

}  // namespace

TEST_F(ObjectiveGradientCheckFixture, RunElementDensities)
{
    const auto tObjectiveGradientCheckInput =
        make_objective_gradient_check_input(mMeshFilePath, input_parser::DesignVariableTypes::kElementDensity,
                                            input_parser::DirectionVectorTypes::kUniformNegative);

    run_density_gradient_check(tObjectiveGradientCheckInput,
                               TEST_CONTEXT("Run criteria gradient check on element densitities."));
}

TEST_F(ObjectiveGradientCheckFixture, RunNodalDensities)
{
    const auto tObjectiveGradientCheckInput =
        make_objective_gradient_check_input(mMeshFilePath, input_parser::DesignVariableTypes::kNodalDensity,
                                            input_parser::DirectionVectorTypes::kUniformPositive);

    run_density_gradient_check(tObjectiveGradientCheckInput,
                               TEST_CONTEXT("Run criteria gradient check on nodal densitities."));
}

}  // namespace plato::process_manager::extension::unittest
