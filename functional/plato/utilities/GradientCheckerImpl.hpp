#include <cmath>
#include <format>
#include <iterator>
#include <numeric>
#include <type_traits>

#include "plato/utilities/FixedWidthFloatingPointOutput.hpp"
#include "plato/utilities/NamedType.hpp"

namespace plato::utilities
{
namespace impl
{
constexpr inline int kTableEntryWidth = 24;
constexpr inline int kPrecision = 16;

template <typename ArgType>
using Argument = utilities::NamedType<ArgType, struct ArgumentTag>;
template <typename ArgType>
using Direction = utilities::NamedType<ArgType, struct DirectionTag>;

template <typename ArgType>
using Argument = utilities::NamedType<ArgType, struct ArgumentTag>;
template <typename ArgType>
using Direction = utilities::NamedType<ArgType, struct DirectionTag>;

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
    /// @param aFunction Function whose derivative will be approximated by finite differences
    /// @param aArgument The point at which to compute the finite difference approximation
    /// @param aParameters The finite difference parameters for the sequence, such as initial step size and step
    /// reduction.
    template <typename F>
    FiniteDifferenceGenerator(F aFunction,
                              Argument<ArgType> aArgument,
                              Direction<ArgType> aDirection,
                              const GradientCheckParameters& aParameters);

    [[nodiscard]] auto operator()() -> StepAndValue;

   private:
    std::function<double(ArgType)> mFunction;
    GradientCheckParameters mParameters;
    ArgType mArgument = 0.0;
    ArgType mDirection = 1.0;
    double mFAtX = 0.0;
    unsigned int mIndex = 0;
};

template <typename ArgType>
template <typename F>
FiniteDifferenceGenerator<ArgType>::FiniteDifferenceGenerator(F aFunction,
                                                              Argument<ArgType> aArgument,
                                                              Direction<ArgType> aDirection,
                                                              const GradientCheckParameters& aParameters)
    : mFunction{std::move(aFunction)},
      mParameters{aParameters},
      mArgument{std::move(aArgument.mValue)},
      mDirection{std::move(aDirection.mValue)},
      mFAtX{mFunction(mArgument)}
{
}

template <typename ArgType>
auto FiniteDifferenceGenerator<ArgType>::operator()() -> StepAndValue
{
    const double tStep = mParameters.mInitialStepSize * std::pow(mParameters.mStepDelta, mIndex++);
    return {tStep, (mFunction(mArgument + tStep * mDirection) - mFAtX) / tStep};
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
    using TableOutput = utilities::FixedWidthFloatingPointOutput<double, kPrecision, kTableEntryWidth>;
    aStream << TableOutput{aTableRow.mStep} << TableOutput{aTableRow.mAnalytic}
            << TableOutput{aTableRow.mFiniteDifference} << TableOutput{aTableRow.mError};
    return aStream;
}

template <typename ArgType>
[[nodiscard]] auto finite_difference_sequence(const std::function<double(ArgType)>& aFunction,
                                              const Argument<ArgType>& aArgument,
                                              const Direction<ArgType>& aDirection,
                                              const GradientCheckParameters& aParameters) -> std::vector<StepAndValue>
{
    auto tFiniteDifferences = std::vector<StepAndValue>{};
    tFiniteDifferences.reserve(aParameters.mNumSteps);
    std::generate_n(std::back_inserter(tFiniteDifferences), aParameters.mNumSteps,
                    FiniteDifferenceGenerator{aFunction, aArgument, aDirection, aParameters});
    return tFiniteDifferences;
}

[[nodiscard]] [[maybe_unused]] auto finite_difference_error_vs_analytic(
    const double aAnalyticValue, const std::vector<StepAndValue>& aFiniteDifferences) -> std::vector<StepAndValue>
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

[[nodiscard]] [[maybe_unused]] auto print_table(const double aAnalyticValue,
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
[[nodiscard]] auto directional_derivative(const F& aFunction, const ArgType& aArg, const ArgType& aDirection) -> double
{
    if constexpr (std::is_arithmetic_v<ArgType>)
    {
        // Scalar type
        return aDirection * aFunction(aArg);
    }
    else if constexpr (std::is_invocable_v<F, ArgType, ArgType>)
    {
        // Custom implementation of directional derivative
        return aFunction(aArg, aDirection);
    }
    else
    {
        // Non-scalar
        using std::begin;
        using std::end;
        const auto tResult = aFunction(aArg);
        return std::inner_product(begin(tResult), end(tResult), begin(aDirection), 0.0);
    }
    return 0.0;  // Suppress nvcc spurious warning
}

}  // namespace impl

template <typename ArgType>
template <typename F, typename dF>
GradientChecker<ArgType>::GradientChecker(F aFunction, dF aDf)
    : mFunction{std::move(aFunction)},
      mFunctionDirectionalDerivative{[mDf = std::move(aDf)](const ArgType& aArg, const ArgType& aDirection)
                                     { return directional_derivative(mDf, aArg, aDirection); }}
{
}

template <typename ArgType>
auto GradientChecker<ArgType>::finiteDifferenceErrors(const ArgType& aArgument,
                                                      const ArgType& aDirection,
                                                      const GradientCheckParameters& aParameters) const
    -> std::vector<StepAndValue>
{
    const auto tFiniteDifferences = finite_difference_sequence(mFunction, Argument<ArgType>{aArgument},
                                                               Direction<ArgType>{aDirection}, aParameters);
    return finite_difference_error_vs_analytic(mFunctionDirectionalDerivative(aArgument, aDirection),
                                               tFiniteDifferences);
}

template <typename ArgType>
double GradientChecker<ArgType>::maxFirstOrderTruncationError(const ArgType& aArgument,
                                                              const ArgType& aDirection,
                                                              const GradientCheckParameters& aParameters) const
{
    const auto tErrors = finiteDifferenceErrors(aArgument, aDirection, aParameters);
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
auto GradientChecker<ArgType>::header() const -> std::string
{
    constexpr auto tStepLabel = std::string_view{"Step"};
    constexpr auto tAnalyticLabel = std::string_view{"Gradient*Direction"};
    constexpr auto tFiniteDifferenceLabel = std::string_view{"Finite Difference"};
    constexpr auto tErrorLabel = std::string_view{"Relative Error"};

    return std::format("{0:<{4}}{1:<{4}}{2:<{4}}{3:<{4}}", tStepLabel, tAnalyticLabel, tFiniteDifferenceLabel,
                       tErrorLabel, kTableEntryWidth);
}

template <typename ArgType>
auto GradientChecker<ArgType>::table(const ArgType& aArgument,
                                     const ArgType& aDirection,
                                     const GradientCheckParameters& aParameters) const -> std::string
{
    const auto tFiniteDifferences = finite_difference_sequence(mFunction, Argument<ArgType>{aArgument},
                                                               Direction<ArgType>{aDirection}, aParameters);
    const auto tAnalyticValue = mFunctionDirectionalDerivative(aArgument, aDirection);
    const auto tErrors = finite_difference_error_vs_analytic(tAnalyticValue, tFiniteDifferences);
    const auto tTable = print_table(tAnalyticValue, tFiniteDifferences, tErrors);

    auto tStream = std::stringstream{};
    tStream << std::setprecision(impl::kPrecision);
    std::copy(tTable.cbegin(), tTable.cend(), std::ostream_iterator<impl::PrintTableRow>{tStream, "\n"});
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
GradientChecker(F aF, dF aDf) -> GradientChecker<std::remove_cv_t<std::remove_reference_t<detail::CallableArg<F>>>>;

}  // namespace plato::utilities
