#ifndef PLATO_TESTUTILITIES_GRADIENTCHECKER
#define PLATO_TESTUTILITIES_GRADIENTCHECKER

<<<<<<< HEAD
#include "plato/utilities/GradientChecker.hpp"
=======
#include <functional>
#include <vector>
>>>>>>> 0cdde90332 (Fixes some clang-tidy errors in GradientChecker)

namespace plato::test_utilities
{

/// @brief Gradient checker parameters such as the number of steps sizes at which to compute finite differences and the
/// ratio used to reduce the step size at each iteration.
<<<<<<< HEAD
/// @deprecated To maintain backward compatibility this header relays any external users of this GradientChecker.
using GradientCheckParameters = utilities::GradientCheckParameters;
=======
struct GradientCheckParameters
{
    static constexpr auto mStepDeltaDefault = 0.1;
    static constexpr auto mNumStepsDefault = 8;
    static constexpr auto mInitialStepSizeDefault = 1.0;

    double mStepDelta = mStepDeltaDefault;
    unsigned int mNumSteps = mNumStepsDefault;
    double mInitialStepSize = mInitialStepSizeDefault;
};
>>>>>>> 0cdde90332 (Fixes some clang-tidy errors in GradientChecker)

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
