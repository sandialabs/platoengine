#include <cmath>
#include <iomanip>
#include <iterator>
#include <numeric>
#include <type_traits>

namespace plato::test_utilities
{
namespace
{
/// @brief Generates a sequence of first-order finite difference approximations
///
/// The first evaluation of this generator will compute the finite difference
/// approximation of the function passed on construction at the initial step
/// size given on construction. Subsequent evaluations geometrically reduce the
/// step size by `step_delta`.
template <typename ArgType>
class FiniteDifferenceGenerator
{
   public:
    /// @param aF Function whose derivative will be approximated by finite differences
    /// @param aX The point at which to compute the finite difference approximation
    /// @param aParameters The finite difference parameters for the sequence, such as initial step size and step
    /// reduction.
    template <typename F>
    FiniteDifferenceGenerator(F aF, ArgType aX, ArgType aDirection, const GradientCheckParameters& aParameters);

    [[nodiscard]] auto operator()() -> StepAndValue;

   private:
    std::function<double(ArgType)> mFunction;
    GradientCheckParameters mParameters;
    ArgType mX = 0.0;
    ArgType mDirection = 1.0;
    double mFAtX = 0.0;
    unsigned int mIndex = 0;
};

template <typename ArgType>
template <typename F>
FiniteDifferenceGenerator<ArgType>::FiniteDifferenceGenerator(F aF,
                                                              ArgType aX,
                                                              ArgType aDirection,
                                                              const GradientCheckParameters& aParameters)
    : mFunction{std::move(aF)},
      mParameters{aParameters},
      mX{std::move(aX)},
      mDirection{std::move(aDirection)},
      mFAtX{mFunction(mX)}
{
}

template <typename ArgType>
auto FiniteDifferenceGenerator<ArgType>::operator()() -> StepAndValue
{
    const double tH = mParameters.mInitialStepSize * std::pow(mParameters.mStepDelta, mIndex++);
    return {tH, (mFunction(mX + tH * mDirection) - mFAtX) / tH};
}

struct PrintTableRow
{
    double mStep = 0.0;
    double mAnalytic = 0.0;
    double mFiniteDifference = 0.0;
    double mError = 0.0;
};

template <typename Stream>
auto operator<<(Stream& aStream, const PrintTableRow& aTableRow) -> Stream&
{
    aStream << aTableRow.mStep << " " << aTableRow.mAnalytic << " " << aTableRow.mAnalytic << " " << aTableRow.mError;
    return aStream;
}

template <typename ArgType>
[[nodiscard]] auto finite_difference_sequence(const std::function<double(ArgType)>& aF,
                                              const ArgType& aX,
                                              const ArgType& aDirection,
                                              const GradientCheckParameters& aParameters) -> std::vector<StepAndValue>
{
    auto aFiniteDifferences = std::vector<StepAndValue>{};
    aFiniteDifferences.reserve(aParameters.mNumSteps);
    std::generate_n(std::back_inserter(aFiniteDifferences), aParameters.mNumSteps,
                    FiniteDifferenceGenerator{aF, aX, aDirection, aParameters});
    return aFiniteDifferences;
}

[[nodiscard]] auto finite_difference_error_vs_analytic(const double aAnalyticValue,
                                                       const std::vector<StepAndValue>& aFiniteDifferences)
    -> std::vector<StepAndValue>
{
    auto tErrors = std::vector<StepAndValue>{};
    tErrors.reserve(aFiniteDifferences.size());
    std::transform(aFiniteDifferences.cbegin(), aFiniteDifferences.cend(), std::back_inserter(tErrors),
                   [aAnalyticValue](const StepAndValue& aStepAndFD)
                   {
                       const double tError = std::abs(aAnalyticValue - aStepAndFD.mValue) / std::abs(aAnalyticValue);
                       return StepAndValue{aStepAndFD.mStep, tError};
                   });
    return tErrors;
}

[[nodiscard]] auto print_table(const double aAnalyticValue,
                               const std::vector<StepAndValue>& aFiniteDifferences,
                               const std::vector<StepAndValue>& aErrors) -> std::vector<PrintTableRow>
{
    auto tTable = std::vector<PrintTableRow>{};
    tTable.reserve(aFiniteDifferences.size());
    for (std::size_t tIndex = 0; tIndex < aFiniteDifferences.size(); ++tIndex)
    {
        tTable.push_back(PrintTableRow{/*.mStep=*/aFiniteDifferences.at(tIndex).mStep,
                                       /*.mAnalytic=*/aAnalyticValue,
                                       /*.mFiniteDifference=*/aFiniteDifferences.at(tIndex).mValue,
                                       /*.mError=*/aErrors.at(tIndex).mValue});
    }
    return tTable;
}

template <typename F, typename ArgType>
[[nodiscard]] auto directional_derivative(const F& aF, const ArgType& aArg, const ArgType& aDirection) -> double
{
    if constexpr (std::is_arithmetic_v<ArgType>)
    {
        // Scalar type
        return aDirection * aF(aArg);
    }
    else if constexpr (std::is_invocable_v<F, ArgType, ArgType>)
    {
        // Custom implementation of directional derivative
        return aF(aArg, aDirection);
    }
    else
    {
        // Non-scalar
        using std::begin;
        using std::end;
        const auto tResult = aF(aArg);
        return std::inner_product(begin(tResult), end(tResult), begin(aDirection), 0.0);
    }
    return 0.0;  // Suppress nvcc spurious warning
}

}  // namespace

template <typename ArgType>
template <typename F, typename dF>
GradientChecker<ArgType>::GradientChecker(F aF, dF aDf)
    : mFunction{std::move(aF)},
      mFunctionDirectionalDerivative{[mDf = std::move(aDf)](const ArgType& aArg, const ArgType& aDirection)
                                     { return directional_derivative(mDf, aArg, aDirection); }}
{
}

template <typename ArgType>
auto GradientChecker<ArgType>::finiteDifferenceErrors(const ArgType& aX,
                                                      const ArgType& aDirection,
                                                      const GradientCheckParameters& aParameters) const
    -> std::vector<StepAndValue>
{
    const auto tFiniteDifferences = finite_difference_sequence(mFunction, aX, aDirection, aParameters);
    return finite_difference_error_vs_analytic(mFunctionDirectionalDerivative(aX, aDirection), tFiniteDifferences);
}

template <typename ArgType>
double GradientChecker<ArgType>::maxFirstOrderTruncationError(const ArgType& aX,
                                                              const ArgType& aDirection,
                                                              const GradientCheckParameters& aParameters) const
{
    const auto tErrors = finiteDifferenceErrors(aX, aDirection, aParameters);
    auto tAdjacentQuotients = std::vector<StepAndValue>{};
    tAdjacentQuotients.reserve(tErrors.size());
    std::adjacent_difference(tErrors.cbegin(), tErrors.cend(), std::back_inserter(tAdjacentQuotients),
                             [](const StepAndValue& aStepAndError1, const StepAndValue& aStepAndError2) {
                                 return StepAndValue{aStepAndError1.mStep / aStepAndError2.mStep,
                                                     aStepAndError1.mValue / aStepAndError2.mValue};
                             });

    auto tExpectedTruncationDifference = std::vector<double>{};
    std::transform(std::next(tAdjacentQuotients.cbegin()), tAdjacentQuotients.cend(),
                   std::back_inserter(tExpectedTruncationDifference),
                   [](const StepAndValue& aStemAndErrorQuotient) {
                       return std::fabs(aStemAndErrorQuotient.mValue - aStemAndErrorQuotient.mStep) /
                              aStemAndErrorQuotient.mStep;
                   });
    return *std::max_element(tExpectedTruncationDifference.cbegin(), tExpectedTruncationDifference.cend());
}

template <typename ArgType>
auto GradientChecker<ArgType>::table(const ArgType& aX,
                                     const ArgType& aDirection,
                                     const GradientCheckParameters& aParameters) const -> std::string
{
    const auto tFiniteDifferences = finite_difference_sequence(mFunction, aX, aDirection, aParameters);
    const auto tAnalyticValue = mFunctionDirectionalDerivative(aX, aDirection);
    const auto tErrors = finite_difference_error_vs_analytic(tAnalyticValue, tFiniteDifferences);
    const auto tTable = print_table(tAnalyticValue, tFiniteDifferences, tErrors);

    auto tStream = std::stringstream{};
    tStream << std::setprecision(15);
    std::copy(tTable.cbegin(), tTable.cend(), std::ostream_iterator<PrintTableRow>{tStream, "\n"});
    return tStream.str();
}

namespace detail
{
template <typename Callable>
struct CallableArgImpl
{
};

template <typename F, typename R, typename Arg>
struct CallableArgImpl<R (F::*)(Arg) const>
{
    using ArgType = Arg;
};

template <typename F>
using CallableArg = typename CallableArgImpl<decltype(&F::operator())>::ArgType;
}  // namespace detail

/// Deduction guide for deducing GradientChecker argument type from the function passed to the ctor.
/// Assumes F has a `const` `operator()` member.
template <typename F, typename dF>
GradientChecker(F f, dF df) -> GradientChecker<std::remove_cv_t<std::remove_reference_t<detail::CallableArg<F>>>>;

}  // namespace plato::test_utilities
