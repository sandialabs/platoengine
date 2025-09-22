#include <gtest/gtest.h>

#include <vector>

#include "plato/geometry/extension/test_utilities/KrinoWrapperGradientCheckUtilities.hpp"
#include "plato/geometry/extension/test_utilities/KrinoWrapperTestUtilities.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/test_utilities/GradientChecker.hpp"
#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/utilities/ContainerHelpers.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::geometry::extension::unittest
{

namespace
{
namespace tpik = third_party_integration::krino;
const auto kRectangleMeshFilePath = utilities::data_file_path("rectangle_3x4_tri3.cdf");
const auto kCutPlane = tpik::Plane{{0, -1, 0}, 0.6};
const auto kLevelSetPrimitives = tpik::LevelSetPrimitives{{kCutPlane}, {}};

using tpik::test_utilities::KrinoTestFixture;

[[nodiscard]] auto flatten_sensitivities(
    const std::unordered_map<tpik::CutMeshSurfaceNodeId, tpik::LevelSetJacobianColumn>& aMap) -> std::vector<double>
{
    constexpr auto tDimensions = 2U;
    constexpr auto tNumberOfParentNodes = 2U;
    auto tFlattenedSensitivity =
        utilities::reserved_container<std::vector<double>>(aMap.size() * tDimensions * tNumberOfParentNodes);
    for (const auto& tEntry : aMap)
    {
        for (const auto& tNodalSensitivity : tEntry.second.mNodalSensitivities)
        {
            tFlattenedSensitivity.push_back(tNodalSensitivity.x);
            tFlattenedSensitivity.push_back(tNodalSensitivity.y);
        }
    }

    return tFlattenedSensitivity;
}

[[nodiscard]] auto retrieve_sensitivities(const std::vector<double>& aPerturbedField) -> std::vector<double>
{
    const auto tWrapper = test_utilities::make_krino_wrapper_from_vector_values(
        kRectangleMeshFilePath.value(), test_utilities::InitialLevelSetValues{aPerturbedField});
    return flatten_sensitivities(tWrapper.sensitivities());
}

}  // namespace

TEST(KrinoWrapperGradientCheck, MeshFilePath) { ASSERT_TRUE(kRectangleMeshFilePath.has_value()); }

/* This is a specialized example - the 2d rectangle mesh has a linear level set profile. The 0 contour happens in a nice
 * place where mesh changes are well defined. The direction vector is uniform, so the level set fields all move up and
 * the boundary moves linearly in y-hat direction. The numerical gradient is exact for even large step size and so the
 * GC really produces the truncation error region. */
TEST_F(KrinoTestFixture, CheckGradientForPerturbationOfLevelSetPlane)
{
    const auto tF = [](const linear_algebra::DynamicVector<double>& aX) -> double
    { return test_utilities::accumulate_cut_node_coordinates(kRectangleMeshFilePath.value(), aX.stdVector()); };
    const auto tDf = [](const linear_algebra::DynamicVector<double>& aX,
                        const linear_algebra::DynamicVector<double>& aV) -> double
    {
        const auto tSensitivities = linear_algebra::DynamicVector<double>{retrieve_sensitivities(aX.stdVector())};
        return tSensitivities.dot(aV);
    };

    const auto tGradientCheckParameters = plato::test_utilities::GradientCheckParameters{0.1, 7, 0.1};
    const auto tGradientCheck = plato::test_utilities::GradientChecker{tF, tDf};

    const auto tFixedBlocks = std::set<std::string>{};
    const auto tX = linear_algebra::DynamicVector<double>{make_initial_guess_from_level_set_primitives(
        kRectangleMeshFilePath.value(), kLevelSetPrimitives, tFixedBlocks)};

    const auto tDirection = linear_algebra::DynamicVector(std::vector<double>(tX.size(), 0.1));

    const auto tErrors = tGradientCheck.finiteDifferenceErrors(tX, tDirection, tGradientCheckParameters);
    ASSERT_FALSE(tErrors.empty());

    constexpr auto tTolerance = 2e-8;
    for (const auto& tTruncationError : tErrors)
    {
        EXPECT_NEAR(tTruncationError.mValue, 0.0, tTolerance)
            << tGradientCheck.table(tX, tDirection, tGradientCheckParameters);
    }

    auto tMonotonicIterator =
        std::adjacent_find(tErrors.begin(), tErrors.end(),
                           [](const auto aSecond, const auto aFirst) { return aSecond.mValue >= aFirst.mValue; });
    EXPECT_EQ(tMonotonicIterator, tErrors.end()) << tGradientCheck.table(tX, tDirection, tGradientCheckParameters);
}

}  // namespace plato::geometry::extension::unittest
