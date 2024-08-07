#ifndef PLATO_CRITERIA_LIBRARY_CONSTRAINTFACTORY
#define PLATO_CRITERIA_LIBRARY_CONSTRAINTFACTORY

#include <string>

#include "plato/core/Function.hpp"
#include "plato/core/ValidatedInputTypeWrapper.hpp"
#include "plato/input_parser/InputBlocks.hpp"
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

enum class ConstraintType
{
    kEquality,
    kLessThan,
    kGreaterThan
};

/// @brief Holds members for defining a Constraint
/// @tparam FunctionArg The argument of the function used to define the constraint.

///   Typically, this is either MeshDesignVariables or a vector type such as DynamicVector.
template <typename FunctionArg>
struct Constraint
{
    using ConstraintFunction = core::Function<double, linear_algebra::DynamicVector<double>, FunctionArg>;

    std::string mName;
    ConstraintFunction mConstraintFunction;
    double mConstraintTarget = 0;
    bool mLinear = false;
    ConstraintType mConstraintType;
};

template <typename FunctionArg>
struct VectorConstraint
{
    using ConstraintFunction =
        core::Function<linear_algebra::DynamicVector<double>, linear_algebra::JacobianMultiplier, FunctionArg>;

    std::string mName;
    ConstraintFunction mConstraintJacobianFunction;
    ConstraintFunction mConstraintAdjointJacobianFunction;
    double mConstraintTarget = 0;
    unsigned int mNumberOfConstraints = 1;
    bool mLinear = false;
    ConstraintType mConstraintType;
};

using ConstraintVariant = std::variant<Constraint<const analysis::AnalysisDomainMesh&>,
                                       VectorConstraint<const analysis::AnalysisDomainMesh&>>;

// using VectorConstraint =
//    Constraint_t<linear_algebra::DynamicVector<double>, linear_algebra::JacobianMultiplier, FunctionArg>;

/// @brief Factory to create Constraint objects from input data.
/// @post The return vector will have the same size as @a aInput.
[[nodiscard]] auto make_constraints(const ValidatedConstraints& aInput)
    -> std::vector<Constraint<const analysis::AnalysisDomainMesh&>>;

/// @brief Helper for providing ROL a dual vector for constraints sized with @a aSize.
/// @note The default behavior is for most constraints that are scalar and so the dual vector always has dimension 1.
[[nodiscard]] auto make_dual_vector(unsigned int aSize = 1) -> linear_algebra::DynamicVector<double>;

namespace detail
{
[[nodiscard]] auto make_constraint(const core::ValidatedInputTypeWrapper<input_parser::constraint>& aConstraintInput)
    -> Constraint<const analysis::AnalysisDomainMesh&>;

}  // namespace detail
}  // namespace plato::criteria::library

#endif
