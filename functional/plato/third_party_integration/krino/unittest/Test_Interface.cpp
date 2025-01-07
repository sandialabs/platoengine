#include <gtest/gtest.h>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshOperators.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/krino/Interface.hpp"
#include "plato/third_party_integration/krino/unittest/KrinoTestFixture.hpp"

namespace plato::third_party_integration::krino::unittest
{
namespace
{
const auto kMeshFile = std::filesystem::path{"not-a-mesh.exo"};

using LevelSetJacobianSparseMatrix = std::unordered_map<stk::mesh::EntityId, LevelSetJacobianColumn>;
const auto kLevelSetJacobian = LevelSetJacobianSparseMatrix{
    {3, LevelSetJacobianColumn{{7, 12, 19}, {{0.5, 0.5, -0.5}, {-0.25, 0.25, 0.25}, {-0.125, 0.125, -0.125}}}},
    {1, LevelSetJacobianColumn{{34, 22, 2}, {{-0.125, -0.125, 0.125}, {0.25, 0.25, -0.25}, {0.75, -0.75, -0.75}}}},
    {2, LevelSetJacobianColumn{{19, 10}, {{1.5, -1.5, 1.5}, {0.25, -0.25, -0.25}}}},
};

void check_row_vector_jacobian_product(const std::vector<double>& aRowVector,
                                       const LevelSetJacobianSparseMatrix& aLevelSetJacobian,
                                       const std::vector<analysis::ScalarFieldValue>& aFieldVector,
                                       const plato::test_utilities::TestContext& aTestContext)
{
    constexpr auto tBlockID = 1;
    const auto tCutMeshField = plato::analysis::AnalysisDomainMesh{
        kMeshFile, analysis::AnalysisDomainMesh::BlockScalarField{{tBlockID, aFieldVector}}};

    const auto tBackgroundNodemapValues = std::vector<analysis::ScalarFieldValue>{
        {2, 0, 0.0}, {7, 1, 0.0}, {10, 2, 0.0}, {12, 3, 0.0}, {19, 4, 0.0}, {22, 5, 0.0}, {34, 6, 0.0}};
    auto tBackgroundNodemap = plato::analysis::AnalysisDomainMesh{
        kMeshFile, analysis::AnalysisDomainMesh::BlockScalarField{{tBlockID, tBackgroundNodemapValues}}};

    const auto tVectorJacobianProduct = level_set_row_vector_jacobian_product(
        aRowVector, tCutMeshField, aLevelSetJacobian, std::move(tBackgroundNodemap));

    const auto tExpected = std::vector<analysis::ScalarFieldValue>{
        {2, 0, 0.9375},      {7, 1, 0.28125}, {10, 2, 0.34375}, {12, 3, -0.171875},
        {19, 4, -0.9921875}, {22, 5, 0.5625}, {34, 6, -0.28125}};
    auto tComputed = std::vector<analysis::ScalarFieldValue>{};
    const auto tComputedView = analysis::AnalysisDomainMeshSequentialView{tVectorJacobianProduct};
    std::copy(tComputedView.begin(), tComputedView.end(), std::back_inserter(tComputed));

    EXPECT_EQ(tComputed, tExpected) << aTestContext;
}

void check_row_vector_adjoint_jacobian_product(
    const LevelSetJacobianSparseMatrix& aLevelSetJacobian,
    const plato::test_utilities::TestContext& aTestContext,
    const std::optional<std::pair<int, stk::math::Vector3d>>& aAdditionalEntry = std::nullopt)
{
    constexpr auto tBlockID = 1;
    const auto tFieldVector = std::vector<analysis::ScalarFieldValue>{
        {2, 0, 1.0}, {7, 1, 2.0}, {10, 2, 3.0}, {12, 3, 4.0}, {19, 4, 5.0}, {22, 5, 6.0}, {34, 6, 7.0}};
    const auto tBackgroundLevelSetField = plato::analysis::AnalysisDomainMesh{
        kMeshFile, analysis::AnalysisDomainMesh::BlockScalarField{{tBlockID, tFieldVector}}};

    const auto tVectorJacobianProduct =
        level_set_row_vector_adjoint_jacobian_product(tBackgroundLevelSetField, aLevelSetJacobian);

    auto tExpected =
        std::unordered_map<unsigned int, stk::math::Vector3d>{{1, stk::math::Vector3d{1.375, -0.125, -1.375}},
                                                              {2, stk::math::Vector3d{8.25, -8.25, 6.75}},
                                                              {3, stk::math::Vector3d{-0.625, 2.625, -0.625}}};
    if (aAdditionalEntry)
    {
        tExpected[aAdditionalEntry->first] = aAdditionalEntry->second;
    }
    EXPECT_EQ(tExpected, tVectorJacobianProduct) << aTestContext;
}

}  // namespace

TEST_F(KrinoTestFixture, LevelSetRowVectorJacobianProduct)
{
    const auto tRowVector = std::vector<double>{2.0, 0.5, 0.25, 0.25, -0.125, -1.0, 0.5, -0.0625, -0.125};
    const auto tFieldVector = std::vector<analysis::ScalarFieldValue>{{1, 0, 0.0}, {2, 1, 0.0}, {3, 2, 0.0}};
    check_row_vector_jacobian_product(tRowVector, kLevelSetJacobian, tFieldVector,
                                      TEST_CONTEXT("Vector-Jacobian-product, all node IDs match"));
}

TEST_F(KrinoTestFixture, LevelSetRowVectorAdjointJacobianProduct)
{
    check_row_vector_adjoint_jacobian_product(kLevelSetJacobian,
                                              TEST_CONTEXT("Vector-adjoint-Jacobian-product, all node IDs match"));
}

TEST_F(KrinoTestFixture, LevelSetRowVectorJacobianProductCutMeshIDNotInBackgroundMesh)
{
    auto tLevelSetJacobian = kLevelSetJacobian;
    // Add a row with non-existent node ids in the background mesh
    tLevelSetJacobian[4] = LevelSetJacobianColumn{{200, 300}, {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}}};

    const auto tRowVector =
        std::vector<double>{2.0, 0.5, 0.25, 0.25, -0.125, -1.0, 0.5, -0.0625, -0.125, 0.0, 0.0, 0.0};
    const auto tFieldVector =
        std::vector<analysis::ScalarFieldValue>{{1, 0, 0.0}, {2, 1, 0.0}, {3, 2, 0.0}, {4, 3, 0.0}};
    check_row_vector_jacobian_product(
        tRowVector, tLevelSetJacobian, tFieldVector,
        TEST_CONTEXT("Vector-Jacobian-product, cut mesh has ids not in background mesh."));
}

TEST_F(KrinoTestFixture, LevelSetRowVectorAdjointJacobianProductCutMeshIDNotInBackgroundMesh)
{
    auto tLevelSetJacobian = kLevelSetJacobian;
    tLevelSetJacobian[4] = LevelSetJacobianColumn{{400, 600}, {{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}}};

    check_row_vector_adjoint_jacobian_product(
        tLevelSetJacobian, TEST_CONTEXT("Vector-adjoint-Jacobian-product, cut mesh has ids not in background mesh."),
        std::make_pair(4, stk::math::Vector3d(0.0, 0.0, 0.0)));
}

}  // namespace plato::third_party_integration::krino::unittest
