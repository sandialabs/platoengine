#ifndef PLATO_THIRD_PARTY_INTEGRATION_SNOPT_SNOPTTYPES
#define PLATO_THIRD_PARTY_INTEGRATION_SNOPT_SNOPTTYPES

#include <cstdint>
#include <utility>
#include <vector>

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::third_party_integration::snopt
{
/// @brief Value that SNOPT uses to denote unbounded domains
constexpr static auto kSNOPTUnbounded = double{1e20};

using IndexType = int;

using CriterionType =
    core::Function<const linear_algebra::DynamicVector<double>&,
                   core::FunctionInfo<double, core::evaluation::kFunction>,
                   core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFirstDerivative>>;

enum struct Linearity : std::uint8_t
{
    kLinear,
    kNonlinear
};

enum struct ConstraintType : std::uint8_t
{
    kEqualTo,
    kLesserThan,
    kGreaterThan
};

/// @brief Holds data describing a constraint: Function for evaluating the constraint, the constraint target, and
/// whether or not it is a linear function.
struct InterfaceConstraintData
{
    using ConstraintFunction =
        core::Function<const linear_algebra::DynamicVector<double>&,
                       core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFunction>,
                       core::FunctionInfo<linear_algebra::JacobianMultiplier, core::evaluation::kFirstDerivative>>;

    ConstraintFunction mFunction;
    std::vector<double> mTargets;
    Linearity mLinearity = Linearity::kLinear;
    std::size_t mConstraintDimension = 1U;
    ConstraintType mConstraintType = ConstraintType::kEqualTo;
};

using ObjectiveType = CriterionType;
using InterfaceConstraintType = InterfaceConstraintData;
using InterfaceConstraintVectorType = std::vector<InterfaceConstraintType>;
using SNOPTBounds = std::pair<std::vector<double>, std::vector<double>>;
using ConstraintFunctionArgument = linear_algebra::DynamicVector<double>;
}  // namespace plato::third_party_integration::snopt

#endif
