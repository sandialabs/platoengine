#include "plato/geometry/library/OutputManager.hpp"

#include "plato/geometry/library/OutputInfo.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::geometry::library
{
OutputManager::OutputManager(OutputFunction aOutputFunction, OutputMode aOutputMode)
    : mOutputFunction{std::move(aOutputFunction)}, mOutputMode{aOutputMode}
{
}

void OutputManager::output(const linear_algebra::DynamicVector<double>& aDesignVariables)
{
    if (mOutputMode == OutputMode::kNever)
    {
        return;
    }

    const auto tShouldOverwrite = mOutputMode == OutputMode::kEveryIterationOverwrite || mIteration == 0;
    mOutputFunction(aDesignVariables, {tShouldOverwrite, ++mIteration});
}
}  // namespace plato::geometry::library
