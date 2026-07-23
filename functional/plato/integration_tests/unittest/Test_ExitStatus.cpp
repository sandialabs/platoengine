#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/library/SharedLibCriterion.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/integration_tests/utilities/AppConfigurationTestUtilities.hpp"
#include "plato/integration_tests/utilities/MassAppTestUtilities.hpp"
#include "plato/main/library/CommandLineInputHandler.hpp"
#include "plato/main/library/ExitStatus.hpp"
#include "plato/test_utilities/FileCreatingTestFixture.hpp"

namespace plato::integration_tests::unittest
{
namespace
{
constexpr auto kRuntimeThrowCriterionName = std::string_view{"runtime_throw"};
constexpr auto kInputFileName = std::string_view{"input.i"};

auto input_with_runtime_throw_objective(const std::string& aProcessManager) -> std::string
{
    return std::format(
        "begin brick_shape_geometry\n"
        "mesh_name void.exo \n"
        "end \n"

        "begin objective error\n"
        "   app test-mass-app\n"
        "   criterion {}\n"
        "   aggregation_weight 1.0\n"
        "end \n"

        "begin {}\n"
        "end",
        kRuntimeThrowCriterionName, aProcessManager);
}

struct ExitStatusInputFileFixture : public test_utilities::FileCreatingTestFixture
{
    ExitStatusInputFileFixture() : FileCreatingTestFixture{std::filesystem::path{kInputFileName}} {}

    auto writeInputForProcessManager(const std::string& aProcessManager) const -> void
    {
        auto tStream = std::ofstream(filePath());
        tStream << input_with_runtime_throw_objective(aProcessManager);
    }
};

}  // namespace

TEST(RuntimeThrowCriterion, ValueAndGradientEvaluationsThrow)
{
    const auto tTestConfiguration = utilities::test_app_configuration(utilities::mass_app_library_file_name());
    const auto tRuntimeThrowCriterion = std::find_if(
        tTestConfiguration.mConfiguration.mCriteria.cbegin(), tTestConfiguration.mConfiguration.mCriteria.cend(),
        [](const auto& aCriterion) { return aCriterion.mName == kRuntimeThrowCriterionName; });
    ASSERT_NE(tRuntimeThrowCriterion, tTestConfiguration.mConfiguration.mCriteria.cend());

    const auto tSharedLib = criteria::library::SharedLibCriterion{tTestConfiguration, *tRuntimeThrowCriterion, {}};

    EXPECT_THROW([[maybe_unused]] const auto tVal = tSharedLib.f(analysis::AnalysisDomainMesh{}), std::runtime_error);
    EXPECT_THROW([[maybe_unused]] const auto tVal = tSharedLib.df(analysis::AnalysisDomainMesh{}), std::runtime_error);
}

TEST_F(ExitStatusInputFileFixture, RuntimeErrorExitStatus)
{
    const auto tAppName = input_parser::AppName{"test-mass-app"};
    [[maybe_unused]] const auto tConfigurationTempDirectory =
        utilities::register_test_mass_app(tAppName.mToken, boost::mpi::communicator{});

    writeInputForProcessManager("rol_optimization");
    const auto tExitStatus = main::library::detail::run_plato(filePath());
    EXPECT_EQ(tExitStatus, main::library::ExitStatus::kRuntimeError);
}

TEST_F(ExitStatusInputFileFixture, ExecutionSuccessfulExitStatus)
{
    const auto tAppName = input_parser::AppName{"test-mass-app"};
    [[maybe_unused]] const auto tConfigurationTempDirectory =
        utilities::register_test_mass_app(tAppName.mToken, boost::mpi::communicator{});

    writeInputForProcessManager("initial_geometry_writer");
    const auto tExitStatus = main::library::detail::run_plato(filePath());
    EXPECT_EQ(tExitStatus, main::library::ExitStatus::kExecutionSuccessful);
}
}  // namespace plato::integration_tests::unittest
