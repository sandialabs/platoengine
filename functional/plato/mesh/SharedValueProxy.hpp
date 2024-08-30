#ifndef PLATO_MESH_SHAREDVALUEPROXY
#define PLATO_MESH_SHAREDVALUEPROXY

#include <cassert>
#include <functional>
#include <type_traits>
#include <utility>

namespace plato::mesh
{
/// @brief Compile time check if an iterator refers to a const object.
template <typename IteratorType>
constexpr static bool kIsConstIterator =
    std::is_const_v<std::remove_reference_t<typename std::iterator_traits<IteratorType>::reference>>;

/// @brief A helper for mutable views of nodal scalar values.
///
/// Since MeshDesignVariables is organized in blocks, blocks sharing faces will share nodes on those faces.
/// MeshDesignVariables stores shared nodes in both blocks, and so assigning to a single value requires a proxy object.
/// This allows assignment to all shared nodes.
template <typename Value, typename InnerIteratorType>
struct SharedValueProxy
{
    static_assert(std::is_convertible_v<decltype(*std::declval<InnerIteratorType>()), Value>,
                  "The Value type must match the dereferenced iterator type.");

    /// @brief Assignment operator for assigning to all references held by this proxy.
    ///
    /// Disabled for const objects.
    template <typename Iterator = InnerIteratorType>
    auto operator=(const Value& aValue) -> std::enable_if_t<!kIsConstIterator<Iterator>, SharedValueProxy&>;

    /// @brief Conversion to a Density value, the object that this proxy object represents.
    operator Value() const;

    std::vector<InnerIteratorType> mIterators;
};

template <typename Value, typename InnerIteratorType>
template <typename Iterator>
auto SharedValueProxy<Value, InnerIteratorType>::operator=(const Value& aValue)
    -> std::enable_if_t<!kIsConstIterator<Iterator>, SharedValueProxy&>
{
    for (auto& tValue : mIterators)
    {
        *tValue = aValue;
    }
    return *this;
}

template <typename Value, typename InnerIteratorType>
SharedValueProxy<Value, InnerIteratorType>::operator Value() const
{
    assert(!mIterators.empty());
    return *mIterators.front();
}

}  // namespace plato::mesh

#endif
