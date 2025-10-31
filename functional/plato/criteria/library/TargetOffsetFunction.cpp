#include "plato/criteria/library/TargetOffsetFunction.hpp"

#include <algorithm>

namespace plato::criteria::library
{
namespace
{
[[nodiscard]] auto offset_vector(linear_algebra::DynamicVector<double> aVector,
                                 const double aOffset) -> linear_algebra::DynamicVector<double>
{
    auto tValues = std::move(aVector).stdVector();
    std::ranges::transform(tValues, tValues.begin(), [aOffset](const auto tValue) { return tValue + aOffset; });
    return linear_algebra::DynamicVector<double>(std::move(tValues));
}

[[nodiscard]] auto identity_operation()
{
    return linear_algebra::JacobianMultiplier{[](const linear_algebra::DynamicVector<double>& aRowVector)
                                              { return aRowVector; }};
}

}  // namespace

auto make_target_offset_function(std::vector<double> aTargets) -> TargetOffsetFunction
{
    return TargetOffsetFunction{[mOffset = -1.0 * linear_algebra::DynamicVector<double>(std::move(aTargets))](
                                    const linear_algebra::DynamicVector<double>& aX) { return aX + mOffset; },
                                [](const linear_algebra::DynamicVector<double>&) { return identity_operation(); },
                                [](const linear_algebra::DynamicVector<double>&)
                                { return linear_algebra::AdjointJacobianMultiplier{identity_operation()}; }};
}

auto make_target_offset_function(const double aTarget) -> TargetOffsetFunction
{
    return TargetOffsetFunction{[mOffset = -aTarget](const linear_algebra::DynamicVector<double>& aX)
                                { return offset_vector(aX, mOffset); }, [](const linear_algebra::DynamicVector<double>&)
                                { return identity_operation(); }, [](const linear_algebra::DynamicVector<double>&)
                                { return linear_algebra::AdjointJacobianMultiplier{identity_operation()}; }};
}

}  // namespace plato::criteria::library
