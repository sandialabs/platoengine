#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string_view>

#include "plato/criteria/extension/PluginCriteria.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/criteria/library/CriterionValidation.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/services/AppConfiguration.hpp"
#include "plato/services/AppConfigurationUtilities.hpp"
#include "plato/services/PluginDirectoryPath.hpp"
#include "plato/test_utilities/TestDirectorySetupTeardown.hpp"
#include "plato/utilities/OptionalToVector.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::criteria::extension::unittest
{
namespace
{
const auto kTestCriterionName = std::string{"test-criterion"};

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
[[nodiscard]] test_utilities::TestDirectorySetupTeardown create_test_app_configurations_impl(
    const std::vector<std::string_view>& aAppNames, const SharedLibWriter& aSharedLibWriter)
{
    const auto tConfigurationTempDirectory = test_utilities::TestDirectorySetupTeardown{"test-plugin-directory"};

    for (const auto tAppName : aAppNames)
    {
        const auto tLibName = utilities::concatenate("lib", tAppName, ".so");
        const auto tConfigName = utilities::concatenate(tAppName, ".config");
        const auto tCriterionConfiguration = services::CriterionConfiguration{
            /*.mName=*/kTestCriterionName,
            /*.mIsParallelized=*/true,
            /*.mFunctionName=*/"plato_create_criterion",
        };
        auto tAppConfiguration = services::AppConfiguration{/*.mName=*/std::string{tAppName},
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
[[nodiscard]] test_utilities::TestDirectorySetupTeardown create_test_app_configurations_with_fake_shared_libs(
    const std::vector<std::string_view>& aAppNames)
{
    return create_test_app_configurations_impl(aAppNames, FakeSharedLibWriter{});
}

/// @brief Creates a directory with test AppConfigurations with names given in @a aAppNames. This does
/// not create any shared libraries so that the apps passed to this function will not be registered correctly.
[[nodiscard]] test_utilities::TestDirectorySetupTeardown create_test_app_configurations(
    const std::vector<std::string_view>& aAppNames)
{
    return create_test_app_configurations_impl(aAppNames, NullSharedLibWriter{});
}
}  // namespace

TEST(PluginCriteria, NumberOfPluginsRegistered)
{
    EXPECT_EQ(services::app_configurations(utilities::optional_to_vector(services::plugin_directory_path())).size(),
              number_of_plugins_registered_at_startup());
}

TEST(PluginCriteria, NothingRegisteredForEmptyPaths)
{
    const auto tNumberOfRegisteredApps = register_plugin_apps({});
    EXPECT_EQ(tNumberOfRegisteredApps, 0u);
}

TEST(PluginCriteria, NonexistentSharedLibrary)
{
    const auto tTheBlobAppName = std::string{"the-blob"};
    const auto tConfigurationTempDirectory = create_test_app_configurations({tTheBlobAppName});
    const auto tNumKnownConfigurations = services::app_configurations({tConfigurationTempDirectory.directory()}).size();
    const auto tNumRegistered = register_plugin_apps({tConfigurationTempDirectory.directory()});
    EXPECT_NE(tNumRegistered, tNumKnownConfigurations);

    EXPECT_FALSE(library::is_criterion_function_registered(library::criterion_registration_name(
        input_parser::AppName{tTheBlobAppName}, input_parser::CriterionName{kTestCriterionName})));
    EXPECT_FALSE(library::is_parallel_criterion_function_registered(library::criterion_registration_name(
        input_parser::AppName{tTheBlobAppName}, input_parser::CriterionName{kTestCriterionName})));
}

TEST(PluginCriteria, RegisterApps)
{
    // Checks that some fake apps get registered via register_plugin_apps
    const auto tVampireAppName = std::string{"vampire"};
    const auto tMummyAppName = std::string{"mummy"};
    const auto tConfigurationTempDirectory =
        create_test_app_configurations_with_fake_shared_libs({tVampireAppName, tMummyAppName});
    const auto tNumRegistered = register_plugin_apps({tConfigurationTempDirectory.directory()});
    EXPECT_EQ(tNumRegistered, 2u);
    EXPECT_TRUE(library::is_parallel_criterion_function_registered(library::criterion_registration_name(
        input_parser::AppName{tVampireAppName}, input_parser::CriterionName{kTestCriterionName})));
    EXPECT_TRUE(library::is_parallel_criterion_function_registered(library::criterion_registration_name(
        input_parser::AppName{tMummyAppName}, input_parser::CriterionName{kTestCriterionName})));
}

TEST(PluginCriteria, ValidateValidApps)
{
    const auto tFrankensteinAppName = std::string{"frankenstein"};
    const auto tMedusaAppName = std::string{"medusa"};
    const auto tConfigurationTempDirectory =
        create_test_app_configurations_with_fake_shared_libs({tFrankensteinAppName, tMedusaAppName});
    const auto tNumRegistered = register_plugin_apps({tConfigurationTempDirectory.directory()});
    EXPECT_EQ(tNumRegistered, 2u);

    auto tCriteria = input_parser::objective{};
    EXPECT_TRUE(library::detail::validate_criterion_is_registered(tCriteria).has_value());
    tCriteria.app = input_parser::AppName{tFrankensteinAppName};
    tCriteria.criterion = input_parser::CriterionName{kTestCriterionName};
    EXPECT_FALSE(library::detail::validate_criterion_is_registered(tCriteria).has_value())
        << library::detail::validate_criterion_is_registered(tCriteria).value();
    tCriteria.app = input_parser::AppName{tMedusaAppName};
    EXPECT_FALSE(library::detail::validate_criterion_is_registered(tCriteria).has_value())
        << library::detail::validate_criterion_is_registered(tCriteria).value();
}

TEST(PluginCriteria, ValidateInvalidApp)
{
    const auto tWolfmanAppName = std::string{"wolfman"};
    const auto tConfigurationTempDirectory = create_test_app_configurations({tWolfmanAppName});
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
    const auto tConfigurationTempDirectory =
        create_test_app_configurations_with_fake_shared_libs({tMartianAppName, tCerberusAppName});
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
}

}  // namespace plato::criteria::extension::unittest
