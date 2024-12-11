#include <gtest/gtest.h>

#include "plato/mesh/DesignVariableAdapter.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::mesh::unittest
{
namespace
{
class DesignVariableAdapterMeshFixture : public third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh
{
};

auto analysis_domain_mesh_iota(const mesh::Mesh& aMesh) -> analysis::AnalysisDomainMesh
{
    const auto tNumberOfCutMeshNodes = mesh::EntityCounts{aMesh}.numberOfNodes();
    auto tVector = std::vector(tNumberOfCutMeshNodes, 0.0);
    std::iota(tVector.begin(), tVector.end(), 1.0);
    return mesh::DesignVariablesConversion{aMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tVector});
}

auto adapter_test_objects(const std::filesystem::path& aMeshFilePath, const unsigned int aNumberOfNodes)
    -> std::pair<linear_algebra::DynamicVector<double>, DesignVariableToAnalysisMeshAdapterFunction>
{
    const auto tMesh = mesh::Mesh{aMeshFilePath};
    const auto tDesignVariableAdapter = design_variables_to_analysis_mesh_adapter(tMesh);

    auto tVector = std::vector<double>(aNumberOfNodes);
    std::iota(tVector.begin(), tVector.end(), 1.0);
    auto tDesignVariables = linear_algebra::DynamicVector<double>(std::move(tVector));

    return {tDesignVariables, tDesignVariableAdapter};
}

}  // namespace

TEST_F(DesignVariableAdapterMeshFixture, DesignVariablesToAnalysisMeshFunctionEvaluation)
{
    const auto [tDesignVariables, tDesignVariableAdapter] = adapter_test_objects(mMeshFilePath, mExpectedNumberOfNodes);
    const auto tAnalysisDomainMesh = tDesignVariableAdapter.evaluate<core::evaluation::kFunction>(tDesignVariables);

    EXPECT_EQ(tAnalysisDomainMesh.mFileName, mMeshFilePath);
    EXPECT_EQ(tAnalysisDomainMesh.mBlockScalarField.size(), mExpectedNumberOfBlocks);
    // Result is sorted by global ID, and the design variable should match the vector ordering
    for (const auto& [tBlockID, tBlockScalarField] : tAnalysisDomainMesh.mBlockScalarField)
    {
        for (const auto& [tGlobalID, tVectorID, tFieldValue] : tBlockScalarField)
        {
            EXPECT_EQ(tVectorID + 1, tFieldValue);
        }
    }
}

TEST_F(DesignVariableAdapterMeshFixture, DesignVariablesFirstDerivativesRegularOrder)
{
    const auto [tDesignVariables, tDesignVariableAdapter] = adapter_test_objects(mMeshFilePath, mExpectedNumberOfNodes);
    const auto tJacobian = tDesignVariableAdapter.evaluate<core::evaluation::kFirstDerivative>(
        linear_algebra::DynamicVector<double>{tDesignVariables});
    const auto tMultiplicationResult = tDesignVariables * tJacobian;
    EXPECT_EQ(tMultiplicationResult, tDesignVariables);
}

TEST_F(DesignVariableAdapterMeshFixture, DesignVariablesFirstDerivativesAdjointOrder)
{
    const auto [tDesignVariables, tDesignVariableAdapter] = adapter_test_objects(mMeshFilePath, mExpectedNumberOfNodes);
    const auto tJacobian =
        tDesignVariableAdapter.evaluate<core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>(
            linear_algebra::DynamicVector<double>{});
    const auto tMultiplicationResult = tDesignVariables * tJacobian;
    EXPECT_EQ(tMultiplicationResult, tDesignVariables);
}

TEST_F(DesignVariableAdapterMeshFixture, AnalysisMeshToDesignVariablesFunctionEvaluation)
{
    const auto tMesh = mesh::Mesh{mMeshFilePath};
    const auto tAnalysisMeshAdapter = analysis_mesh_to_design_variables_adapter(tMesh);
    const auto tDesignVariables =
        tAnalysisMeshAdapter.evaluate<core::evaluation::kFunction>(analysis_domain_mesh_iota(tMesh));

    auto tExpectedVector = std::vector<double>(mExpectedNumberOfNodes);
    std::iota(tExpectedVector.begin(), tExpectedVector.end(), 1.0);

    EXPECT_EQ(tExpectedVector, tDesignVariables.stdVector());
}

TEST_F(DesignVariableAdapterMeshFixture, AnalysisMeshFirstDerivativesRegularOrder)
{
    const auto tAnalysisMeshAdapter = analysis_mesh_to_design_variables_adapter(mesh::Mesh{mMeshFilePath});
    const auto tJacobian =
        tAnalysisMeshAdapter.evaluate<core::evaluation::kFirstDerivative>(analysis::AnalysisDomainMesh{});
    const auto tRowVector = linear_algebra::DynamicVector{1.0, 2.0, 3.0, 4.0};
    const auto tMultiplicationResult = tRowVector * tJacobian;
    EXPECT_EQ(tMultiplicationResult, tRowVector);
}

TEST_F(DesignVariableAdapterMeshFixture, AnalysisMeshFirstDerivativesAdjointOrder)
{
    const auto tAnalysisMeshAdapter = analysis_mesh_to_design_variables_adapter(mesh::Mesh{mMeshFilePath});
    const auto tJacobian =
        tAnalysisMeshAdapter.evaluate<core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>(
            analysis::AnalysisDomainMesh{});
    const auto tRowVector = linear_algebra::DynamicVector{4.0, 3.0, 2.0, 1.0};
    const auto tMultiplicationResult = tRowVector * tJacobian;
    EXPECT_EQ(tMultiplicationResult, tRowVector);
}

}  // namespace plato::mesh::unittest
