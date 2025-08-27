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

/// @brief Reads the entire contents of the file at path @a aPath to a string and returns it.
[[nodiscard]] auto file_to_string(const std::filesystem::path& aPath) -> std::string;

}  // namespace plato::test_utilities

#endif
