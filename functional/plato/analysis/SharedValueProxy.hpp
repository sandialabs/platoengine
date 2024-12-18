#ifndef PLATO_ANALYSIS_SHAREDVALUEPROXY
#define PLATO_ANALYSIS_SHAREDVALUEPROXY

#include <cassert>
#include <functional>
#include <type_traits>
#include <utility>

#include "plato/utilities/FunctionArgType.hpp"

namespace plato::analysis
{
/// @brief Compile time check if an iterator refers to a const object.
template <typename IteratorType>
constexpr static bool kIsConstIterator =
    std::is_const_v<std::remove_reference_t<typename std::iterator_traits<IteratorType>::reference>>;

/// @brief A helper for mutable views of nodal scalar values.
///
/// Since AnalysisDomainMesh is organized in blocks, blocks sharing faces will share nodes on those faces.
/// AnalysisDomainMesh stores shared nodes in both blocks, and so assigning to a single value requires a proxy object.
/// This allows assignment to all shared nodes.
template <typename Value, typename InnerIteratorType, typename AssignmentPolicy = void>
struct SharedValueProxy
{
    static_assert(std::is_convertible_v<decltype(*std::declval<InnerIteratorType>()), Value>,
                  "The Value type must match the dereferenced iterator type.");

    /// @brief Assignment operator for assigning to all references held by this proxy.
    ///
    /// Disabled for const objects.
    template <typename Iterator = InnerIteratorType>
    auto operator=(const Value& aValue) -> std::enable_if_t<!kIsConstIterator<Iterator>, SharedValueProxy&>;

    /// @brief Overloaded assignment operator that uses the AssignmentPolicy
    ///
    /// This can be used to assign to a member of a struct without having to unwrap the proxy object.
    template <typename Arg, typename Iterator = InnerIteratorType>
    auto operator=(Arg&& aValue)
        -> std::enable_if_t<!kIsConstIterator<Iterator> && !std::is_same_v<AssignmentPolicy, void>, SharedValueProxy&>;

    /// @brief Conversion to a Value, the object that this proxy object represents.
    /// @pre This object must hold valid iterators.
    operator Value() const;

    /// @brief Returns whether or not this object holds any iterators.
    auto empty() const -> bool;

    std::vector<InnerIteratorType> mIterators;
};

template <typename Value, typename InnerIteratorType, typename AssignmentPolicy>
template <typename Iterator>
auto SharedValueProxy<Value, InnerIteratorType, AssignmentPolicy>::operator=(const Value& aValue)
    -> std::enable_if_t<!kIsConstIterator<Iterator>, SharedValueProxy&>
{
    for (auto& tValue : mIterators)
    {
        *tValue = aValue;
    }
    return *this;
}

template <typename Value, typename InnerIteratorType, typename AssignmentPolicy>
template <typename Arg, typename Iterator>
auto SharedValueProxy<Value, InnerIteratorType, AssignmentPolicy>::operator=(Arg&& aValue)
    -> std::enable_if_t<!kIsConstIterator<Iterator> && !std::is_same_v<AssignmentPolicy, void>, SharedValueProxy&>
{
    static_assert(std::is_convertible_v<Arg, typename utilities::FunctionArgType<AssignmentPolicy>::template arg<1U>>);

    auto tAssignmentFunction = AssignmentPolicy{};
    for (auto& tValue : mIterators)
    {
        tAssignmentFunction(*tValue, aValue);
    }
    return *this;
}

template <typename Value, typename InnerIteratorType, typename AssignmentPolicy>
SharedValueProxy<Value, InnerIteratorType, AssignmentPolicy>::operator Value() const
{
    assert(!mIterators.empty());
    return *mIterators.front();
}

template <typename Value, typename InnerIteratorType, typename AssignmentPolicy>
auto SharedValueProxy<Value, InnerIteratorType, AssignmentPolicy>::empty() const -> bool
{
    return mIterators.empty();
}

}  // namespace plato::analysis

#endif
