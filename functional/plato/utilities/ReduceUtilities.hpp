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

}  // namespace plato::utilities

#endif
