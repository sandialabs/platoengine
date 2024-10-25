#ifndef PLATO_UTILITIES_MULTIDIMENSIONALVECTORVIEW
#define PLATO_UTILITIES_MULTIDIMENSIONALVECTORVIEW

#include <cassert>
#include <functional>

namespace plato::utilities
{
/// @brief The purpose of this view type is to facilitate indexing operations into a contiguous array that represents a
/// matrix-like 2D array of N-dimensional vectors, with dimension N known at compile-time.
///
/// The main use-case is an array of points. For example, a `std::vector` may be adapted to store an array of points
/// using this view type as
/// @code{.cpp}
/// constexpr kDimension = std::size_t{3};
/// auto tPoints = std::vector<double>(kDimension * tNumberOfPoints);
/// auto tPointView = MultidimensionalView<kDimension>{tPoints};
/// tPointView(0, 0) = 0.0; // x-component of first point
/// tPointView(0, 1) = 0.0; // y-component of first point
/// tPointView(0, 2) = 0.0; // z-component of first point
/// ...
/// @endcode
template <std::size_t kDimensions, typename Container>
class MultidimensionalView
{
   public:
    MultidimensionalView(Container& aContainer);

    /// @brief The number of vectors.
    auto size() const -> std::size_t;

    /// @brief Accessor for the @a aComponentIndex component of the @a aVectorIndex vector.
    /// @pre @a aVectorIndex must be less than the size of the container held by the view times the number of
    /// dimensions.
    /// @pre @a aComponentIndex must be less than the size `kDimensions`
    auto& operator()(const std::size_t aVectorIndex, const std::size_t aComponentIndex) const;
    auto& operator()(const std::size_t aVectorIndex, const std::size_t aComponentIndex);

   private:
    std::reference_wrapper<Container> mContainer;
};

/// @brief Helper function for creating a MultidimensionalView and deducing the container type.
template <std::size_t kDimensions, typename Container>
auto make_multidimensional_view(Container& aContainer) -> MultidimensionalView<kDimensions, Container>
{
    return MultidimensionalView<kDimensions, Container>{aContainer};
}

template <std::size_t kDimensions, typename Container>
MultidimensionalView<kDimensions, Container>::MultidimensionalView(Container& aContainer) : mContainer{aContainer}
{
}

template <std::size_t kDimensions, typename Container>
auto MultidimensionalView<kDimensions, Container>::size() const -> std::size_t
{
    return mContainer.get().size() / kDimensions;
}

template <std::size_t kDimensions, typename Container>
auto& MultidimensionalView<kDimensions, Container>::operator()(const std::size_t aVectorIndex,
                                                               const std::size_t aComponentIndex) const
{
    assert(aComponentIndex < kDimensions);
    assert(aVectorIndex < size());
    return mContainer.get()[aVectorIndex * kDimensions + aComponentIndex];
}

template <std::size_t kDimensions, typename Container>
auto& MultidimensionalView<kDimensions, Container>::operator()(const std::size_t aVectorIndex,
                                                               const std::size_t aComponentIndex)
{
    const auto* const tConstThis = this;
    return (*tConstThis)(aVectorIndex, aComponentIndex);
}

}  // namespace plato::utilities

#endif
