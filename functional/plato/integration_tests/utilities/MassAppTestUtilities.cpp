#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <optional>

#include "plato/criteria/extension/PluginCriteria.hpp"
#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/ObjectiveFactory.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/integration_tests/utilities/InputGeneration.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/services/AppConfiguration.hpp"
#include "plato/services/AppConfigurationUtilities.hpp"
#include "plato/services/PluginDirectoryPath.hpp"
#include "plato/test_utilities/TestDirectorySetupTeardown.hpp"

namespace plato::integration_tests::utilities
{
namespace
{
/// The test mass app gets installed to the lib directory, not the plugin directory since
/// we don't want it to be used as an actual app. This gets the lib directory so we can find it.
[[nodiscard]] auto lib_directory_from_plugin_directory() -> std::optional<std::filesystem::path>
{
    if (auto tPluginPath = services::plugin_directory_path())
    {
        return std::move(tPluginPath).value() / ".." / "lib";
    }
    else
    {
        return std::nullopt;
    }
}

/// Attempts to find the test mass app shared lib. If the test calling this function is running
/// with ctest or from the build directory, the shared lib should be in the working directory.
/// If running the test from the install directory, we try to find it in the installation.
[[nodiscard]] auto lib_path(const std::string_view aLibraryName) -> std::filesystem::path
{
    auto tMassAppPath = std::filesystem::path{aLibraryName};
    if (const auto tLibPath = lib_directory_from_plugin_directory(); !std::filesystem::exists(tMassAppPath) && tLibPath)
    {
        tMassAppPath = tLibPath.value() / tMassAppPath;
    }
    return tMassAppPath;
}

[[nodiscard]] auto mass_app_lib_path() -> std::filesystem::path { return lib_path(mass_app_library_file_name()); }

void write_mass_app_config(const std::string_view& aAppName,
                           const std::filesystem::path& aTestPluginDirectory,
                           const test_utilities::TestDirectorySetupTeardown& aConfigurationTempDirectory,
                           const boost::mpi::communicator& aComm)
{
    const auto tMassLibPath = std::filesystem::relative(mass_app_lib_path(), aTestPluginDirectory);
    const auto tCriterionSerialConfiguration =
        services::CriterionConfiguration{.mName = "mass",
                                         .mIsParallelized = false,
                                         .mIsScalar = true,
                                         .mFunctionName = "plato_create_test_mass_criterion"};
    const auto tCriterionParallelConfiguration =
        services::CriterionConfiguration{.mName = "mass",
                                         .mIsParallelized = true,
                                         .mIsScalar = true,
                                         .mFunctionName = "plato_create_parallel_test_mass_criterion"};
    const auto tCriterionVectorConfiguration =
        services::CriterionConfiguration{.mName = "mass-all-densities",
                                         .mIsParallelized = false,
                                         .mIsScalar = false,
                                         .mFunctionName = "plato_create_vector_test_mass_criterion"};
    const auto tCriterionVectorWithComponentsConfiguration =
        services::CriterionConfiguration{.mName = "mass-properties",
                                         .mIsParallelized = false,
                                         .mIsScalar = false,
                                         .mFunctionName = "plato_create_mass_properties_vector_criterion",
                                         .mVectorComponents = std::map<std::size_t, std::string>{
                                             {0U, "mass"}, {1U, "cg_x"}, {2U, "cg_y"}, {3U, "cg_z"}}};

    auto tAppConfiguration = services::AppConfiguration{
        .mName = std::string{aAppName},
        .mLibraryFileName = tMassLibPath.string(),
        .mCriteria = {tCriterionSerialConfiguration, tCriterionParallelConfiguration, tCriterionVectorConfiguration,
                      tCriterionVectorWithComponentsConfiguration}};
    aConfigurationTempDirectory.writeFile(services::AppConfigurationWriter{std::move(tAppConfiguration)},
                                          "test-mass-app.config");
    aComm.barrier();
}

}  // namespace

auto register_test_mass_app(const std::string_view aAppName, const boost::mpi::communicator& aComm)
    -> test_utilities::TestDirectorySetupTeardown
{
    const auto tTestPluginDirectory = std::filesystem::path{"test-plugin-directory"};
    auto tConfigurationTempDirectory = test_utilities::TestDirectorySetupTeardown{tTestPluginDirectory, aComm};

    write_mass_app_config(aAppName, tTestPluginDirectory, tConfigurationTempDirectory, aComm);

    criteria::extension::register_plugin_apps({tTestPluginDirectory});
    return tConfigurationTempDirectory;
}

namespace
{
[[nodiscard]] auto create_mass_objective(const input_parser::AppName& aMassAppName,
                                         const input_parser::CriterionName& aCriterionName,
                                         const unsigned int aNumProcessors) -> input_parser::objective
{
    auto tObjective = input_parser::objective{};
    tObjective.number_of_processors = aNumProcessors;
    tObjective.aggregation_weight = 1.0;
    tObjective.app = aMassAppName;
    tObjective.criterion = aCriterionName;
    tObjective.name = "test_1";
    return tObjective;
}

/// @brief Creates test input that has a brick shape geometry, ROL optimization, ROL constraint check a mass objective,
/// and a vector mass constraint with name @a aMassAppName
auto create_test_mass_vector_constraint_input(const input_parser::AppName& aMassAppName,
                                              const input_parser::CriterionName& aConstraintName,
                                              const std::filesystem::path& aMeshName)
    -> input_validation::ValidatedInput
{
    auto tConstraint = input_parser::constraint{};
    tConstraint.number_of_processors = 1U;
    tConstraint.app = aMassAppName;
    tConstraint.criterion = aConstraintName;
    tConstraint.name = "test_2";
    tConstraint.constraint_type = input_parser::ConstraintTypes::kGreaterThan;
    tConstraint.constraint_value = 0.7;

    const auto tObjectiveName = input_parser::CriterionName{"mass"};
    const auto tObjective = create_mass_objective(aMassAppName, tObjectiveName, 1U);

    auto tGeometryAndFilterInput =
        integration_tests::utilities::create_valid_density_topology_geometry_with_element_centered_kernel_filter_input(
            aMeshName);

    const auto tInput = tGeometryAndFilterInput | tObjective | tConstraint |
                        process_manager::extension::test_utilities::create_valid_example_rol_optimization_input() |
                        process_manager::extension::test_utilities::create_valid_example_constraint_check_input();
    return input_validation::make_validated_input(tInput).value();
}

/// @brief Creates test input that has a brick shape geometry, ROL optimization, and an objective
/// with name @a aMassAppName and number of processors @a aNumProcessors.
[[nodiscard]] auto create_test_mass_app_input(const input_parser::AppName& aMassAppName,
                                              const input_parser::CriterionName& aCriterionName,
                                              unsigned int aNumProcessors) -> input_validation::ValidatedInput
{
    const auto tObjective = create_mass_objective(aMassAppName, aCriterionName, aNumProcessors);

    const auto tInput = tObjective | geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                        process_manager::extension::test_utilities::create_valid_example_rol_optimization_input();

    return input_validation::make_validated_input(tInput).value();
}
}  // namespace

auto brick_shape_geometry_controls_with_volume() -> std::pair<linear_algebra::DynamicVector<double>, double>
{
    constexpr auto tX = double{2.0};
    constexpr auto tY = double{4.0};
    constexpr auto tZ = double{6.0};
    constexpr auto tCenter = double{0.0};
    return std::make_pair(linear_algebra::DynamicVector<double>{tCenter, tCenter, tCenter, tX, tY, tZ}, tX * tY * tZ);
}

void register_load_run_test(const boost::mpi::communicator& aComm, const test_utilities::TestContext& aTestContext)
{
    ASSERT_TRUE(std::filesystem::exists(mass_app_lib_path())) << aTestContext;

    const auto tAppName = input_parser::AppName{"test-mass-app"};
    const auto tConfigurationTempDirectory = register_test_mass_app(tAppName.mToken, aComm);
    const auto tCriterionName = input_parser::CriterionName{"mass"};
    const auto tValidInput = create_test_mass_app_input(tAppName, tCriterionName, aComm.size());

    const auto [tControls, tExpectedValue] = integration_tests::utilities::brick_shape_geometry_controls_with_volume();
    const auto tGeometry =
        geometry::extension::make_brick_shape_geometry(geometry::extension::BrickShapeGeometry{"brick.exo"});
    const auto tDomainMesh = tGeometry.evaluate<core::evaluation::kFunction>(tControls);
    const auto tObjectiveFunction = criteria::library::make_aggregate_objective_function(
        tValidInput.get<components::ComponentType::kObjective>(), tDomainMesh);

    const auto tResult = tObjectiveFunction.evaluate<core::evaluation::kFunction>(tDomainMesh);
    EXPECT_DOUBLE_EQ(tResult, tExpectedValue) << aTestContext;
}

auto setup_mass_app_for_test(const std::filesystem::path& aMeshFileName)
    -> std::pair<test_utilities::TestDirectorySetupTeardown, input_validation::ValidatedInput>
{
    const auto tAppName = input_parser::AppName{"test-mass-app"};
    auto tConfigurationTempDirectory = register_test_mass_app(tAppName.mToken, boost::mpi::communicator{});

    const auto tConstraintName = input_parser::CriterionName{"mass-all-densities"};
    return std::make_pair(std::move(tConfigurationTempDirectory),
                          create_test_mass_vector_constraint_input(tAppName, tConstraintName, aMeshFileName));
}
}  // namespace plato::integration_tests::utilities
