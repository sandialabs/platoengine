#ifndef PLATO_TEST_UTILITIES_FILESYSTEMTESTUTILITY
#define PLATO_TEST_UTILITIES_FILESYSTEMTESTUTILITY

#include <filesystem>
#include <vector>

namespace plato::test_utilities
{
struct TestContext;
}

namespace plato::test_utilities
{
/// @brief Checks using gtest assertion macros that each file in @a aFilesToCheck exists on
///  disk and if so deletes them.
void test_for_existence_and_remove(const std::vector<std::filesystem::path>& aFilesToCheck,
                                   const TestContext& aTestContext);
}  // namespace plato::test_utilities

#endif
