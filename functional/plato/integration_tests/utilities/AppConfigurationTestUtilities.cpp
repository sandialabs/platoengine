#include "plato/integration_tests/utilities/AppConfigurationTestUtilities.hpp"

#include <filesystem>
#include <utility>

#include "plato/services/AppConfiguration.hpp"

namespace plato::integration_tests::utilities
{
namespace
{
constexpr auto kDefaultCriterionName = std::string_view{"default"};
constexpr auto kRuntimeThrowCriterionName = std::string_view{"runtime_throw"};
constexpr auto kDefaultSerialFunctionName = std::string_view{"plato_create_criterion"};
constexpr auto kDefaultParallelFunctionName = std::string_view{"plato_create_parallel_criterion"};
constexpr auto kDefaultVectorFunctionName = std::string_view{"plato_create_vector_criterion"};
constexpr auto kDefaultRuntimeThrowFunctionName = std::string_view{"plato_create_runtime_throw_criterion"};
constexpr auto kLibraryLoadFunctionName = std::string_view{"mass_startup"};
constexpr auto kLibraryUnloadFunctionName = std::string_view{"mass_shutdown"};
}  // namespace

auto test_app_configuration(const std::filesystem::path& aSharedLibPath) -> services::AppConfigurationWithDirectory
{
    auto tSerialCriterion = services::CriterionConfiguration{.mName = std::string{kDefaultCriterionName},
                                                             .mIsParallelized = false,
                                                             .mIsScalar = true,
                                                             .mFunctionName = std::string{kDefaultSerialFunctionName}};
    auto tParallelCriterion =
        services::CriterionConfiguration{.mName = std::string{kDefaultCriterionName},
                                         .mIsParallelized = true,
                                         .mIsScalar = true,
                                         .mFunctionName = std::string{kDefaultParallelFunctionName}};

    auto tVectorCriterion = services::CriterionConfiguration{.mName = std::string{kDefaultCriterionName},
                                                             .mIsParallelized = false,
                                                             .mIsScalar = false,
                                                             .mFunctionName = std::string{kDefaultVectorFunctionName}};

    auto tRuntimeThrowCriterion =
        services::CriterionConfiguration{.mName = std::string{kRuntimeThrowCriterionName},
                                         .mIsParallelized = false,
                                         .mIsScalar = true,
                                         .mFunctionName = std::string{kDefaultRuntimeThrowFunctionName}};

    auto tAppConfiguration =
        services::AppConfiguration{.mName = std::string{kDefaultCriterionName},
                                   .mLibraryFileName = aSharedLibPath.filename().string(),
                                   .mCriteria = {std::move(tSerialCriterion), std::move(tParallelCriterion),
                                                 std::move(tVectorCriterion), std::move(tRuntimeThrowCriterion)},
                                   .mLibraryLoadFunctionName = std::string{kLibraryLoadFunctionName},
                                   .mLibraryShutdownFunctionName = std::string{kLibraryUnloadFunctionName}};
    return {.mConfiguration = std::move(tAppConfiguration), .mLibraryDirectory = aSharedLibPath.parent_path()};
}

}  // namespace plato::integration_tests::utilities
