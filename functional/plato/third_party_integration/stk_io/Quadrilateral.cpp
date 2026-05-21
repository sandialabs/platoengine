#include "plato/third_party_integration/stk_io/Quadrilateral.hpp"

#include <array>
#include <cassert>
#include <cmath>

#include "plato/third_party_integration/common/QuadratureRules.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::stk_io
{
namespace
{
constexpr int kDimensions{2};
constexpr auto kQuadraturePoints = common::kDegree3OneDimensionalQuadraturePoints;

[[nodiscard]] auto change_of_area_factor(const common::Vector3& aEtaTangentVector,
                                         const common::Vector3& aXiTangentVector) -> double
{
    const double tA11 = aXiTangentVector.x * aXiTangentVector.x + aXiTangentVector.y * aXiTangentVector.y +
                        aXiTangentVector.z * aXiTangentVector.z;
    const double tA12 = aXiTangentVector.x * aEtaTangentVector.x + aXiTangentVector.y * aEtaTangentVector.y +
                        aXiTangentVector.z * aEtaTangentVector.z;
    const double tA22 = aEtaTangentVector.x * aEtaTangentVector.x + aEtaTangentVector.y * aEtaTangentVector.y +
                        aEtaTangentVector.z * aEtaTangentVector.z;

    return std::sqrt(tA11 * tA22 - tA12 * tA12);
}

template <int kDim, common::Container3Type V>
auto vector_with_zeroed_components(const V& aVector)
{
    if constexpr (kDim == 0)
    {
        return V{aVector.x, 0.0, 0.0};
    }
    else if constexpr (kDim == 1)
    {
        return V{0.0, aVector.y, 0.0};
    }
    return V{0.0, 0.0, aVector.z};
}
}  // namespace

auto operator+(const Quadrilateral::VertexSensitivities& aLeftHandSide,
               const Quadrilateral::VertexSensitivities& aRightHandSide) -> Quadrilateral::VertexSensitivities
{
    return Quadrilateral::VertexSensitivities{std::array{
        aLeftHandSide.mValue[0] + aRightHandSide.mValue[0], aLeftHandSide.mValue[1] + aRightHandSide.mValue[1],
        aLeftHandSide.mValue[2] + aRightHandSide.mValue[2], aLeftHandSide.mValue[3] + aRightHandSide.mValue[3]}};
}

auto operator*(const Quadrilateral::VertexSensitivities& aSensitivities,
               const double aScalar) -> Quadrilateral::VertexSensitivities
{
    return Quadrilateral::VertexSensitivities{
        std::array{aSensitivities.mValue[0] * aScalar, aSensitivities.mValue[1] * aScalar,
                   aSensitivities.mValue[2] * aScalar, aSensitivities.mValue[3] * aScalar}};
}

auto Quadrilateral::etaTangentVector(const double aXiCoordinate) const -> common::Vector3
{
    return {.x = ((mP00.x * (aXiCoordinate - 1)) - (mP10.x * (aXiCoordinate + 1)) + (mP11.x * (aXiCoordinate + 1)) -
                  (mP01.x * (aXiCoordinate - 1))) /
                 4,
            .y = ((mP00.y * (aXiCoordinate - 1)) - (mP10.y * (aXiCoordinate + 1)) + (mP11.y * (aXiCoordinate + 1)) -
                  (mP01.y * (aXiCoordinate - 1))) /
                 4,
            .z = ((mP00.z * (aXiCoordinate - 1)) - (mP10.z * (aXiCoordinate + 1)) + (mP11.z * (aXiCoordinate + 1)) -
                  (mP01.z * (aXiCoordinate - 1))) /
                 4};
}

auto Quadrilateral::xiTangentVector(const double aEtaCoordinate) const -> common::Vector3
{
    return {.x = ((mP00.x * (aEtaCoordinate - 1)) - (mP10.x * (aEtaCoordinate - 1)) + (mP11.x * (aEtaCoordinate + 1)) -
                  (mP01.x * (aEtaCoordinate + 1))) /
                 4,
            .y = ((mP00.y * (aEtaCoordinate - 1)) - (mP10.y * (aEtaCoordinate - 1)) + (mP11.y * (aEtaCoordinate + 1)) -
                  (mP01.y * (aEtaCoordinate + 1))) /
                 4,
            .z = ((mP00.z * (aEtaCoordinate - 1)) - (mP10.z * (aEtaCoordinate - 1)) + (mP11.z * (aEtaCoordinate + 1)) -
                  (mP01.z * (aEtaCoordinate + 1))) /
                 4};
}

auto Quadrilateral::volume() const -> double
{
    return common::tensor_product_unit_integral<kDimensions, kQuadraturePoints.size(), kQuadraturePoints>(
        [*this](const auto aXi, const auto aEta)
        { return change_of_area_factor(etaTangentVector(aXi), xiTangentVector(aEta)); });
}

auto Quadrilateral::centroid() const -> common::Coordinate
{
    return common::tensor_product_unit_integral<kDimensions, kQuadraturePoints.size(), kQuadraturePoints>(
               [*this](const auto aXi, const auto aEta)
               {
                   const double tM1 = ((mP00.x * (aEta - 1) * (aXi - 1)) - (mP10.x * (aEta - 1) * (aXi + 1)) +
                                       (mP01.x * (aEta + 1) * (aXi + 1)) - (mP11.x * (aEta + 1) * (aXi - 1))) /
                                      4;
                   const double tM2 = ((mP00.y * (aEta - 1) * (aXi - 1)) - (mP10.y * (aEta - 1) * (aXi + 1)) +
                                       (mP01.y * (aEta + 1) * (aXi + 1)) - (mP11.y * (aEta + 1) * (aXi - 1))) /
                                      4;
                   const double tM3 = ((mP00.z * (aEta - 1) * (aXi - 1)) - (mP10.z * (aEta - 1) * (aXi + 1)) +
                                       (mP11.z * (aEta + 1) * (aXi + 1)) - (mP11.z * (aEta + 1) * (aXi - 1))) /
                                      4;

                   return common::Coordinate{tM1, tM2, tM3} *
                          change_of_area_factor(etaTangentVector(aXi), xiTangentVector(aEta));
               }) /
           volume();
}

auto Quadrilateral::volumeVertexSensitivities() const -> std::array<common::Vector3, mNumVertices>
{
    return common::tensor_product_unit_integral<kDimensions, kQuadraturePoints.size(), kQuadraturePoints>(
               [*this](const auto aXi, const auto aEta)
               {
                   const common::Vector3 tEtaTangentVector = etaTangentVector(aXi);

                   const common::Vector3 tDa2dmP00{(aXi - 1) / 4, (aXi - 1) / 4, (aXi - 1) / 4};
                   const common::Vector3 tDa2dmP10{-(aXi + 1) / 4, -(aXi + 1) / 4, -(aXi + 1) / 4};
                   const common::Vector3 tDa2dmP11{(aXi + 1) / 4, (aXi + 1) / 4, (aXi + 1) / 4};
                   const common::Vector3 tDa2dmP01{-(aXi - 1) / 4, -(aXi - 1) / 4, -(aXi - 1) / 4};

                   const auto tXiTangentVector = xiTangentVector(aEta);

                   const double tA11 = tXiTangentVector.x * tXiTangentVector.x +
                                       tXiTangentVector.y * tXiTangentVector.y +
                                       tXiTangentVector.z * tXiTangentVector.z;
                   const double tA12 = tXiTangentVector.x * tEtaTangentVector.x +
                                       tXiTangentVector.y * tEtaTangentVector.y +
                                       tXiTangentVector.z * tEtaTangentVector.z;
                   const double tA22 = tEtaTangentVector.x * tEtaTangentVector.x +
                                       tEtaTangentVector.y * tEtaTangentVector.y +
                                       tEtaTangentVector.z * tEtaTangentVector.z;

                   const common::Vector3 tDa1dmP00{(aEta - 1) / 4, (aEta - 1) / 4, (aEta - 1) / 4};
                   const common::Vector3 tDa1dmP10{-(aEta - 1) / 4, -(aEta - 1) / 4, -(aEta - 1) / 4};
                   const common::Vector3 tDa1dmP11{(aEta + 1) / 4, (aEta + 1) / 4, (aEta + 1) / 4};
                   const common::Vector3 tDa1dmP01{-(aEta + 1) / 4, -(aEta + 1) / 4, -(aEta + 1) / 4};

                   const auto tGetSensitivityComponent =
                       [&tA22, &tA11, &tA12, &tXiTangentVector, &tEtaTangentVector]<int kIndex>(common::Vector3 aDa1,
                                                                                                common::Vector3 aDa2)
                   {
                       aDa1 = vector_with_zeroed_components<kIndex>(aDa1);
                       aDa2 = vector_with_zeroed_components<kIndex>(aDa2);
                       return dot(aDa1, tXiTangentVector) * tA22 + tA11 * dot(aDa2, tEtaTangentVector) -
                              tA12 * (dot(aDa1, tEtaTangentVector) + dot(aDa2, tXiTangentVector));
                   };

                   const double tScale = 1.0 / change_of_area_factor(tEtaTangentVector, tXiTangentVector);

                   return VertexSensitivities {
                       std::array{
                           common::Vector3{tGetSensitivityComponent.template operator()<0>(tDa1dmP00, tDa2dmP00),
                                           tGetSensitivityComponent.template operator()<1>(tDa1dmP00, tDa2dmP00),
                                           tGetSensitivityComponent.template operator()<2>(tDa1dmP00, tDa2dmP00)} *
                               tScale,
                           common::Vector3{tGetSensitivityComponent.template operator()<0>(tDa1dmP10, tDa2dmP10),
                                           tGetSensitivityComponent.template operator()<1>(tDa1dmP10, tDa2dmP10),
                                           tGetSensitivityComponent.template operator()<2>(tDa1dmP10, tDa2dmP10)} *
                               tScale,
                           common::Vector3{tGetSensitivityComponent.template operator()<0>(tDa1dmP11, tDa2dmP11),
                                           tGetSensitivityComponent.template operator()<1>(tDa1dmP11, tDa2dmP11),
                                           tGetSensitivityComponent.template operator()<2>(tDa1dmP11, tDa2dmP11)} *
                               tScale,
                           common::Vector3{tGetSensitivityComponent.template operator()<0>(tDa1dmP01, tDa2dmP01),
                                           tGetSensitivityComponent.template operator()<1>(tDa1dmP01, tDa2dmP01),
                                           tGetSensitivityComponent.template operator()<2>(tDa1dmP01, tDa2dmP01)} *
                               tScale}
                   };
               })
        .mValue;
}
}  // namespace plato::third_party_integration::stk_io
