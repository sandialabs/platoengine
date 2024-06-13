#ifndef PLATO_UTILITIES_RANKDISTRIBUTEDVECTOR
#define PLATO_UTILITIES_RANKDISTRIBUTEDVECTOR

#include <array>
#include <boost/mpi/communicator.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <iterator>
#include <numeric>
#include <type_traits>
#include <vector>

#include "plato/utilities/NamedType.hpp"

namespace plato::utilities
{
using RankType = decltype(std::declval<const boost::mpi::communicator&>().rank());
using SizeType = decltype(std::declval<const boost::mpi::communicator&>().size());
using ColorType = int;
using RankNamedType = NamedType<RankType, struct RankTypeTag>;
using SizeNamedType = NamedType<SizeType, struct SizeTypeTag>;
using ColorNamedType = NamedType<ColorType, struct ColorTypeTag>;

/// @brief Splits the elements of @a aVector among available groups.
///
/// This attempts to distribute the elements as evenly as possible. If the number of elements is
/// evenly divisible by the number of groups, then the first `N` elements are distributed to
/// group 0, the next `N` to rank 1, and so on, where `N` is `aVector.size / num_ranks`.
///
/// If the number of elements is not divisible by the number of groups, then the remainder are
/// distributed one at a time to each group until all are exhausted, so that no rank will have
/// more than `ceil(aVector.size() / aSize)` elements.
///
/// @pre @a aGroupColor must be less than @a aSize
template <typename T>
[[nodiscard]] std::vector<T> group_split_vector(const std::vector<T>& aVector,
                                                const ColorNamedType aGroupColor,
                                                const SizeNamedType aSize);

/// @brief Determines the group color (ID) corresponding to @a aRank.
///
/// For a given set of group sizes (given in @a aGroupSizes ), the group ID (or color in MPI terminology)
/// is found by allocating each rank to a group in increasing order of rank. For example, if the group
/// sizes are `{3, 2, 1}`, then ranks 0, 1, and 2 will have color 0, ranks 3 and 4 will have color 1,
/// and rank 5 will have color 2.
///
/// @pre @a aRank must be less than the sum of the entries of @a aGroupSizes.
/// @pre All entries of @a aGroupSizes must be 1 or greater.
/// @post The returned color will be less than the size of @a aGroupSizes
template <typename T>
[[nodiscard]] ColorNamedType rank_group_color(const std::vector<unsigned int>& aGroupSizes, const RankNamedType aRank);

namespace detail
{
/// @brief Divides @a aNumElements by @a aNumRanks and returns the quotient in the first element and
///  the remainder in the second.
template <typename T, typename U>
auto num_elements_per_rank(const T aNumElements, const U aNumRanks) -> std::array<std::common_type_t<T, U>, 2>
{
    static_assert(std::is_integral_v<T>, "aNumElements must have an integer type.");
    static_assert(std::is_integral_v<U>, "aNumRanks must have an integer type.");
    return {aNumElements / aNumRanks, aNumElements % aNumRanks};
}

template <typename T>
bool assign_remainder_element_to_group(const ColorNamedType aGroupColor, const T aRemainder)
{
    static_assert(std::is_integral_v<T>, "aRemainder must have an integer type.");
    return static_cast<T>(aGroupColor.mValue) < aRemainder;
}
}  // namespace detail

template <typename T>
std::vector<T> group_split_vector(const std::vector<T>& aVector,
                                  const ColorNamedType aGroupColor,
                                  const SizeNamedType aSize)
{
    assert(aGroupColor.mValue < aSize.mValue);

    auto tDistributedVector = std::vector<T>{};
    const auto [tNumElementsPerRank, tRemainder] = detail::num_elements_per_rank(aVector.size(), aSize.mValue);
    const auto tFirstIndex = aGroupColor.mValue * tNumElementsPerRank;
    const auto tLastIndex = (aGroupColor.mValue + 1) * tNumElementsPerRank;
    tDistributedVector.reserve(tNumElementsPerRank);
    std::copy(std::next(aVector.cbegin(), tFirstIndex), std::next(aVector.cbegin(), tLastIndex),
              std::back_inserter(tDistributedVector));
    if (detail::assign_remainder_element_to_group(aGroupColor, tRemainder))
    {
        const int tNumDistributed = tNumElementsPerRank * aSize.mValue;
        const int tRemainderForRankIndex = tNumDistributed + aGroupColor.mValue;
        tDistributedVector.push_back(aVector.at(tRemainderForRankIndex));
    }
    return tDistributedVector;
}

ColorNamedType rank_group_color(const std::vector<unsigned int>& aGroupSizes, const RankNamedType aRank)
{
    assert(aRank.mValue >= 0);
    const auto tUnsignedRank = boost::numeric_cast<unsigned int>(aRank.mValue);
    [[maybe_unused]] const auto tTotalSize = std::accumulate(aGroupSizes.cbegin(), aGroupSizes.cend(), 0u);
    assert(tUnsignedRank < tTotalSize);

    auto tPartialSums = std::vector<unsigned int>{};
    tPartialSums.reserve(aGroupSizes.size());
    std::partial_sum(aGroupSizes.cbegin(), aGroupSizes.cend(), std::back_inserter(tPartialSums));
    const auto tGroupIter =
        std::find_if(tPartialSums.cbegin(), tPartialSums.cend(),
                     [tUnsignedRank](const auto aGroupTotal) { return tUnsignedRank < aGroupTotal; });
    const auto tGroupColor = std::distance(tPartialSums.cbegin(), tGroupIter);
    return ColorNamedType{boost::numeric_cast<ColorType>(tGroupColor)};
}

}  // namespace plato::utilities

#endif
