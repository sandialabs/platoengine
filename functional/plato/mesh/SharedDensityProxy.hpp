#ifndef PLATO_MESH_SHAREDDENSITYPROXY
#define PLATO_MESH_SHAREDDENSITYPROXY

#include <cassert>
#include <functional>

#include "plato/mesh/MeshDesignVariables.hpp"

namespace plato::mesh
{
/// @brief Compile time check if an iterator refers to a const object.
template <typename IteratorType>
constexpr static bool kIsConstIterator =
    std::is_const_v<std::remove_reference_t<typename std::iterator_traits<IteratorType>::reference>>;

/// @brief A helper for mutable views of nodal densities.
///
/// Since MeshDesignVariables is organized in blocks, blocks sharing faces will share nodes on those faces.
/// MeshDesignVariables stores shared nodes in both blocks, and so assigning to a single value requires a proxy object.
/// This allows assignment to all shared nodes.
template <typename InnerIteratorType>
struct SharedDensityProxy
{
    /// @brief Assignment operator for assigning to all references held by this proxy.
    ///
    /// Disabled for non-const objects.
    template <typename Iterator = InnerIteratorType>
    auto operator=(const Density& aDensity) -> std::enable_if_t<!kIsConstIterator<Iterator>, SharedDensityProxy&>;

    /// @brief Conversion to a Density value, the object that this proxy object represents.
    operator Density() const;

    std::vector<InnerIteratorType> mIterators;
};

template <typename InnerIteratorType>
template <typename Iterator>
auto SharedDensityProxy<InnerIteratorType>::operator=(const Density& aDensity)
    -> std::enable_if_t<!kIsConstIterator<Iterator>, SharedDensityProxy&>
{
    for (auto& tMyDensity : mIterators)
    {
        *tMyDensity = aDensity;
    }
    return *this;
}

template <typename InnerIteratorType>
SharedDensityProxy<InnerIteratorType>::operator Density() const
{
    assert(!mIterators.empty());
    return *mIterators.front();
}

}  // namespace plato::mesh

#endif
