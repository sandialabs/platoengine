#ifndef PLATO_UTILITIES_SINGLEDIMENSIONMULTIVECTORRANGE
#define PLATO_UTILITIES_SINGLEDIMENSIONMULTIVECTORRANGE

#include <ranges>

#include "plato/utilities/MultiVectorView.hpp"

namespace plato::utilities
{
/// @brief Given an existing MultiVectorView, this provides a new range that iterates over a single dimension only. This
/// facilitates iterating over, e.g., only y-coordinates in a MultiVectorView representing points.
///
/// As an example, given a MultiVectorView constructed as:
/// @code{.cpp}
/// constexpr kDimensions = std::size_t{3};
/// auto tPoints = std::vector<double>(kDimensions * tNumberOfPoints);
/// auto tPointView = MultiVectorView{tPoints, kDimensions};
/// @endcode
/// a SingleDimensionMultiVectorView is then constructed as
/// @code{.cpp}
/// constexpr kYDimension = 1U;
/// auto tYView = SingleDimensionMultiVectorView{tPointView, kYDimension};
/// @endcode
/// which then may be used to iterate only over the y-dimension.
///
/// @note This is similar to `std::views::stride`, which it should be replaced with when upgrading to c++23.
/// @sa MultiVectorView
template <MultiVectorViewContainer Container>
class SingleDimensionMultiVectorView : public std::ranges::view_interface<SingleDimensionMultiVectorView<Container>>
{
   public:
    SingleDimensionMultiVectorView(MultiVectorView<Container> aMultiVectorView, std::size_t aDimension);

    class Iterator;

    struct EndSentinel
    {
        [[nodiscard]] auto operator==(const Iterator&) const -> bool;
        [[nodiscard]] auto operator!=(const Iterator&) const -> bool;
    };

    class Iterator
    {
       public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = typename Container::value_type;
        using size_type = typename Container::size_type;
        using difference_type = typename Container::difference_type;
        using pointer = typename Container::value_type*;
        using reference = typename Container::reference;

        Iterator(MultiVectorView<Container> aMultiVectorView, std::size_t aDimension);

        [[nodiscard]] auto operator*() const -> decltype(auto);

        auto operator++() -> Iterator&;
        auto operator++(int) -> Iterator;
        auto operator--() -> Iterator&;
        auto operator--(int) -> Iterator;

        [[nodiscard]] auto operator+(difference_type aIncrement) const -> Iterator;
        [[nodiscard]] auto operator-(difference_type aDecrement) const -> Iterator;
        [[nodiscard]] auto operator-(const Iterator& aIterator) const -> difference_type;

        auto operator+=(difference_type aIncrement) -> Iterator&;
        auto operator-=(difference_type aDecrement) -> Iterator&;

        [[nodiscard]] auto operator[](size_type aIndex) const -> decltype(auto);

        [[nodiscard]] auto operator<=>(const Iterator&) const = default;
        [[nodiscard]] auto operator==(const EndSentinel) const -> bool;
        [[nodiscard]] auto operator!=(const EndSentinel) const -> bool;

       private:
        MultiVectorView<Container> mMultiVectorView;
        size_type mDimension = 0U;
        difference_type mCurrent = 0;
    };

    [[nodiscard]] auto begin() const -> Iterator;
    [[nodiscard]] auto end() const -> EndSentinel;

