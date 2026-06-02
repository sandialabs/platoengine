#include <gtest/gtest.h>

#include <string_view>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/criteria/extension/QuadraticObjective.hpp"
#include "plato/mesh/DesignVariableAdapter.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshWithFieldWriter.hpp"
#include "plato/utilities/ContainerHelpers.hpp"

namespace plato::criteria::extension::unittest
{
namespace
{
using QuadraticObjectiveFixture = third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;

[[nodiscard]] auto make_iota_vector(const std::size_t aSize) -> std::vector<double>
{
    auto tVector = std::vector<double>(aSize, 0.0);
    std::iota(tVector.begin(), tVector.end(), 1.0);
    return tVector;
}

void check_value_and_gradient_of_quadratic_objective(const analysis::AnalysisDomainMesh& aAnalysisDomainMesh,
                                                     const double aGoldValue,
                                                     const std::vector<double>& aGoldGradient,
                                                     const test_utilities::TestContext& aTestContext)
{
    const auto tValue = QuadraticObjective{}.value(aAnalysisDomainMesh);
    EXPECT_DOUBLE_EQ(tValue, aGoldValue) << aTestContext;
    const auto tGradient = QuadraticObjective{}.gradient(aAnalysisDomainMesh).stdVector();
    EXPECT_EQ(tGradient, aGoldGradient) << aTestContext;
}

}  // namespace

TEST_F(QuadraticObjectiveFixture, NodalValueAndGradient)
{
    const auto tMesh = mesh::Mesh{mMeshFilePath};
    const auto tNumberOfDesignNodes = mesh::EntityCounts{tMesh}.numberOfDesignDomainNodes();
    const auto tNodalField = make_iota_vector(tNumberOfDesignNodes);
    const auto tAnalysisDomainMesh = mesh::DesignVariablesConversion{tMesh}.nodalFieldToAnalysisDomainMesh(
        mesh::NodalFieldVectorReference{tNodalField});
    constexpr auto tGold = 748.0;  // sum([1:16].^2)*0.5
    check_value_and_gradient_of_quadratic_objective(tAnalysisDomainMesh, tGold, tNodalField,
                                                    TEST_CONTEXT("Nodal centered analysis domain mesh."));
}

TEST_F(QuadraticObjectiveFixture, ElementValueAndGradient)
{
    const auto tMesh = mesh::Mesh{mMeshFilePath};
    const auto tNumberOfDesignElements = mesh::EntityCounts{tMesh}.numberOfDesignDomainElements();
    const auto tElementField = make_iota_vector(tNumberOfDesignElements);
    const auto tAnalysisDomainMesh = mesh::DesignVariablesConversion{tMesh}.elementFieldToAnalysisDomainMesh(
        mesh::ElementFieldVectorReference{tElementField});
    constexpr auto tGold = 7.0;  // sum([1:3].^2)*0.5
    check_value_and_gradient_of_quadratic_objective(tAnalysisDomainMesh, tGold, tElementField,
                                                    TEST_CONTEXT("Element centered analysis domain mesh."));
}

TEST_F(QuadraticObjectiveFixture, CoordinateValueAndGradient)
{
    const auto tMesh = mesh::Mesh{mMeshFilePath};
    const auto tGoldGradient =
        third_party_integration::common::flatten(mesh::EntityRetrieval{tMesh}.nodalCoordinates());
    const auto tAnalysisDomainMesh = analysis::AnalysisDomainMesh{mMeshFilePath, {}};
    constexpr auto tGold = 36.0;  // sum([1:3].^2)*0.5
    check_value_and_gradient_of_quadratic_objective(tAnalysisDomainMesh, tGold, tGoldGradient,
                                                    TEST_CONTEXT("Coordinate based analysis domain mesh."));
}

}  // namespace plato::criteria::extension::unittest
