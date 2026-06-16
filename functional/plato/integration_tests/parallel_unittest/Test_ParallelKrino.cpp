#include <gtest/gtest.h>

#include <numeric>
#include <utility>
#include <vector>

#include "plato/core/Compose.hpp"
#include "plato/core/Function.hpp"
#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/filter/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/FileList.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::integration_test::parallel_unittest
{
namespace
{
const auto kRectangleMeshFilePath = utilities::data_file_path("rectangle_3x4_tri3.cdf");
}

TEST(ParallelKrino, NodalSumObjectivesRegression)
{
    ASSERT_TRUE(kRectangleMeshFilePath);
    auto tGeometryInput = geometry::extension::test_utilities::create_valid_level_set_topology_geometry_input();
    tGeometryInput.mesh_name = input_parser::FileName{*kRectangleMeshFilePath};
    tGeometryInput.max_edge_length_percentage_for_snapping = 0.0;  // turns off snapping for regression

    const auto tSpherePatternBoundingBoxMin = input_parser::Point{-1.5, -2, -1};
    tGeometryInput.sphere_pattern.value().min = tSpherePatternBoundingBoxMin;

    const auto tSpherePatternBoundingBoxMax = input_parser::Point{1.5, 2, 1};
    tGeometryInput.sphere_pattern.value().max = tSpherePatternBoundingBoxMax;

    constexpr auto tSphereRadius = 0.5;
    tGeometryInput.sphere_pattern.value().radius = tSphereRadius;

    auto tNodalSumInput = criteria::library::test_utilities::create_valid_example_objective_input();
    tNodalSumInput.aggregation_weight = 1.0 / static_cast<double>(boost::mpi::communicator{}.size());

    auto tCriteria = std::vector(boost::mpi::communicator{}.size(), tNodalSumInput);
    const auto tAllCriteria = std::accumulate(tCriteria.begin(), tCriteria.end(), input_parser::ParsedInput{},
                                              [](input_parser::ParsedInput aAllInput, const auto& aCriterion)
                                              { return std::move(aAllInput) | aCriterion; });

    const auto tInput = tAllCriteria | tGeometryInput |
                        filter::extension::test_utilities::create_valid_identity_filter_input() |
                        process_manager::extension::test_utilities::create_valid_example_gradient_check_input();
    const auto tValidatedInputOrError = input_validation::make_validated_input(tInput);
    ASSERT_TRUE(tValidatedInputOrError.hasValue()) << tValidatedInputOrError.error();
    const auto& tValidatedInput = tValidatedInputOrError.value();

    const auto tProcessManagerData = process_manager::library::make_process_manager_data(tValidatedInput);
    const auto tFunction = core::compose(tProcessManagerData.mObjective, tProcessManagerData.mGeometry.mCompute);

    const auto tInitialGuess = tProcessManagerData.mGeometry.mInitialGuess;

    // Regression values were computed on a single processor.
    // This checks the parallel consistency and can be run on any number of ranks.
    const auto tObjective = tFunction.evaluate<core::evaluation::kFunction>(tInitialGuess);
    constexpr auto tExpectedNodalSum = -2.6252168254382302;
    constexpr auto tTolerance = 1e-14;
    EXPECT_NEAR(tObjective, tExpectedNodalSum, tTolerance);

    const auto tGradient = tFunction.evaluate<core::evaluation::kFirstDerivative>(tInitialGuess);
    const auto tOnes = linear_algebra::DynamicVector<double>(std::vector<double>(tInitialGuess.size(), 1.0));
    constexpr auto tExpectedGradientSum = -15.7396954226510886;
    EXPECT_NEAR(tGradient.dot(tOnes), tExpectedGradientSum, tTolerance);
}
}  // namespace plato::integration_test::parallel_unittest
