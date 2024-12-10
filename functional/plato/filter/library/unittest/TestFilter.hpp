#ifndef PLATO_FILTER_LIBRARY_UNITTEST_TESTFILTER
#define PLATO_FILTER_LIBRARY_UNITTEST_TESTFILTER

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/filter/library/FilterInterface.hpp"

namespace plato::filter::library::unittest
{
/// @brief Returns the total size of each vector in each block of @a aAnalysisDomainMesh.
/// @warning This is for testing and does not represent the actual size of @a aAnalysisDomainMesh if it contains a nodal
/// field. Use analysis::AnalysisDomainMeshSequentialView::size instead.
[[nodiscard]] inline auto analysis_domain_mesh_size(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
    -> std::size_t;

/// @brief A filter that may be used for testing.
class TestFilter : public FilterInterface
{
    /// @brief Returns the argument @a aAnalysisDomainMesh, like an identity filter.
    [[nodiscard]] auto filter(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh) const
        -> analysis::AnalysisDomainMesh override
    {
        return aAnalysisDomainMesh;
    }

    /// @brief Returns a vector with all ones and size equal to the sum of the size of @a aAnalysisDomainMesh and @a aV.
    [[nodiscard]] auto rowVectorTimesJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                              const linear_algebra::DynamicVector<double>& aV) const
        -> linear_algebra::DynamicVector<double> override
    {
        return linear_algebra::DynamicVector<double>(aV.size() + analysis_domain_mesh_size(aAnalysisDomainMesh), 1.0);
    }

    /// @brief Returns a vector with all twos and size equal to the sum of the size of @a aAnalysisDomainMesh and @a aV.
    [[nodiscard]] auto rowVectorTimesAdjointJacobian(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                     const plato::linear_algebra::DynamicVector<double>& aV) const
        -> plato::linear_algebra::DynamicVector<double> override
    {
        return linear_algebra::DynamicVector<double>(aV.size() + analysis_domain_mesh_size(aAnalysisDomainMesh), 2.0);
    }
};

[[nodiscard]] inline auto analysis_domain_mesh_size(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
    -> std::size_t
{
    return std::accumulate(aAnalysisDomainMesh.mBlockScalarField.cbegin(), aAnalysisDomainMesh.mBlockScalarField.cend(),
                           std::size_t{0},
                           [](const auto aTotalSize, const auto& aBlock) { return aTotalSize + aBlock.second.size(); });
}

}  // namespace plato::filter::library::unittest

#endif
