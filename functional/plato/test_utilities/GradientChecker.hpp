#ifndef PLATO_TESTUTILITIES_GRADIENTCHECKER
#define PLATO_TESTUTILITIES_GRADIENTCHECKER

#include "plato/utilities/GradientChecker.hpp"

namespace plato::test_utilities
{

/// @brief Gradient checker parameters such as the number of steps sizes at which to compute finite differences and the
/// ratio used to reduce the step size at each iteration.
/// @deprecated To maintain backward compatibility this header relays any external users of this GradientChecker.
using GradientCheckParameters = utilities::GradientCheckParameters;

/// @brief A finite difference result pairing the step size and value.
/// @deprecated To maintain backward compatibility this header relays any external users of this GradientChecker.
using StepAndValue = utilities::StepAndValue;

/// @brief A testing utility for comparing analytically computed gradients with first order finite difference
/// approximations.
/// @deprecated To maintain backward compatibility this header relays any external users of this GradientChecker.
template <typename ArgType>
using GradientChecker = utilities::GradientChecker<ArgType>;
}  // namespace plato::test_utilities
#endif
