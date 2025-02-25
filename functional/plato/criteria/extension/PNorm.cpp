#include "plato/criteria/extension/PNorm.hpp"

#include <algorithm>
#include <boost/math/special_functions/sign.hpp>
#include <cmath>

#include "plato/utilities/PairWiseAccumulate.hpp"

namespace plato::criteria::extension
{

auto make_p_norm_function(const double aPower) -> PNormFunction
{
    return core::make_function_with_first_derivative(
        [aPower](const linear_algebra::DynamicVector<double>& aVector)
        { return detail::p_norm_value(aVector.stdVector(), aPower); },
        [aPower](const linear_algebra::DynamicVector<double>& aVector)
        { return linear_algebra::DynamicVector<double>{detail::p_norm_gradient(aVector.stdVector(), aPower)}; });
}

namespace detail
{

double p_norm_value(std::vector<double> aVector, const double aPower)
{
    assert(aPower >= 1.0);
    assert(!aVector.empty());
    return std::pow(detail::p_norm_expression(std::move(aVector), aPower), 1.0 / aPower);
}

auto p_norm_gradient(std::vector<double> aVector, const double aPower) -> std::vector<double>
{
    assert(aPower >= 1.0);
    assert(!aVector.empty());
    const auto tPrefactor = std::pow(detail::p_norm_expression(aVector, aPower), (1.0 / aPower) - 1.0);

    std::transform(aVector.begin(), aVector.end(), aVector.begin(),
                   [aPower, tPrefactor](const auto aValue)
                   { return boost::math::sign(aValue) * std::pow(std::abs(aValue), aPower - 1.0) * tPrefactor; });
    return aVector;
}

auto p_norm_expression(std::vector<double> aVector, const double aPower) -> double
{
    assert(aPower >= 1.0);
    assert(!aVector.empty());
    std::transform(aVector.begin(), aVector.end(), aVector.begin(),
                   [aPower](const double aValue) { return std::pow(std::abs(aValue), aPower); });
    std::sort(aVector.begin(), aVector.end());
    return utilities::pair_wise_accumulate(aVector);
}

}  // namespace detail

}  // namespace plato::criteria::extension
