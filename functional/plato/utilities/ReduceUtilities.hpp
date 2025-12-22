#ifndef PLATO_UTILITIES_REDUCEUTILITIES
#define PLATO_UTILITIES_REDUCEUTILITIES

#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>

namespace plato::utilities
{

/// @brief Using communicator @a aCommunicator, perform function @a aFunction on root and broadcast to other ranks
template <typename Value, typename Function>
[[nodiscard]] auto compute_on_root(const boost::mpi::communicator& aCommunicator, const Function& aFunction) -> Value;

/// @brief Take a vector on this rank @a aVector, and using the communicator @a aCommunicator return a reduced vector
/// that is the same on all ranks.
template <typename Type>
[[nodiscard]] auto reduce_vector(const std::vector<Type>& aVector,
                                 const boost::mpi::communicator& aCommunicator) -> std::vector<Type>;

/// @brief Take a vector on this rank @a aVector, and using the communicator @a aCommunicator gather up all the vectors,
/// sort them, and only keep the unique entries. Return a vector that is the same on all ranks.
template <typename Type>
[[nodiscard]] auto unique_vector_gather(const std::vector<Type>& aVector,
                                        const boost::mpi::communicator& aCommunicator) -> std::vector<Type>;

/// @brief Take a vector on this rank @a aVector, and using the communicator @a aCommunicator gather up all the vectors
/// into one sorted vector. This vector can have duplicate entries.
template <typename Type>
[[nodiscard]] auto merge_on_all_ranks(const std::vector<Type>& aVector,
                                      const boost::mpi::communicator& aCommunicator) -> std::vector<Type>;

/// @brief Take an unordered map @a aMap from each rank and reduce it so that all ranks in communicator @a aCommunicator
/// have the same map.
template <typename KeyType, typename ValueType>
[[nodiscard]] auto reduce_map(const std::unordered_map<KeyType, ValueType>& aMap,
                              const boost::mpi::communicator& aCommunicator) -> std::unordered_map<KeyType, ValueType>;

template <typename Value, typename Function>
auto compute_on_root(const boost::mpi::communicator& aCommunicator, const Function& aFunction) -> Value
{
    auto tResult = Value{};
    constexpr auto tRootRank = 0;
    if (aCommunicator.rank() == tRootRank)
    {
        tResult = aFunction();
    }
    boost::mpi::broadcast(aCommunicator, tResult, tRootRank);
    return tResult;
}

template <typename Type>
auto reduce_vector(const std::vector<Type>& aVector, const boost::mpi::communicator& aCommunicator) -> std::vector<Type>
{
    auto tVectorPlus = [](const std::vector<Type>& aLHS, const std::vector<Type>& aRHS) -> std::vector<Type>
    {
        std::vector<Type> tResult(aLHS.size(), 0.0);
        std::transform(aLHS.begin(), aLHS.end(), aRHS.begin(), tResult.begin(), std::plus<Type>());
        return tResult;
    };

    std::vector<Type> tGlobal(aVector.size(), 0.0);
    boost::mpi::all_reduce(aCommunicator, aVector, tGlobal, tVectorPlus);
    return tGlobal;
}

template <typename Type>
auto unique_vector_gather(const std::vector<Type>& aVector,
                          const boost::mpi::communicator& aCommunicator) -> std::vector<Type>
{
    auto tMergedSorted = merge_on_all_ranks(aVector, aCommunicator);
    return compute_on_root<std::vector<Type>>(aCommunicator,
                                              [&tMergedSorted]() -> std::vector<Type>
                                              {
                                                  auto tLastEntry =
                                                      std::unique(tMergedSorted.begin(), tMergedSorted.end());
                                                  tMergedSorted.erase(tLastEntry, tMergedSorted.end());
                                                  return tMergedSorted;
                                              });
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
    // https://github.com/boostorg/mpi/issues/166
    std::vector<Type> tConcatenatedData(tTotalSize);
    auto* tSendBuffer = aVector.empty() ? nullptr : aVector.data();
    // NOLINTBEGIN(bugprone-casting-through-void)
    MPI_Allgatherv(tSendBuffer, tSizes[aCommunicator.rank()], MPI_UINT64_T, tConcatenatedData.data(), tSizes.data(),
                   tOffsets.data(), MPI_UINT64_T, aCommunicator);
    // NOLINTEND(bugprone-casting-through-void)

    std::sort(tConcatenatedData.begin(), tConcatenatedData.end());

    return tConcatenatedData;
}

template <typename KeyType, typename ValueType>
auto reduce_map(const std::unordered_map<KeyType, ValueType>& aMap,
                const boost::mpi::communicator& aCommunicator) -> std::unordered_map<KeyType, ValueType>
{
    constexpr int tRootRank = 0;
    std::vector<std::unordered_map<KeyType, ValueType>> tGatheredMaps;
    boost::mpi::gather(aCommunicator, aMap, tGatheredMaps, tRootRank);

    return utilities::compute_on_root<std::unordered_map<KeyType, ValueType>>(
        aCommunicator,
        [&tGatheredMaps]()
        {
            std::unordered_map<KeyType, ValueType> tRootMap;
            for (const auto& tMap : tGatheredMaps)
            {
                for (const auto& [tEntityId, tLevelSetValue] : tMap)
                {
                    tRootMap[tEntityId] = tLevelSetValue;
                }
            }
            return tRootMap;
        });
}

}  // namespace plato::utilities

#endif
