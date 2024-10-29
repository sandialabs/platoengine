#ifndef PLATO_UTILITIES_MULTIDIMENSIONALVECTORVIEW
#define PLATO_UTILITIES_MULTIDIMENSIONALVECTORVIEW

#include <cassert>
#include <functional>

#include "plato/utilities/NamedType.hpp"

namespace plato::utilities
{

using VectorIndex = NamedType<std::size_t, struct VectorIndexTag>;
using ComponentIndex = NamedType<std::size_t, struct ComponentIndexTag>;

/// @brief The purpose of this view type is to facilitate indexing operations into a contiguous array that represents a
/// matrix-like 2D array of N-dimensional vectors, with dimension N known at compile-time.
///
/// The main use-case is an array of points. For example, a `std::vector` may be adapted to store an array of points
/// using this view type as
/// @code{.cpp}
/// constexpr kDimension = std::size_t{3};
/// auto tPoints = std::vector<double>(kDimension * tNumberOfPoints);
/// auto tPointView = MultiVectorView<kDimension>{tPoints};
/// tPointView(0, 0) = 0.0; // x-component of first point
/// tPointView(0, 1) = 0.0; // y-component of first point
/// tPointView(0, 2) = 0.0; // z-component of first point
/// ...
/// @endcode
///
/// The assumption is that the entries are arranged such that the components are contiguous, i.e. indexing the view with
/// vector index `m` and component index `n` indexes into the underlying container as `m * kDimensions + n`.
///
/// @tparam Container Must have an `operator[]` defined.
template <std::size_t kDimensions, typename Container>
class MultiVectorView
{
   public:
    /// @brief Construction from @a aContainer.
    /// @note This class holds a reference to @a aContainer, and so the lifetime of @a aContainer must exceed the
    /// lifetime of this object.
    /// @pre The size of the container must be an integer multiple of `kDimensions`.
    MultiVectorView(Container& aContainer);

    /// @brief The number of vectors.
    auto numberOfVectors() const -> std::size_t;

    /// @brief The total size of all entries, i.e., `kDimensions` times numberOfVectors.
    auto size() const -> std::size_t;

    /// @brief Accessor for the @a aComponentIndex component of the @a aVectorIndex vector.
    /// @pre @a aVectorIndex must be less than the size of the container held by the view times the number of
    /// dimensions.
    /// @pre @a aComponentIndex must be less than the size `kDimensions`
    auto& operator()(VectorIndex aVectorIndex, ComponentIndex aComponentIndex) const;
    auto& operator()(VectorIndex aVectorIndex, ComponentIndex aComponentIndex);

   private:
    std::reference_wrapper<Container> mContainer;
};

/// @brief Helper function for creating a MultiVectorView and deducing the container type.
template <std::size_t kDimensions, typename Container>
auto make_multi_vector_view(Container& aContainer) -> MultiVectorView<kDimensions, Container>
{
    return MultiVectorView<kDimensions, Container>{aContainer};
}

template <std::size_t kDimensions, typename Container>
MultiVectorView<kDimensions, Container>::MultiVectorView(Container& aContainer) : mContainer{aContainer}
{
    assert(aContainer.size() % kDimensions == 0);
}

template <std::size_t kDimensions, typename Container>
auto MultiVectorView<kDimensions, Container>::numberOfVectors() const -> std::size_t
{
    return mContainer.get().size() / kDimensions;
}

template <std::size_t kDimensions, typename Container>
auto MultiVectorView<kDimensions, Container>::size() const -> std::size_t
{
    return mContainer.get().size();
}

template <std::size_t kDimensions, typename Container>
auto& MultiVectorView<kDimensions, Container>::operator()(const VectorIndex aVectorIndex,
                                                          const ComponentIndex aComponentIndex) const
{
    assert(aComponentIndex.mValue < kDimensions);
    assert(aVectorIndex.mValue < numberOfVectors());
    return mContainer.get()[aVectorIndex.mValue * kDimensions + aComponentIndex.mValue];
}

template <std::size_t kDimensions, typename Container>
auto& MultiVectorView<kDimensions, Container>::operator()(const VectorIndex aVectorIndex,
                                                          const ComponentIndex aComponentIndex)
{
    const auto* const tConstThis = this;
    return (*tConstThis)(aVectorIndex, aComponentIndex);
}

}  // namespace plato::utilities

#endif
