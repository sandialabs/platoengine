#ifndef PLATO_TEST_UTILITIES_COPYCOUNTER
#define PLATO_TEST_UTILITIES_COPYCOUNTER

namespace plato::test_utilities
{

/// @brief A test utility for counting the number of copies and moves performed
///
/// This is useful for testing whether or not certain operations result in a copy or a move.
struct CopyCounter
{
    unsigned int mCopies = 0;
    unsigned int mMoves = 0;

    CopyCounter() = default;

    CopyCounter(const CopyCounter& aCopy);
    CopyCounter& operator=(const CopyCounter& aCopy);

    CopyCounter(CopyCounter&& aCopy) noexcept;
    CopyCounter& operator=(CopyCounter&& aCopy) noexcept;
};
}  // namespace plato::test_utilities

#endif
