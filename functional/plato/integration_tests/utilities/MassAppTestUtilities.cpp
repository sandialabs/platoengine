#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <optional>

#include "plato/criteria/extension/PluginCriteria.hpp"
#include "plato/criteria/library/ObjectiveFactory.hpp"
#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/services/AppConfiguration.hpp"
#include "plato/services/AppConfigurationUtilities.hpp"
#include "plato/services/PluginDirectoryPath.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestDirectorySetupTeardown.hpp"

namespace plato::integration_tests::utilities
{
namespace
{
/// The test mass app gets installed to the lib directory, not the plugin directory since
/// we don't want it to be used as an actual app. This gets the lib directory so we can find it.
std::optional<std::filesystem::path> lib_directory_from_plugin_directory()
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
std::filesystem::path mass_app_lib_path()
{
    constexpr auto tMassAppLibName = std::string_view{"libPlatoTestMassObjective.so"};
    auto tMassAppPath = std::filesystem::path{tMassAppLibName};
    if (const auto tLibPath = lib_directory_from_plugin_directory(); !std::filesystem::exists(tMassAppPath) && tLibPath)
    {
        tMassAppPath = tLibPath.value() / tMassAppPath;
    }
    return tMassAppPath;
}
}  // namespace

test_utilities::TestDirectorySetupTeardown register_test_mass_app(const std::string_view aAppName,
                                                                  const boost::mpi::communicator& aComm)
{
    const auto tTestPluginDirectory = std::filesystem::path{"test-plugin-directory"};
    const auto tConfigurationTempDirectory = test_utilities::TestDirectorySetupTeardown{tTestPluginDirectory, aComm};
    const auto tMassLibPath = std::filesystem::relative(mass_app_lib_path(), tTestPluginDirectory);
    const auto tCriterionSerialConfiguration = services::CriterionConfiguration{
        /*.mName=*/"mass", /*.mIsParallelized=*/false, /*.mFunctionName=*/"plato_create_test_mass_criterion"};
    const auto tCriterionParallelConfiguration = services::CriterionConfiguration{
        /*.mName=*/"mass", /*.mIsParallelized=*/true, /*.mFunctionName=*/"plato_create_parallel_test_mass_criterion"};
    auto tAppConfiguration =
        services::AppConfiguration{/*.mName=*/
                                   std::string{aAppName},
                                   /*.mLibraryFileName=*/tMassLibPath.string(),
                                   /*.mCriteria=*/{tCriterionSerialConfiguration, tCriterionParallelConfiguration}};
    tConfigurationTempDirectory.writeFile(services::AppConfigurationWriter{std::move(tAppConfiguration)},
                                          "test-mass-app.config");
    aComm.barrier();
    criteria::extension::register_plugin_apps({tTestPluginDirectory});
    return tConfigurationTempDirectory;
}

process_manager::library::ValidatedInput create_test_mass_app_input(const input_parser::AppName& aMassAppName,
                                                                    const input_parser::CriterionName& aCriterionName,
                                                                    const unsigned int aNumProcessors)
{
    auto tObjective = input_parser::objective{};
    tObjective.number_of_processors = aNumProcessors;
    tObjective.aggregation_weight = 1.0;
    tObjective.app = aMassAppName;
    tObjective.criterion = aCriterionName;
    tObjective.name = "test_1";

    const auto tInput = tObjective | test_utilities::create_valid_brick_shape_geometry() |
                        test_utilities::create_valid_example_rol_optimization();
    return process_manager::library::make_validated_input(tInput);
}

std::pair<linear_algebra::DynamicVector<double>, double> brick_shape_geometry_controls_with_volume()
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
    const auto tConfigurationTempDirectory =
        integration_tests::utilities::register_test_mass_app(tAppName.mToken, aComm);
    const auto tCriterionName = input_parser::CriterionName{"mass"};
    const auto tValidInput =
        integration_tests::utilities::create_test_mass_app_input(tAppName, tCriterionName, aComm.size());

    const auto tObjectiveFunction = criteria::library::make_aggregate_objective_function(tValidInput.objectives());
    const auto tGeometry =
        geometry::extension::make_brick_shape_geometry(geometry::extension::BrickShapeGeometry{"brick.exo"});

    const auto [tControls, tExpectedValue] = integration_tests::utilities::brick_shape_geometry_controls_with_volume();
    const auto tResult = tObjectiveFunction.f(tGeometry.f(tControls));
    EXPECT_DOUBLE_EQ(tResult, tExpectedValue) << aTestContext;
}

}  // namespace plato::integration_tests::utilities
