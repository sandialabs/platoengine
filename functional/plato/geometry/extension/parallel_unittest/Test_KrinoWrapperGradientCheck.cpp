#include <gtest/gtest.h>

#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include <stk_util/environment/EnvData.hpp>  //get stk mpi env
#include <vector>

#include "plato/geometry/extension/KrinoWrapper.hpp"
#include "plato/geometry/extension/test_utilities/KrinoWrapperGradientCheckUtilities.hpp"
#include "plato/geometry/extension/test_utilities/KrinoWrapperTestUtilities.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/GradientChecker.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/krino/LevelSetPrimitives.hpp"
#include "plato/third_party_integration/krino/SensitivityMapUtilities.hpp"
#include "plato/third_party_integration/krino/test_utilities/KrinoTestFixture.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::geometry::extension::unittest
{

namespace
{
namespace tpik = third_party_integration::krino;
const auto kRectangleMeshFilePath = utilities::data_file_path("rectangle_3x4_tri3.cdf");
const auto kFourTriTwoBlockMeshFilePath = utilities::data_file_path("four_tri_two_block.cdf");
const auto kCutPlane = tpik::Plane{{0, -1, 0}, 0.6};
const auto kThreeQuarterOffsetXHatPlane = tpik::Plane{{-1, 0, 0}, 0.75};
const auto kLevelSetPrimitives = tpik::LevelSetPrimitives{{kCutPlane}, {}};

using tpik::test_utilities::KrinoTestFixture;
constexpr double kMPISummedTolerance = 1e-14;

}  // namespace

TEST(KrinoWrapperGradientCheck, MeshFilePath) { ASSERT_TRUE(kRectangleMeshFilePath.has_value()); }

TEST_F(KrinoTestFixture, AccumulateInitialGuess)
{
    const auto tX =
        make_initial_guess_from_level_set_primitives(kRectangleMeshFilePath.value(), kLevelSetPrimitives, std::nullopt);
    const auto tResultantSum = std::accumulate(tX.begin(), tX.end(), 0.0);

    EXPECT_NEAR(tResultantSum, 21.62420765215437, kMPISummedTolerance);
}

TEST_F(KrinoTestFixture, AccumulateCutNodeCoordinates)
{
    const auto tX = linear_algebra::DynamicVector<double>{make_initial_guess_from_level_set_primitives(
        kRectangleMeshFilePath.value(), kLevelSetPrimitives, std::nullopt)};
    const auto tResultantSum =
        test_utilities::accumulate_cut_node_coordinates(kRectangleMeshFilePath.value(), tX.stdVector());

    EXPECT_NEAR(tResultantSum, 4.9232365427954026, kMPISummedTolerance);
}

namespace
{
[[nodiscard]] auto make_example_krino_wrapper()
{
    const auto tX =
        make_initial_guess_from_level_set_primitives(kRectangleMeshFilePath.value(), kLevelSetPrimitives, std::nullopt);
    const auto tAnalysisDomainMesh =
        mesh::DesignVariablesConversion{mesh::Mesh{kRectangleMeshFilePath.value()}}.nodalFieldToAnalysisDomainMesh(
            mesh::NodalFieldVectorReference{tX});

    return make_krino_wrapper_from_analysis_domain_mesh(tAnalysisDomainMesh, 1.0);
}

}  // namespace

TEST_F(KrinoTestFixture, RowVectorToVector3)
{
    const auto tSpatialDimension = 2U;
    const auto tKrinoWrapper = make_example_krino_wrapper();
    const auto tX = linear_algebra::DynamicVector<double>{make_initial_guess_from_level_set_primitives(
        kRectangleMeshFilePath.value(), kLevelSetPrimitives, std::nullopt)};
    auto tIota = std::vector<double>(tX.size(), 0.0);
    std::iota(tIota.begin(), tIota.end(), 1.0);
    std::transform(tIota.begin(), tIota.end(), tIota.begin(),
                   [tScale = tX.size()](const auto tValue) { return tValue / tScale; });

    {
        const auto tIndex = 0U;
        const auto tResult =
            extension::detail::row_vector_to_vector3(tIota, utilities::VectorIndex{tIndex}, tSpatialDimension);
        const auto tGold = third_party_integration::common::Vector3{tIota[2 * tIndex], tIota[2 * tIndex + 1], 0.0};
        third_party_integration::common::test_utilities::test_double_equality_of_components(
            tResult, tGold, TEST_CONTEXT("Checking index 0"));
    }
    {
        const auto tIndex = 4U;
        const auto tResult =
            extension::detail::row_vector_to_vector3(tIota, utilities::VectorIndex{tIndex}, tSpatialDimension);
        const auto tGold = third_party_integration::common::Vector3{tIota[2 * tIndex], tIota[2 * tIndex + 1], 0.0};
        third_party_integration::common::test_utilities::test_double_equality_of_components(
            tResult, tGold, TEST_CONTEXT("Checking index 4"));
    }
    {
        const auto tIndex = 134U;
        const auto tResult =
            extension::detail::row_vector_to_vector3(tIota, utilities::VectorIndex{tIndex}, tSpatialDimension);
        const auto tGold = third_party_integration::common::Vector3{0, 0, 0};
        third_party_integration::common::test_utilities::test_double_equality_of_components(
            tResult, tGold, TEST_CONTEXT("Checking index 134"));
    }
}

/* This is a specialized example - the 2d rectangle mesh has a linear level set profile. The 0 contour happens in a nice
 * place where mesh changes are well defined. The direction vector is uniform, so the level set fields all move up and
 * the boundary moves linearly in y-hat direction. The numerical gradient is exact for even large step size and so the
 * GC really produces the truncation error region. */
TEST_F(KrinoTestFixture, CheckGradientForPerturbationOfLevelSetPlane)
{
    const auto tLevelSetPrimitives = tpik::LevelSetPrimitives{{kThreeQuarterOffsetXHatPlane}, {}};

    const auto tMeshFile = kRectangleMeshFilePath;
    const auto tX = linear_algebra::DynamicVector<double>{
        make_initial_guess_from_level_set_primitives(tMeshFile.value(), tLevelSetPrimitives, std::nullopt)};

    auto tIota = std::vector<double>(tX.size(), 0.0);
    std::iota(tIota.begin(), tIota.end(), 1.0);
    std::transform(tIota.begin(), tIota.end(), tIota.begin(),
                   [tScale = static_cast<double>(tX.size())](const auto tValue) { return tValue / tScale - 0.5; });
    const auto tDirection = linear_algebra::DynamicVector<double>{tIota};

    const auto tF = [&tMeshFile](const linear_algebra::DynamicVector<double>& aX) -> double
    { return test_utilities::accumulate_cut_node_coordinates(tMeshFile.value(), aX.stdVector()); };
    const auto tDf = [&tMeshFile](const linear_algebra::DynamicVector<double>& aX,
                                  const linear_algebra::DynamicVector<double>& aV) -> double
    {
        const auto tFileName = std::filesystem::path{"out.exo"};
        const auto tAnalysisDomainMesh =
            mesh::DesignVariablesConversion{mesh::Mesh{tMeshFile.value()}}.nodalFieldToAnalysisDomainMesh(
                mesh::NodalFieldVectorReference{aX.stdVector()});

        const auto tWrapper = make_krino_wrapper_from_analysis_domain_mesh(tAnalysisDomainMesh, 1.0);
        tWrapper.writeCutMesh(tFileName, third_party_integration::krino::VoidPhase::kIncludeInMesh);
        const auto tMesh = mesh::Mesh{tFileName};
        const auto tCutMeshNodeSize = mesh::EntityCounts{tMesh}.numberOfNodes();
        const auto tOnesVector = std::vector<double>(tCutMeshNodeSize * 2, 1.0);
        const auto tCommunicator = boost::mpi::communicator{};
        if (tCommunicator.rank() == 0)
        {
            std::filesystem::remove(tFileName);
        }
        const auto tRowVectorJacobianResult = linear_algebra::DynamicVector<double>{
            tWrapper.rowVectorJacobianProduct(tOnesVector, third_party_integration::krino::VoidPhase::kIncludeInMesh)};
        const auto tXJV = tRowVectorJacobianResult.dot(aV);

        const auto tRowVectorAdjointJacobianResult =
            linear_algebra::DynamicVector<double>{tWrapper.rowVectorAdjointJacobianProduct(
                aV.stdVector(), third_party_integration::krino::VoidPhase::kIncludeInMesh)};
        const auto tVJTX = tRowVectorAdjointJacobianResult.dot(linear_algebra::DynamicVector<double>{tOnesVector});
        EXPECT_DOUBLE_EQ(tXJV, tVJTX);

        return tXJV;
    };
    constexpr auto tNumSteps = 10;
    const auto tGradientCheckParameters = plato::test_utilities::GradientCheckParameters{0.5, tNumSteps, 0.125};
    const auto tGradientCheck = plato::test_utilities::GradientChecker{tF, tDf};

    const auto tMaxError = tGradientCheck.maxFirstOrderTruncationError(tX, tDirection, tGradientCheckParameters);
    EXPECT_NEAR(tMaxError, 0.0, 5e-2) << tGradientCheck.table(tX, tDirection, tGradientCheckParameters);

    /*
    const auto tErrors = tGradientCheck.finiteDifferenceErrors(tX, tDirection, tGradientCheckParameters);
    ASSERT_FALSE(tErrors.empty());

    constexpr auto tTolerance = 1e-14;
    for (const auto& tTruncationError : tErrors)
    {
        ASSERT_NEAR(tTruncationError.mValue, 0.0, tTolerance)
            << tGradientCheck.table(tX, tDirection, tGradientCheckParameters);
    }*/
}

}  // namespace plato::geometry::extension::unittest
