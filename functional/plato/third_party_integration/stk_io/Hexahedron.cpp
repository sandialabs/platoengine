#include "plato/third_party_integration/stk_io/Hexahedron.hpp"

#include "plato/third_party_integration/common/QuadratureRules.hpp"
#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::stk_io
{
namespace
{
constexpr auto kHexahedronVolumeFactor = 1.0 / 12.0;
}

auto Hexahedron::volume() const -> double
{
    return ((mP000.mX * mP110.mY * mP100.mZ) - (mP000.mX * mP100.mY * mP110.mZ) + (mP100.mX * mP000.mY * mP110.mZ) -
            (mP100.mX * mP110.mY * mP000.mZ) - (mP110.mX * mP000.mY * mP100.mZ) + (mP110.mX * mP100.mY * mP000.mZ) -
            (mP000.mX * mP100.mY * mP010.mZ) + (mP000.mX * mP010.mY * mP100.mZ) + (mP100.mX * mP000.mY * mP010.mZ) -
            (mP100.mX * mP010.mY * mP000.mZ) - (mP010.mX * mP000.mY * mP100.mZ) + (mP010.mX * mP100.mY * mP000.mZ) +
            (mP000.mX * mP100.mY * mP001.mZ) - (mP000.mX * mP110.mY * mP010.mZ) + (mP000.mX * mP010.mY * mP110.mZ) -
            (mP000.mX * mP001.mY * mP100.mZ) - (mP100.mX * mP000.mY * mP001.mZ) + (mP100.mX * mP001.mY * mP000.mZ) +
            (mP110.mX * mP000.mY * mP010.mZ) - (mP110.mX * mP010.mY * mP000.mZ) - (mP010.mX * mP000.mY * mP110.mZ) +
            (mP010.mX * mP110.mY * mP000.mZ) + (mP001.mX * mP000.mY * mP100.mZ) - (mP001.mX * mP100.mY * mP000.mZ) +
            (mP000.mX * mP100.mY * mP101.mZ) - (mP000.mX * mP101.mY * mP100.mZ) - (mP100.mX * mP000.mY * mP101.mZ) -
            (mP100.mX * mP110.mY * mP010.mZ) + (mP100.mX * mP010.mY * mP110.mZ) + (mP100.mX * mP101.mY * mP000.mZ) +
            (mP110.mX * mP100.mY * mP010.mZ) - (mP110.mX * mP010.mY * mP100.mZ) - (mP010.mX * mP100.mY * mP110.mZ) +
            (mP010.mX * mP110.mY * mP100.mZ) + (mP101.mX * mP000.mY * mP100.mZ) - (mP101.mX * mP100.mY * mP000.mZ) -
            (mP000.mX * mP010.mY * mP001.mZ) + (mP000.mX * mP001.mY * mP010.mZ) + (mP010.mX * mP000.mY * mP001.mZ) -
            (mP010.mX * mP001.mY * mP000.mZ) - (mP001.mX * mP000.mY * mP010.mZ) + (mP001.mX * mP010.mY * mP000.mZ) +
            (mP100.mX * mP110.mY * mP101.mZ) - (mP100.mX * mP101.mY * mP110.mZ) - (mP110.mX * mP100.mY * mP101.mZ) +
            (mP110.mX * mP101.mY * mP100.mZ) + (mP101.mX * mP100.mY * mP110.mZ) - (mP101.mX * mP110.mY * mP100.mZ) -
            (mP000.mX * mP001.mY * mP101.mZ) + (mP000.mX * mP101.mY * mP001.mZ) + (mP100.mX * mP110.mY * mP111.mZ) -
            (mP100.mX * mP111.mY * mP110.mZ) - (mP110.mX * mP100.mY * mP111.mZ) + (mP110.mX * mP111.mY * mP100.mZ) +
            (mP001.mX * mP000.mY * mP101.mZ) - (mP001.mX * mP101.mY * mP000.mZ) - (mP101.mX * mP000.mY * mP001.mZ) +
            (mP101.mX * mP001.mY * mP000.mZ) + (mP111.mX * mP100.mY * mP110.mZ) - (mP111.mX * mP110.mY * mP100.mZ) -
            (mP000.mX * mP010.mY * mP011.mZ) + (mP000.mX * mP011.mY * mP010.mZ) - (mP100.mX * mP001.mY * mP101.mZ) +
            (mP100.mX * mP101.mY * mP001.mZ) + (mP010.mX * mP000.mY * mP011.mZ) - (mP010.mX * mP011.mY * mP000.mZ) +
            (mP001.mX * mP100.mY * mP101.mZ) - (mP001.mX * mP101.mY * mP100.mZ) - (mP101.mX * mP100.mY * mP001.mZ) +
            (mP101.mX * mP001.mY * mP100.mZ) - (mP011.mX * mP000.mY * mP010.mZ) + (mP011.mX * mP010.mY * mP000.mZ) +
            (mP000.mX * mP001.mY * mP011.mZ) - (mP000.mX * mP011.mY * mP001.mZ) + (mP110.mX * mP010.mY * mP111.mZ) -
            (mP110.mX * mP111.mY * mP010.mZ) - (mP010.mX * mP110.mY * mP111.mZ) + (mP010.mX * mP111.mY * mP110.mZ) -
            (mP001.mX * mP000.mY * mP011.mZ) + (mP001.mX * mP011.mY * mP000.mZ) + (mP111.mX * mP110.mY * mP010.mZ) -
            (mP111.mX * mP010.mY * mP110.mZ) + (mP011.mX * mP000.mY * mP001.mZ) - (mP011.mX * mP001.mY * mP000.mZ) -
            (mP100.mX * mP101.mY * mP111.mZ) + (mP100.mX * mP111.mY * mP101.mZ) + (mP110.mX * mP010.mY * mP011.mZ) -
            (mP110.mX * mP011.mY * mP010.mZ) - (mP010.mX * mP110.mY * mP011.mZ) + (mP010.mX * mP011.mY * mP110.mZ) +
            (mP101.mX * mP100.mY * mP111.mZ) - (mP101.mX * mP111.mY * mP100.mZ) - (mP111.mX * mP100.mY * mP101.mZ) +
            (mP111.mX * mP101.mY * mP100.mZ) + (mP011.mX * mP110.mY * mP010.mZ) - (mP011.mX * mP010.mY * mP110.mZ) -
            (mP110.mX * mP101.mY * mP111.mZ) + (mP110.mX * mP111.mY * mP101.mZ) + (mP101.mX * mP110.mY * mP111.mZ) -
            (mP101.mX * mP111.mY * mP110.mZ) - (mP111.mX * mP110.mY * mP101.mZ) + (mP111.mX * mP101.mY * mP110.mZ) +
            (mP010.mX * mP001.mY * mP011.mZ) - (mP010.mX * mP011.mY * mP001.mZ) - (mP001.mX * mP010.mY * mP011.mZ) +
            (mP001.mX * mP011.mY * mP010.mZ) + (mP011.mX * mP010.mY * mP001.mZ) - (mP011.mX * mP001.mY * mP010.mZ) -
            (mP110.mX * mP111.mY * mP011.mZ) + (mP110.mX * mP011.mY * mP111.mZ) + (mP001.mX * mP101.mY * mP111.mZ) -
            (mP001.mX * mP111.mY * mP101.mZ) - (mP101.mX * mP001.mY * mP111.mZ) + (mP101.mX * mP111.mY * mP001.mZ) +
            (mP111.mX * mP110.mY * mP011.mZ) + (mP111.mX * mP001.mY * mP101.mZ) - (mP111.mX * mP101.mY * mP001.mZ) -
            (mP111.mX * mP011.mY * mP110.mZ) - (mP011.mX * mP110.mY * mP111.mZ) + (mP011.mX * mP111.mY * mP110.mZ) -
            (mP010.mX * mP111.mY * mP011.mZ) + (mP010.mX * mP011.mY * mP111.mZ) + (mP001.mX * mP101.mY * mP011.mZ) -
            (mP001.mX * mP011.mY * mP101.mZ) - (mP101.mX * mP001.mY * mP011.mZ) + (mP101.mX * mP011.mY * mP001.mZ) +
            (mP111.mX * mP010.mY * mP011.mZ) - (mP111.mX * mP011.mY * mP010.mZ) - (mP011.mX * mP010.mY * mP111.mZ) +
            (mP011.mX * mP001.mY * mP101.mZ) - (mP011.mX * mP101.mY * mP001.mZ) + (mP011.mX * mP111.mY * mP010.mZ) +
            (mP001.mX * mP111.mY * mP011.mZ) - (mP001.mX * mP011.mY * mP111.mZ) - (mP111.mX * mP001.mY * mP011.mZ) +
            (mP111.mX * mP011.mY * mP001.mZ) + (mP011.mX * mP001.mY * mP111.mZ) - (mP011.mX * mP111.mY * mP001.mZ) +
            (mP101.mX * mP111.mY * mP011.mZ) - (mP101.mX * mP011.mY * mP111.mZ) - (mP111.mX * mP101.mY * mP011.mZ) +
            (mP111.mX * mP011.mY * mP101.mZ) + (mP011.mX * mP101.mY * mP111.mZ) - (mP011.mX * mP111.mY * mP101.mZ)) *
           kHexahedronVolumeFactor;
}

auto Hexahedron::centroid() const -> common::Coordinate
{
    constexpr int tDimensions{3};
    constexpr auto tQuadraturePoints = common::kDegree3OneDimensionalQuadraturePoints;
    constexpr auto tNumberOfPoints = 8.0;
    return common::tensor_product_unit_integral<tDimensions, tQuadraturePoints.size(), tQuadraturePoints>(
               [*this](const auto aXi, const auto aEta, const auto aMu)
               {
                   const double tJ11 = ((mP100.mX * (aEta - 1) * (aMu - 1)) - (mP000.mX * (aEta - 1) * (aMu - 1)) -
                                        (mP110.mX * (aEta + 1) * (aMu - 1)) + (mP010.mX * (aEta + 1) * (aMu - 1)) +
                                        (mP001.mX * (aEta - 1) * (aMu + 1)) - (mP101.mX * (aEta - 1) * (aMu + 1)) +
                                        (mP111.mX * (aEta + 1) * (aMu + 1)) - (mP011.mX * (aEta + 1) * (aMu + 1))) /
                                       tNumberOfPoints;
                   const double tJ12 = ((mP100.mY * (aEta - 1) * (aMu - 1)) - (mP000.mY * (aEta - 1) * (aMu - 1)) -
                                        (mP110.mY * (aEta + 1) * (aMu - 1)) + (mP010.mY * (aEta + 1) * (aMu - 1)) +
                                        (mP001.mY * (aEta - 1) * (aMu + 1)) - (mP101.mY * (aEta - 1) * (aMu + 1)) +
                                        (mP111.mY * (aEta + 1) * (aMu + 1)) - (mP011.mY * (aEta + 1) * (aMu + 1))) /
                                       tNumberOfPoints;
                   const double tJ13 = ((mP100.mZ * (aEta - 1) * (aMu - 1)) - (mP000.mZ * (aEta - 1) * (aMu - 1)) -
                                        (mP110.mZ * (aEta + 1) * (aMu - 1)) + (mP010.mZ * (aEta + 1) * (aMu - 1)) +
                                        (mP001.mZ * (aEta - 1) * (aMu + 1)) - (mP101.mZ * (aEta - 1) * (aMu + 1)) +
                                        (mP111.mZ * (aEta + 1) * (aMu + 1)) - (mP011.mZ * (aEta + 1) * (aMu + 1))) /
                                       tNumberOfPoints;
                   const double tJ21 = ((mP100.mX * (aMu - 1) * (aXi + 1)) - (mP000.mX * (aMu - 1) * (aXi - 1)) -
                                        (mP110.mX * (aMu - 1) * (aXi + 1)) + (mP010.mX * (aMu - 1) * (aXi - 1)) +
                                        (mP001.mX * (aMu + 1) * (aXi - 1)) - (mP101.mX * (aMu + 1) * (aXi + 1)) +
                                        (mP111.mX * (aMu + 1) * (aXi + 1)) - (mP011.mX * (aMu + 1) * (aXi - 1))) /
                                       tNumberOfPoints;
                   const double tJ22 = ((mP100.mY * (aMu - 1) * (aXi + 1)) - (mP000.mY * (aMu - 1) * (aXi - 1)) -
                                        (mP110.mY * (aMu - 1) * (aXi + 1)) + (mP010.mY * (aMu - 1) * (aXi - 1)) +
                                        (mP001.mY * (aMu + 1) * (aXi - 1)) - (mP101.mY * (aMu + 1) * (aXi + 1)) +
                                        (mP111.mY * (aMu + 1) * (aXi + 1)) - (mP011.mY * (aMu + 1) * (aXi - 1))) /
                                       tNumberOfPoints;
                   const double tJ23 = ((mP100.mZ * (aMu - 1) * (aXi + 1)) - (mP000.mZ * (aMu - 1) * (aXi - 1)) -
                                        (mP110.mZ * (aMu - 1) * (aXi + 1)) + (mP010.mZ * (aMu - 1) * (aXi - 1)) +
                                        (mP001.mZ * (aMu + 1) * (aXi - 1)) - (mP101.mZ * (aMu + 1) * (aXi + 1)) +
                                        (mP111.mZ * (aMu + 1) * (aXi + 1)) - (mP011.mZ * (aMu + 1) * (aXi - 1))) /
                                       tNumberOfPoints;
                   const double tJ31 = ((mP100.mX * (aEta - 1) * (aXi + 1)) - (mP000.mX * (aEta - 1) * (aXi - 1)) -
                                        (mP110.mX * (aEta + 1) * (aXi + 1)) + (mP010.mX * (aEta + 1) * (aXi - 1)) +
                                        (mP001.mX * (aEta - 1) * (aXi - 1)) - (mP101.mX * (aEta - 1) * (aXi + 1)) +
                                        (mP111.mX * (aEta + 1) * (aXi + 1)) - (mP011.mX * (aEta + 1) * (aXi - 1))) /
                                       tNumberOfPoints;
                   const double tJ32 = ((mP100.mY * (aEta - 1) * (aXi + 1)) - (mP000.mY * (aEta - 1) * (aXi - 1)) -
                                        (mP110.mY * (aEta + 1) * (aXi + 1)) + (mP010.mY * (aEta + 1) * (aXi - 1)) +
                                        (mP001.mY * (aEta - 1) * (aXi - 1)) - (mP101.mY * (aEta - 1) * (aXi + 1)) +
                                        (mP111.mY * (aEta + 1) * (aXi + 1)) - (mP011.mY * (aEta + 1) * (aXi - 1))) /
                                       tNumberOfPoints;
                   const double tJ33 = ((mP100.mZ * (aEta - 1) * (aXi + 1)) - (mP000.mZ * (aEta - 1) * (aXi - 1)) -
                                        (mP110.mZ * (aEta + 1) * (aXi + 1)) + (mP010.mZ * (aEta + 1) * (aXi - 1)) +
                                        (mP001.mZ * (aEta - 1) * (aXi - 1)) - (mP101.mZ * (aEta - 1) * (aXi + 1)) +
                                        (mP111.mZ * (aEta + 1) * (aXi + 1)) - (mP011.mZ * (aEta + 1) * (aXi - 1))) /
                                       tNumberOfPoints;
                   const double tM1 = ((mP100.mX * (aEta - 1) * (aMu - 1) * (aXi + 1)) -
                                       (mP000.mX * (aEta - 1) * (aMu - 1) * (aXi - 1)) -
                                       (mP110.mX * (aEta + 1) * (aMu - 1) * (aXi + 1)) +
                                       (mP010.mX * (aEta + 1) * (aMu - 1) * (aXi - 1)) +
                                       (mP001.mX * (aEta - 1) * (aMu + 1) * (aXi - 1)) -
                                       (mP101.mX * (aEta - 1) * (aMu + 1) * (aXi + 1)) +
                                       (mP111.mX * (aEta + 1) * (aMu + 1) * (aXi + 1)) -
                                       (mP011.mX * (aEta + 1) * (aMu + 1) * (aXi - 1))) /
                                      tNumberOfPoints;
                   const double tM2 = ((mP100.mY * (aEta - 1) * (aMu - 1) * (aXi + 1)) -
                                       (mP000.mY * (aEta - 1) * (aMu - 1) * (aXi - 1)) -
                                       (mP110.mY * (aEta + 1) * (aMu - 1) * (aXi + 1)) +
                                       (mP010.mY * (aEta + 1) * (aMu - 1) * (aXi - 1)) +
                                       (mP001.mY * (aEta - 1) * (aMu + 1) * (aXi - 1)) -
                                       (mP101.mY * (aEta - 1) * (aMu + 1) * (aXi + 1)) +
                                       (mP111.mY * (aEta + 1) * (aMu + 1) * (aXi + 1)) -
                                       (mP011.mY * (aEta + 1) * (aMu + 1) * (aXi - 1))) /
                                      tNumberOfPoints;
                   const double tM3 = ((mP100.mZ * (aEta - 1) * (aMu - 1) * (aXi + 1)) -
                                       (mP000.mZ * (aEta - 1) * (aMu - 1) * (aXi - 1)) -
                                       (mP110.mZ * (aEta + 1) * (aMu - 1) * (aXi + 1)) +
                                       (mP010.mZ * (aEta + 1) * (aMu - 1) * (aXi - 1)) +
                                       (mP001.mZ * (aEta - 1) * (aMu + 1) * (aXi - 1)) -
                                       (mP101.mZ * (aEta - 1) * (aMu + 1) * (aXi + 1)) +
                                       (mP111.mZ * (aEta + 1) * (aMu + 1) * (aXi + 1)) -
                                       (mP011.mZ * (aEta + 1) * (aMu + 1) * (aXi - 1))) /
                                      tNumberOfPoints;

                   const double tDetJ = tJ11 * tJ22 * tJ33 - tJ11 * tJ23 * tJ32 - tJ12 * tJ21 * tJ33 +
                                        tJ12 * tJ23 * tJ31 + tJ13 * tJ21 * tJ32 - tJ13 * tJ22 * tJ31;

                   return common::Coordinate{tM1, tM2, tM3} * tDetJ;
               }) /
           volume();
}

auto Hexahedron::volumeVertexSensitivities() const -> VertexSensitivities
{
    return VertexSensitivities{
        common::Vector3{
            (mP110.mY * mP100.mZ) - (mP100.mY * mP110.mZ) - (mP100.mY * mP010.mZ) + (mP010.mY * mP100.mZ) +
                (mP100.mY * mP001.mZ) - (mP110.mY * mP010.mZ) + (mP010.mY * mP110.mZ) - (mP001.mY * mP100.mZ) +
                (mP100.mY * mP101.mZ) - (mP101.mY * mP100.mZ) - (mP010.mY * mP001.mZ) + (mP001.mY * mP010.mZ) -
                (mP001.mY * mP101.mZ) + (mP101.mY * mP001.mZ) - (mP010.mY * mP011.mZ) + (mP011.mY * mP010.mZ) +
                (mP001.mY * mP011.mZ) - (mP011.mY * mP001.mZ),
            (mP100.mX * mP110.mZ) - (mP110.mX * mP100.mZ) + (mP100.mX * mP010.mZ) - (mP010.mX * mP100.mZ) -
                (mP100.mX * mP001.mZ) + (mP110.mX * mP010.mZ) - (mP010.mX * mP110.mZ) + (mP001.mX * mP100.mZ) -
                (mP100.mX * mP101.mZ) + (mP101.mX * mP100.mZ) + (mP010.mX * mP001.mZ) - (mP001.mX * mP010.mZ) +
                (mP001.mX * mP101.mZ) - (mP101.mX * mP001.mZ) + (mP010.mX * mP011.mZ) - (mP011.mX * mP010.mZ) -
                (mP001.mX * mP011.mZ) + (mP011.mX * mP001.mZ),
            (mP110.mX * mP100.mY) - (mP100.mX * mP110.mY) - (mP100.mX * mP010.mY) + (mP010.mX * mP100.mY) +
                (mP100.mX * mP001.mY) - (mP110.mX * mP010.mY) + (mP010.mX * mP110.mY) - (mP001.mX * mP100.mY) +
                (mP100.mX * mP101.mY) - (mP101.mX * mP100.mY) - (mP010.mX * mP001.mY) + (mP001.mX * mP010.mY) -
                (mP001.mX * mP101.mY) + (mP101.mX * mP001.mY) - (mP010.mX * mP011.mY) + (mP011.mX * mP010.mY) +
                (mP001.mX * mP011.mY) - (mP011.mX * mP001.mY)} *
            kHexahedronVolumeFactor,
        common::Vector3{
            (mP000.mY * mP110.mZ) - (mP110.mY * mP000.mZ) + (mP000.mY * mP010.mZ) - (mP010.mY * mP000.mZ) -
                (mP000.mY * mP001.mZ) + (mP001.mY * mP000.mZ) - (mP000.mY * mP101.mZ) - (mP110.mY * mP010.mZ) +
                (mP010.mY * mP110.mZ) + (mP101.mY * mP000.mZ) + (mP110.mY * mP101.mZ) - (mP101.mY * mP110.mZ) +
                (mP110.mY * mP111.mZ) - (mP111.mY * mP110.mZ) - (mP001.mY * mP101.mZ) + (mP101.mY * mP001.mZ) -
                (mP101.mY * mP111.mZ) + (mP111.mY * mP101.mZ),
            (mP110.mX * mP000.mZ) - (mP000.mX * mP110.mZ) - (mP000.mX * mP010.mZ) + (mP010.mX * mP000.mZ) +
                (mP000.mX * mP001.mZ) - (mP001.mX * mP000.mZ) + (mP000.mX * mP101.mZ) + (mP110.mX * mP010.mZ) -
                (mP010.mX * mP110.mZ) - (mP101.mX * mP000.mZ) - (mP110.mX * mP101.mZ) + (mP101.mX * mP110.mZ) -
                (mP110.mX * mP111.mZ) + (mP111.mX * mP110.mZ) + (mP001.mX * mP101.mZ) - (mP101.mX * mP001.mZ) +
                (mP101.mX * mP111.mZ) - (mP111.mX * mP101.mZ),
            (mP000.mX * mP110.mY) - (mP110.mX * mP000.mY) + (mP000.mX * mP010.mY) - (mP010.mX * mP000.mY) -
                (mP000.mX * mP001.mY) + (mP001.mX * mP000.mY) - (mP000.mX * mP101.mY) - (mP110.mX * mP010.mY) +
                (mP010.mX * mP110.mY) + (mP101.mX * mP000.mY) + (mP110.mX * mP101.mY) - (mP101.mX * mP110.mY) +
                (mP110.mX * mP111.mY) - (mP111.mX * mP110.mY) - (mP001.mX * mP101.mY) + (mP101.mX * mP001.mY) -
                (mP101.mX * mP111.mY) + (mP111.mX * mP101.mY)} *
            kHexahedronVolumeFactor,
        common::Vector3{
            (mP100.mY * mP000.mZ) - (mP000.mY * mP100.mZ) + (mP000.mY * mP010.mZ) - (mP010.mY * mP000.mZ) +
                (mP100.mY * mP010.mZ) - (mP010.mY * mP100.mZ) - (mP100.mY * mP101.mZ) + (mP101.mY * mP100.mZ) -
                (mP100.mY * mP111.mZ) + (mP111.mY * mP100.mZ) + (mP010.mY * mP111.mZ) - (mP111.mY * mP010.mZ) +
                (mP010.mY * mP011.mZ) - (mP011.mY * mP010.mZ) - (mP101.mY * mP111.mZ) + (mP111.mY * mP101.mZ) -
                (mP111.mY * mP011.mZ) + (mP011.mY * mP111.mZ),
            (mP000.mX * mP100.mZ) - (mP100.mX * mP000.mZ) - (mP000.mX * mP010.mZ) + (mP010.mX * mP000.mZ) -
                (mP100.mX * mP010.mZ) + (mP010.mX * mP100.mZ) + (mP100.mX * mP101.mZ) - (mP101.mX * mP100.mZ) +
                (mP100.mX * mP111.mZ) - (mP111.mX * mP100.mZ) - (mP010.mX * mP111.mZ) + (mP111.mX * mP010.mZ) -
                (mP010.mX * mP011.mZ) + (mP011.mX * mP010.mZ) + (mP101.mX * mP111.mZ) - (mP111.mX * mP101.mZ) +
                (mP111.mX * mP011.mZ) - (mP011.mX * mP111.mZ),
            (mP100.mX * mP000.mY) - (mP000.mX * mP100.mY) + (mP000.mX * mP010.mY) - (mP010.mX * mP000.mY) +
                (mP100.mX * mP010.mY) - (mP010.mX * mP100.mY) - (mP100.mX * mP101.mY) + (mP101.mX * mP100.mY) -
                (mP100.mX * mP111.mY) + (mP111.mX * mP100.mY) + (mP010.mX * mP111.mY) - (mP111.mX * mP010.mY) +
                (mP010.mX * mP011.mY) - (mP011.mX * mP010.mY) - (mP101.mX * mP111.mY) + (mP111.mX * mP101.mY) -
                (mP111.mX * mP011.mY) + (mP011.mX * mP111.mY)} *
            kHexahedronVolumeFactor,
        common::Vector3{
            (mP100.mY * mP000.mZ) - (mP000.mY * mP100.mZ) - (mP000.mY * mP110.mZ) + (mP110.mY * mP000.mZ) -
                (mP100.mY * mP110.mZ) + (mP110.mY * mP100.mZ) + (mP000.mY * mP001.mZ) - (mP001.mY * mP000.mZ) +
                (mP000.mY * mP011.mZ) - (mP011.mY * mP000.mZ) - (mP110.mY * mP111.mZ) + (mP111.mY * mP110.mZ) -
                (mP110.mY * mP011.mZ) + (mP011.mY * mP110.mZ) + (mP001.mY * mP011.mZ) - (mP011.mY * mP001.mZ) -
                (mP111.mY * mP011.mZ) + (mP011.mY * mP111.mZ),
            (mP000.mX * mP100.mZ) - (mP100.mX * mP000.mZ) + (mP000.mX * mP110.mZ) - (mP110.mX * mP000.mZ) +
                (mP100.mX * mP110.mZ) - (mP110.mX * mP100.mZ) - (mP000.mX * mP001.mZ) + (mP001.mX * mP000.mZ) -
                (mP000.mX * mP011.mZ) + (mP011.mX * mP000.mZ) + (mP110.mX * mP111.mZ) - (mP111.mX * mP110.mZ) +
                (mP110.mX * mP011.mZ) - (mP011.mX * mP110.mZ) - (mP001.mX * mP011.mZ) + (mP011.mX * mP001.mZ) +
                (mP111.mX * mP011.mZ) - (mP011.mX * mP111.mZ),
            (mP100.mX * mP000.mY) - (mP000.mX * mP100.mY) - (mP000.mX * mP110.mY) + (mP110.mX * mP000.mY) -
                (mP100.mX * mP110.mY) + (mP110.mX * mP100.mY) + (mP000.mX * mP001.mY) - (mP001.mX * mP000.mY) +
                (mP000.mX * mP011.mY) - (mP011.mX * mP000.mY) - (mP110.mX * mP111.mY) + (mP111.mX * mP110.mY) -
                (mP110.mX * mP011.mY) + (mP011.mX * mP110.mY) + (mP001.mX * mP011.mY) - (mP011.mX * mP001.mY) -
                (mP111.mX * mP011.mY) + (mP011.mX * mP111.mY)} *
            kHexahedronVolumeFactor,
        common::Vector3{
            (mP000.mY * mP100.mZ) - (mP100.mY * mP000.mZ) - (mP000.mY * mP010.mZ) + (mP010.mY * mP000.mZ) +
                (mP000.mY * mP101.mZ) - (mP101.mY * mP000.mZ) + (mP100.mY * mP101.mZ) - (mP101.mY * mP100.mZ) -
                (mP000.mY * mP011.mZ) + (mP011.mY * mP000.mZ) - (mP010.mY * mP011.mZ) + (mP011.mY * mP010.mZ) +
                (mP101.mY * mP111.mZ) - (mP111.mY * mP101.mZ) + (mP101.mY * mP011.mZ) - (mP011.mY * mP101.mZ) +
                (mP111.mY * mP011.mZ) - (mP011.mY * mP111.mZ),
            (mP100.mX * mP000.mZ) - (mP000.mX * mP100.mZ) + (mP000.mX * mP010.mZ) - (mP010.mX * mP000.mZ) -
                (mP000.mX * mP101.mZ) + (mP101.mX * mP000.mZ) - (mP100.mX * mP101.mZ) + (mP101.mX * mP100.mZ) +
                (mP000.mX * mP011.mZ) - (mP011.mX * mP000.mZ) + (mP010.mX * mP011.mZ) - (mP011.mX * mP010.mZ) -
                (mP101.mX * mP111.mZ) + (mP111.mX * mP101.mZ) - (mP101.mX * mP011.mZ) + (mP011.mX * mP101.mZ) -
                (mP111.mX * mP011.mZ) + (mP011.mX * mP111.mZ),
            (mP000.mX * mP100.mY) - (mP100.mX * mP000.mY) - (mP000.mX * mP010.mY) + (mP010.mX * mP000.mY) +
                (mP000.mX * mP101.mY) - (mP101.mX * mP000.mY) + (mP100.mX * mP101.mY) - (mP101.mX * mP100.mY) -
                (mP000.mX * mP011.mY) + (mP011.mX * mP000.mY) - (mP010.mX * mP011.mY) + (mP011.mX * mP010.mY) +
                (mP101.mX * mP111.mY) - (mP111.mX * mP101.mY) + (mP101.mX * mP011.mY) - (mP011.mX * mP101.mY) +
                (mP111.mX * mP011.mY) - (mP011.mX * mP111.mY)} *
            kHexahedronVolumeFactor,
        common::Vector3{
            (mP000.mY * mP100.mZ) - (mP100.mY * mP000.mZ) + (mP100.mY * mP110.mZ) - (mP110.mY * mP100.mZ) -
                (mP000.mY * mP001.mZ) + (mP001.mY * mP000.mZ) - (mP100.mY * mP001.mZ) + (mP001.mY * mP100.mZ) +
                (mP100.mY * mP111.mZ) - (mP111.mY * mP100.mZ) + (mP110.mY * mP111.mZ) - (mP111.mY * mP110.mZ) -
                (mP001.mY * mP111.mZ) + (mP111.mY * mP001.mZ) - (mP001.mY * mP011.mZ) + (mP011.mY * mP001.mZ) +
                (mP111.mY * mP011.mZ) - (mP011.mY * mP111.mZ),
            (mP100.mX * mP000.mZ) - (mP000.mX * mP100.mZ) - (mP100.mX * mP110.mZ) + (mP110.mX * mP100.mZ) +
                (mP000.mX * mP001.mZ) - (mP001.mX * mP000.mZ) + (mP100.mX * mP001.mZ) - (mP001.mX * mP100.mZ) -
                (mP100.mX * mP111.mZ) + (mP111.mX * mP100.mZ) - (mP110.mX * mP111.mZ) + (mP111.mX * mP110.mZ) +
                (mP001.mX * mP111.mZ) - (mP111.mX * mP001.mZ) + (mP001.mX * mP011.mZ) - (mP011.mX * mP001.mZ) -
                (mP111.mX * mP011.mZ) + (mP011.mX * mP111.mZ),
            (mP000.mX * mP100.mY) - (mP100.mX * mP000.mY) + (mP100.mX * mP110.mY) - (mP110.mX * mP100.mY) -
                (mP000.mX * mP001.mY) + (mP001.mX * mP000.mY) - (mP100.mX * mP001.mY) + (mP001.mX * mP100.mY) +
                (mP100.mX * mP111.mY) - (mP111.mX * mP100.mY) + (mP110.mX * mP111.mY) - (mP111.mX * mP110.mY) -
                (mP001.mX * mP111.mY) + (mP111.mX * mP001.mY) - (mP001.mX * mP011.mY) + (mP011.mX * mP001.mY) +
                (mP111.mX * mP011.mY) - (mP011.mX * mP111.mY)} *
            kHexahedronVolumeFactor,
        common::Vector3{
            (mP100.mY * mP110.mZ) - (mP110.mY * mP100.mZ) + (mP110.mY * mP010.mZ) - (mP010.mY * mP110.mZ) -
                (mP100.mY * mP101.mZ) + (mP101.mY * mP100.mZ) - (mP110.mY * mP101.mZ) + (mP101.mY * mP110.mZ) +
                (mP110.mY * mP011.mZ) + (mP001.mY * mP101.mZ) - (mP101.mY * mP001.mZ) - (mP011.mY * mP110.mZ) +
                (mP010.mY * mP011.mZ) - (mP011.mY * mP010.mZ) - (mP001.mY * mP011.mZ) + (mP011.mY * mP001.mZ) -
                (mP101.mY * mP011.mZ) + (mP011.mY * mP101.mZ),
            (mP110.mX * mP100.mZ) - (mP100.mX * mP110.mZ) - (mP110.mX * mP010.mZ) + (mP010.mX * mP110.mZ) +
                (mP100.mX * mP101.mZ) - (mP101.mX * mP100.mZ) + (mP110.mX * mP101.mZ) - (mP101.mX * mP110.mZ) -
                (mP110.mX * mP011.mZ) - (mP001.mX * mP101.mZ) + (mP101.mX * mP001.mZ) + (mP011.mX * mP110.mZ) -
                (mP010.mX * mP011.mZ) + (mP011.mX * mP010.mZ) + (mP001.mX * mP011.mZ) - (mP011.mX * mP001.mZ) +
                (mP101.mX * mP011.mZ) - (mP011.mX * mP101.mZ),
            (mP100.mX * mP110.mY) - (mP110.mX * mP100.mY) + (mP110.mX * mP010.mY) - (mP010.mX * mP110.mY) -
                (mP100.mX * mP101.mY) + (mP101.mX * mP100.mY) - (mP110.mX * mP101.mY) + (mP101.mX * mP110.mY) +
                (mP110.mX * mP011.mY) + (mP001.mX * mP101.mY) - (mP101.mX * mP001.mY) - (mP011.mX * mP110.mY) +
                (mP010.mX * mP011.mY) - (mP011.mX * mP010.mY) - (mP001.mX * mP011.mY) + (mP011.mX * mP001.mY) -
                (mP101.mX * mP011.mY) + (mP011.mX * mP101.mY)} *
            kHexahedronVolumeFactor,
        common::Vector3{
            (mP010.mY * mP000.mZ) - (mP000.mY * mP010.mZ) + (mP000.mY * mP001.mZ) - (mP001.mY * mP000.mZ) +
                (mP110.mY * mP010.mZ) - (mP010.mY * mP110.mZ) + (mP010.mY * mP001.mZ) - (mP001.mY * mP010.mZ) -
                (mP110.mY * mP111.mZ) + (mP111.mY * mP110.mZ) - (mP010.mY * mP111.mZ) + (mP001.mY * mP101.mZ) -
                (mP101.mY * mP001.mZ) + (mP111.mY * mP010.mZ) + (mP001.mY * mP111.mZ) - (mP111.mY * mP001.mZ) +
                (mP101.mY * mP111.mZ) - (mP111.mY * mP101.mZ),
            (mP000.mX * mP010.mZ) - (mP010.mX * mP000.mZ) - (mP000.mX * mP001.mZ) + (mP001.mX * mP000.mZ) -
                (mP110.mX * mP010.mZ) + (mP010.mX * mP110.mZ) - (mP010.mX * mP001.mZ) + (mP001.mX * mP010.mZ) +
                (mP110.mX * mP111.mZ) - (mP111.mX * mP110.mZ) + (mP010.mX * mP111.mZ) - (mP001.mX * mP101.mZ) +
                (mP101.mX * mP001.mZ) - (mP111.mX * mP010.mZ) - (mP001.mX * mP111.mZ) + (mP111.mX * mP001.mZ) -
                (mP101.mX * mP111.mZ) + (mP111.mX * mP101.mZ),
            (mP010.mX * mP000.mY) - (mP000.mX * mP010.mY) + (mP000.mX * mP001.mY) - (mP001.mX * mP000.mY) +
                (mP110.mX * mP010.mY) - (mP010.mX * mP110.mY) + (mP010.mX * mP001.mY) - (mP001.mX * mP010.mY) -
                (mP110.mX * mP111.mY) + (mP111.mX * mP110.mY) - (mP010.mX * mP111.mY) + (mP001.mX * mP101.mY) -
                (mP101.mX * mP001.mY) + (mP111.mX * mP010.mY) + (mP001.mX * mP111.mY) - (mP111.mX * mP001.mY) +
                (mP101.mX * mP111.mY) - (mP111.mX * mP101.mY)} *
            kHexahedronVolumeFactor};
}
}  // namespace plato::third_party_integration::stk_io
