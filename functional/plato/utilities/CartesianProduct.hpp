#ifndef PLATO_UTILITIES_CARTESIANPRODUCT
#define PLATO_UTILITIES_CARTESIANPRODUCT

#include <compare>
#include <vector>

namespace plato::utilities
{
class CartesianProductIterator;
///@brief A cartesian product generator... Think of a really strange odometer where the digits are not base ten but are
/// specified by the sizes provided.
class CartesianProduct
{
   public:
    ///@brief Create a cartesian product generator that can traverse the combinations specified in the size vector @a
    /// aSizes.
    CartesianProduct(const std::vector<std::size_t>& aSizes);

    [[nodiscard]] auto begin() const -> CartesianProductIterator;
    [[nodiscard]] auto end() const -> CartesianProductIterator;
    [[nodiscard]] auto operator<=>(const CartesianProduct&) const = default;

   private:
    std::vector<std::size_t> mMaxSizes;

    friend class CartesianProductIterator;
};

class CartesianProductIterator
{
   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = std::vector<std::size_t>;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type*;
    using reference = const value_type&;

    CartesianProductIterator(std::vector<std::size_t> aSizes);
    static CartesianProductIterator end(const std::vector<std::size_t>& aSizes);

    [[nodiscard]] reference operator*() const;
    [[nodiscard]] pointer operator->() const;

    CartesianProductIterator& operator++();
    CartesianProductIterator operator++(int);
    [[nodiscard]] auto operator<=>(const CartesianProductIterator&) const = default;

   private:
    std::vector<std::size_t> mMaxSizes;
    std::vector<std::size_t> mCurrentState;
    std::size_t mCurrentLinearIndex = 0;
};

}  // namespace plato::utilities

#endif
