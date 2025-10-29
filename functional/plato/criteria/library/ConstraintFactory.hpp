#ifndef PLATO_CRITERIA_LIBRARY_CONSTRAINTFACTORY
#define PLATO_CRITERIA_LIBRARY_CONSTRAINTFACTORY

#include <cstdint>
#include <string>

#include "plato/core/Function.hpp"
#include "plato/criteria/library/ConstraintTarget.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::input_parser
{
struct constraint;
}

namespace plato::services
{
struct AppConfigurationWithDirectory;
}

namespace plato::criteria::library
{
using ValidatedConstraints = input_validation::ValidatedComponentType<components::ComponentType::kConstraint>;

enum class ConstraintType : std::uint8_t
{
    kEqualTo,
    kLessThan,
    kGreaterThan
};

/// @brief Holds members for defining a vector-valued constraint
/// @tparam FunctionArg The argument of the function used to define the constraint.
///   Typically, this is either AnalysisDomainMesh or a vector type such as DynamicVector.
template <typename FunctionArg>
struct VectorConstraint
{
    using ConstraintFunction =
        core::Function<FunctionArg,
                       core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFunction>,
                       core::FunctionInfo<linear_algebra::JacobianMultiplier, core::evaluation::kFirstDerivative>,
                       core::FunctionInfo<linear_algebra::AdjointJacobianMultiplier,
                                          core::evaluation::kFirstDerivative,
                                          core::MatrixOrdering::kAdjoint>>;

    std::string mName;
    ConstraintFunction mConstraintFunction;
    ConstraintTarget mConstraintTarget;
    bool mLinear = false;
    ConstraintType mConstraintType;
};

/// @brief Factory to create Constraint objects from input data.
/// @post The return vector will have the same size as @a aInput.
[[nodiscard]] auto make_constraints(const ValidatedConstraints& aInput)
    -> std::vector<VectorConstraint<const analysis::AnalysisDomainMesh&>>;

/// @brief Helper for providing ROL a dual vector for constraints sized with @a aSize.
[[nodiscard]] auto make_dual_vector(std::size_t aSize) -> linear_algebra::DynamicVector<double>;

namespace detail
{
[[nodiscard]] auto make_constraint(
    const input_validation::ValidatedInputDataBlock<components::ComponentType::kConstraint>& aConstraintInput)
    -> VectorConstraint<const analysis::AnalysisDomainMesh&>;

/// @brief Creates a ConstraintTarget object based on the input contained in @a aInput and uses the component names, if
/// necessary, in @a aAppConfigurations.
/// @pre The app and criterion names in @a aInput are set (not `nullopt`) and refer to an existing configuration in @a
/// aAppConfigurations.
/// @pre The app and criterion names in @a aInput refer to a vector criterion with named components, so that the
/// CriterionConfiguration object has a non-empty `mVectorComponents` field.
[[nodiscard]] auto make_constraint_target(
    const input_parser::constraint& aInput,
    const std::vector<services::AppConfigurationWithDirectory>& aAppConfigurations) -> ConstraintTarget;
}  // namespace detail
}  // namespace plato::criteria::library

#endif
