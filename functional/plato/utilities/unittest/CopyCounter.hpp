namespace plato::utilities::unittest
{

/// @brief A test utility for counting the number of copies and moves performed
struct CopyCounter
{
    unsigned int mCopies = 0;
    unsigned int mMoves = 0;

    CopyCounter() = default;

    CopyCounter(const CopyCounter& aCopy) : mCopies{aCopy.mCopies}, mMoves{aCopy.mMoves} { ++mCopies; }
    CopyCounter& operator=(const CopyCounter& aCopy)
    {
        mCopies = aCopy.mCopies;
        mMoves = aCopy.mMoves;
        ++mCopies;
        return *this;
    }

    CopyCounter(CopyCounter&& aCopy) noexcept : mCopies{aCopy.mCopies}, mMoves{aCopy.mMoves} { ++mMoves; }
    CopyCounter& operator=(CopyCounter&& aCopy) noexcept
    {
        mCopies = aCopy.mCopies;
        mMoves = aCopy.mMoves;
        ++mMoves;
        return *this;
    }
};
}  // namespace plato::utilities::unittest
