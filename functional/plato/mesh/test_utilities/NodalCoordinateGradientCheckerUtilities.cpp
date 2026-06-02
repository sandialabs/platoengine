#include "plato/mesh/test_utilities/NodalCoordinateGradientCheckerUtilities.hpp"

#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <cstddef>
#include <filesystem>
#include <numeric>
#include <random>

#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/RandomPerturbationVector.hpp"
#include "plato/mesh/CoordinateUtilities.hpp"
#include "plato/mesh/EntityRetrieval.hpp"
#include "plato/mesh/NodalCoordinateGradientChecker.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/GradientChecker.hpp"

namespace plato::mesh::test_utilities
{
namespace
{
const auto kQuadraticCoordinateSumFunction = [](const std::filesystem::path& aMeshFilePath) -> double
{
    const auto tCoordinates = mesh::EntityRetrieval{mesh::Mesh{aMeshFilePath}}.nodalCoordinates();
    return std::accumulate(tCoordinates.begin(), tCoordinates.end(), 0.0,
                           [](const auto aSum, const auto aCoordinate)
                           {
                               constexpr auto tNormalization = 0.5;
                               return aSum + tNormalization * aCoordinate.x * aCoordinate.x +
                                      tNormalization * aCoordinate.y * aCoordinate.y +
                                      tNormalization * aCoordinate.z * aCoordinate.z;
                           });
};

const auto kQuadraticCoordinateSumDerivativeFunction =
    [](const std::filesystem::path& aMeshFilePath) -> linear_algebra::DynamicVector<double>
{ return test_utilities::original_coordinates(aMeshFilePath); };

}  // namespace

void run_coordinate_gradient_check_test(const std::filesystem::path& aMeshFilePath,
                                        const plato::test_utilities::TestContext& aTestContext,
                                        const boost::mpi::communicator& aCommunicator)
{
    const auto tGradientCheckParameters =
        plato::utilities::GradientCheckParameters{.mStepDelta = 0.1, .mNumSteps = 7, .mInitialStepSize = 1.0};
    const auto tNodalCoordinateGradientChecker = make_nodal_coordinate_gradient_checker(
        kQuadraticCoordinateSumFunction, kQuadraticCoordinateSumDerivativeFunction, aMeshFilePath, aCommunicator);

    const auto tOriginalCoordinates = test_utilities::original_coordinates(aMeshFilePath);
    const auto tDirection = test_utilities::random_direction(aMeshFilePath);

    constexpr auto tFirstOrderTruncationErrorTolerance = 5e-2;
    EXPECT_NEAR(tNodalCoordinateGradientChecker.maxFirstOrderTruncationError(tOriginalCoordinates, tDirection,
                                                                             tGradientCheckParameters),
                0.0, tFirstOrderTruncationErrorTolerance)
        << aTestContext
        << tNodalCoordinateGradientChecker.table(tOriginalCoordinates, tDirection, tGradientCheckParameters);
    const auto tErrors = tNodalCoordinateGradientChecker.finiteDifferenceErrors(tOriginalCoordinates, tDirection,
                                                                                tGradientCheckParameters);
    ASSERT_FALSE(tErrors.empty());
    constexpr auto tLastFiniteDifferenceError{1e-6};
    EXPECT_NEAR(tErrors.back().mValue, 0.0, tLastFiniteDifferenceError)
        << aTestContext << "Full gradient check:\n"
        << tNodalCoordinateGradientChecker.table(tOriginalCoordinates, tDirection, tGradientCheckParameters);
}

auto gradient_size(const std::filesystem::path& aFilePath) -> std::size_t
{
    const auto tMesh = mesh::Mesh{aFilePath};
    const auto tSpatialDimensions = mesh::EntityCounts{tMesh}.spatialDimensions();
    const auto tNumberOfNodes = static_cast<std::size_t>(mesh::EntityCounts{tMesh}.numberOfNodes());
    return tSpatialDimensions * tNumberOfNodes;
}

auto random_direction(const std::filesystem::path& aMeshFilePath) -> linear_algebra::DynamicVector<double>
{
    constexpr auto tRandomSeed = 123;
    auto tRandomEngine = std::default_random_engine{tRandomSeed};  // NOLINT(cert-msc51-cpp,cert-msc32-c)
    return linear_algebra::random_perturbation_vector(gradient_size(aMeshFilePath), tRandomEngine);
}

auto original_coordinates(const std::filesystem::path& aMeshFilePath) -> linear_algebra::DynamicVector<double>
{
    const auto tMesh = mesh::Mesh{aMeshFilePath};
    const auto tSpatialDimensions = mesh::EntityCounts{tMesh}.spatialDimensions();
    return mesh::nodal_vector_field_to_dynamic_vector(mesh::EntityRetrieval{tMesh}.nodalCoordinates(),
                                                      tSpatialDimensions);
}

}  // namespace plato::mesh::test_utilities
