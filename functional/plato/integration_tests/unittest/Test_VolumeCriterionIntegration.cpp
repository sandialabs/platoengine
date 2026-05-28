#include <gtest/gtest.h>

#include <boost/none.hpp>

#include "plato/components/ComponentType.hpp"
#include "plato/core/Compose.hpp"
#include "plato/core/Function.hpp"
#include "plato/criteria/library/ObjectiveFactory.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/input_parser/InputFieldTypes.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/RandomPerturbationVector.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/utilities/GradientChecker.hpp"

namespace plato::integration_tests::unittest
{
TEST(VolumeCriterionIntegration, BrickShapeGeometry)
{
    constexpr auto tX = 2.0;
    constexpr auto tY = 4.0;
    constexpr auto tZ = 6.0;
    constexpr auto tCenter = 0.0;
    const auto tControls = linear_algebra::DynamicVector<double>{tCenter, tCenter, tCenter, tX, tY, tZ};

    const auto tVolumeObjective = input_parser::objective{/*.name=*/std::string{"volume_criterion"},
                                                          /*.active=*/true,
                                                          /*.app=*/input_parser::AppName{"platoengine"},
                                                          /*.criterion=*/input_parser::CriterionName{"volume"},
                                                          /*.number_of_processors=*/1U,
                                                          /*.input_files=*/boost::none,
                                                          /*.aggregation_weight=*/1.0,
                                                          /*.normalize_by_initial_value=*/false,
                                                          /*.objective_goal=*/boost::none};
    const auto tInputBase = geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                            process_manager::extension::test_utilities::create_valid_example_rol_optimization_input();
    const auto tInput = tInputBase | tVolumeObjective;
    const auto tValidInput = input_validation::make_validated_input(tInput).value();

    const auto tGeometry =
        geometry::extension::make_brick_shape_geometry(geometry::extension::BrickShapeGeometry{"brick.exo"});
    const auto tDomainMesh = tGeometry.evaluate<core::evaluation::kFunction>(tControls);
    const auto tObjectiveFunction = criteria::library::make_aggregate_objective_function(
        tValidInput.get<components::ComponentType::kObjective>(), tDomainMesh);

    const auto tComposedObjective = core::compose(tObjectiveFunction, tGeometry);

    // check volume
    const auto tResult = tComposedObjective.evaluate<core::evaluation::kFunction>(tControls);
    EXPECT_DOUBLE_EQ(tResult, 48);

    // check volume gradient
    const auto tChecker = plato::utilities::GradientChecker{
        [&tComposedObjective](const linear_algebra::DynamicVector<double>& aControls)
        { return tComposedObjective.evaluate<core::evaluation::kFunction>(aControls); },
        [&tComposedObjective](const linear_algebra::DynamicVector<double>& aControls,
                              const linear_algebra::DynamicVector<double>& aDirection)
        {
            const auto tGradient = tComposedObjective.evaluate<core::evaluation::kFirstDerivative>(aControls);
            return tGradient.dot(aDirection);
        }};

    auto tRandomEngine = std::default_random_engine{123};
    const auto tDirection = linear_algebra::random_perturbation_vector(tControls.size(), tRandomEngine);

    const auto tGradientCheckParameters =
        plato::utilities::GradientCheckParameters{.mStepDelta = 0.1, .mNumSteps = 7, .mInitialStepSize = 1.0};
    constexpr auto tFirstOrderTruncationErrorTolerance = 5e-2;

    EXPECT_NEAR(tChecker.maxFirstOrderTruncationError(tControls, tDirection, tGradientCheckParameters), 0.0,
                tFirstOrderTruncationErrorTolerance)
        << tChecker.table(tControls, tDirection, tGradientCheckParameters);
}
}  // namespace plato::integration_tests::unittest
