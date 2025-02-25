#ifndef PLATO_TEST_UTILITIES_GRADIENTCHECKER
#define PLATO_TEST_UTILITIES_GRADIENTCHECKER

#include <functional>
#include <ostream>
#include <utility>
#include <vector>

namespace plato::test_utilities
{
/// @brief Gradient checker parameters such as the number of steps sizes at which to compute finite differences and the
/// ratio used to reduce the step size at each iteration.
struct GradientCheckParameters
{
    double mStepDelta = 0.1;
    unsigned int mNumSteps = 8;
    double mInitialStepSize = 1.0;
};

/// @brief A finite difference result pairing the step size and value.
struct StepAndValue
{
    double mStep = 0.0;
    double mValue = 0.0;
};

/// @brief A testing utility for comparing analytically computed gradients with first order finite difference
/// approximations.
///
/// For each function, a directional derivative is used to reduce the computational burden of computing
/// finite difference gradients. For non-linear functions, the first order finite difference approximations should
/// converge to the analytic derivative with slope -1 on a log scale until subtractive cancellation swamps the result.
/// At that point, the errors generally increase with slope 1, creating a "V" shape on a log plot.
template <typename ArgType>
class GradientChecker
{
   public:
    /// @param f Function whose derivative will be approximated with finite differences.
    /// @param df Function that computes the gradient of @a aF analytically. This function is assumed to implement
    ///  a custom directional derivative if it takes two arguments, or use a default implementation (using
    ///  `std::inner_product`) if it takes one argument. The default requires `begin` and `end` functions that provide
    ///  iterators for non-scalar types.
    template <typename F, typename dF>
    GradientChecker(F aF, dF aDf);

    /// @return A sequence of successive finite difference approximation errors computed relative to the analytic
    /// derivative. The size of the returned vector will be the number of steps specified in @a aParameters.
    [[nodiscard]] auto finiteDifferenceErrors(const ArgType& aX,
                                              const ArgType& aDirection,
                                              const GradientCheckParameters& aParameters = {}) const
        -> std::vector<StepAndValue>;

    /// @brief Computes the max deviation from the expected truncation error decrease among the finite difference
    /// approximations.
    ///
    /// This is computed by comparing adjacent FD approximations against the expected first order rate. For example,
    /// if the step delta is 0.1, the expected improvement in error between successive approximations should be about
    /// 0.1. This returns the maximum deviation from this expected rate as a relative difference.
    /// @note This should only be used in the truncation error region of the finite difference approximations.
    /// @note This may not work with the default settings of GradientCheckParameters (mostly `mNumSteps` and
    /// `mInitialStepSize`), and so the table function can help to manually tweak the parameters.
    [[nodiscard]] auto maxFirstOrderTruncationError(const ArgType& aX,
                                                    const ArgType& aDirection,
                                                    const GradientCheckParameters& aParameters = {}) const -> double;

    /// @brief Returns a gradient check table with step sizes, analytic gradient, finite difference approximations and
    /// error.
    auto table(const ArgType& aX, const ArgType& aDirection, const GradientCheckParameters& aParameters = {}) const
        -> std::string;

   private:
    std::function<double(ArgType)> mFunction;
    std::function<double(ArgType, ArgType)> mFunctionDirectionalDerivative;
};

}  // namespace plato::test_utilities

#include "plato/test_utilities/GradientCheckerImpl.hpp"

#endif
