#ifndef PLATO_MESH_MESHPROXYVIEWS
#define PLATO_MESH_MESHPROXYVIEWS

#include <functional>

namespace plato::mesh
{
struct MeshProxy;
}

namespace plato::mesh
{
/// @brief An iterator type for using MeshProxyDensitiesView in std algorithms.
struct MeshProxyDensitiesViewIterator
{
    using iterator_type = std::vector<double>::const_iterator;

    using value_type = typename std::iterator_traits<iterator_type>::value_type;
    using iterator_category = std::input_iterator_tag;
    using difference_type = typename std::iterator_traits<iterator_type>::difference_type;
    using pointer = typename std::iterator_traits<iterator_type>::pointer;
    using reference = typename std::iterator_traits<iterator_type>::reference;

    MeshProxyDensitiesViewIterator& operator++();
    [[nodiscard]] const double& operator*() const;

    [[nodiscard]] bool operator==(const MeshProxyDensitiesViewIterator& aRHSIterator) const;
    [[nodiscard]] bool operator!=(const MeshProxyDensitiesViewIterator& aRHSIterator) const;

    iterator_type mIterator;
};

/// @brief The purpose of this object is to provide an interface with the density field in MeshProxy.
///
/// Its main use is for facilitating copying density data from MeshProxy to some other data structure,
/// such as that used by an external physics app.
struct MeshProxyDensitiesView
{
    std::reference_wrapper<const MeshProxy> mMeshProxy;

    [[nodiscard]] double operator[](unsigned) const;
    [[nodiscard]] std::size_t size() const;

    [[nodiscard]] MeshProxyDensitiesViewIterator begin() const;
    [[nodiscard]] MeshProxyDensitiesViewIterator end() const;
};

/// @brief Converts the densities associated with the mesh in @a aMeshView to a `std::vector`.
std::vector<double> to_vector(MeshProxyDensitiesView aMeshView);

}  // namespace plato::mesh

#endif
