#pragma once

#include "Plato_NamedType.hpp"

#include <cstddef>
#include <iterator>

namespace Plato
{

using NumPerformers = Plato::Utils::NamedType<std::size_t, struct NumPerformersTag>;
using NumSamples = Plato::Utils::NamedType<std::size_t, struct NumSamplesTag>;

struct SampleIndices
{
    std::size_t mPerformerIndex;
    std::size_t mSampleIndex;
};

class PerformerIndexIterator
{
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = SampleIndices;

public:
    PerformerIndexIterator(const NumPerformers aNumPerformers, const NumSamples aNumSamples, const std::size_t aSampleIndex);

    [[nodiscard]] SampleIndices operator*() const;
    PerformerIndexIterator& operator++();
    [[nodiscard]] bool operator!=(const PerformerIndexIterator& rhs) const;

private:
    std::size_t mNumPerformers;
    std::size_t mNumSamples;
    std::size_t mSampleIndex;
};

/// @brief Provides iterators for looping over sample indices in a batch
///  provided by SampleIndexing.
class SampleBatch
{
public:
    SampleBatch(const NumPerformers aNumPerformers, const NumSamples aNumSamples, const std::size_t aBatchIndex);

    [[nodiscard]] PerformerIndexIterator begin() const;
    [[nodiscard]] PerformerIndexIterator end() const;

private:
    NumPerformers mNumPerformers;
    NumSamples mNumSamples;
    std::size_t mBatchIndex;
};

class SampleBatchIterator
{
public:
    using iterator_category = std::input_iterator_tag;
    using value_type = SampleBatch;

public:
    SampleBatchIterator(NumPerformers aNumPerformers, NumSamples aNumSamples, const std::size_t aBatchIndex);

    [[nodiscard]] SampleBatch operator*() const;
    SampleBatchIterator& operator++();
    [[nodiscard]] bool operator!=(const SampleBatchIterator& rhs) const;

private:
    NumPerformers mNumPerformers;
    NumSamples mNumSamples;
    std::size_t mBatchIndex;
};

/// @brief The purpose of this class is to facilitate indexing samples in a
///  parallel setting.
///
/// The iterators provide two levels of looping, an outer loop over batches and
/// an inner loop over actual samples. The number of batches is `ceil(aNumSamples / aNumPerformers)`.
/// If the number of samples is not divisible by the number of performers, then 
/// the last batch will loop back and start at the beginning. This may duplicate work,
/// but will work best for code structures requiring all performers perform some task
/// in parallel.
///
/// For example:
/// @code{.cpp}
/// for(const SampleBatch tBatch : SampleIndexing{tNumPerformers, tNumSamples}) {
///   for(const std::size_t tSampleIndex : tBatch) {
///     ...
///   }
//  }
/// @endcode
class SampleIndexing
{
public:
    SampleIndexing(NumPerformers aNumPerformers, NumSamples aNumSamples);

    [[nodiscard]] SampleBatchIterator begin() const;
    [[nodiscard]] SampleBatchIterator end() const;

private:
    NumPerformers mNumPerformers;
    NumSamples mNumSamples;
};

}
