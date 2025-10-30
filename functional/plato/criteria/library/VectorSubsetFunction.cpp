#include "plato/criteria/library/VectorSubsetFunction.hpp"

#include "plato/utilities/ContainerHelpers.hpp"

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

[[nodiscard]] auto make_vector_subset_jacobian_product(const std::size_t aNumberOfRows)
{
    return linear_algebra::JacobianMultiplier{[aNumberOfRows](const linear_algebra::DynamicVector<double>& aRowVector)
                                              {
                                                  assert(aRowVector.size() == aNumberOfRows);
                                                  return aRowVector;
                                              }};
}

}  // namespace

[[nodiscard]] auto make_vector_subset_function(const std::set<std::size_t>& aIndices) -> VectorSubsetFunction
{
    return VectorSubsetFunction{
        make_vector_subset_evaluation_function(aIndices),
        [mNumberOfRows = aIndices.size()](const linear_algebra::DynamicVector<double>&)
        { return make_vector_subset_jacobian_product(mNumberOfRows); },
        [aIndices](const linear_algebra::DynamicVector<double>&)
        { return linear_algebra::make_adjoint_jacobian_multiplier(make_vector_subset_evaluation_function(aIndices)); }};
}
}  // namespace plato::criteria::library
