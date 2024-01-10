#include "Plato_SampleIndexing.hpp"

namespace Plato
{

PerformerIndexIterator::PerformerIndexIterator(
    const NumPerformers aNumPerformers, NumSamples aNumSamples, const std::size_t aSampleIndex) :
    mNumPerformers(aNumPerformers.mValue),
    mNumSamples(aNumSamples.mValue),
    mSampleIndex(aSampleIndex)
{}

Plato::SampleIndices PerformerIndexIterator::operator*() const
{
    return {
        /*.mPerformerIndex = */ mSampleIndex % mNumPerformers, 
        /*.mSampleIndex = */ mSampleIndex % mNumSamples};
}

PerformerIndexIterator& PerformerIndexIterator::operator++()
{
    ++mSampleIndex;
    return *this;
}

bool PerformerIndexIterator::operator!=(const PerformerIndexIterator& rhs) const
{
    return 
        mNumPerformers != rhs.mNumPerformers ||
        mNumSamples != rhs.mNumSamples || 
        mSampleIndex != rhs.mSampleIndex;
}

SampleBatch::SampleBatch(const NumPerformers aNumPerformers, const NumSamples aNumSamples, const std::size_t aBatchIndex) : 
    mNumPerformers(aNumPerformers),
    mNumSamples(aNumSamples),
    mBatchIndex(aBatchIndex)
{}

PerformerIndexIterator SampleBatch::begin() const
{
    const std::size_t tPerformerIndexBegin = mBatchIndex * mNumPerformers.mValue;
    return PerformerIndexIterator{mNumPerformers, mNumSamples, tPerformerIndexBegin};
}

PerformerIndexIterator SampleBatch::end() const
{
    const std::size_t tPerformerIndexEnd = (mBatchIndex + 1) * mNumPerformers.mValue;
    return PerformerIndexIterator{mNumPerformers, mNumSamples, tPerformerIndexEnd};
}

SampleBatchIterator::SampleBatchIterator(
    const NumPerformers aNumPerformers, const NumSamples aNumSamples, const std::size_t aBatchIndex) :
    mNumPerformers(aNumPerformers),
    mNumSamples(aNumSamples),
    mBatchIndex(aBatchIndex)
{}

SampleBatch SampleBatchIterator::operator*() const
{
    return SampleBatch{mNumPerformers, mNumSamples, mBatchIndex};
}

SampleBatchIterator& SampleBatchIterator::operator++()
{
    ++mBatchIndex;
    return *this;
}

bool SampleBatchIterator::operator!=(const SampleBatchIterator& rhs) const
{
    return 
        mNumPerformers.mValue != rhs.mNumPerformers.mValue ||
        mNumSamples.mValue != rhs.mNumSamples.mValue ||
        mBatchIndex != rhs.mBatchIndex;
}

SampleIndexing::SampleIndexing(const NumPerformers aNumPerformers, const NumSamples aNumSamples) :
    mNumPerformers(aNumPerformers),
    mNumSamples(aNumSamples)
{}

SampleBatchIterator SampleIndexing::begin() const
{
    return SampleBatchIterator{mNumPerformers, mNumSamples, 0};
}

SampleBatchIterator SampleIndexing::end() const
{
    const auto tNumFullBatches = mNumSamples.mValue / mNumPerformers.mValue;
    const auto tNumRemainderEvals = mNumSamples.mValue % mNumPerformers.mValue;
    const auto tNumPartialBatches = tNumRemainderEvals > 0 ? 1 : 0;
    const auto tNumBatches = tNumFullBatches + tNumPartialBatches;
    return SampleBatchIterator{mNumPerformers, mNumSamples, tNumBatches};
}
}
