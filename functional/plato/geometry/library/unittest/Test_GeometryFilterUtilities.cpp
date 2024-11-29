#include <gtest/gtest.h>

#include "plato/filter/extension/KernelFilter.hpp"
#include "plato/filter/library/FilterJacobian.hpp"
#include "plato/geometry/library/GeometryFilterUtilities.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::geometry::library
{
namespace
{
class GeometryFilterUtilitiesMeshFixture : public third_party_integration::stk_io::test_utilities::Tet4MeshOnDisk
{
};

auto mesh_test_data(const mesh::Mesh& aMesh) -> std::vector<double>
{
    const auto tNumberOfNodes = mesh::EntityCounts{aMesh}.numberOfNodes();
    auto tDesignVariables = std::vector<double>(tNumberOfNodes, 0.0);
    std::iota(tDesignVariables.begin(), tDesignVariables.end(), 0.0);
    return tDesignVariables;
}

auto analysis_domain_mesh_test_data(const mesh::Mesh& aMesh) -> analysis::AnalysisDomainMesh
{
    return mesh::DesignVariablesConversion{aMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{mesh_test_data(aMesh)});
}

auto make_kernel_filter_test_function(const std::filesystem::path& aMeshFilePath) -> filter::library::FilterFunction
{
    constexpr auto tFilterRadius = filter::extension::FilterRadius{1.0};
    const auto tMesh = mesh::Mesh{aMeshFilePath};
    const auto tKernelFilter = std::make_shared<filter::extension::KernelFilter>(
        tMesh, tFilterRadius, input_parser::KernelFilterCenteringTypes::kNodeCentered, boost::mpi::communicator{});

    return filter::library::FilterFunction{
        [tKernelFilter](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        { return tKernelFilter->filter(aAnalysisDomainMesh); },
        [tKernelFilter](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        { return filter::library::make_filter_jacobian(tKernelFilter, aAnalysisDomainMesh); },
        [tKernelFilter](const analysis::AnalysisDomainMesh& aAnalysisDomainMesh)
        { return filter::library::make_filter_adjoint_jacobian(tKernelFilter, aAnalysisDomainMesh); }};
}
}  // namespace

TEST_F(GeometryFilterUtilitiesMeshFixture, AdaptedFilterMatchesOriginalFilter)
{
    const auto tKernelFilter = make_kernel_filter_test_function(mMeshFilePath);
    const auto tMesh = mesh::Mesh{mMeshFilePath};
    const auto tAdaptedFilter = adapt_filter(tKernelFilter, tMesh);

    const auto tTestDataOnMesh = analysis_domain_mesh_test_data(tMesh);
    const auto tTestData = linear_algebra::DynamicVector(mesh_test_data(tMesh));

    // Filter
    {
        const auto tPlainFilterResultOnMesh = tKernelFilter.evaluate<core::evaluation::kFunction>(tTestDataOnMesh);
        const auto tPlainFilterResult =
            mesh::DesignVariablesConversion{tMesh}.analysisDomainMeshToNodalFieldVector(tPlainFilterResultOnMesh);
        const auto tAdaptedFilterResult = tAdaptedFilter.evaluate<core::evaluation::kFunction>(tTestData);
        EXPECT_EQ(tPlainFilterResult.mValue, tAdaptedFilterResult.stdVector());
    }
    // Jacobian
    {
        const auto tPlainFilterJacobian =
            tTestData * tKernelFilter.evaluate<core::evaluation::kFirstDerivative>(tTestDataOnMesh);
        const auto tAdaptedFilterJacobian =
            tTestData * tAdaptedFilter.evaluate<core::evaluation::kFirstDerivative>(tTestData);

        EXPECT_EQ(tPlainFilterJacobian.stdVector(), tAdaptedFilterJacobian.stdVector());
    }
    // Adjoint Jacobian
    {
        const auto tPlainFilterAdjointJacobian =
            tTestData *
            tKernelFilter.evaluate<core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>(tTestDataOnMesh);
        const auto tAdaptedFilterAdjointJacobian =
            tTestData *
            tAdaptedFilter.evaluate<core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>(tTestData);

        EXPECT_EQ(tPlainFilterAdjointJacobian.stdVector(), tAdaptedFilterAdjointJacobian.stdVector());
    }
}

}  // namespace plato::geometry::library
