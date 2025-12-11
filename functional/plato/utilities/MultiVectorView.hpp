#ifndef PLATO_UTILITIES_MULTIDIMENSIONALVECTORVIEW
#define PLATO_UTILITIES_MULTIDIMENSIONALVECTORVIEW

#include <cassert>
#include <concepts>
#include <functional>
#include <span>

#include "plato/utilities/NamedType.hpp"

namespace plato::utilities
{

using VectorIndex = NamedType<std::size_t, struct VectorIndexTag>;
using ComponentIndex = NamedType<std::size_t, struct ComponentIndexTag>;

/// Template constraint for container-like types supported by MultiVectorView
template <typename Container>
concept MultiVectorViewContainer = requires(Container aContainer) {
    typename Container::value_type;
    typename Container::size_type;
    { aContainer.size() } -> std::convertible_to<typename Container::size_type>;
    { aContainer[typename Container::size_type{}] } -> std::convertible_to<typename Container::value_type>;
};

/// @brief Helper struct to propagate the constness of a container to its contained type
template <typename Container, typename = std::true_type>
struct ContainedTypeWithPropagatedConst
{
    using value_type = const typename Container::value_type;
};
template <typename Container>
struct ContainedTypeWithPropagatedConst<Container, std::false_type>
{
    using value_type = typename Container::value_type;
};

/// @brief The purpose of this view type is to facilitate indexing operations into a contiguous array that represents a
/// matrix-like 2D array of N-dimensional vectors, with dimension N known at run-time.
///
/// The main use-case is an array of points. For example, a `std::vector` may be adapted to store an array of points
/// using this view type as
/// @code{.cpp}
/// constexpr kDimension = std::size_t{3};
/// auto tPoints = std::vector<double>(kDimension * tNumberOfPoints);
/// auto tPointView = MultiVectorView{tPoints, kDimension};
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
template <MultiVectorViewContainer Container>
class MultiVectorView
{
   public:
    MultiVectorView() = default;

    /// @brief Construction from @a aContainer.
    /// @note This class holds a reference to @a aContainer, and so the lifetime of @a aContainer must exceed the
    /// lifetime of this object.
    /// @pre The size of the container must be an integer multiple of @a aDimensions.
    MultiVectorView(Container& aContainer, const std::size_t aDimensions);

    /// @brief The number of vectors.
    auto numberOfVectors() const -> std::size_t;

    /// @brief The total size of all entries, i.e., `mDimensions` times numberOfVectors.
    auto size() const -> std::size_t;

    /// @brief Accessor for the @a aComponentIndex component of the @a aVectorIndex vector.
    /// @pre @a aVectorIndex must be less than the size of the container held by the view times the number of
    /// dimensions.
    /// @pre @a aComponentIndex must be less than the size `kDimensions`
    auto operator()(VectorIndex aVectorIndex, ComponentIndex aComponentIndex) const -> decltype(auto);

    /// @brief Returns true if the data pointed to by this view and the dimension are equal
    [[nodiscard]] auto operator==(const MultiVectorView&) const -> bool;

   private:
    using value_type =
        typename ContainedTypeWithPropagatedConst<Container, typename std::is_const<Container>::type>::value_type;

    std::span<value_type> mContainer;
    std::size_t mDimensions = 0U;
};

/// @brief Helper function for creating a MultiVectorView and deducing the container type.
template <typename Container>
auto make_multi_vector_view(Container& aContainer, const std::size_t aDimensions) -> MultiVectorView<Container>
{
    return MultiVectorView<Container>{aContainer, aDimensions};
}

template <MultiVectorViewContainer Container>
MultiVectorView<Container>::MultiVectorView(Container& aContainer, const std::size_t aDimensions)
    : mContainer(aContainer.data(), aContainer.size()), mDimensions(aDimensions)
{
    static_assert(std::is_const_v<Container> == std::is_const_v<value_type>);
}

template <MultiVectorViewContainer Container>
auto MultiVectorView<Container>::numberOfVectors() const -> std::size_t
{
    return mContainer.size() / mDimensions;
}

template <MultiVectorViewContainer Container>
auto MultiVectorView<Container>::size() const -> std::size_t
{
    return mContainer.size();
}

template <MultiVectorViewContainer Container>
auto MultiVectorView<Container>::operator()(const VectorIndex aVectorIndex, const ComponentIndex aComponentIndex) const -> decltype(auto)
{
    assert(aComponentIndex.mValue < mDimensions);
    assert(aVectorIndex.mValue < numberOfVectors());
    return mContainer[aVectorIndex.mValue * mDimensions + aComponentIndex.mValue];
}

template <MultiVectorViewContainer Container>
auto MultiVectorView<Container>::operator==(const MultiVectorView& aOtherView) const -> bool
{
    return mContainer.data() == aOtherView.mContainer.data() && mDimensions == aOtherView.mDimensions;
}
}  // namespace plato::utilities

#endif
