#ifndef PLATO_CRITERIA_EXTENSION_UNITTEST_HIMMELBLAUVECTORFUNCTION
#define PLATO_CRITERIA_EXTENSION_UNITTEST_HIMMELBLAUVECTORFUNCTION

#include "plato/core/Function.hpp"
#include "plato/core/FunctionHelpers.hpp"
#include "plato/linear_algebra/DynamicVector.hpp"
#include "plato/linear_algebra/JacobianMultiplier.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::test_utilities
{
struct Himmelblau;
}

namespace plato::criteria::extension::unittest
{

using VectorObjectiveInfo = core::FunctionInfo<linear_algebra::DynamicVector<double>, core::evaluation::kFunction>;
using VectorObjectiveFirstDerivativeInfo =
    core::FunctionInfo<linear_algebra::JacobianMultiplier, core::evaluation::kFirstDerivative>;
using VectorObjectiveFirstDerivativeAdjointInfo = core::FunctionInfo<linear_algebra::AdjointJacobianMultiplier,
                                                                     core::evaluation::kFirstDerivative,
                                                                     core::MatrixOrdering::kAdjoint>;
using VectorObjectiveFunction = core::Function<linear_algebra::DynamicVector<double>,
                                               VectorObjectiveInfo,
                                               VectorObjectiveFirstDerivativeInfo,
                                               VectorObjectiveFirstDerivativeAdjointInfo>;

using FirstHimmelblau = ::plato::utilities::NamedType<test_utilities::Himmelblau, struct FirstHimmelblauTag>;
using SecondHimmelblau = ::plato::utilities::NamedType<test_utilities::Himmelblau, struct SecondHimmelblauTag>;

/// @brief Creates a vector objective Function using two Himmelblau functions, with an interface that uses DynamicVector
[[nodiscard]] auto make_vector_himmelblau_dynamic_vector_objective_function(const FirstHimmelblau& aFirstHimmelblau,
                                                                            const SecondHimmelblau& aSecondHimmelblau)
    -> VectorObjectiveFunction;
}  // namespace plato::criteria::extension::unittest

#endif
