#include "plato/criteria/extension/PluginCriteria.hpp"

#include <filesystem>
#include <utility>

#include "plato/criteria/extension/SharedLibCriterion.hpp"
#include "plato/criteria/extension/SharedLibraryVectorCriterion.hpp"
#include "plato/services/AppConfiguration.hpp"
#include "plato/services/AppConfigurationUtilities.hpp"
#include "plato/services/PluginDirectoryPath.hpp"
#include "plato/utilities/OptionalToVector.hpp"

namespace plato::criteria::extension
{
namespace
{
static const auto kNumberOfPluginsLoaded =
    register_plugin_apps(utilities::optional_to_vector(services::plugin_directory_path()));

template <typename... Args>
[[nodiscard]] auto make_plugin_app_function(const services::AppConfigurationWithDirectory& aAppConfiguration,
                                            const services::CriterionConfiguration& aCriterionConfiguration,
                                            const criteria::library::CriterionInput& aInput,
                                            Args&&... aAdditionalArgs)
{
    return make_shared_lib_function(SharedLibCriterion{
        aAppConfiguration, aCriterionConfiguration, aInput.mInputFiles.mList, std::forward<Args>(aAdditionalArgs)...});
}

template <typename... Args>
[[nodiscard]] auto make_plugin_app_vector_function(const services::AppConfigurationWithDirectory& aAppConfiguration,
                                                   const services::CriterionConfiguration& aCriterionConfiguration,
                                                   const criteria::library::CriterionInput& aInput,
                                                   Args&&... aAdditionalArgs)
{
    return make_shared_library_vector_function(SharedLibraryVectorCriterion{
        aAppConfiguration, aCriterionConfiguration, aInput.mInputFiles.mList, std::forward<Args>(aAdditionalArgs)...});
}

void register_all_criteria(const services::AppConfigurationWithDirectory& aAppConfiguration)
{
    for (const auto& tCriterionConfiguration : aAppConfiguration.mConfiguration.mCriteria)
    {
        const auto tParallelization = library::to_parallelization(tCriterionConfiguration.mIsParallelized);
        const auto tFunctionDimension = library::to_function_dimension(tCriterionConfiguration.mIsScalar);
        const auto tFactoryIndex = library::detail::factory_index(tParallelization, tFunctionDimension);
        auto tNameForRegistration =
            library::criterion_registration_name(aAppConfiguration.mConfiguration, tCriterionConfiguration);

        if (tFactoryIndex ==
            library::detail::factory_index(library::Parallelization::kParallel, library::FunctionDimension::kScalar))
        {
            using Registration = library::CriterionRegistration<library::Parallelization::kParallel,
                                                                library::FunctionDimension::kScalar>;

            [[maybe_unused]] auto tAppRegistration = Registration{
                std::move(tNameForRegistration),
                [aAppConfiguration, tCriterionConfiguration](const criteria::library::CriterionInput& aInput,
                                                             const boost::mpi::communicator& aComm)
                { return make_plugin_app_function(aAppConfiguration, tCriterionConfiguration, aInput, aComm); }};
        }
        else if (tFactoryIndex ==
                 library::detail::factory_index(library::Parallelization::kSerial, library::FunctionDimension::kScalar))
        {
            using Registration =
                library::CriterionRegistration<library::Parallelization::kSerial, library::FunctionDimension::kScalar>;
            [[maybe_unused]] auto tAppRegistration = Registration{
                std::move(tNameForRegistration),
                [aAppConfiguration, tCriterionConfiguration](const criteria::library::CriterionInput& aInput)
                { return make_plugin_app_function(aAppConfiguration, tCriterionConfiguration, aInput); }};
        }
        else if (tFactoryIndex == library::detail::factory_index(library::Parallelization::kParallel,
                                                                 library::FunctionDimension::kVector))
        {
            using Registration = library::CriterionRegistration<library::Parallelization::kParallel,
                                                                library::FunctionDimension::kVector>;
            [[maybe_unused]] auto tAppRegistration = Registration{
                std::move(tNameForRegistration),
                [aAppConfiguration, tCriterionConfiguration](const criteria::library::CriterionInput& aInput,
                                                             const boost::mpi::communicator& aComm)
                { return make_plugin_app_vector_function(aAppConfiguration, tCriterionConfiguration, aInput, aComm); }};
        }
        else if (tFactoryIndex ==
                 library::detail::factory_index(library::Parallelization::kSerial, library::FunctionDimension::kVector))
        {
            using Registration =
                library::CriterionRegistration<library::Parallelization::kSerial, library::FunctionDimension::kVector>;
            [[maybe_unused]] auto tAppRegistration = Registration{
                std::move(tNameForRegistration),
                [aAppConfiguration, tCriterionConfiguration](const criteria::library::CriterionInput& aInput)
                { return make_plugin_app_vector_function(aAppConfiguration, tCriterionConfiguration, aInput); }};
        }
    }
}
}  // namespace

std::size_t number_of_plugins_registered_at_startup() { return kNumberOfPluginsLoaded; }

std::size_t register_plugin_apps(const std::vector<std::filesystem::path>& aSearchDirectories)
{
    const auto tAppConfigurations = services::app_configurations(aSearchDirectories);
    auto tNumberOfRegisteredApps = std::size_t{0};
    for (const auto& tAppConfiguration : tAppConfigurations)
    {
        if (std::filesystem::exists(services::shared_library_path(tAppConfiguration)))
        {
            register_all_criteria(tAppConfiguration);
            ++tNumberOfRegisteredApps;
        }
        else
        {
            std::cout << "Warning: The shared library " << services::shared_library_path(tAppConfiguration)
                      << ", associated with app \"" << tAppConfiguration.mConfiguration.mName
                      << "\", was not found and will not be available.\n";
        }
    }
    return tNumberOfRegisteredApps;
}

}  // namespace plato::criteria::extension
