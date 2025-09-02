#include "plato/utilities/LogSpaceGenerator.hpp"

#include <algorithm>
#include <iostream>
#include <numeric>

namespace plato::utilities
{
std::vector<double> LogSpaceGenerator::steps() const
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
