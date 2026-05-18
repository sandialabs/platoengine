#include "plato/utilities/LogSpaceGenerator.hpp"

#include <algorithm>
#include <vector>

namespace plato::utilities
{
auto LogSpaceGenerator::steps() const -> std::vector<double>
{
    if (mNumberOfSteps == 0)
    {
        return {};
    }
    std::vector<double> tLogspace(mNumberOfSteps, mStartingMagnitude);
    std::generate_n(tLogspace.begin() + 1, tLogspace.size() - 1,
                    [tCurrentMagnitude = mStartingMagnitude, tRatio = mStepRatio]() mutable
                    { return tCurrentMagnitude = tCurrentMagnitude * tRatio; });
    return tLogspace;
}

}  // namespace plato::utilities
