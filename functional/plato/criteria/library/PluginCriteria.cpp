#include "plato/criteria/library/PluginCriteria.hpp"

#include <cstddef>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "plato/criteria/library/CriterionRegistration.hpp"
#include "plato/criteria/library/CriterionTraits.hpp"
#include "plato/criteria/library/SharedLibCriterion.hpp"
#include "plato/criteria/library/SharedLibraryVectorCriterion.hpp"
#include "plato/services/AppConfiguration.hpp"
#include "plato/services/AppConfigurationUtilities.hpp"
#include "plato/services/SystemLogger.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/OptionalToVector.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::criteria::library
{
namespace
{

/// @brief Function object for generating CriterionFunction objects.
///
/// This base template is specialized for different types of criterion factories, based on the criterion traits.
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
        return library::FunctionWithConfiguration{
            .mFunction = make_shared_lib_function(SharedLibCriterion{mAppConfiguration, mCriterionConfiguration, aInput,
                                                                     std::forward<Args>(aAdditionalArgs)...}),
            .mConfiguration = mCriterionConfiguration};
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
        return library::FunctionWithConfiguration{
            .mFunction = make_shared_library_vector_function(SharedLibraryVectorCriterion{
                mAppConfiguration, mCriterionConfiguration, aInput, std::forward<Args>(aAdditionalArgs)...}),
            .mConfiguration = mCriterionConfiguration};
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
        constexpr auto tCriterionTraits = library::traits_from_index(kFactoryIndex);
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
                             const std::index_sequence<kIndices...> /*unused*/)
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
        const auto tFactoryIndex = library::trait_index(tParallelization, tFunctionDimension);
        const auto tNameForRegistration =
            library::criterion_registration_name(aAppConfiguration.mConfiguration, tCriterionConfiguration);

        register_criterion_impl(tFactoryIndex, tNameForRegistration, aAppConfiguration, tCriterionConfiguration,
                                std::make_index_sequence<library::number_of_traits()>());
    }
}
}  // namespace

auto register_plugin_apps(const std::vector<std::filesystem::path>& aSearchDirectories) -> std::vector<std::string>
{
    const auto tAppConfigurations = services::app_configurations(aSearchDirectories);
    auto tRegisteredAppNames = utilities::reserved_container<std::vector<std::string>>(tAppConfigurations.size());

    for (const auto& tAppConfiguration : tAppConfigurations)
    {
        if (std::filesystem::exists(services::shared_library_path(tAppConfiguration)))
        {
            register_all_criteria(tAppConfiguration);
            tRegisteredAppNames.push_back(tAppConfiguration.mConfiguration.mName);
        }
        else
        {
            services::system_logger().logWarning(utilities::concatenate(
                "The shared library ", services::shared_library_path(tAppConfiguration), ", associated with app \"",
                tAppConfiguration.mConfiguration.mName, "\", was not found and will not be available."));
        }
    }
    return tRegisteredAppNames;
}

}  // namespace plato::criteria::library
