#include "plato/test_utilities/TestDirectorySetupTeardown.hpp"

namespace plato::test_utilities
{
TestDirectorySetupTeardown::TestDirectorySetupTeardown(std::filesystem::path aDirectory,
                                                       const boost::mpi::communicator& aComm)
    : mDirectory{std::move(aDirectory)}, mComm{aComm}
{
    if (mComm.rank() == kRootRank)
    {
        std::filesystem::create_directories(mDirectory);
    }
    mComm.barrier();
}

TestDirectorySetupTeardown::~TestDirectorySetupTeardown()
{
    if (mComm.rank() == kRootRank && !mDirectory.empty())
    {
        std::filesystem::remove_all(mDirectory);
    }
    mComm.barrier();
}

TestDirectorySetupTeardown::TestDirectorySetupTeardown(TestDirectorySetupTeardown&& aOther) noexcept
    : mDirectory{std::move(aOther.mDirectory)}, mComm{std::move(aOther.mComm)}
{
    aOther.mDirectory.clear();
}

TestDirectorySetupTeardown& TestDirectorySetupTeardown::operator=(TestDirectorySetupTeardown&& aOther) noexcept
{
    if (&aOther != this)
    {
        std::swap(aOther.mDirectory, mDirectory);
        std::swap(aOther.mComm, mComm);
    }
    return *this;
}

const std::filesystem::path& TestDirectorySetupTeardown::directory() const { return mDirectory; }

}  // namespace plato::test_utilities
