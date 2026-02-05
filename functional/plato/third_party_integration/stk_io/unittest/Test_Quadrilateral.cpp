#include <gtest/gtest.h>

#include <valarray>

#include "plato/test_utilities/GradientChecker.hpp"
#include "plato/test_utilities/RandomPerturbationVector.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/Quadrilateral.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/SensitivityTestUtilities.hpp"

namespace plato::third_party_integration::stk_io::unittest
{
namespace
{
Quadrilateral create_planar_quadrilateral_from_dimensions(const double aLength, const double aHeight)
{
    return Quadrilateral{.p00 = common::Coordinate{.x = 0.0, .y = 0.0, .z = 0.0},
                         .p10 = common::Coordinate{.x = aLength, .y = 0.0, .z = 0.0},
                         .p11 = common::Coordinate{.x = aLength, .y = aHeight, .z = 0.0},
                         .p01 = common::Coordinate{.x = 0.0, .y = aHeight, .z = 0.0}};
}

Quadrilateral quadrilateral_from_coordinates(const std::valarray<double>& aCoordinates)
{
    assert(aCoordinates.size() == 12);
    return Quadrilateral{
        .p00 = common::Coordinate{.x = aCoordinates[0], .y = aCoordinates[1], .z = aCoordinates[2]},
        .p10 = common::Coordinate{.x = aCoordinates[3], .y = aCoordinates[4], .z = aCoordinates[5]},
        .p11 = common::Coordinate{.x = aCoordinates[6], .y = aCoordinates[7], .z = aCoordinates[8]},
        .p01 = common::Coordinate{.x = aCoordinates[9], .y = aCoordinates[10], .z = aCoordinates[11]},
    };
}

std::valarray<double> valarray_from_sensitivities(const std::array<common::Vector3, 4>& aSensitivities)
{
    return std::valarray<double>{aSensitivities[0].x, aSensitivities[0].y, aSensitivities[0].z, aSensitivities[1].x,
                                 aSensitivities[1].y, aSensitivities[1].z, aSensitivities[2].x, aSensitivities[2].y,
                                 aSensitivities[2].z, aSensitivities[3].x, aSensitivities[3].y, aSensitivities[3].z};
}

void check_gradient_of_volume(const std::valarray<double>& aInitialCoordinates,
                              const plato::test_utilities::GradientCheckParameters& aGradientCheckParameters,
                              const double aFirstOrderTruncationErrorTolerance)
{
    const auto tChecker = plato::test_utilities::GradientChecker{
        [](const std::valarray<double>& aCoordinates) { return quadrilateral_from_coordinates(aCoordinates).volume(); },
        [](const std::valarray<double>& aCoordinates, const std::valarray<double>& aDirection)
        {
            const auto tSensitivities =
                valarray_from_sensitivities(quadrilateral_from_coordinates(aCoordinates).volumeVertexSensitivities());
            return std::inner_product(begin(tSensitivities), end(tSensitivities), begin(aDirection), 0.0);
        }};

    auto tRandomEngine = std::default_random_engine{123};
    const auto tRandomPerturbation = plato::test_utilities::random_perturbation_vector(12, tRandomEngine);
    std::valarray<double> tDirection(tRandomPerturbation.stdVector().data(), tRandomPerturbation.stdVector().size());

    EXPECT_NEAR(tChecker.maxFirstOrderTruncationError(aInitialCoordinates, tDirection, aGradientCheckParameters), 0.0,
                aFirstOrderTruncationErrorTolerance)
        << tChecker.table(aInitialCoordinates, tDirection, aGradientCheckParameters);
}

}  // namespace

TEST(Quadrilateral, VertexSensitivitiesAddition)
{
    const auto tLHS =
        Quadrilateral::VertexSensitivities{std::array{common::Vector3{1., 2., 3.}, common::Vector3{4., 5., 6.},
                                                      common::Vector3{7., 8., 9.}, common::Vector3{10., 11., 12.}}};
    const auto tRHS =
        Quadrilateral::VertexSensitivities{std::array{common::Vector3{1., 2., 3.}, common::Vector3{4., 5., 6.},
                                                      common::Vector3{7., 8., 9.}, common::Vector3{10., 11., 12.}}};

    const auto tSum = tLHS + tRHS;

    const auto tGold =
        Quadrilateral::VertexSensitivities{std::array{common::Vector3{2., 4., 6.}, common::Vector3{8., 10., 12.},
                                                      common::Vector3{14., 16., 18.}, common::Vector3{20., 22., 24.}}};

    EXPECT_EQ(tSum.mValue, tGold.mValue);
}

TEST(Quadrilateral, VertexSensitivitiesScalarMultiplication)
{
    constexpr auto tScalar{2.0};
    const auto tSensitivities =
        Quadrilateral::VertexSensitivities{std::array{common::Vector3{1., 2., 3.}, common::Vector3{4., 5., 6.},
                                                      common::Vector3{7., 8., 9.}, common::Vector3{10., 11., 12.}}};

    const auto tScaled = tSensitivities * tScalar;

    const auto tGold =
        Quadrilateral::VertexSensitivities{std::array{common::Vector3{2., 4., 6.}, common::Vector3{8., 10., 12.},
                                                      common::Vector3{14., 16., 18.}, common::Vector3{20., 22., 24.}}};

    EXPECT_EQ(tScaled.mValue, tGold.mValue);
}

TEST(Quadrilateral, Volume)
{
    constexpr double tLength{86.0};
    constexpr double tHeight{51.0};
    const Quadrilateral tQuad = create_planar_quadrilateral_from_dimensions(tLength, tHeight);
    const double tResult = tQuad.volume();
    constexpr double tGold = tLength * tHeight;

    EXPECT_DOUBLE_EQ(tResult, tGold);
}

TEST(Quadrilateral, VolumeNotInXYPlane)
{
    constexpr double tLength{86.0};
    constexpr double tHeight{51.0};
    const Quadrilateral tQuad = Quadrilateral{.p00 = common::Coordinate{.x = 0.0, .y = 0.0, .z = 0.0},
                                              .p10 = common::Coordinate{.x = 0.0, .y = tLength, .z = 0.0},
                                              .p11 = common::Coordinate{.x = 0.0, .y = tLength, .z = tHeight},
                                              .p01 = common::Coordinate{.x = 0.0, .y = 0.0, .z = tHeight}};
    const double tResult = tQuad.volume();
    constexpr double tGold = tLength * tHeight;

    EXPECT_DOUBLE_EQ(tResult, tGold);
}

TEST(Quadrilateral, VolumeOfPerturbedUnitQuad)
{
    constexpr double tLength{1.0};
    constexpr double tHeight{1.0};
    Quadrilateral tQuad = create_planar_quadrilateral_from_dimensions(tLength, tHeight);
    tQuad.p10.x += 0.1;
    const double tResult = tQuad.volume();
    constexpr double tGold = 21. / 20;  // computed via matlab

    EXPECT_DOUBLE_EQ(tResult, tGold);
}

TEST(Quadrilateral, Centroid)
{
    constexpr double tLength{38.0};
    constexpr double tHeight{37.0};
    const Quadrilateral tQuad = create_planar_quadrilateral_from_dimensions(tLength, tHeight);
    const auto tResult = tQuad.centroid();
    constexpr common::Coordinate tGold{tLength / 2, tHeight / 2, 0.0};

    common::test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Quadrilateral Centroid"));
}

TEST(Quadrilateral, CentroidOfParallelogram)
{
    const common::Coordinate tCornerPoint{.x = 0.0, .y = 0.0, .z = 0.0};
    const common::Coordinate tVector1{.x = 21.0, .y = 12.0, .z = 0.0};
    const common::Coordinate tVector2{.x = 39.0, .y = 71.0, .z = 0.0};
    const Quadrilateral tQuad{.p00 = tCornerPoint,
                              .p10 = tCornerPoint + tVector1,
                              .p11 = tCornerPoint + tVector1 + tVector2,
                              .p01 = tCornerPoint + tVector2};
    const auto tResult = tQuad.centroid();
    const common::Coordinate tGold{(tQuad.p00 + tQuad.p10 + tQuad.p11 + tQuad.p01) / 4.};

    common::test_utilities::test_double_equality_of_components(tResult, tGold, TEST_CONTEXT("Parallelepiped Centroid"));
}

TEST(Quadrilateral, VolumeVertexSensitivities)
{
    constexpr double tLength{88.0};
    constexpr double tHeight{71.0};
    const Quadrilateral tQuad = create_planar_quadrilateral_from_dimensions(tLength, tHeight);

    const auto tVertexSensitivities = tQuad.volumeVertexSensitivities();
    const auto tExpectedSensitivities = test_utilities::axes_aligned_quad_volume_sensitivities(
        test_utilities::Rectangle{.mLength = tLength, .mWidth = tHeight});

    test_utilities::test_nodal_sensitivities_with_expected(tVertexSensitivities, tExpectedSensitivities,
                                                           TEST_CONTEXT("Quadrilateral volume vertex sensitivities"));
}

TEST(Quadrilateral, VolumeVertexSensitivities_PlanarQuadGradientCheck)
{
    const auto tOriginalCoordinates = std::valarray<double>{0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0};

    const auto tGradientCheckParameters =
        plato::test_utilities::GradientCheckParameters{.mStepDelta = 0.1, .mNumSteps = 6, .mInitialStepSize = 0.1};
    constexpr auto tFirstOrderTruncationErrorTolerance = 8e-2;
    check_gradient_of_volume(tOriginalCoordinates, tGradientCheckParameters, tFirstOrderTruncationErrorTolerance);
}

TEST(Quadrilateral, VolumeVertexSensitivities_NonPlanarQuadGradientCheck)
{
    const auto tOriginalCoordinates =
        std::valarray<double>{0.888395631757642,  -1.147070106969150, -1.068870458168032, -1.711516418853698,
                              -0.102242446085491, -0.241447041607358, -0.809498694424876, -2.944284161994896,
                              1.438380292815098,  0.325190539456198,  -0.754928319169703, 1.370298540095228};

    const auto tGradientCheckParameters =
        plato::test_utilities::GradientCheckParameters{.mStepDelta = 0.1, .mNumSteps = 7, .mInitialStepSize = 1.0};
    constexpr auto tFirstOrderTruncationErrorTolerance = 1e-1;
    check_gradient_of_volume(tOriginalCoordinates, tGradientCheckParameters, tFirstOrderTruncationErrorTolerance);
}
}  // namespace plato::third_party_integration::stk_io::unittest
