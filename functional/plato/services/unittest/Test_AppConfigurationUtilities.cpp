#include <gtest/gtest.h>

#include "plato/services/AppConfigurationUtilities.hpp"

namespace plato::services::unittest
{
TEST(AppConfigurationUtilities, SharedLibraryPath)
{
    const auto tTestPath = std::filesystem::path{"/fake/path/to/"};
    const auto tTestLibName = std::filesystem::path{"libpatterns.so"};
    const auto tPaisleyCriterion =
        CriterionConfiguration{/*.mName=*/"paisley", /*.mIsParallelized=*/false, /*.mFunctionName=*/"paisley_function"};
    const auto tConfiguration = AppConfiguration{/*.mName=*/"patterns",
                                                 /*.mLibraryFileName=*/tTestLibName.string(),
                                                 {tPaisleyCriterion}};
    EXPECT_EQ(tTestPath / tTestLibName, shared_library_path(AppConfigurationWithDirectory{tConfiguration, tTestPath}));
}

}  // namespace plato::services::unittest
