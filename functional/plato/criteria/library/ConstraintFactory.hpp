#ifndef PLATO_CRITERIA_LIBRARY_CONSTRAINTFACTORY
#define PLATO_CRITERIA_LIBRARY_CONSTRAINTFACTORY

#include <string>

#include "plato/core/Function.hpp"
#include "plato/core/ValidatedInputTypeWrapper.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::analysis
{
struct AnalysisDomainMesh;
}

namespace plato::criteria::library
{
using ValidatedConstraints =
    core::ValidatedInputTypeWrapper<std::vector<core::ValidatedInputTypeWrapper<input_parser::constraint>>>;

using NewValidatedConstraints = input_validation::ValidatedComponentType<input_parser::ComponentType::kConstraint>;

enum class ConstraintType
{
    kEqualTo,
    kLessThan,
    kGreaterThan
};

/// @brief Holds members for defining a scalar constraint
/// @tparam FunctionArg The argument of the function used to define the constraint.
///   Typically, this is either AnalysisDomainMesh or a vector type such as DynamicVector.
template <typename FunctionArg>
struct Constraint
{
    using ConstraintFunction =
        core::Function<FunctionArg,
                       core::FunctionInfo<double, core::evaluation::kFunction>,
                       core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFirstDerivative>>;

    std::string mName;
    ConstraintFunction mConstraintFunction;
    double mConstraintTarget = 0;
    bool mLinear = false;
    ConstraintType mConstraintType = ConstraintType::kEqualTo;
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
    double mConstraintTarget = 0;
    bool mLinear = false;
    ConstraintType mConstraintType;
};

/// @brief Factory to create Constraint objects from input data.
/// @post The return vector will have the same size as @a aInput.
[[nodiscard]] auto make_constraints(const ValidatedConstraints& aInput)
    -> std::vector<VectorConstraint<const analysis::AnalysisDomainMesh&>>;
[[nodiscard]] auto make_constraints(const NewValidatedConstraints& aInput)
    -> std::vector<VectorConstraint<const analysis::AnalysisDomainMesh&>>;

/// @brief Helper for providing ROL a dual vector for constraints sized with @a aSize.
[[nodiscard]] auto make_dual_vector(std::size_t aSize) -> linear_algebra::DynamicVector<double>;

namespace detail
{
[[nodiscard]] auto make_constraint(const core::ValidatedInputTypeWrapper<input_parser::constraint>& aConstraintInput)
    -> VectorConstraint<const analysis::AnalysisDomainMesh&>;
[[nodiscard]] auto make_constraint(
    const input_validation::ValidatedInputDataBlock<input_parser::ComponentType::kConstraint>& aConstraintInput)
    -> VectorConstraint<const analysis::AnalysisDomainMesh&>;

}  // namespace detail
}  // namespace plato::criteria::library

#endif
