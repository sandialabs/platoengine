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
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/test_utilities/GradientChecker.hpp"
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
const auto kCutPlane = tpik::Plane{{0, -1, 0}, 0.6};
const auto kLevelSetPrimitives = tpik::LevelSetPrimitives{{kCutPlane}, {}};

using tpik::test_utilities::KrinoTestFixture;

}  // namespace

TEST(KrinoWrapperGradientCheck, MeshFilePath) { ASSERT_TRUE(kRectangleMeshFilePath.has_value()); }

/* This is a specialized example - the 2d rectangle mesh has a linear level set profile. The 0 contour happens in a nice
 * place where mesh changes are well defined. The direction vector is uniform, so the level set fields all move up and
 * the boundary moves linearly in y-hat direction. The numerical gradient is exact for even large step size and so the
 * GC really produces the truncation error region. */
TEST_F(KrinoTestFixture, CheckGradientForPerturbationOfLevelSetPlane)
{
    const auto tX = linear_algebra::DynamicVector<double>{make_initial_guess_from_level_set_primitives(
        kRectangleMeshFilePath.value(), kLevelSetPrimitives, std::nullopt)};

    const auto tDirection = linear_algebra::DynamicVector(std::vector<double>(tX.size(), 0.1));

    const auto tF = [](const linear_algebra::DynamicVector<double>& aX) -> double
    { return test_utilities::accumulate_cut_node_coordinates(kRectangleMeshFilePath.value(), aX.stdVector()); };
    const auto tDf = [tDirection](const linear_algebra::DynamicVector<double>& aX,
                                  const linear_algebra::DynamicVector<double>& /*aV*/) -> double
    {
        const auto tFileName = std::filesystem::path{"out.exo"};
        const auto tWrapper = test_utilities::make_krino_wrapper_from_vector_values(
            kRectangleMeshFilePath.value(), test_utilities::InitialLevelSetValues{aX.stdVector()}, std::nullopt);
        tWrapper.writeCutMesh(tFileName, third_party_integration::krino::VoidPhase::kIncludeInMesh);
        const auto tMesh = mesh::Mesh{tFileName};
        const auto tCutMeshNodeSize = mesh::EntityCounts{tMesh}.numberOfNodes();
        const auto tOnesVector = std::vector<double>(tCutMeshNodeSize * 2, 1.0);
        const auto tCommunicator = boost::mpi::communicator{};
        if (tCommunicator.rank() == 0)
        {
            std::filesystem::remove(tFileName);
        }
        const auto tResult = linear_algebra::DynamicVector<double>{
            tWrapper.rowVectorJacobianProduct(tOnesVector, third_party_integration::krino::VoidPhase::kIncludeInMesh)};

        return tResult.dot(linear_algebra::DynamicVector<double>{tDirection});
    };

    const auto tGradientCheckParameters = plato::test_utilities::GradientCheckParameters{0.5, 7, 1.0};
    const auto tGradientCheck = plato::test_utilities::GradientChecker{tF, tDf};

    const auto tErrors = tGradientCheck.finiteDifferenceErrors(tX, tDirection, tGradientCheckParameters);
    ASSERT_FALSE(tErrors.empty());

    constexpr auto tTolerance = 1e-12;
    for (const auto& tTruncationError : tErrors)
    {
        ASSERT_NEAR(tTruncationError.mValue, 0.0, tTolerance)
            << tGradientCheck.table(tX, tDirection, tGradientCheckParameters);
    }
}

}  // namespace plato::geometry::extension::unittest
