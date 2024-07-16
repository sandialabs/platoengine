#include "plato/integration_tests/utilities/AppConfigurationTestUtilities.hpp"

namespace plato::integration_tests::utilities
{
namespace
{
constexpr auto kDefaultCriterionName = std::string_view{"default"};
constexpr auto kDefaultSerialFunctionName = std::string_view{"plato_create_criterion"};
constexpr auto kDefaultParallelFunctionName = std::string_view{"plato_create_parallel_criterion"};
}  // namespace

services::AppConfigurationWithDirectory test_app_configuration(const std::filesystem::path& aSharedLibPath)
{
    auto tSerialCriterion = services::CriterionConfiguration{
        /*.mName=*/std::string{kDefaultCriterionName},
        /*.mIsParallelized=*/false, /*.mFunctionName=*/std::string{kDefaultSerialFunctionName}};
    auto tParallelCriterion =
        services::CriterionConfiguration{/*.mName=*/std::string{kDefaultCriterionName}, /*.mIsParallelized=*/true,
                                         /*.mFunctionName=*/std::string{kDefaultParallelFunctionName}};
    auto tAppConfiguration = services::AppConfiguration{
        /*.mName=*/std::string{kDefaultCriterionName}, /*mLibraryName=*/aSharedLibPath.filename().string(),
        /*.mCriteria=*/{std::move(tSerialCriterion), std::move(tParallelCriterion)}};
    return {/*.mConfiguration=*/std::move(tAppConfiguration), /*.mLibraryDirectory=*/aSharedLibPath.parent_path()};
}
}  // namespace plato::integration_tests::utilities
