#ifndef PLATO_UTILITIES_PAIRWISEACCUMULATE
#define PLATO_UTILITIES_PAIRWISEACCUMULATE

#include <iterator>
#include <numeric>

namespace plato::utilities
{
namespace detail
{
template <typename Iter>
    requires std::is_floating_point_v<typename std::iterator_traits<Iter>::value_type>
[[nodiscard]] auto pair_wise_accumulate_impl(const Iter aBegin, const Iter aEnd)
{
    using DistanceType = typename std::iterator_traits<Iter>::difference_type;
    using ValueType = typename std::iterator_traits<Iter>::value_type;

    constexpr auto kMinSizeWhereAccumulateAccurate = DistanceType{32};

    const auto tNumElements = std::distance(aBegin, aEnd);

    if (tNumElements <= kMinSizeWhereAccumulateAccurate)
    {
        return std::accumulate(aBegin, aEnd, ValueType{});
    }

    auto tBuffer = std::vector<ValueType>{};
    tBuffer.reserve(static_cast<std::size_t>(tNumElements));
    std::copy(aBegin, aEnd, std::back_inserter(tBuffer));

    auto tCurrentSize = tBuffer.size();
    while (tCurrentSize > 1)
    {
        auto tWriteIndex = std::size_t{0};
        auto tReadIndex = std::size_t{0};
        for (; (tReadIndex + 1) < tCurrentSize; tReadIndex += 2, ++tWriteIndex)
        {
            tBuffer[tWriteIndex] = tBuffer[tReadIndex] + tBuffer[tReadIndex + 1];
        }
        if (tReadIndex < tCurrentSize)
        {
            tBuffer[tWriteIndex] = tBuffer[tReadIndex];
            ++tWriteIndex;
        }
        tCurrentSize = tWriteIndex;
    }
    return tBuffer.front();
}
}  // namespace detail

/// @brief Performs a cascade sum over the entries in @a aContainer.
///
/// Both std::reduce and std::accumulate traverse the container and build up a summed value.
/// When a lot of the entries are similar in magnitude, the sum grows large enough that there is round off errors that
/// become significant. Pair wise accumulate repeatedly subdivides the container into two semi-equal sized container so
/// that you are summing two numbers of similar magnitude.
template <typename Container>
auto pair_wise_accumulate(const Container& aContainer)
{
    return detail::pair_wise_accumulate_impl(std::begin(aContainer), std::end(aContainer));
}

}  // namespace plato::utilities

#endif
