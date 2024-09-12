#ifndef PLATO_TEST_UTILITIES_FILECREATINGTESTFIXTURE
#define PLATO_TEST_UTILITIES_FILECREATINGTESTFIXTURE

#include <gtest/gtest.h>

#include <boost/mpi.hpp>
#include <filesystem>

namespace plato::test_utilities
{
/// @brief A test fixture that creates an empty file at a given path on startup, and removes it when the test is
/// finished.
///
/// gtest requires an empty constructor for test fixtures, so the most convenient way to use this fixture is to
/// make a derived class in the test cpp file with an empty constructor that constructs this class with the correct file
/// path.
class FileCreatingTestFixture : public ::testing::Test
{
   public:
    FileCreatingTestFixture(std::filesystem::path aFilePath);
    ~FileCreatingTestFixture();

    const std::filesystem::path& filePath() const;

   private:
    std::filesystem::path mFilePath{};
    boost::mpi::communicator mComm{};
};
}  // namespace plato::test_utilities

#endif
