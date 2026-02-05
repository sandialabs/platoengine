#include "plato/third_party_integration/stk_io/Quadrilateral.hpp"

#include <cassert>

#include "plato/third_party_integration/common/QuadratureRules.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::stk_io
{
namespace
{
constexpr int kDimensions{2};
constexpr auto kQuadraturePoints = common::kDegree3OneDimensionalQuadraturePoints;

[[nodiscard]] double change_of_area_factor(const common::Vector3& aEtaTangentVector,
                                           const common::Vector3& aXiTangentVector)
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

Quadrilateral::VertexSensitivities operator+(const Quadrilateral::VertexSensitivities& aLeftHandSide,
                                             const Quadrilateral::VertexSensitivities& aRightHandSide)
{
    return Quadrilateral::VertexSensitivities{std::array{
        aLeftHandSide.mValue[0] + aRightHandSide.mValue[0], aLeftHandSide.mValue[1] + aRightHandSide.mValue[1],
        aLeftHandSide.mValue[2] + aRightHandSide.mValue[2], aLeftHandSide.mValue[3] + aRightHandSide.mValue[3]}};
}

Quadrilateral::VertexSensitivities operator*(const Quadrilateral::VertexSensitivities& aSensitivities,
                                             const double aScalar)
{
    return Quadrilateral::VertexSensitivities{
        std::array{aSensitivities.mValue[0] * aScalar, aSensitivities.mValue[1] * aScalar,
                   aSensitivities.mValue[2] * aScalar, aSensitivities.mValue[3] * aScalar}};
}

common::Vector3 Quadrilateral::etaTangentVector(const double aXiCoordinate) const
{
    return {.x = ((p00.x * (aXiCoordinate - 1)) - (p10.x * (aXiCoordinate + 1)) + (p11.x * (aXiCoordinate + 1)) -
                  (p01.x * (aXiCoordinate - 1))) /
                 4,
            .y = ((p00.y * (aXiCoordinate - 1)) - (p10.y * (aXiCoordinate + 1)) + (p11.y * (aXiCoordinate + 1)) -
                  (p01.y * (aXiCoordinate - 1))) /
                 4,
            .z = ((p00.z * (aXiCoordinate - 1)) - (p10.z * (aXiCoordinate + 1)) + (p11.z * (aXiCoordinate + 1)) -
                  (p01.z * (aXiCoordinate - 1))) /
                 4};
}

common::Vector3 Quadrilateral::xiTangentVector(const double aEtaCoordinate) const
{
    return {.x = ((p00.x * (aEtaCoordinate - 1)) - (p10.x * (aEtaCoordinate - 1)) + (p11.x * (aEtaCoordinate + 1)) -
                  (p01.x * (aEtaCoordinate + 1))) /
                 4,
            .y = ((p00.y * (aEtaCoordinate - 1)) - (p10.y * (aEtaCoordinate - 1)) + (p11.y * (aEtaCoordinate + 1)) -
                  (p01.y * (aEtaCoordinate + 1))) /
                 4,
            .z = ((p00.z * (aEtaCoordinate - 1)) - (p10.z * (aEtaCoordinate - 1)) + (p11.z * (aEtaCoordinate + 1)) -
                  (p01.z * (aEtaCoordinate + 1))) /
                 4};
}

double Quadrilateral::volume() const
{
    return common::tensor_product_unit_integral<kDimensions, kQuadraturePoints.size(), kQuadraturePoints>(
        [*this](const auto tXi, const auto tEta)
        { return change_of_area_factor(etaTangentVector(tXi), xiTangentVector(tEta)); });
}

common::Coordinate Quadrilateral::centroid() const
{
    return common::tensor_product_unit_integral<kDimensions, kQuadraturePoints.size(), kQuadraturePoints>(
               [*this](const auto tXi, const auto tEta)
               {
                   const double tM1 = ((p00.x * (tEta - 1) * (tXi - 1)) - (p10.x * (tEta - 1) * (tXi + 1)) +
                                       (p01.x * (tEta + 1) * (tXi + 1)) - (p11.x * (tEta + 1) * (tXi - 1))) /
                                      4;
                   const double tM2 = ((p00.y * (tEta - 1) * (tXi - 1)) - (p10.y * (tEta - 1) * (tXi + 1)) +
                                       (p01.y * (tEta + 1) * (tXi + 1)) - (p11.y * (tEta + 1) * (tXi - 1))) /
                                      4;
                   const double tM3 = ((p00.z * (tEta - 1) * (tXi - 1)) - (p10.z * (tEta - 1) * (tXi + 1)) +
                                       (p11.z * (tEta + 1) * (tXi + 1)) - (p11.z * (tEta + 1) * (tXi - 1))) /
                                      4;

                   return common::Coordinate{tM1, tM2, tM3} *
                          change_of_area_factor(etaTangentVector(tXi), xiTangentVector(tEta));
               }) /
           volume();
}

auto Quadrilateral::volumeVertexSensitivities() const -> std::array<common::Vector3, kNumVertices>
{
    return common::tensor_product_unit_integral<kDimensions, kQuadraturePoints.size(), kQuadraturePoints>(
               [*this](const auto tXi, const auto tEta)
               {
                   const common::Vector3 tEtaTangentVector = etaTangentVector(tXi);

                   const common::Vector3 tDa2dp00{(tXi - 1) / 4, (tXi - 1) / 4, (tXi - 1) / 4};
                   const common::Vector3 tDa2dp10{-(tXi + 1) / 4, -(tXi + 1) / 4, -(tXi + 1) / 4};
                   const common::Vector3 tDa2dp11{(tXi + 1) / 4, (tXi + 1) / 4, (tXi + 1) / 4};
                   const common::Vector3 tDa2dp01{-(tXi - 1) / 4, -(tXi - 1) / 4, -(tXi - 1) / 4};

                   const auto tXiTangentVector = xiTangentVector(tEta);

                   const double tA11 = tXiTangentVector.x * tXiTangentVector.x +
                                       tXiTangentVector.y * tXiTangentVector.y +
                                       tXiTangentVector.z * tXiTangentVector.z;
                   const double tA12 = tXiTangentVector.x * tEtaTangentVector.x +
                                       tXiTangentVector.y * tEtaTangentVector.y +
                                       tXiTangentVector.z * tEtaTangentVector.z;
                   const double tA22 = tEtaTangentVector.x * tEtaTangentVector.x +
                                       tEtaTangentVector.y * tEtaTangentVector.y +
                                       tEtaTangentVector.z * tEtaTangentVector.z;

                   const common::Vector3 tDa1dp00{(tEta - 1) / 4, (tEta - 1) / 4, (tEta - 1) / 4};
                   const common::Vector3 tDa1dp10{-(tEta - 1) / 4, -(tEta - 1) / 4, -(tEta - 1) / 4};
                   const common::Vector3 tDa1dp11{(tEta + 1) / 4, (tEta + 1) / 4, (tEta + 1) / 4};
                   const common::Vector3 tDa1dp01{-(tEta + 1) / 4, -(tEta + 1) / 4, -(tEta + 1) / 4};

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

                   return VertexSensitivities{
                       std::array{common::Vector3{tGetSensitivityComponent.template operator()<0>(tDa1dp00, tDa2dp00),
                                                  tGetSensitivityComponent.template operator()<1>(tDa1dp00, tDa2dp00),
                                                  tGetSensitivityComponent.template operator()<2>(tDa1dp00, tDa2dp00)} *
                                      tScale,
                                  common::Vector3{tGetSensitivityComponent.template operator()<0>(tDa1dp10, tDa2dp10),
                                                  tGetSensitivityComponent.template operator()<1>(tDa1dp10, tDa2dp10),
                                                  tGetSensitivityComponent.template operator()<2>(tDa1dp10, tDa2dp10)} *
                                      tScale,
                                  common::Vector3{tGetSensitivityComponent.template operator()<0>(tDa1dp11, tDa2dp11),
                                                  tGetSensitivityComponent.template operator()<1>(tDa1dp11, tDa2dp11),
                                                  tGetSensitivityComponent.template operator()<2>(tDa1dp11, tDa2dp11)} *
                                      tScale,
                                  common::Vector3{tGetSensitivityComponent.template operator()<0>(tDa1dp01, tDa2dp01),
                                                  tGetSensitivityComponent.template operator()<1>(tDa1dp01, tDa2dp01),
                                                  tGetSensitivityComponent.template operator()<2>(tDa1dp01, tDa2dp01)} *
                                      tScale}};
               })
        .mValue;
}
}  // namespace plato::third_party_integration::stk_io
