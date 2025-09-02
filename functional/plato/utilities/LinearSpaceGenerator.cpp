#include "plato/utilities/LinearSpaceGenerator.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace plato::utilities
{
auto LinearSpaceGenerator::steps() const -> std::vector<double>
{
    const auto tSize = static_cast<std::size_t>(std::max(1.0, std::floor((mEndingValue - mStartingValue) / mStep) + 1));
    std::vector<double> tLinearSpace(tSize, mStartingValue);
    std::generate_n(tLinearSpace.begin() + 1, tLinearSpace.size() - 1,
                    [tCurrentValue = mStartingValue, tStep = mStep]() mutable
                    { return tCurrentValue = tCurrentValue + tStep; });

    return tLinearSpace;
}

}  // namespace plato::utilities
