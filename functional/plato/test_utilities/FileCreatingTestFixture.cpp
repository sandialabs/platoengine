#include "plato/test_utilities/FileCreatingTestFixture.hpp"

#include <fstream>

namespace plato::test_utilities
{
FileCreatingTestFixture::FileCreatingTestFixture(std::filesystem::path aFilePath) : mFilePath{std::move(aFilePath)}
{
    auto tStream = std::ofstream{mFilePath};
    tStream.close();
}

FileCreatingTestFixture::~FileCreatingTestFixture() { std::filesystem::remove(mFilePath); }

const std::filesystem::path& FileCreatingTestFixture::filePath() const { return mFilePath; }

}  // namespace plato::test_utilities
