#include "plato/test_utilities/CopyCounter.hpp"

namespace plato::test_utilities
{
CopyCounter::CopyCounter(const CopyCounter& aCopy) : mCopies{aCopy.mCopies}, mMoves{aCopy.mMoves} { ++mCopies; }

CopyCounter& CopyCounter::operator=(const CopyCounter& aCopy)
{
    mCopies = aCopy.mCopies;
    mMoves = aCopy.mMoves;
    ++mCopies;
    return *this;
}

CopyCounter::CopyCounter(CopyCounter&& aCopy) noexcept : mCopies{aCopy.mCopies}, mMoves{aCopy.mMoves} { ++mMoves; }

CopyCounter& CopyCounter::operator=(CopyCounter&& aCopy) noexcept
{
    mCopies = aCopy.mCopies;
    mMoves = aCopy.mMoves;
    ++mMoves;
    return *this;
}

}  // namespace plato::test_utilities
