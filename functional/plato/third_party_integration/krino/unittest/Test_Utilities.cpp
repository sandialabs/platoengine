#include <gtest/gtest.h>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshOperators.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/third_party_integration/krino/Utilities.hpp"
#include "plato/third_party_integration/krino/unittest/KrinoTestFixture.hpp"

namespace plato::third_party_integration::krino::unittest
{
namespace
{
const auto kMeshFile = std::filesystem::path{"not-a-mesh.exo"};
}

TEST_F(KrinoTestFixture, CalculateDFDLS)
{
    const auto tDXDP = std::unordered_map<stk::mesh::EntityId, InterfaceNodeDXDP>{
        {3, InterfaceNodeDXDP{{7, 12, 19}, {{0.5, 0.5, -0.5}, {-0.25, 0.25, 0.25}, {-0.125, 0.125, -0.125}}}},
        {1, InterfaceNodeDXDP{{34, 22, 2}, {{-0.125, -0.125, 0.125}, {0.25, 0.25, -0.25}, {0.75, -0.75, -0.75}}}},
        {2, InterfaceNodeDXDP{{19, 10}, {{1.5, -1.5, 1.5}, {0.25, -0.25, -0.25}}}},
    };

    // Row vector times Jacobian matrix
    {
        const auto tDFDX = std::vector<double>{2.0, 0.5, 0.25, 0.25, -0.125, -1.0, 0.5, -0.0625, -0.125};
        constexpr auto tBlockID = 1;
        const auto tFieldVector = std::vector<analysis::ScalarFieldValue>{{1, 0, 0.0}, {2, 1, 0.0}, {3, 2, 0.0}};
        const auto tCutMeshField = plato::analysis::AnalysisDomainMesh{
            kMeshFile, analysis::AnalysisDomainMesh::BlockScalarField{{tBlockID, tFieldVector}}};

        const auto tBackgroundNodemapValues = std::vector<analysis::ScalarFieldValue>{
            {2, 0, 0.0}, {7, 1, 0.0}, {10, 2, 0.0}, {12, 3, 0.0}, {19, 4, 0.0}, {22, 5, 0.0}, {34, 6, 0.0}};
        auto tBackgroundNodemap = plato::analysis::AnalysisDomainMesh{
            kMeshFile, analysis::AnalysisDomainMesh::BlockScalarField{{tBlockID, tBackgroundNodemapValues}}};

        const auto tDFDLS = calculate_dfdls(tDFDX, tCutMeshField, tDXDP, std::move(tBackgroundNodemap));

        const auto tExpected = std::vector<analysis::ScalarFieldValue>{
            {2, 0, 0.9375},      {7, 1, 0.28125}, {10, 2, 0.34375}, {12, 3, -0.171875},
            {19, 4, -0.9921875}, {22, 5, 0.5625}, {34, 6, -0.28125}};
        auto tComputed = std::vector<analysis::ScalarFieldValue>{};
        const auto tComputedView = analysis::AnalysisDomainMeshSequentialView{tDFDLS};
        std::copy(tComputedView.begin(), tComputedView.end(), std::back_inserter(tComputed));

        EXPECT_EQ(tComputed, tExpected);
    }
    // Row vector times transpose Jacobian matrix
    {
        constexpr auto tBlockID = 1;
        const auto tFieldVector = std::vector<analysis::ScalarFieldValue>{
            {2, 0, 1.0}, {7, 1, 2.0}, {10, 2, 3.0}, {12, 3, 4.0}, {19, 4, 5.0}, {22, 5, 6.0}, {34, 6, 7.0}};
        const auto tBackgroundLevelSetField = plato::analysis::AnalysisDomainMesh{
            kMeshFile, analysis::AnalysisDomainMesh::BlockScalarField{{tBlockID, tFieldVector}}};

        const auto tDFDLS = calculate_adjoint_dfdls(tBackgroundLevelSetField, tDXDP);

        auto tExpected =
            std::unordered_map<unsigned int, stk::math::Vector3d>{{1, stk::math::Vector3d{1.375, -0.125, -1.375}},
                                                                  {2, stk::math::Vector3d{8.25, -8.25, 6.75}},
                                                                  {3, stk::math::Vector3d{-0.625, 2.625, -0.625}}};

        EXPECT_EQ(tExpected, tDFDLS);
    }
}

}  // namespace plato::third_party_integration::krino::unittest
