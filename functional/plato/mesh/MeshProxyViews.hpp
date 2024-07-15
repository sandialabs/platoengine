#ifndef PLATO_MESH_MESHPROXYVIEWS
#define PLATO_MESH_MESHPROXYVIEWS

#include <functional>
#include <optional>

#include "plato/mesh/MeshProxy.hpp"

namespace plato::mesh
{
template <typename IteratorType>
constexpr static bool kIsConstIterator =
    std::is_const_v<std::remove_reference_t<typename std::iterator_traits<IteratorType>::reference>>;

/// @brief An iterator type for using MeshProxyDensitiesView in std algorithms.
template <typename OuterIteratorType, typename InnerIteratorType, typename IteratorCategory>
struct MeshProxyDensitiesViewIterator
{
    using OuterIterator = OuterIteratorType;
    using InnerIterator = InnerIteratorType;

    using value_type = typename std::iterator_traits<InnerIteratorType>::value_type;
    using iterator_category = IteratorCategory;
    using difference_type = typename std::iterator_traits<InnerIteratorType>::difference_type;
    using pointer = typename std::iterator_traits<InnerIteratorType>::pointer;
    using reference = typename std::iterator_traits<InnerIteratorType>::reference;

    MeshProxyDensitiesViewIterator& operator++();
    [[nodiscard]] const reference operator*() const;

    template <typename Iterator = OuterIteratorType>
    [[nodiscard]] auto operator*() -> std::enable_if_t<!kIsConstIterator<Iterator>, reference>;

    [[nodiscard]] bool operator==(const MeshProxyDensitiesViewIterator& aRHSIterator) const;
    [[nodiscard]] bool operator!=(const MeshProxyDensitiesViewIterator& aRHSIterator) const;

    OuterIteratorType mOuterIterator;
    OuterIteratorType mOuterIteratorEnd;
    std::optional<InnerIteratorType> mInnerIterator;
};

namespace detail
{
template <typename MeshProxyType>
struct IteratorType
{
};

template <>
struct IteratorType<MeshProxy>
{
    using type = MeshProxyDensitiesViewIterator<MeshProxy::BlockDensities::iterator,
                                                MeshProxy::DensityVector::iterator,
                                                std::output_iterator_tag>;
};

template <>
struct IteratorType<const MeshProxy>
{
    using type = MeshProxyDensitiesViewIterator<MeshProxy::BlockDensities::const_iterator,
                                                MeshProxy::DensityVector::const_iterator,
                                                std::forward_iterator_tag>;
};
}  // namespace detail

/// @brief The purpose of this object is to provide an interface for the density field in MeshProxy.
///
/// Its main use is for facilitating copying density data from MeshProxy to some other data structure,
/// such as that used by an external physics app.
template <typename MeshProxyType>
struct MeshProxyDensitiesViewTemplate
{
    std::reference_wrapper<MeshProxyType> mMeshProxy;

    [[nodiscard]] std::size_t size() const;

    using IteratorType = typename detail::IteratorType<MeshProxyType>::type;
    [[nodiscard]] IteratorType begin() const;
    [[nodiscard]] IteratorType end() const;
};

using MeshProxyDensitiesView = MeshProxyDensitiesViewTemplate<const MeshProxy>;
using MeshProxyDensitiesMutableView = MeshProxyDensitiesViewTemplate<MeshProxy>;

/// @brief Converts the densities associated with the mesh in @a aMeshView to a `std::vector`.
std::vector<Density> mesh_proxy_to_vector(MeshProxyDensitiesView aMeshView);

/// @brief Stores the densities in @a aDensities in @a aMeshProxy
/// @note This overload assumes a `0` to `N-1` ordering of densities, i.e., no node/element map.
MeshProxy vector_to_mesh_proxy(const std::vector<double>& aDensities, MeshProxy&& aMeshProxy);

/// @brief Splits a vector of Density objects into the density values and node/element map.
auto split_densities(const std::vector<Density>& aDensities)
    -> std::pair<std::vector<double>, std::vector<std::size_t>>;

template <typename OuterIteratorType, typename InnerIteratorType, typename IteratorCategory>
template <typename Iterator>
auto MeshProxyDensitiesViewIterator<OuterIteratorType, InnerIteratorType, IteratorCategory>::operator*()
    -> std::enable_if_t<!kIsConstIterator<Iterator>, reference>
{
    assert(mInnerIterator.has_value());
    return *mInnerIterator.value();
}

}  // namespace plato::mesh

#endif
