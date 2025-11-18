#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string_view>

#include "plato/criteria/extension/PluginCriteria.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/criteria/library/CriterionValidation.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/services/AppConfiguration.hpp"
#include "plato/services/AppConfigurationUtilities.hpp"
#include "plato/services/PluginDirectoryPath.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/test_utilities/TestDirectorySetupTeardown.hpp"
#include "plato/utilities/OptionalToVector.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::criteria::extension::unittest
{
namespace
{
const auto kTestCriterionName = std::string{"test-criterion"};
constexpr auto kParallelScalar =
    library::CriterionTraits{library::Parallelization::kParallel, library::FunctionDimension::kScalar};
constexpr auto kSerialScalar =
    library::CriterionTraits{library::Parallelization::kSerial, library::FunctionDimension::kScalar};
constexpr auto kSerialVector =
    library::CriterionTraits{library::Parallelization::kSerial, library::FunctionDimension::kVector};
constexpr auto kParallelVector =
    library::CriterionTraits{library::Parallelization::kParallel, library::FunctionDimension::kVector};

using AppConfigVector = std::vector<std::pair<std::string_view, library::CriterionTraits>>;

/// @brief A file writer that outputs some text for mimicking a shared library on disk.
struct FakeSharedLibWriter
{
    void operator()(const std::filesystem::path& aFilePath) const
    {
        auto tOutFile = std::ofstream{aFilePath};
        tOutFile << "Not really a shared lib, please don't open me.\n";
    }
};

/// @brief A file writer that doesn't write anything.
struct NullSharedLibWriter
{
    void operator()(const std::filesystem::path&) const {}
};

template <typename SharedLibWriter>
[[nodiscard]] auto create_test_app_configurations_impl(const AppConfigVector& aAppConfigs,
                                                       const SharedLibWriter& aSharedLibWriter)
    -> test_utilities::TestDirectorySetupTeardown
{
    auto tConfigurationTempDirectory = test_utilities::TestDirectorySetupTeardown{"test-plugin-directory"};

    for (const auto& tAppConfig : aAppConfigs)
    {
        const auto tLibName = utilities::concatenate("lib", tAppConfig.first, ".so");
        const auto tConfigName = utilities::concatenate(tAppConfig.first, ".config");
        const auto tCriterionConfiguration = services::CriterionConfiguration{
            /*.mName=*/kTestCriterionName,
            /*.mIsParallelized=*/tAppConfig.second.mParallelization == library::Parallelization::kParallel,
            /*.mIsScalar=*/tAppConfig.second.mDimension == library::FunctionDimension::kScalar,
            /*.mFunctionName=*/"plato_create_criterion",
        };
        auto tAppConfiguration = services::AppConfiguration{/*.mName=*/std::string{tAppConfig.first},
                                                            /*.mLibraryFileName=*/tLibName,
                                                            /*.mCriteria=*/{tCriterionConfiguration}};
        tConfigurationTempDirectory
            .writeFile(services::AppConfigurationWriter{std::move(tAppConfiguration)}, tConfigName)
            .writeFile(aSharedLibWriter, tLibName);
    }
    return tConfigurationTempDirectory;
}

/// @brief Creates a directory with test AppConfigurations with names given in @a aAppNames and creates
///  fake shared libraries so that the app can be correctly registered.
[[nodiscard]] auto create_test_app_configurations_with_fake_shared_libs(const AppConfigVector& aAppConfigs)
    -> test_utilities::TestDirectorySetupTeardown
{
    return create_test_app_configurations_impl(aAppConfigs, FakeSharedLibWriter{});
}

/// @brief Creates a directory with test AppConfigurations with names given in @a aAppNames. This does
/// not create any shared libraries so that the apps passed to this function will not be registered correctly.
[[nodiscard]] auto create_test_app_configurations(const AppConfigVector& aAppConfigs)
    -> test_utilities::TestDirectorySetupTeardown
{
    return create_test_app_configurations_impl(aAppConfigs, NullSharedLibWriter{});
}
}  // namespace

TEST(PluginCriteria, NothingRegisteredForEmptyPaths)
{
    const auto tNumberOfRegisteredApps = register_plugin_apps({});
    EXPECT_EQ(tNumberOfRegisteredApps, 0u);
}

TEST(PluginCriteria, NonexistentSharedLibrary)
{
    const auto tTheBlobAppName = std::string{"the-blob"};
    const auto tConfigurationTempDirectory = create_test_app_configurations({{tTheBlobAppName, kParallelScalar}});
    const auto tNumKnownConfigurations = services::app_configurations({tConfigurationTempDirectory.directory()}).size();
    const auto tNumRegistered = register_plugin_apps({tConfigurationTempDirectory.directory()});
    EXPECT_NE(tNumRegistered, tNumKnownConfigurations);

    const auto tFunctionName = library::criterion_registration_name(input_parser::AppName{tTheBlobAppName},
                                                                    input_parser::CriterionName{kTestCriterionName});
    const auto tSerialScalarTraits =
        library::CriterionTraits{library::Parallelization::kSerial, library::FunctionDimension::kScalar};
    EXPECT_FALSE(library::criterion_function_has_traits(tFunctionName, tSerialScalarTraits));

    const auto tParallelScalarTraits =
        library::CriterionTraits{library::Parallelization::kParallel, library::FunctionDimension::kScalar};
    EXPECT_FALSE(library::criterion_function_has_traits(tFunctionName, tParallelScalarTraits));

    EXPECT_FALSE(library::is_criterion_function_registered(tFunctionName));
}

TEST(PluginCriteria, RegisterApps)
{
    // Checks that some fake apps get registered via register_plugin_apps
    const auto tVampireAppName = std::string{"vampire"};
    const auto tMummyAppName = std::string{"mummy"};
    const auto tZombieAppName = std::string{"zombie"};
    const auto tDraculaAppName = std::string{"dracula"};
    const auto tConfigurationTempDirectory =
        create_test_app_configurations_with_fake_shared_libs({{tVampireAppName, kParallelScalar},
                                                              {tMummyAppName, kSerialScalar},
                                                              {tZombieAppName, kSerialVector},
                                                              {tDraculaAppName, kParallelVector}});
    const auto tNumRegistered = register_plugin_apps({tConfigurationTempDirectory.directory()});
    EXPECT_EQ(tNumRegistered, 4U);

    const auto tCheckIsCriterionRegistered = [](const std::string_view aFunctionName,
                                                const library::CriterionTraits aRegisteredTraits,
                                                const std::vector<library::CriterionTraits>& aUnregisteredTraits,
                                                const test_utilities::TestContext& aTestContext)
    {
        EXPECT_TRUE(library::is_criterion_function_registered(aFunctionName)) << aTestContext;
        EXPECT_TRUE(library::criterion_function_has_traits(aFunctionName, aRegisteredTraits)) << aTestContext;
        for (const auto& tUnregisteredTrait : aUnregisteredTraits)
        {
            EXPECT_FALSE(library::criterion_function_has_traits(aFunctionName, tUnregisteredTrait)) << aTestContext;
        }
    };

    const auto tVampireFunctionName = library::criterion_registration_name(
        input_parser::AppName{tVampireAppName}, input_parser::CriterionName{kTestCriterionName});
    tCheckIsCriterionRegistered(tVampireFunctionName, kParallelScalar, {kSerialScalar, kSerialVector, kParallelVector},
                                TEST_CONTEXT("Parallel scalar registered"));

    const auto tMummyFunctionName = library::criterion_registration_name(
        input_parser::AppName{tMummyAppName}, input_parser::CriterionName{kTestCriterionName});
    tCheckIsCriterionRegistered(tMummyFunctionName, kSerialScalar, {kParallelScalar, kSerialVector, kParallelVector},
                                TEST_CONTEXT("Serial scalar registered"));

    const auto tZombieFunctionName = library::criterion_registration_name(
        input_parser::AppName{tZombieAppName}, input_parser::CriterionName{kTestCriterionName});
    tCheckIsCriterionRegistered(tZombieFunctionName, kSerialVector, {kSerialScalar, kParallelScalar, kParallelVector},
                                TEST_CONTEXT("Serial vector registered"));

    const auto tDraculaFunctionName = library::criterion_registration_name(
        input_parser::AppName{tDraculaAppName}, input_parser::CriterionName{kTestCriterionName});
    tCheckIsCriterionRegistered(tDraculaFunctionName, kParallelVector, {kSerialScalar, kSerialVector, kParallelScalar},
                                TEST_CONTEXT("Parallel vector registered"));
}

TEST(PluginCriteria, ValidateValidApps)
{
    const auto tFrankensteinAppName = std::string{"frankenstein"};
    const auto tMedusaAppName = std::string{"medusa"};
    const auto tItAppName = std::string{"it"};
    const auto tThingAppName = std::string{"thing"};
    const auto tConfigurationTempDirectory =
        create_test_app_configurations_with_fake_shared_libs({{tFrankensteinAppName, kSerialScalar},
                                                              {tMedusaAppName, kParallelScalar},
                                                              {tItAppName, kSerialVector},
                                                              {tThingAppName, kParallelVector}});
    const auto tNumRegistered = register_plugin_apps({tConfigurationTempDirectory.directory()});
    EXPECT_EQ(tNumRegistered, 4u);

    auto tCriteria = input_parser::objective{};
    EXPECT_TRUE(library::detail::validate_criterion_is_registered(tCriteria).has_value());

    tCriteria.criterion = input_parser::CriterionName{kTestCriterionName};
    for (const auto& tAppName : {tFrankensteinAppName, tMedusaAppName, tItAppName, tThingAppName})
    {
        tCriteria.app = input_parser::AppName{tAppName};
        EXPECT_FALSE(library::detail::validate_criterion_is_registered(tCriteria).has_value())
            << library::detail::validate_criterion_is_registered(tCriteria).value();
    }
}

TEST(PluginCriteria, ValidateInvalidApp)
{
    const auto tWolfmanAppName = std::string{"wolfman"};
    const auto tConfigurationTempDirectory = create_test_app_configurations({{tWolfmanAppName, kParallelScalar}});
    register_plugin_apps({tConfigurationTempDirectory.directory()});

    auto tCriteria = input_parser::objective{};
    tCriteria.app = input_parser::AppName{tWolfmanAppName};
    EXPECT_TRUE(library::detail::validate_criterion_is_registered(tCriteria).has_value());

    tCriteria.app = input_parser::AppName{std::string{"bog-monster"}};
    EXPECT_TRUE(library::detail::validate_criterion_is_registered(tCriteria).has_value());
}

TEST(CriterionRegistration, RegisterAppsList)
{
    const auto tMartianAppName = std::string{"martian"};
    const auto tCerberusAppName = std::string{"cerberus"};
    const auto tVanHelsingAppName = std::string{"van-helsing"};
    const auto tLurchAppName = std::string{"lurch"};
    const auto tConfigurationTempDirectory =
        create_test_app_configurations_with_fake_shared_libs({{tMartianAppName, kSerialScalar},
                                                              {tCerberusAppName, kParallelScalar},
                                                              {tVanHelsingAppName, kParallelVector},
                                                              {tLurchAppName, kSerialVector}});
    register_plugin_apps({tConfigurationTempDirectory.directory()});

    const auto tRegisteredApps = library::registered_criteria_names();
    const auto tCriterionIsInRegisteredNames = [&tRegisteredApps](const std::string& tAppName)
    {
        const auto tRegistrationName = library::criterion_registration_name(
            input_parser::AppName{tAppName}, input_parser::CriterionName{kTestCriterionName});
        return tRegisteredApps.count(tRegistrationName) == 1u;
    };
    EXPECT_TRUE(tCriterionIsInRegisteredNames(tMartianAppName));
    EXPECT_TRUE(tCriterionIsInRegisteredNames(tCerberusAppName));
    EXPECT_TRUE(tCriterionIsInRegisteredNames(tVanHelsingAppName));
    EXPECT_TRUE(tCriterionIsInRegisteredNames(tLurchAppName));
}

}  // namespace plato::criteria::extension::unittest
