#ifndef PLATO_UTILITIES_REDUCEUTILITIES
#define PLATO_UTILITIES_REDUCEUTILITIES

#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/vector.hpp>

namespace plato::utilities
{
template <typename Type>
[[nodiscard]] auto unique_vector_gather(const std::vector<Type>& aVector,
                                        const boost::mpi::communicator& aCommunicator) -> std::vector<Type>;

template <typename Type>
[[nodiscard]] auto merge_on_all_ranks(const std::vector<Type>& aVector,
                                      const boost::mpi::communicator& aCommunicator) -> std::vector<Type>;

template <typename Type>
auto unique_vector_gather(const std::vector<Type>& aVector,
                          const boost::mpi::communicator& aCommunicator) -> std::vector<Type>
{
    std::vector<std::vector<Type>> tGatheredData;
    constexpr int tRootRank = 0;

    boost::mpi::gather(aCommunicator, aVector, tGatheredData, tRootRank);
    std::vector<Type> tConcatenatedData;
    if (aCommunicator.rank() == tRootRank)
    {
        for (const auto& tSubData : tGatheredData)
        {
            tConcatenatedData.insert(tConcatenatedData.end(), tSubData.begin(), tSubData.end());
        }
        std::sort(tConcatenatedData.begin(), tConcatenatedData.end());

        auto tLastEntry = std::unique(tConcatenatedData.begin(), tConcatenatedData.end());
        tConcatenatedData.erase(tLastEntry, tConcatenatedData.end());
    }
    boost::mpi::broadcast(aCommunicator, tConcatenatedData, tRootRank);
    return tConcatenatedData;
}

template <typename Type>
auto merge_on_all_ranks(const std::vector<Type>& aVector,
                        const boost::mpi::communicator& aCommunicator) -> std::vector<Type>
{
    std::vector<int> tSizes;
    boost::mpi::all_gather(aCommunicator, static_cast<int>(aVector.size()), tSizes);
    const auto tTotalSize = std::accumulate(tSizes.cbegin(), tSizes.cend(), 0);

    auto tOffsets = std::vector<int>{0};
    tOffsets.reserve(tSizes.size());
    std::partial_sum(tSizes.cbegin(), tSizes.cend(), std::back_inserter(tOffsets));

    // boost::mpi::all_gatherv will dereference a zero-length vector and so it is not usable if aVector is empty,
    // so we're using the raw MPI_Allgatherv instead.
    std::vector<Type> tConcatenatedData(tTotalSize);
    auto* tSendBuffer = aVector.empty() ? nullptr : aVector.data();
    MPI_Allgatherv(tSendBuffer, tSizes[aCommunicator.rank()], MPI_UINT64_T, tConcatenatedData.data(), tSizes.data(),
                   tOffsets.data(), MPI_UINT64_T, aCommunicator);

    std::sort(tConcatenatedData.begin(), tConcatenatedData.end());

    return tConcatenatedData;
}

}  // namespace plato::utilities

#endif
