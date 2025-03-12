#include "plato/integration_tests/utilities/AppConfigurationTestUtilities.hpp"

namespace plato::integration_tests::utilities
{
namespace
{
constexpr auto kDefaultCriterionName = std::string_view{"default"};
constexpr auto kDefaultSerialFunctionName = std::string_view{"plato_create_criterion"};
constexpr auto kDefaultParallelFunctionName = std::string_view{"plato_create_parallel_criterion"};
constexpr auto kDefaultVectorFunctionName = std::string_view{"plato_create_vector_criterion"};
}  // namespace

auto test_app_configuration(const std::filesystem::path& aSharedLibPath) -> services::AppConfigurationWithDirectory
{
    auto tSerialCriterion =
        services::CriterionConfiguration{/*.mName=*/std::string{kDefaultCriterionName},
                                         /*.mIsParallelized=*/false, /*.mIsScalar=*/true,
                                         /*.mFunctionName=*/std::string{kDefaultSerialFunctionName}};
    auto tParallelCriterion = services::CriterionConfiguration{
        /*.mName=*/std::string{kDefaultCriterionName}, /*.mIsParallelized=*/true, /*.mIsScalar=*/true,
        /*.mFunctionName=*/std::string{kDefaultParallelFunctionName}};
    auto tVectorCriterion = services::CriterionConfiguration{
        /*.mName=*/std::string{kDefaultCriterionName}, /*.mIsParallelized=*/false, /*.mIsScalar=*/false,
        /*.mFunctionName=*/std::string{kDefaultVectorFunctionName}};

    auto tAppConfiguration = services::AppConfiguration{
        /*.mName=*/std::string{kDefaultCriterionName}, /*mLibraryName=*/aSharedLibPath.filename().string(),
        /*.mCriteria=*/{std::move(tSerialCriterion), std::move(tParallelCriterion), std::move(tVectorCriterion)}};
    return {/*.mConfiguration=*/std::move(tAppConfiguration), /*.mLibraryDirectory=*/aSharedLibPath.parent_path()};
}

}  // namespace plato::integration_tests::utilities
