#include <gtest/gtest.h>

#include "plato/main/library/BuildInfo.hpp"

namespace plato::main::library::unittest
{

TEST(BuildInfo, BuildCommit)
{
    const auto tCommit = git_commit_hash();
    EXPECT_FALSE(tCommit.empty());
    EXPECT_NE(tCommit, "@PLATO_GIT_COMMIT_HASH@");
}

TEST(BuildInfo, ProjectVersion)
{
    const auto tProjectVersion = project_version();
    EXPECT_FALSE(tProjectVersion.empty());
    EXPECT_NE(tProjectVersion, "@PROJECT_VERSION@");
}

}  // namespace plato::main::library::unittest
