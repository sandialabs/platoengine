#include "plato/test_utilities/FileCreatingTestFixture.hpp"

#include <mpi.h>

#include <iostream>

namespace plato::test_utilities
{
namespace
{

MPI_File open_file_with_mpi(const boost::mpi::communicator& aComm,
                            const std::filesystem::path& aFilePath,
                            const int aFileMode)
{
    auto tMPIFileHandle = MPI_File{};
    MPI_Comm tComm = aComm;
    const auto tErrorCode = MPI_File_open(tComm, aFilePath.c_str(), aFileMode, MPI_INFO_NULL, &tMPIFileHandle);
    if (tErrorCode != MPI_SUCCESS && aComm.rank() == 0)
    {
        std::cerr << "FileCreatingTestFixture non-zero mpi error code on file operation: " << tErrorCode << std::endl;
    }
    aComm.barrier();
    return tMPIFileHandle;
}

void create_file_with_mpi(const boost::mpi::communicator& aComm, const std::filesystem::path& aFilePath)
{
    auto tMPIFileHandle = open_file_with_mpi(aComm, aFilePath, MPI_MODE_RDWR | MPI_MODE_CREATE);
    MPI_File_close(&tMPIFileHandle);
}

void remove_file_with_mpi(const boost::mpi::communicator& aComm, const std::filesystem::path& aFilePath)
{
    auto tMPIFileHandle = open_file_with_mpi(aComm, aFilePath, MPI_MODE_RDWR | MPI_MODE_DELETE_ON_CLOSE);
    MPI_File_close(&tMPIFileHandle);
}
}  // namespace

FileCreatingTestFixture::FileCreatingTestFixture(std::filesystem::path aFilePath) : mFilePath{std::move(aFilePath)}
{
    create_file_with_mpi(mComm, mFilePath);
}

FileCreatingTestFixture::~FileCreatingTestFixture() { remove_file_with_mpi(mComm, mFilePath); }

const std::filesystem::path& FileCreatingTestFixture::filePath() const { return mFilePath; }

}  // namespace plato::test_utilities
