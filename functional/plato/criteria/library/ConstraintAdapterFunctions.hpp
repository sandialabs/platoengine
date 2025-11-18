#ifndef PLATO_CRITERIA_LIBRARY_CONSTRAINTADAPTERFUNCTIONS
#define PLATO_CRITERIA_LIBRARY_CONSTRAINTADAPTERFUNCTIONS

#include <set>
#include <vector>

#include "plato/core/Function.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"

namespace plato::criteria::library
{
using ConstraintAdapterFunction =
    core::Function<const linear_algebra::DynamicVector<double>&,
                   core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFunction>,
                   core::FunctionInfo<linear_algebra::JacobianMultiplier, core::evaluation::kFirstDerivative>,
                   core::FunctionInfo<linear_algebra::AdjointJacobianMultiplier,
                                      core::evaluation::kFirstDerivative,
                                      core::MatrixOrdering::kAdjoint>>;

/// @brief Creates a vector function that maps another vector function's output to a subset based on the component
/// indices in @a aIndices.
///
/// This is a linear mapping. For example, if the subset indices are 1 and 3 in 0-based indexing and the input dimension
/// is 4, then this implements:
/// @code
/// A = [0 1 0 0; 0 0 0 1];
/// y = A * x;
/// @endcode
[[nodiscard]] auto make_vector_subset_function(const std::set<std::size_t>& aIndices) -> ConstraintAdapterFunction;

/// @brief Creates a vector function that subtracts a vector of target values from a vector.
[[nodiscard]] auto make_target_offset_function(std::vector<double> aTargets) -> ConstraintAdapterFunction;

/// @brief Creates a vector function that subtracts a single scalar value from a vector.
[[nodiscard]] auto make_target_offset_function(double aTarget) -> ConstraintAdapterFunction;

}  // namespace plato::criteria::library

#endif
