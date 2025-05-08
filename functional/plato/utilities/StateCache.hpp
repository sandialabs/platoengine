#ifndef PLATO_UTILITIES_STATECACHE
#define PLATO_UTILITIES_STATECACHE

#include <functional>
#include <optional>

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

    auto compute(Args...) -> const State&;

    /// @brief Returns `true` if compute has been called at least once, initializing the cache.
    [[nodiscard]] auto isInitialized() const -> bool;

   private:
    StateComputationFunction mComputeState;
    HashingFunction mGenerateHash;
    std::optional<std::size_t> mDesignHash;
    State mState;
};

template <typename State, typename... Args>
StateCache<State, Args...>::StateCache(StateComputationFunction aSF, HashingFunction aHF)
    : mComputeState(std::move(aSF)), mGenerateHash(std::move(aHF))
{
}

template <typename State, typename... Args>
auto StateCache<State, Args...>::compute(Args... aArgs) -> const State&
{
    const std::size_t tDesignHash = mGenerateHash(aArgs...);
    if (!mDesignHash.has_value() || tDesignHash != mDesignHash.value())
    {
        mState = mComputeState(aArgs...);
        mDesignHash = tDesignHash;
    }
    return mState;
}

template <typename State, typename... Args>
auto StateCache<State, Args...>::isInitialized() const -> bool
{
    return mDesignHash.has_value();
}

}  // namespace plato::utilities

#endif
