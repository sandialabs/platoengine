#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>
#include <numeric>
#include <stk_util/parallel/Parallel.hpp>
#include <string_view>

#include "plato/core/MeshProxy.hpp"
#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/linear_algebra/JacobianColumnEvaluator.hpp"
#include "plato/utilities/STKUtilities.hpp"

namespace plato::geometry::extension::unittest
{
TEST(Brick, CenterAndDims)
{
    namespace pfu = plato::utilities;
    ASSERT_EQ(stk::parallel_machine_size(MPI_COMM_WORLD), 1);

    constexpr std::string_view tFileName = "test.exo";
    constexpr BrickDesign tDesignParameters = {1.0, -2.0, 3.0, 1.80, 3.90, 6.0};

    {
        constexpr double tDiscretizationSize = 1.0;
        auto mesh = detail::create_mesh(tDesignParameters, tDiscretizationSize);
        pfu::write_mesh(tFileName, mesh);
        constexpr unsigned tExpectedNumElements = 2 * 4 * 6;
        EXPECT_EQ(tExpectedNumElements, pfu::element_size(tFileName));
    }
    {
        auto mesh = detail::create_mesh(tDesignParameters);
        pfu::write_mesh(tFileName, mesh);
        constexpr unsigned tExpectedNumElements = 1;
        EXPECT_EQ(tExpectedNumElements, pfu::element_size(tFileName));
    }

    EXPECT_TRUE(std::filesystem::exists(tFileName));
    EXPECT_TRUE(std::filesystem::remove(tFileName));
}

TEST(Brick, SensitivityCenterX)
{
    constexpr int tCenterXIndex = 0;
    const auto tSensitivities = detail::sensitivities(tCenterXIndex);
    const std::vector<double> tGold = {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0};
    EXPECT_EQ(tSensitivities.size(), tGold.size());
    EXPECT_EQ(tSensitivities, tGold);
}

TEST(Brick, SensitivityCenterY)
{
    constexpr int tCenterYIndex = 1;
    const auto tSensitivities = detail::sensitivities(tCenterYIndex);
    const std::vector<double> tGold = {0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0};
    EXPECT_EQ(tSensitivities.size(), tGold.size());
    EXPECT_EQ(tSensitivities, tGold);
}

TEST(Brick, SensitivityCenterZ)
{
    constexpr int tCenterZIndex = 2;
    const auto tSensitivities = detail::sensitivities(tCenterZIndex);
    const std::vector<double> tGold = {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1};
    EXPECT_EQ(tSensitivities.size(), tGold.size());
    EXPECT_EQ(tSensitivities, tGold);
}

TEST(Brick, SensitivityLengthX)
{
    // Nodes 1,3,5,7 should be negative (alternating x)
    constexpr int tLengthXIndex = 3;
    const auto tSensitivities = detail::sensitivities(tLengthXIndex);
    const std::vector<double> tGold = {-0.5, 0, 0, 0.5, 0, 0, -0.5, 0, 0, 0.5, 0, 0,
                                       -0.5, 0, 0, 0.5, 0, 0, -0.5, 0, 0, 0.5, 0, 0};
    EXPECT_EQ(tSensitivities.size(), tGold.size());
    EXPECT_EQ(tSensitivities, tGold);
}

TEST(Brick, SensitivityLengthY)
{
    // Nodes 1,2,5,6 should be negative
    // Nodes 3,4,7,8 positive
    constexpr int tLengthYIndex = 4;
    const auto tSensitivities = detail::sensitivities(tLengthYIndex);
    const std::vector<double> tGold = {0, -0.5, 0, 0, -0.5, 0, 0, 0.5, 0, 0, 0.5, 0,
                                       0, -0.5, 0, 0, -0.5, 0, 0, 0.5, 0, 0, 0.5, 0};
    EXPECT_EQ(tSensitivities.size(), tGold.size());
    EXPECT_EQ(tSensitivities, tGold);
}

TEST(Brick, SensitivityLengthZ)
{
    // Nodes 1,2,3,4 should be negative
    // Nodes 5,6,7,8 positive
    constexpr int tLengthZIndex = 5;
    const auto tSensitivities = detail::sensitivities(tLengthZIndex);
    const std::vector<double> tGold = {0, 0, -0.5, 0, 0, -0.5, 0, 0, -0.5, 0, 0, -0.5,
                                       0, 0, 0.5,  0, 0, 0.5,  0, 0, 0.5,  0, 0, 0.5};
    EXPECT_EQ(tSensitivities.size(), tGold.size());
    EXPECT_EQ(tSensitivities, tGold);
}

TEST(BrickSensitivities, JacobianEvaluator)
{
    const linear_algebra::JacobianColumnEvaluator tJacobian = {
        /*.mColumns=*/6,
        /*.mX=*/linear_algebra::DynamicVector<double>{1.0, 2.0, 3.0, 4.0, 5.0, 6.0},
        /*.mColumnFunction=*/[](unsigned int i, linear_algebra::DynamicVector<double>) {
            return linear_algebra::DynamicVector<double>(detail::sensitivities(i));
        }};

    std::vector<double> tVec(24, 0.0);
    std::iota(tVec.begin(), tVec.end(), 1.0);
    const linear_algebra::DynamicVector<double> tRolvec(tVec);

    const std::vector<double> tGold{92, 100, 108, 6, 12, 24};
    const linear_algebra::DynamicVector<double> tRes = tRolvec * tJacobian;
    EXPECT_EQ(tRes.stdVector(), tGold);
}

TEST(Brick, ABrick)
{
    namespace pfu = plato::utilities;
    const std::string tFileName = "brick.exo";

    constexpr BrickDesign tDesignParameters = {/*.center_x = */ 1,
                                               /*.center_y = */ -2,
                                               /*.center_z = */ -3,
                                               /*.dimension_x = */ 2,
                                               /*.dimension_y = */ 4,
                                               /*.dimension_z = */ 6};

    constexpr double tDiscretizationSize = 1.0;
    BrickShapeGeometry tBrick(tFileName, tDiscretizationSize);

    const core::MeshProxy tMP = tBrick.generateMesh(tDesignParameters);
    EXPECT_EQ(tMP.mFileName, tFileName);

    constexpr unsigned tExpectedNumElements = 2 * 4 * 6;
    EXPECT_EQ(tExpectedNumElements, pfu::element_size(tFileName));

    EXPECT_TRUE(std::filesystem::exists(tFileName));
    EXPECT_TRUE(std::filesystem::remove(tFileName));
}

TEST(Brick, ConvertDesignParametersToROLStdVector)
{
    namespace pfu = plato::utilities;
    constexpr BrickDesign tDesignParameters = {/*.center_x = */ 1,
                                               /*.center_y = */ -2,
                                               /*.center_z = */ -3,
                                               /*.dimension_x = */ 2,
                                               /*.dimension_y = */ 4,
                                               /*.dimension_z = */ 6};
    const linear_algebra::DynamicVector<double> tResult = detail::to_dynamic_vector(tDesignParameters);

    const std::vector<double> tGold{tDesignParameters.center_x,    tDesignParameters.center_y,
                                    tDesignParameters.center_z,    tDesignParameters.dimension_x,
                                    tDesignParameters.dimension_y, tDesignParameters.dimension_z};

    EXPECT_EQ(tResult.stdVector(), tGold);
}

TEST(Brick, Jacobian)
{
    const std::string tFileName = "brick.exo";

    constexpr BrickDesign tDesignParameters = {/*.center_x = */ 1,
                                               /*.center_y = */ -2,
                                               /*.center_z = */ -3,
                                               /*.dimension_x = */ 2,
                                               /*.dimension_y = */ 4,
                                               /*.dimension_z = */ 6};

    const BrickShapeGeometry tBrick(tFileName);
    const linear_algebra::JacobianColumnEvaluator tJacobian = tBrick.jacobian(tDesignParameters);

    constexpr unsigned int tNumNodes = 8;
    constexpr unsigned int tNumCoordinates = 3;
    std::vector<double> tVec(tNumNodes * tNumCoordinates, 0.0);
    std::iota(tVec.begin(), tVec.end(), 1.0);
    const linear_algebra::DynamicVector<double> tRolvec(tVec);

    const std::vector<double> tGold{92, 100, 108, 6, 12, 24};
    const linear_algebra::DynamicVector<double> tRes = tRolvec * tJacobian;
    EXPECT_EQ(tRes.stdVector(), tGold);
}

TEST(Brick, ToROLStdVector)
{
    namespace pfu = plato::utilities;
    constexpr BrickDesign tDesignParameters = {/*.center_x = */ 1,
                                               /*.center_y = */ -2,
                                               /*.center_z = */ -3,
                                               /*.dimension_x = */ 2,
                                               /*.dimension_y = */ 4,
                                               /*.dimension_z = */ 6};

    const linear_algebra::DynamicVector<double> tAsDynamicVector = detail::to_dynamic_vector(tDesignParameters);
    EXPECT_EQ(tDesignParameters.center_x, tAsDynamicVector.stdVector().at(0));
    EXPECT_EQ(tDesignParameters.center_y, tAsDynamicVector.stdVector().at(1));
    EXPECT_EQ(tDesignParameters.center_z, tAsDynamicVector.stdVector().at(2));
    EXPECT_EQ(tDesignParameters.dimension_x, tAsDynamicVector.stdVector().at(3));
    EXPECT_EQ(tDesignParameters.dimension_y, tAsDynamicVector.stdVector().at(4));
    EXPECT_EQ(tDesignParameters.dimension_z, tAsDynamicVector.stdVector().at(5));
}
}  // namespace plato::geometry::extension::unittest