#include "plato/criteria/extension/PluginCriteria.hpp"

#include <filesystem>
#include <utility>

#include "plato/criteria/extension/SharedLibCriterion.hpp"
#include "plato/criteria/extension/SharedLibraryVectorCriterion.hpp"
#include "plato/criteria/library/CriterionRegistration.hpp"
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

template <library::FunctionDimension kDimension>
struct CreateCriterionFunction
{
};

template <>
struct CreateCriterionFunction<library::FunctionDimension::kScalar>
{
    services::AppConfigurationWithDirectory mAppConfiguration;
    services::CriterionConfiguration mCriterionConfiguration;

    template <typename... Args>
    [[nodiscard]] auto operator()(const criteria::library::CriterionInput& aInput, Args&&... aAdditionalArgs) const
    {
        return make_shared_lib_function(SharedLibCriterion{mAppConfiguration, mCriterionConfiguration,
                                                           aInput.mInputFiles.mList,
                                                           std::forward<Args>(aAdditionalArgs)...});
    }
};

template <>
struct CreateCriterionFunction<library::FunctionDimension::kVector>
{
    services::AppConfigurationWithDirectory mAppConfiguration;
    services::CriterionConfiguration mCriterionConfiguration;

    template <typename... Args>
    [[nodiscard]] auto operator()(const criteria::library::CriterionInput& aInput, Args&&... aAdditionalArgs) const
    {
        return make_shared_library_vector_function(
            SharedLibraryVectorCriterion{mAppConfiguration, mCriterionConfiguration, aInput.mInputFiles.mList,
                                         std::forward<Args>(aAdditionalArgs)...});
    }
};

template <std::size_t kFactoryIndex>
auto register_criterion_impl(const std::size_t aFactoryIndexToInstantiate,
                             const std::string& aNameForRegistration,
                             const services::AppConfigurationWithDirectory& aAppConfiguration,
                             const services::CriterionConfiguration& aCriterionConfiguration) -> bool
{
    const auto tShouldInstantiate = kFactoryIndex == aFactoryIndexToInstantiate;
    if (tShouldInstantiate)
    {
        using Registration = library::detail::FactoryRegistrationWithTraits<kFactoryIndex>;
        constexpr auto tCriterionTraits = library::detail::factory_traits_from_index(kFactoryIndex);
        [[maybe_unused]] auto tAppRegistration = Registration{
            aNameForRegistration,
            CreateCriterionFunction<tCriterionTraits.mDimension>{aAppConfiguration, aCriterionConfiguration}};
    }
    return tShouldInstantiate;
}

template <std::size_t... kIndices>
void register_criterion_impl(const std::size_t aFactoryIndexToInstantiate,
                             const std::string& aNameForRegistration,
                             const services::AppConfigurationWithDirectory& aAppConfiguration,
                             const services::CriterionConfiguration& aCriterionConfiguration,
                             const std::index_sequence<kIndices...>)
{
    (register_criterion_impl<kIndices>(aFactoryIndexToInstantiate, aNameForRegistration, aAppConfiguration,
                                       aCriterionConfiguration) ||
     ...);
}

void register_all_criteria(const services::AppConfigurationWithDirectory& aAppConfiguration)
{
    for (const auto& tCriterionConfiguration : aAppConfiguration.mConfiguration.mCriteria)
    {
        const auto tParallelization = library::to_parallelization(tCriterionConfiguration.mIsParallelized);
        const auto tFunctionDimension = library::to_function_dimension(tCriterionConfiguration.mIsScalar);
        const auto tFactoryIndex = library::detail::factory_index(tParallelization, tFunctionDimension);
        const auto tNameForRegistration =
            library::criterion_registration_name(aAppConfiguration.mConfiguration, tCriterionConfiguration);

        register_criterion_impl(tFactoryIndex, tNameForRegistration, aAppConfiguration, tCriterionConfiguration,
                                std::make_index_sequence<library::detail::number_of_factories()>());
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
