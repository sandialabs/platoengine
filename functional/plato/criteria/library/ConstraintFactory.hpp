#ifndef PLATO_CRITERIA_LIBRARY_CONSTRAINTFACTORY
#define PLATO_CRITERIA_LIBRARY_CONSTRAINTFACTORY

#include <string>

#include "plato/core/Function.hpp"
#include "plato/core/ValidatedInputTypeWrapper.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::core
{
struct MeshProxy;
}

namespace plato::criteria::library
{
using ValidatedConstraints =
    core::ValidatedInputTypeWrapper<std::vector<core::ValidatedInputTypeWrapper<input_parser::constraint>>>;

/// @brief Holds members for defining a Constraint
/// @tparam FunctionArg The argument of the function used to define the constraint.
///   Typically, this is either MeshProxy or a vector type such as DynamicVector.
template <typename FunctionArg>
struct Constraint
{
    using ConstraintFunction = core::Function<double, linear_algebra::DynamicVector<double>, FunctionArg>;

    std::string mName;
    ConstraintFunction mConstraintFunction;
    double mConstraintTarget = 0;
    bool mLinear = false;
};

/// @brief Factory to create Constraint objects from input data.
/// @post The return vector will have the same size as @a aInput.
[[nodiscard]] std::vector<Constraint<const core::MeshProxy&>> make_constraints(const ValidatedConstraints& aInput);

/// @brief Helper for providing ROL a dual vector for constraints.
/// @note Currently, constraints are scalar, and so the dual vector always has dimension 1.
[[nodiscard]] linear_algebra::DynamicVector<double> make_dual_vector();

namespace detail
{
[[nodiscard]] Constraint<const core::MeshProxy&> make_constraint(
    const core::ValidatedInputTypeWrapper<input_parser::constraint>& aConstraintInput);

}  // namespace detail
}  // namespace plato::criteria::library

#endif
