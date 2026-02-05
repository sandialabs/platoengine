#ifndef PLATO_THIRDPARTYINTEGRATION_COMMON_QUADRATURERULES
#define PLATO_THIRDPARTYINTEGRATION_COMMON_QUADRATURERULES

#include <array>
#include <numbers>
#include <numeric>

#include "plato/utilities/MultidimensionalRange.hpp"

namespace plato::third_party_integration::common
{
/// @brief struct for holding 1D quadrature point and weight
struct QuadraturePoint1D
{
    double mPoint;
    double mWeight;
};

/// @brief One dimensional quadrature points and weights for integrating up to third degree polynomials
static constexpr std::array kDegree3OneDimensionalQuadraturePoints = {
    QuadraturePoint1D{.mPoint = -std::numbers::inv_sqrt3_v<double>, .mWeight = 1.},
    QuadraturePoint1D{.mPoint = std::numbers::inv_sqrt3_v<double>, .mWeight = 1.}};

/// @brief Integrate the function @a aFunction over the domain [-1,1] in @tparam kDimensions using a tensor product of
/// 1D quadrature rules @tparam kQuadratureRule.
/// @pre kDimensions must be 2 or 3.
template <int kDimensions, int kPoints, std::array<common::QuadraturePoint1D, kPoints> kQuadratureRule, typename F>
    requires(kDimensions == 2 || kDimensions == 3)
[[nodiscard]] constexpr auto tensor_product_unit_integral(const F& aFunction);

template <int kDimensions, int kPoints, std::array<common::QuadraturePoint1D, kPoints> kQuadratureRule, typename F>
    requires(kDimensions == 2 || kDimensions == 3)
[[nodiscard]] constexpr auto tensor_product_unit_integral(const F& aFunction)
{
    using ResultType = decltype(std::apply(std::declval<F>(), std::array<double, kDimensions>{}));

    constexpr auto tNumberOf1DQuadraturePoints = common::kDegree3OneDimensionalQuadraturePoints.size();
    auto tIndexRanges = std::array<int, kDimensions>{};
    tIndexRanges.fill(tNumberOf1DQuadraturePoints);
    const auto tIndices = std::apply(
        [](const auto... aIndexRanges) { return utilities::MultidimensionalRange{aIndexRanges...}; }, tIndexRanges);

    return std::accumulate(
        tIndices.begin(), tIndices.end(), ResultType{},
        [&aFunction](const auto& aCurrentAccumulatedValue, const auto& aIndices)
        {
            return aCurrentAccumulatedValue +
                   std::apply(
                       [&aFunction](const auto... aCurrentIndices)
                       {
                           const double tTotalWeight = (kQuadratureRule[aCurrentIndices].mWeight * ...);
                           return aFunction(kQuadratureRule[aCurrentIndices].mPoint...) * tTotalWeight;
                       },
                       aIndices);
        });
}
}  // namespace plato::third_party_integration::common

#endif
