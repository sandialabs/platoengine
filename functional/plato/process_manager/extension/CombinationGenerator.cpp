#include "plato/process_manager/extension/CombinationGenerator.hpp"

#include <cassert>
#include <cstddef>
#include <ranges>
#include <vector>

#include "plato/utilities/Enumerate.hpp"

namespace plato::process_manager::extension
{

CombinationGenerator::CombinationGenerator(const StepValues& aStepValues,
                                           const SubIndexVector& aSubIndexVector,
                                           const std::vector<double>& aInitialValues)
    : mCartesianProduct(detail::sizes_vector(aStepValues).mValue),
      mCartesianProductIterator(mCartesianProduct.begin()),
      mStepValues(aStepValues),
      mSubIndices(aSubIndexVector.mValue),
      mCurrentStateValues(aInitialValues)
{
    assert(aStepValues.size() == aSubIndexVector.mValue.size());
}

auto CombinationGenerator::begin() -> CombinationGenerator
{
    CombinationGenerator tBeginGenerator = *this;
    tBeginGenerator.mCurrentStateValues = detail::state_values(
        tBeginGenerator.mCurrentStateValues, tBeginGenerator.mStepValues, SubIndexVector(tBeginGenerator.mSubIndices),
        IndexStateVector(*tBeginGenerator.mCartesianProductIterator));
    return tBeginGenerator;
}

auto CombinationGenerator::end() -> CombinationGenerator
{
    CombinationGenerator tEndGenerator = *this;
    tEndGenerator.mCartesianProductIterator = mCartesianProduct.end();
    tEndGenerator.mCurrentStateValues.clear();
    return tEndGenerator;
}

auto CombinationGenerator::operator*() const -> const std::vector<double>& { return mCurrentStateValues; }

auto CombinationGenerator::operator++() -> CombinationGenerator&
{
    ++mCartesianProductIterator;
    if (mCartesianProductIterator != mCartesianProduct.end())
    {
        mCurrentStateValues = detail::state_values(mCurrentStateValues, mStepValues, SubIndexVector(mSubIndices),
                                                   IndexStateVector(*mCartesianProductIterator));
    }
    else
    {
        mCurrentStateValues.clear();
    }
    return *this;
}

namespace detail
{
auto sizes_vector(const StepValues& aStepValues) -> IndexSizeVector
{
    std::vector<std::size_t> tSizesVector;
    tSizesVector.reserve(aStepValues.size());
    std::ranges::transform(aStepValues, std::back_inserter(tSizesVector),
                           [](const auto aStepValue) { return aStepValue.size(); });
    return IndexSizeVector{tSizesVector};
}

auto state_values(const std::vector<double>& aCurrentValues,
                  const StepValues& aStepValues,
                  const SubIndexVector& aSubIndexVector,
                  const IndexStateVector& aIndexStateVector) -> std::vector<double>
{
    std::vector<double> tStateVector = aCurrentValues;
    for (const auto& [tIndex, tSubIndex] : utilities::enumerate(aSubIndexVector.mValue))
    {
        assert(tSubIndex < tStateVector.size());
        assert(tIndex < aStepValues.size());
        tStateVector[tSubIndex] = aStepValues[tIndex][aIndexStateVector.mValue[tIndex]];
    }
    return tStateVector;
}

}  // namespace detail
}  // namespace plato::process_manager::extension
