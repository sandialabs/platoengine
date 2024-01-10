#pragma once

#include "Plato_StochasticSample.hpp"

#include <functional>
#include <optional>

namespace Plato
{

/// @brief Class template for associating a sample with a response.
///
/// The sample is a stochastic parameter sample from some distribution, stored as
/// a vector. The response is typically an evaluation of an objective function or
/// the objective function gradient.
///
/// @tparam StoredType The type of the response, typically a scalar double or some vector.
template<typename StoredType>
class SampleResponseCache
{
public:
    /// @brief Inserts a sample @a aKey and response @a aValue into the cache.
    /// @post Calling `get` with @a aKey will return a non-empty optional with value @a aValue.
    void insert(Plato::Sample aKey, StoredType aValue);

    /// @brief Inserts all @a aKeys and @a aValues into the cache as if by calling insert in a loop
    /// @pre The size of @a aKeys and @a aValues must be the same, checked by assertion.
    /// @post Calling get with any entry in @a aKey will return a non-empty optional with value @a aValue.
    void insert(std::vector<Plato::Sample> aKeys, std::vector<StoredType> aValues);

    /// @brief Retrieves the response associated with @a key.
    /// @return An empty optional is returned if @a key has not been inserted previously.
    [[nodiscard]] std::optional<std::reference_wrapper<const StoredType>> get(const Plato::Sample& aKey) const;

    /// @brief Deletes all cache entries.
    void clear();
private:
    std::unordered_map<Plato::Sample, StoredType> mCache;
};

template<typename StoredType>
void SampleResponseCache<StoredType>::insert(Sample aKey, StoredType aValue)
{
    mCache.emplace(std::move(aKey), std::move(aValue));
}

template<typename StoredType>
void SampleResponseCache<StoredType>::insert(std::vector<Plato::Sample> aKeys, std::vector<StoredType> aValues)
{
    assert(aKeys.size() == aValues.size());
    for(std::size_t index = 0; index < aKeys.size(); ++index)
    {
        mCache.emplace(std::move(aKeys[index]), std::move(aValues[index]));
    }
}

template<typename StoredType>
auto SampleResponseCache<StoredType>::get(const Sample& aKey) const -> std::optional<std::reference_wrapper<const StoredType>>
{
    const auto tIter = mCache.find(aKey);
    if(tIter != mCache.end())
    {
        return std::cref(tIter->second);
    }
    else
    {
        return std::nullopt;
    }
}

template<typename StoredType>
void SampleResponseCache<StoredType>::clear()
{
    mCache.clear();
}

}
