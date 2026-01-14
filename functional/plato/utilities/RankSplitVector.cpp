#include "plato/utilities/RankSplitVector.hpp"

namespace plato::utilities
{

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
