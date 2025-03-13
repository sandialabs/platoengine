#ifndef PLATO_FUNCTIONAL_TEST_UTILITIES_TESTDIRECTORYSETUPTEARDOWN
#define PLATO_FUNCTIONAL_TEST_UTILITIES_TESTDIRECTORYSETUPTEARDOWN

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <optional>

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

    TestDirectorySetupTeardown(const TestDirectorySetupTeardown&) = delete;
    auto operator=(const TestDirectorySetupTeardown&) -> TestDirectorySetupTeardown& = delete;

    TestDirectorySetupTeardown(TestDirectorySetupTeardown&&) noexcept;
    auto operator=(TestDirectorySetupTeardown&&) noexcept -> TestDirectorySetupTeardown&;

    /// @brief Write a file with name @a aFilename in the directory specified at construction.
    /// @param aWriteFunction Must be callable and have the signature `void(const std::filesystem::path&)` and is
    /// expected to write a file to the argument provided.
    /// @param aFilename A path relative to the test directory created on construction.
    /// @pre @a aFilename must be a relative path or filename, checked with an assertion.
    template <typename WriteFunction>
    auto writeFile(const WriteFunction& aWriteFunction, const std::filesystem::path& aFilename) const
        -> const TestDirectorySetupTeardown&;

    /// @brief Get the directory created on construction.
    [[nodiscard]] auto directory() const -> const std::filesystem::path&;

   private:
    template <typename Function, typename... Args>
    void executeOnRootIfValid(const Function& aFunction, Args&&... aArgs) const;

    std::optional<std::filesystem::path> mDirectory;
    std::optional<boost::mpi::communicator> mComm;
    static constexpr int kRootRank = 0;
};

template <typename WriteFunction>
auto TestDirectorySetupTeardown::writeFile(const WriteFunction& aWriteFunction,
                                           const std::filesystem::path& aFilename) const
    -> const TestDirectorySetupTeardown&
{
    assert(aFilename.is_relative());
    executeOnRootIfValid(aWriteFunction, mDirectory.value_or("/") / aFilename);
    return *this;
}

template <typename Function, typename... Args>
void TestDirectorySetupTeardown::executeOnRootIfValid(const Function& aFunction, Args&&... aArgs) const
{
    if (!mDirectory.has_value() || !mComm.has_value())
    {
        return;
    }
    if (mComm->rank() == kRootRank)
    {
        aFunction(std::forward<Args>(aArgs)...);
    }
    mComm->barrier();
}

}  // namespace plato::test_utilities

#endif
