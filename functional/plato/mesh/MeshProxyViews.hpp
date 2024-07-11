#ifndef PLATO_MESH_MESHPROXYVIEWS
#define PLATO_MESH_MESHPROXYVIEWS

#include <functional>

namespace plato::mesh
{
struct MeshProxy;
}

namespace plato::mesh
{
struct MeshProxyDensitiesViewIterator
{
    using iterator_type = std::vector<double>::const_iterator;

    using value_type = typename std::iterator_traits<iterator_type>::value_type;
    using iterator_category = typename std::iterator_traits<iterator_type>::iterator_category;
    using difference_type = typename std::iterator_traits<iterator_type>::difference_type;
    using pointer = MeshProxyDensitiesViewIterator*;
    using reference = MeshProxyDensitiesViewIterator&;

    MeshProxyDensitiesViewIterator& operator++();
    [[nodiscard]] const double& operator*();

    [[nodiscard]] bool operator==(const MeshProxyDensitiesViewIterator&) const;
    [[nodiscard]] bool operator!=(const MeshProxyDensitiesViewIterator&) const;

    iterator_type mIterator;
};

struct MeshProxyDensitiesView
{
    std::reference_wrapper<const MeshProxy> mMeshProxy;

    [[nodiscard]] double operator[](unsigned) const;
    [[nodiscard]] std::size_t size() const;

    [[nodiscard]] MeshProxyDensitiesViewIterator begin() const;
    [[nodiscard]] MeshProxyDensitiesViewIterator end() const;
};
}  // namespace plato::mesh

#endif
