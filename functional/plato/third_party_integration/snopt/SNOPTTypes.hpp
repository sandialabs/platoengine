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
    core::Function<double, linear_algebra::DynamicVector<double>, const linear_algebra::DynamicVector<double> &>;

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
}  // namespace plato::third_party_integration::snopt

#endif
