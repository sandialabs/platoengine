#ifndef PLATO_UTILITIES_PAIRWISEACCUMULATE
#define PLATO_UTILITIES_PAIRWISEACCUMULATE

#include <iterator>
#include <numeric>

namespace plato::utilities
{

namespace detail
{
template <typename Iter>
double pair_wise_accumulate_impl(const Iter aBegin, const Iter aEnd)
{
    using DistanceType = typename std::iterator_traits<Iter>::difference_type;
    using ValueType = typename std::iterator_traits<Iter>::value_type;
    constexpr auto kMinSizeWhereAccumulateAccurate =
        DistanceType{32};  // 32 was empirically determined as the transition for summing double precision numbers where
                           // accumulate would produce round off errors. Since this algorithm splits the container in
                           // two, this number should be a multiple of 2.
    const auto tNumElements = std::distance(aBegin, aEnd);
    if (tNumElements <= kMinSizeWhereAccumulateAccurate)
    {
        return std::accumulate(aBegin, aEnd, ValueType{});
    }
    const auto tNumSplit = tNumElements / 2;
    const auto tSplitIter = std::next(aBegin, tNumSplit);
    return pair_wise_accumulate_impl(aBegin, tSplitIter) + pair_wise_accumulate_impl(tSplitIter, aEnd);
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
