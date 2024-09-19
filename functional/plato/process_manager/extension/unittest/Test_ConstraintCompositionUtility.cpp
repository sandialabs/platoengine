#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintAdapter.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/process_manager/extension/ConstraintCompositionUtility.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"
#include "plato/test_utilities/InputGeneration.hpp"

namespace plato::process_manager::extension::unittest
{

namespace
{
[[nodiscard]] auto make_geometry_and_constraint()
    -> std::pair<plato::geometry::library::FactoryTypes,
                 plato::criteria::library::Constraint<const plato::mesh::MeshDesignVariables&>>
{
    namespace ptu = test_utilities;
    const input_parser::ParsedInput tInputDeck =
        ptu::create_valid_brick_shape_geometry() | ptu::create_valid_example_constraint() |
        ptu::create_valid_example_rol_optimization() | ptu::create_valid_example_objective();
    const auto tValidatedInput = library::make_validated_input(tInputDeck);
    const auto tProcessManagerData = library::make_process_manager_data(tValidatedInput);
    const auto tMeshDesignConstraint = tProcessManagerData.mConstraints.front();
    const auto tGeometry = tProcessManagerData.mGeometry;

    return std::make_pair(tGeometry, tMeshDesignConstraint);
}

}  // namespace

TEST(ConstraintComposition, CompositionThenVectorize)
{
    const auto [tGeometry, tMeshDesignConstraint] = make_geometry_and_constraint();
    const auto tComposedDynamicVectorConstraint = compose_geometry_with_constraint(tMeshDesignConstraint, tGeometry);

    const linear_algebra::DynamicVector<double> tParameters({0.1, 0.2, 0.3, 1, 2, 3});
    const linear_algebra::DynamicVector<double> tParameterDirection({0.1, 0.1, 0.1, 0.1, 0.1, 0.1});

    const auto tMeshDesignVariables = tGeometry.mCompute.f(tParameters);
    const auto tManualConstraintValue = tMeshDesignConstraint.mConstraintFunction.f(tMeshDesignVariables);
    const auto tCompositionValue = tComposedDynamicVectorConstraint.mConstraintFunction.f(tParameters);
    EXPECT_DOUBLE_EQ(tManualConstraintValue, tCompositionValue);

    const auto tJacobianMultiplierMeshDesignVariables = tGeometry.mCompute.df(tParameters);
    const auto tManualJacobianMultiplier =
        tMeshDesignConstraint.mConstraintFunction.df(tMeshDesignVariables) * tJacobianMultiplierMeshDesignVariables;

    EXPECT_EQ(tManualJacobianMultiplier.size(), tParameterDirection.size());
    const auto tManualGradientResult = tManualJacobianMultiplier.dot(tParameterDirection);

    const auto tCompositionGradient =
        tComposedDynamicVectorConstraint.mConstraintFunction.df(tParameters).dot(tParameterDirection);

    EXPECT_EQ(tCompositionGradient, tManualGradientResult);

    const auto tVectorConstraint = criteria::library::make_vector_constraint(tComposedDynamicVectorConstraint);
    const auto tVectorConstraintValue = tVectorConstraint.mFunctionWithDfAsJacobian.f(tParameters);
    ASSERT_EQ(tVectorConstraintValue.size(), 1U);
    EXPECT_EQ(tCompositionValue, tVectorConstraintValue.stdVector()[0]);

    const auto tVectorConstraintJacobianTimesVector =
        tVectorConstraint.mFunctionWithDfAsJacobian.df(tParameters).mJacobianTimesVectorFunction(tParameterDirection);
    ASSERT_EQ(tVectorConstraintJacobianTimesVector.size(), 1U);
    EXPECT_EQ(tVectorConstraintJacobianTimesVector.stdVector()[0], tCompositionGradient);

    const auto tDual = linear_algebra::DynamicVector<double>({1});
    const auto tVectorConstraintAdjointJacobianTimesVector =
        tVectorConstraint.mFunctionWithDfAsAdjointJacobian.df(tParameters).mJacobianTimesVectorFunction(tDual);
    ASSERT_EQ(tVectorConstraintAdjointJacobianTimesVector.size(), tParameters.size());
    EXPECT_EQ(tManualJacobianMultiplier.stdVector(), tVectorConstraintAdjointJacobianTimesVector.stdVector());
}

}  // namespace plato::process_manager::extension::unittest
