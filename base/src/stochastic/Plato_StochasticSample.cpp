#include "Plato_StochasticSample.hpp"

#include <boost/functional/hash.hpp>

#include <numeric>
#include <vector>

namespace Plato
{
bool operator==(const Sample& aLHS, const Sample& aRHS)
{
    return aLHS.mValue == aRHS.mValue;
}

std::size_t vector_hash(const std::vector<double>& aVector)
{
    constexpr std::size_t tSeed = 0;
    return std::accumulate(aVector.cbegin(), aVector.cend(), tSeed, 
    [](std::size_t aCurSeed, const double aVal){
        const auto tHashedValue = std::hash<double>{}(aVal);
        boost::hash_combine(aCurSeed, tHashedValue);
        return aCurSeed;
    });
}
}
