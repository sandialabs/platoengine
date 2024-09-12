#ifndef PLATO_UTILITIES_STATECACHE
#define PLATO_UTILITIES_STATECACHE

#include <functional>

namespace plato::utilities
{
/// @brief An class that caches state to avoid recomputing unless a change in the design has occurred.
/// @tparam State is type that holds the state to be caches
/// @tparam Args are a set of arguments to the function that computes state as well as the function that computes the
/// hash.
/// In this implementation, the StateComputationFunction and HashingFunction must take the same arguments.
///
template <typename State, typename... Args>
class StateCache
{
   public:
    using StateComputationFunction = std::function<State(Args...)>;
    using HashingFunction = std::function<std::size_t(Args...)>;

    StateCache(StateComputationFunction aSF, HashingFunction aHF);

    State compute(Args...);

   private:
    StateComputationFunction mComputeState;
    HashingFunction mGenerateHash;
    std::size_t mDesignHash;
    State mState;
};

template <typename State, typename... Args>
StateCache<State, Args...>::StateCache(StateComputationFunction aSF, HashingFunction aHF)
    : mComputeState(std::move(aSF)), mGenerateHash(std::move(aHF))
{
}

template <typename State, typename... Args>
State StateCache<State, Args...>::compute(Args... aArgs)
{
    const std::size_t tDesignHash = mGenerateHash(aArgs...);
    if (tDesignHash != mDesignHash)
    {
        mState = mComputeState(aArgs...);
        mDesignHash = tDesignHash;
    }
    return mState;
}
}  // namespace plato::utilities

#endif