   private:
    MultiVectorView<Container> mMultiVectorView;
    std::size_t mDimension;
};

template <MultiVectorViewContainer Container>
SingleDimensionMultiVectorView<Container>::SingleDimensionMultiVectorView(MultiVectorView<Container> aMultiVectorView,
                                                                          const std::size_t aDimension)
    : mMultiVectorView{std::move(aMultiVectorView)}, mDimension{aDimension}
{
}

template <MultiVectorViewContainer Container>
SingleDimensionMultiVectorView<Container>::Iterator::Iterator(MultiVectorView<Container> aMultiVectorView,
                                                              const std::size_t aDimension)
    : mMultiVectorView{std::move(aMultiVectorView)}, mDimension{aDimension}, mCurrent{0U}
{
}

template <MultiVectorViewContainer Container>
[[nodiscard]] auto SingleDimensionMultiVectorView<Container>::Iterator::operator*() const -> decltype(auto)
{
    return mMultiVectorView(VectorIndex{static_cast<size_type>(mCurrent)}, ComponentIndex{mDimension});
}

template <MultiVectorViewContainer Container>
auto SingleDimensionMultiVectorView<Container>::Iterator::operator++() -> Iterator&
{
    ++mCurrent;
    return *this;
}

template <MultiVectorViewContainer Container>
auto SingleDimensionMultiVectorView<Container>::Iterator::operator++(int) -> Iterator
{
    auto tCurrentState = *this;
    operator++();
    return tCurrentState;
}

template <MultiVectorViewContainer Container>
auto SingleDimensionMultiVectorView<Container>::Iterator::operator--() -> Iterator&
{
    --mCurrent;
    return *this;
}

template <MultiVectorViewContainer Container>
auto SingleDimensionMultiVectorView<Container>::Iterator::operator--(int) -> Iterator
{
    auto tCurrentState = *this;
    operator--();
    return tCurrentState;
}

template <MultiVectorViewContainer Container>
auto SingleDimensionMultiVectorView<Container>::Iterator::operator+(const difference_type aIncrement) const -> Iterator
{
    auto tIncrementedIterator = *this;
    tIncrementedIterator.mCurrent += aIncrement;
    return tIncrementedIterator;
}

template <MultiVectorViewContainer Container>
auto SingleDimensionMultiVectorView<Container>::Iterator::operator-(const difference_type aDecrement) const -> Iterator
{
    auto tDecrementedIterator = *this;
    tDecrementedIterator.mCurrent -= aDecrement;
    return tDecrementedIterator;
}

template <MultiVectorViewContainer Container>
auto SingleDimensionMultiVectorView<Container>::Iterator::operator-(const Iterator& aIterator) const -> difference_type
{
    return mCurrent - aIterator.mCurrent;
}

template <MultiVectorViewContainer Container>
auto SingleDimensionMultiVectorView<Container>::Iterator::operator+=(const difference_type aIncrement) -> Iterator&
{
    mCurrent += aIncrement;
    return *this;
}

template <MultiVectorViewContainer Container>
auto SingleDimensionMultiVectorView<Container>::Iterator::operator-=(const difference_type aDecrement) -> Iterator&
{
    mCurrent -= aDecrement;
    return *this;
}

template <MultiVectorViewContainer Container>
auto SingleDimensionMultiVectorView<Container>::Iterator::operator[](const size_type aIndex) const -> decltype(auto)
{
    return mMultiVectorView(VectorIndex{aIndex}, ComponentIndex{mDimension});
}

template <MultiVectorViewContainer Container>
auto SingleDimensionMultiVectorView<Container>::Iterator::operator==(const EndSentinel) const -> bool
{
    return mCurrent == static_cast<difference_type>(mMultiVectorView.numberOfVectors());
}

template <MultiVectorViewContainer Container>
auto SingleDimensionMultiVectorView<Container>::Iterator::operator!=(const EndSentinel) const -> bool
{
    return !operator==(EndSentinel{});
}

template <MultiVectorViewContainer Container>
auto SingleDimensionMultiVectorView<Container>::EndSentinel::operator==(
    const typename SingleDimensionMultiVectorView<Container>::Iterator& aIterator) const -> bool
{
    return aIterator == *this;
}

template <MultiVectorViewContainer Container>
auto SingleDimensionMultiVectorView<Container>::EndSentinel::operator!=(
    const typename SingleDimensionMultiVectorView<Container>::Iterator& aIterator) const -> bool
{
    return !operator==(aIterator);
}

template <MultiVectorViewContainer Container>
auto SingleDimensionMultiVectorView<Container>::begin() const -> Iterator
{
    return Iterator{mMultiVectorView, mDimension};
}

template <MultiVectorViewContainer Container>
auto SingleDimensionMultiVectorView<Container>::end() const -> EndSentinel
{
    return EndSentinel{};
}

}  // namespace plato::utilities

#endif
