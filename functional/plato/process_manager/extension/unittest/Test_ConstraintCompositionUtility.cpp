#include <gtest/gtest.h>

#include "plato/criteria/library/ConstraintAdapter.hpp"
#include "plato/criteria/library/ConstraintInputBlock.hpp"
#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/process_manager/extension/ConstraintCompositionUtility.hpp"
#include "plato/process_manager/extension/ROLOptimization.hpp"
#include "plato/process_manager/library/ProcessManagerData.hpp"

namespace plato::process_manager::extension::unittest
{

namespace
{

[[nodiscard]] auto make_geometry_and_constraint()
    -> std::pair<plato::geometry::library::FactoryTypes,
                 plato::criteria::library::VectorConstraint<const analysis::AnalysisDomainMesh&>>
{
    const auto tInputDeck = geometry::extension::create_valid_brick_shape_geometry_input() |
                            criteria::library::create_valid_example_constraint_input() |
                            process_manager::extension::create_valid_example_rol_optimization_input() |
                            criteria::library::create_valid_example_constraint_input();
    const auto tValidatedInput = input_validation::make_validated_input(tInputDeck).value();
    const auto tProcessManagerData = library::make_process_manager_data(tValidatedInput);
    const auto tMeshDesignConstraint = tProcessManagerData.mConstraints.front();
    const auto tGeometry = tProcessManagerData.mGeometry;

    return std::make_pair(tGeometry, tMeshDesignConstraint);
}

}  // namespace

TEST(ConstraintComposition, ComposeGeometryWithVectorConstraint)
{
    const auto [tGeometry, tMeshDesignConstraint] = make_geometry_and_constraint();
    const auto tComposedDynamicVectorConstraint =
        compose_geometry_with_vector_constraint(tMeshDesignConstraint, tGeometry);

    const auto tParameters = linear_algebra::DynamicVector{0.1, 0.2, 0.3, 1.0, 2.0, 3.0};
    const auto tParameterDirection = linear_algebra::DynamicVector(24U, 0.1);
    const auto tAnalysisDomainMesh = tGeometry.mCompute.evaluate<core::evaluation::kFunction>(tParameters);

    // Check function value
    {
        const auto tExpectedConstraintValue =
            tMeshDesignConstraint.mConstraintFunction.evaluate<core::evaluation::kFunction>(tAnalysisDomainMesh);
        const auto tCompositionValue =
            tComposedDynamicVectorConstraint.mConstraintFunction.evaluate<core::evaluation::kFunction>(tParameters);

        EXPECT_EQ(tExpectedConstraintValue.stdVector(), tCompositionValue.stdVector());
    }

    // Check Jacobian vector product
    {
        // f(X) = sum(X_i), X \in R^24, X is nodal coordinates of a brick
        // g(x) : R^6 -> R^24, maps x, y, z center coordinates and length, width height of brick to nodal coordinates
        // \nabla f = [1 1 ... 1] \in R^24
        // J_x(g) \in R^{24 \times 6}
        // Checking product of: v \nabla f J_x(g)
        // v \in R^1
        // Result should have size 6
        const auto tV = linear_algebra::DynamicVector{0.1};
        const auto tConstraintJacobian =
            tV *
            tMeshDesignConstraint.mConstraintFunction.evaluate<core::evaluation::kFirstDerivative>(tAnalysisDomainMesh);
        const auto tJacobianTimesVectorGeometry =
            tConstraintJacobian * tGeometry.mCompute.evaluate<core::evaluation::kFirstDerivative>(tParameters);

        const auto tCompositionJacobianTimesVector =
            tV * tComposedDynamicVectorConstraint.mConstraintFunction.evaluate<core::evaluation::kFirstDerivative>(
                     tParameters);

        EXPECT_EQ(tCompositionJacobianTimesVector.size(), 6U);
        EXPECT_EQ(tJacobianTimesVectorGeometry.stdVector(), tCompositionJacobianTimesVector.stdVector());
    }
    // Check adjoint adjoint Jacobian vector product
    {
        // Checking product of: u J_x(g)^T \nabla f^T
        // u \in R^6
        // Result should have size 1
        const auto tAdjointGeometryMultiplier =
            tGeometry.mCompute.evaluate<core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>(
                tParameters);
        const auto tAdjointConstraintGradient =
            tMeshDesignConstraint.mConstraintFunction
                .evaluate<core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>(tAnalysisDomainMesh);
        const auto tU = linear_algebra::DynamicVector(6U, 0.1);
        const auto tExpectedAdjointProduct = (tU * tAdjointGeometryMultiplier) * tAdjointConstraintGradient;

        const auto tCompositionAdjoint =
            tU * tComposedDynamicVectorConstraint.mConstraintFunction
                     .evaluate<core::evaluation::kFirstDerivative, core::MatrixOrdering::kAdjoint>(tParameters);

        EXPECT_EQ(tCompositionAdjoint.size(), 1U);
        EXPECT_EQ(tCompositionAdjoint.stdVector(), tExpectedAdjointProduct.stdVector());
    }
}

}  // namespace plato::process_manager::extension::unittest
