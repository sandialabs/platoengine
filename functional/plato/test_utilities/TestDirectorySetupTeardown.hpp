#ifndef PLATO_FUNCTIONAL_TEST_UTILITIES_TESTDIRECTORYSETUPTEARDOWN
#define PLATO_FUNCTIONAL_TEST_UTILITIES_TESTDIRECTORYSETUPTEARDOWN

#include <boost/mpi/communicator.hpp>
#include <filesystem>

namespace plato::test_utilities
{
/// @brief The purpose of this class is to facilitate creating and destroying a temporary directory
/// to hold test data using the RAII idiom.
///
/// On construction, a directory is created, which can then be populated with files
/// using the writeFile member. The directory and its contents are removed on destruction.
class [[nodiscard]] TestDirectorySetupTeardown
{
   public:
    /// @param aDirectory The directory to create, can be an absolute or relative path. For relative paths, it is
    /// created
    ///  relative to the current working directory.
    /// @param aComm For parallelized applications, only rank 0 will create the directory and write files.
    explicit TestDirectorySetupTeardown(std::filesystem::path aDirectory, const boost::mpi::communicator& aComm = {});
    ~TestDirectorySetupTeardown();

    /// @brief Write a file with name @a aFilename in the directory specified at construction.
    /// @param aWriteFunction Must be callable and have the signature `void(const std::filesystem::path&)` and is
    /// expected to write a file to the argument provided.
    /// @param aFilename A path relative to the test directory created on construction.
    /// @pre @a aFilename must be a relative path or filename, checked with an assertion.
    template <typename WriteFunction>
    const TestDirectorySetupTeardown& writeFile(const WriteFunction& aWriteFunction,
                                                const std::filesystem::path& aFilename) const;

    /// @brief Get the directory created on construction.
    [[nodiscard]] const std::filesystem::path& directory() const;

   private:
    std::filesystem::path mDirectory;
    boost::mpi::communicator mComm;
    static constexpr int kRootRank = 0;
};

template <typename WriteFunction>
const TestDirectorySetupTeardown& TestDirectorySetupTeardown::writeFile(const WriteFunction& aWriteFunction,
                                                                        const std::filesystem::path& aFilename) const
{
    assert(aFilename.is_relative());
    if (mComm.rank() == kRootRank)
    {
        aWriteFunction(mDirectory / aFilename);
    }
    mComm.barrier();
    return *this;
}

}  // namespace plato::test_utilities

#endif
