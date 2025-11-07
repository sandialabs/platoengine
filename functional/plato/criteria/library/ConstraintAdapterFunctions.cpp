#include "plato/criteria/library/ConstraintAdapterFunctions.hpp"

#include <algorithm>

#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::criteria::library
{
namespace
{
[[nodiscard]] auto vector_subset(const linear_algebra::DynamicVector<double>& aVector,
                                 const std::set<std::size_t>& aIndices)
{
    assert(aIndices.size() <= aVector.size());

    auto tSubsetVector = utilities::reserved_container<std::vector<double>>(aIndices.size());
    std::ranges::transform(aIndices, std::back_inserter(tSubsetVector),
                           [&aVector](const std::size_t aIndex) { return aVector[aIndex]; });
    return tSubsetVector;
}

[[nodiscard]] auto make_vector_subset_evaluation_function(std::set<std::size_t> aIndices)
{
    return [mIndices = std::move(aIndices)](const linear_algebra::DynamicVector<double>& aVector)
    { return linear_algebra::DynamicVector<double>(vector_subset(aVector, mIndices)); };
}

[[nodiscard]] auto make_vector_subset_jacobian_product(const std::set<std::size_t>& aIndices,
                                                       const std::size_t aNumberOfColumns)
{
    return linear_algebra::JacobianMultiplier{
        [aIndices, aNumberOfColumns](const linear_algebra::DynamicVector<double>& aRowVector)
        {
            assert(aRowVector.size() == aIndices.size());
            auto tMultiplicationResult = std::vector<double>(aNumberOfColumns, 0.0);
            for (const auto [tRowVectorEntry, tResultIndex] : utilities::Zip{aRowVector.stdVector(), aIndices})
            {
                tMultiplicationResult[tResultIndex] = tRowVectorEntry;
            }
            return linear_algebra::DynamicVector<double>(std::move(tMultiplicationResult));
        }};
}

[[nodiscard]] auto offset_vector(linear_algebra::DynamicVector<double> aVector, const double aOffset)
    -> linear_algebra::DynamicVector<double>
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

auto make_vector_subset_function(const std::set<std::size_t>& aIndices) -> ConstraintAdapterFunction
{
    return ConstraintAdapterFunction{
        make_vector_subset_evaluation_function(aIndices),
        [aIndices](const linear_algebra::DynamicVector<double>& aVector)
        { return make_vector_subset_jacobian_product(aIndices, aVector.size()); },
        [aIndices](const linear_algebra::DynamicVector<double>&)
        { return linear_algebra::make_adjoint_jacobian_multiplier(make_vector_subset_evaluation_function(aIndices)); }};
}

auto make_target_offset_function(std::vector<double> aTargets) -> ConstraintAdapterFunction
{
    return ConstraintAdapterFunction{[mOffset = -1.0 * linear_algebra::DynamicVector<double>(std::move(aTargets))](
                                         const linear_algebra::DynamicVector<double>& aX) { return aX + mOffset; },
                                     [](const linear_algebra::DynamicVector<double>&) { return identity_operation(); },
                                     [](const linear_algebra::DynamicVector<double>&)
                                     { return linear_algebra::AdjointJacobianMultiplier{identity_operation()}; }};
}

auto make_target_offset_function(const double aTarget) -> ConstraintAdapterFunction
{
    return ConstraintAdapterFunction{[mOffset = -aTarget](const linear_algebra::DynamicVector<double>& aX)
                                     { return offset_vector(aX, mOffset); },
                                     [](const linear_algebra::DynamicVector<double>&) { return identity_operation(); },
                                     [](const linear_algebra::DynamicVector<double>&)
                                     { return linear_algebra::AdjointJacobianMultiplier{identity_operation()}; }};
}

}  // namespace plato::criteria::library
