#include "plato/test_utilities/FileCreatingTestFixture.hpp"

#include <boost/mpi.hpp>
#include <fstream>

namespace plato::test_utilities
{
template <typename F, typename... Args>
void execute_on_root(const F& aFunction, Args&&... aArgs)
{
    const auto tComm = boost::mpi::communicator{};
    if (tComm.rank() == 0)
    {
        aFunction(std::forward<Args>(aArgs)...);
    }
    tComm.barrier();
}

FileCreatingTestFixture::FileCreatingTestFixture(std::filesystem::path aFilePath) : mFilePath{std::move(aFilePath)}
{
    execute_on_root(
        [](const std::filesystem::path& aFilePath)
        {
            auto tStream = std::ofstream{aFilePath};
            tStream.close();
        },
        mFilePath);
}

FileCreatingTestFixture::~FileCreatingTestFixture()
{
    execute_on_root([](const std::filesystem::path& aFilePath) { std::filesystem::remove(aFilePath); }, mFilePath);
}

const std::filesystem::path& FileCreatingTestFixture::filePath() const { return mFilePath; }

}  // namespace plato::test_utilities
