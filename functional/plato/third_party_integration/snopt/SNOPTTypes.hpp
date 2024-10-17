#ifndef PLATO_THIRD_PARTY_INTEGRATION_SNOPT_SNOPTTYPES
#define PLATO_THIRD_PARTY_INTEGRATION_SNOPT_SNOPTTYPES

#include <utility>
#include <vector>

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"

namespace plato::third_party_integration::snopt
{
/// @brief Value that SNOPT uses to denote unbounded domains
constexpr static auto kSNOPTUnbounded = double{1e20};

using IndexType = int;

using CriterionType =
    core::Function<const linear_algebra::DynamicVector<double>&,
                   core::FunctionInfo<double, core::evaluation::kFunction>,
                   core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFirstDerivative>>;

enum struct Linearity
{
    kLinear,
    kNonlinear
};

/// @brief Holds data describing a constraint: Function for evaluating the constraint, the constraint target, and
/// whether or not it is a linear function.
struct ConstraintData
{
    CriterionType mFunction;
    double mTarget = 0;
    Linearity mLinearity = Linearity::kLinear;
};

using ObjectiveType = CriterionType;
using ConstraintType = ConstraintData;
using ConstraintVectorType = std::vector<ConstraintType>;
using SNOPTBounds = std::pair<std::vector<double>, std::vector<double>>;
using ConstraintFunctionArgument = linear_algebra::DynamicVector<double>;
}  // namespace plato::third_party_integration::snopt

#endif
