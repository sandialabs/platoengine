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
                    [n = mStartingMagnitude, r = mStepRatio]() mutable { return n = n * r; });
    return tLogspace;
}

}  // namespace plato::utilities
