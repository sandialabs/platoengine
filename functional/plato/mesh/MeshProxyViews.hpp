#ifndef PLATO_MESH_MESHPROXYVIEWS
#define PLATO_MESH_MESHPROXYVIEWS

#include <functional>

namespace plato::mesh
{
struct MeshProxy;
}

namespace plato::mesh
{
template <typename IteratorType>
constexpr static bool kIsConstIterator =
    std::is_const_v<std::remove_reference_t<typename std::iterator_traits<IteratorType>::reference>>;

/// @brief An iterator type for using MeshProxyDensitiesView in std algorithms.
template <typename IteratorType, typename IteratorCategory>
struct MeshProxyDensitiesViewIterator
{
    using value_type = typename std::iterator_traits<IteratorType>::value_type;
    using iterator_category = IteratorCategory;
    using difference_type = typename std::iterator_traits<IteratorType>::difference_type;
    using pointer = typename std::iterator_traits<IteratorType>::pointer;
    using reference = typename std::iterator_traits<IteratorType>::reference;

    MeshProxyDensitiesViewIterator& operator++();
    [[nodiscard]] const double& operator*() const;

    template <typename Iterator = IteratorType>
    [[nodiscard]] auto operator*() -> std::enable_if_t<!kIsConstIterator<Iterator>, double&>;

    [[nodiscard]] bool operator==(const MeshProxyDensitiesViewIterator& aRHSIterator) const;
    [[nodiscard]] bool operator!=(const MeshProxyDensitiesViewIterator& aRHSIterator) const;

    IteratorType mIterator;
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
    using type = MeshProxyDensitiesViewIterator<std::vector<double>::iterator, std::output_iterator_tag>;
};

template <>
struct IteratorType<const MeshProxy>
{
    using type = MeshProxyDensitiesViewIterator<std::vector<double>::const_iterator, std::forward_iterator_tag>;
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

    [[nodiscard]] double operator[](unsigned aIndex) const;
    [[nodiscard]] std::size_t size() const;

    using IteratorType = typename detail::IteratorType<MeshProxyType>::type;
    [[nodiscard]] IteratorType begin() const;
    [[nodiscard]] IteratorType end() const;
};

using MeshProxyDensitiesView = MeshProxyDensitiesViewTemplate<const MeshProxy>;
using MeshProxyDensitiesMutableView = MeshProxyDensitiesViewTemplate<MeshProxy>;

/// @brief Converts the densities associated with the mesh in @a aMeshView to a `std::vector`.
std::vector<double> to_vector(MeshProxyDensitiesView aMeshView);

template <typename IteratorType, typename IteratorCategory>
template <typename Iterator>
auto MeshProxyDensitiesViewIterator<IteratorType, IteratorCategory>::operator*()
    -> std::enable_if_t<!kIsConstIterator<Iterator>, double&>
{
    return *mIterator;
}

}  // namespace plato::mesh

#endif
