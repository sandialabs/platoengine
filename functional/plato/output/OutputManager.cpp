#include "plato/output/OutputManager.hpp"

#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::output
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
}  // namespace plato::output
