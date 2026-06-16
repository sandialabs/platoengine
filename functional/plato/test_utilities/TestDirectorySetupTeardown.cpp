#include "plato/test_utilities/TestDirectorySetupTeardown.hpp"

#include <boost/mpi/communicator.hpp>
#include <cassert>
#include <filesystem>
#include <utility>

namespace plato::test_utilities
{
TestDirectorySetupTeardown::TestDirectorySetupTeardown(std::filesystem::path aDirectory,
                                                       const boost::mpi::communicator& aCommunicator)
    : mDirectory{std::move(aDirectory)}, mComm{aCommunicator}
{
    executeOnRootIfValid([this]() { std::filesystem::create_directories(mDirectory.value()); });
}

TestDirectorySetupTeardown::~TestDirectorySetupTeardown()
{
    executeOnRootIfValid([this]() { std::filesystem::remove_all(mDirectory.value()); });
}

TestDirectorySetupTeardown::TestDirectorySetupTeardown(TestDirectorySetupTeardown&& aOther) noexcept
    : mDirectory{std::move(aOther.mDirectory)}, mComm{std::move(aOther.mComm)}
{
    aOther.mDirectory.reset();
}

auto TestDirectorySetupTeardown::operator=(TestDirectorySetupTeardown&& aOther) noexcept -> TestDirectorySetupTeardown&
{
    if (&aOther != this)
    {
        std::swap(aOther.mDirectory, mDirectory);
        std::swap(aOther.mComm, mComm);
    }
    return *this;
}

auto TestDirectorySetupTeardown::directory() const -> const std::filesystem::path&
{
    assert(mDirectory.has_value());
    return mDirectory.value();
}

}  // namespace plato::test_utilities
