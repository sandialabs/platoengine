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
    return ((mP000.x * mP110.y * mP100.z) - (mP000.x * mP100.y * mP110.z) + (mP100.x * mP000.y * mP110.z) -
            (mP100.x * mP110.y * mP000.z) - (mP110.x * mP000.y * mP100.z) + (mP110.x * mP100.y * mP000.z) -
            (mP000.x * mP100.y * mP010.z) + (mP000.x * mP010.y * mP100.z) + (mP100.x * mP000.y * mP010.z) -
            (mP100.x * mP010.y * mP000.z) - (mP010.x * mP000.y * mP100.z) + (mP010.x * mP100.y * mP000.z) +
            (mP000.x * mP100.y * mP001.z) - (mP000.x * mP110.y * mP010.z) + (mP000.x * mP010.y * mP110.z) -
            (mP000.x * mP001.y * mP100.z) - (mP100.x * mP000.y * mP001.z) + (mP100.x * mP001.y * mP000.z) +
            (mP110.x * mP000.y * mP010.z) - (mP110.x * mP010.y * mP000.z) - (mP010.x * mP000.y * mP110.z) +
            (mP010.x * mP110.y * mP000.z) + (mP001.x * mP000.y * mP100.z) - (mP001.x * mP100.y * mP000.z) +
            (mP000.x * mP100.y * mP101.z) - (mP000.x * mP101.y * mP100.z) - (mP100.x * mP000.y * mP101.z) -
            (mP100.x * mP110.y * mP010.z) + (mP100.x * mP010.y * mP110.z) + (mP100.x * mP101.y * mP000.z) +
            (mP110.x * mP100.y * mP010.z) - (mP110.x * mP010.y * mP100.z) - (mP010.x * mP100.y * mP110.z) +
            (mP010.x * mP110.y * mP100.z) + (mP101.x * mP000.y * mP100.z) - (mP101.x * mP100.y * mP000.z) -
            (mP000.x * mP010.y * mP001.z) + (mP000.x * mP001.y * mP010.z) + (mP010.x * mP000.y * mP001.z) -
            (mP010.x * mP001.y * mP000.z) - (mP001.x * mP000.y * mP010.z) + (mP001.x * mP010.y * mP000.z) +
            (mP100.x * mP110.y * mP101.z) - (mP100.x * mP101.y * mP110.z) - (mP110.x * mP100.y * mP101.z) +
            (mP110.x * mP101.y * mP100.z) + (mP101.x * mP100.y * mP110.z) - (mP101.x * mP110.y * mP100.z) -
            (mP000.x * mP001.y * mP101.z) + (mP000.x * mP101.y * mP001.z) + (mP100.x * mP110.y * mP111.z) -
            (mP100.x * mP111.y * mP110.z) - (mP110.x * mP100.y * mP111.z) + (mP110.x * mP111.y * mP100.z) +
            (mP001.x * mP000.y * mP101.z) - (mP001.x * mP101.y * mP000.z) - (mP101.x * mP000.y * mP001.z) +
            (mP101.x * mP001.y * mP000.z) + (mP111.x * mP100.y * mP110.z) - (mP111.x * mP110.y * mP100.z) -
            (mP000.x * mP010.y * mP011.z) + (mP000.x * mP011.y * mP010.z) - (mP100.x * mP001.y * mP101.z) +
            (mP100.x * mP101.y * mP001.z) + (mP010.x * mP000.y * mP011.z) - (mP010.x * mP011.y * mP000.z) +
            (mP001.x * mP100.y * mP101.z) - (mP001.x * mP101.y * mP100.z) - (mP101.x * mP100.y * mP001.z) +
            (mP101.x * mP001.y * mP100.z) - (mP011.x * mP000.y * mP010.z) + (mP011.x * mP010.y * mP000.z) +
            (mP000.x * mP001.y * mP011.z) - (mP000.x * mP011.y * mP001.z) + (mP110.x * mP010.y * mP111.z) -
            (mP110.x * mP111.y * mP010.z) - (mP010.x * mP110.y * mP111.z) + (mP010.x * mP111.y * mP110.z) -
            (mP001.x * mP000.y * mP011.z) + (mP001.x * mP011.y * mP000.z) + (mP111.x * mP110.y * mP010.z) -
            (mP111.x * mP010.y * mP110.z) + (mP011.x * mP000.y * mP001.z) - (mP011.x * mP001.y * mP000.z) -
            (mP100.x * mP101.y * mP111.z) + (mP100.x * mP111.y * mP101.z) + (mP110.x * mP010.y * mP011.z) -
            (mP110.x * mP011.y * mP010.z) - (mP010.x * mP110.y * mP011.z) + (mP010.x * mP011.y * mP110.z) +
            (mP101.x * mP100.y * mP111.z) - (mP101.x * mP111.y * mP100.z) - (mP111.x * mP100.y * mP101.z) +
            (mP111.x * mP101.y * mP100.z) + (mP011.x * mP110.y * mP010.z) - (mP011.x * mP010.y * mP110.z) -
            (mP110.x * mP101.y * mP111.z) + (mP110.x * mP111.y * mP101.z) + (mP101.x * mP110.y * mP111.z) -
            (mP101.x * mP111.y * mP110.z) - (mP111.x * mP110.y * mP101.z) + (mP111.x * mP101.y * mP110.z) +
            (mP010.x * mP001.y * mP011.z) - (mP010.x * mP011.y * mP001.z) - (mP001.x * mP010.y * mP011.z) +
            (mP001.x * mP011.y * mP010.z) + (mP011.x * mP010.y * mP001.z) - (mP011.x * mP001.y * mP010.z) -
            (mP110.x * mP111.y * mP011.z) + (mP110.x * mP011.y * mP111.z) + (mP001.x * mP101.y * mP111.z) -
            (mP001.x * mP111.y * mP101.z) - (mP101.x * mP001.y * mP111.z) + (mP101.x * mP111.y * mP001.z) +
            (mP111.x * mP110.y * mP011.z) + (mP111.x * mP001.y * mP101.z) - (mP111.x * mP101.y * mP001.z) -
            (mP111.x * mP011.y * mP110.z) - (mP011.x * mP110.y * mP111.z) + (mP011.x * mP111.y * mP110.z) -
            (mP010.x * mP111.y * mP011.z) + (mP010.x * mP011.y * mP111.z) + (mP001.x * mP101.y * mP011.z) -
            (mP001.x * mP011.y * mP101.z) - (mP101.x * mP001.y * mP011.z) + (mP101.x * mP011.y * mP001.z) +
            (mP111.x * mP010.y * mP011.z) - (mP111.x * mP011.y * mP010.z) - (mP011.x * mP010.y * mP111.z) +
            (mP011.x * mP001.y * mP101.z) - (mP011.x * mP101.y * mP001.z) + (mP011.x * mP111.y * mP010.z) +
            (mP001.x * mP111.y * mP011.z) - (mP001.x * mP011.y * mP111.z) - (mP111.x * mP001.y * mP011.z) +
            (mP111.x * mP011.y * mP001.z) + (mP011.x * mP001.y * mP111.z) - (mP011.x * mP111.y * mP001.z) +
            (mP101.x * mP111.y * mP011.z) - (mP101.x * mP011.y * mP111.z) - (mP111.x * mP101.y * mP011.z) +
            (mP111.x * mP011.y * mP101.z) + (mP011.x * mP101.y * mP111.z) - (mP011.x * mP111.y * mP101.z)) *
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
                   const double tJ11 = ((mP100.x * (aEta - 1) * (aMu - 1)) - (mP000.x * (aEta - 1) * (aMu - 1)) -
                                        (mP110.x * (aEta + 1) * (aMu - 1)) + (mP010.x * (aEta + 1) * (aMu - 1)) +
                                        (mP001.x * (aEta - 1) * (aMu + 1)) - (mP101.x * (aEta - 1) * (aMu + 1)) +
                                        (mP111.x * (aEta + 1) * (aMu + 1)) - (mP011.x * (aEta + 1) * (aMu + 1))) /
                                       tNumberOfPoints;
                   const double tJ12 = ((mP100.y * (aEta - 1) * (aMu - 1)) - (mP000.y * (aEta - 1) * (aMu - 1)) -
                                        (mP110.y * (aEta + 1) * (aMu - 1)) + (mP010.y * (aEta + 1) * (aMu - 1)) +
                                        (mP001.y * (aEta - 1) * (aMu + 1)) - (mP101.y * (aEta - 1) * (aMu + 1)) +
                                        (mP111.y * (aEta + 1) * (aMu + 1)) - (mP011.y * (aEta + 1) * (aMu + 1))) /
                                       tNumberOfPoints;
                   const double tJ13 = ((mP100.z * (aEta - 1) * (aMu - 1)) - (mP000.z * (aEta - 1) * (aMu - 1)) -
                                        (mP110.z * (aEta + 1) * (aMu - 1)) + (mP010.z * (aEta + 1) * (aMu - 1)) +
                                        (mP001.z * (aEta - 1) * (aMu + 1)) - (mP101.z * (aEta - 1) * (aMu + 1)) +
                                        (mP111.z * (aEta + 1) * (aMu + 1)) - (mP011.z * (aEta + 1) * (aMu + 1))) /
                                       tNumberOfPoints;
                   const double tJ21 = ((mP100.x * (aMu - 1) * (aXi + 1)) - (mP000.x * (aMu - 1) * (aXi - 1)) -
                                        (mP110.x * (aMu - 1) * (aXi + 1)) + (mP010.x * (aMu - 1) * (aXi - 1)) +
                                        (mP001.x * (aMu + 1) * (aXi - 1)) - (mP101.x * (aMu + 1) * (aXi + 1)) +
                                        (mP111.x * (aMu + 1) * (aXi + 1)) - (mP011.x * (aMu + 1) * (aXi - 1))) /
                                       tNumberOfPoints;
                   const double tJ22 = ((mP100.y * (aMu - 1) * (aXi + 1)) - (mP000.y * (aMu - 1) * (aXi - 1)) -
                                        (mP110.y * (aMu - 1) * (aXi + 1)) + (mP010.y * (aMu - 1) * (aXi - 1)) +
                                        (mP001.y * (aMu + 1) * (aXi - 1)) - (mP101.y * (aMu + 1) * (aXi + 1)) +
                                        (mP111.y * (aMu + 1) * (aXi + 1)) - (mP011.y * (aMu + 1) * (aXi - 1))) /
                                       tNumberOfPoints;
                   const double tJ23 = ((mP100.z * (aMu - 1) * (aXi + 1)) - (mP000.z * (aMu - 1) * (aXi - 1)) -
                                        (mP110.z * (aMu - 1) * (aXi + 1)) + (mP010.z * (aMu - 1) * (aXi - 1)) +
                                        (mP001.z * (aMu + 1) * (aXi - 1)) - (mP101.z * (aMu + 1) * (aXi + 1)) +
                                        (mP111.z * (aMu + 1) * (aXi + 1)) - (mP011.z * (aMu + 1) * (aXi - 1))) /
                                       tNumberOfPoints;
                   const double tJ31 = ((mP100.x * (aEta - 1) * (aXi + 1)) - (mP000.x * (aEta - 1) * (aXi - 1)) -
                                        (mP110.x * (aEta + 1) * (aXi + 1)) + (mP010.x * (aEta + 1) * (aXi - 1)) +
                                        (mP001.x * (aEta - 1) * (aXi - 1)) - (mP101.x * (aEta - 1) * (aXi + 1)) +
                                        (mP111.x * (aEta + 1) * (aXi + 1)) - (mP011.x * (aEta + 1) * (aXi - 1))) /
                                       tNumberOfPoints;
                   const double tJ32 = ((mP100.y * (aEta - 1) * (aXi + 1)) - (mP000.y * (aEta - 1) * (aXi - 1)) -
                                        (mP110.y * (aEta + 1) * (aXi + 1)) + (mP010.y * (aEta + 1) * (aXi - 1)) +
                                        (mP001.y * (aEta - 1) * (aXi - 1)) - (mP101.y * (aEta - 1) * (aXi + 1)) +
                                        (mP111.y * (aEta + 1) * (aXi + 1)) - (mP011.y * (aEta + 1) * (aXi - 1))) /
                                       tNumberOfPoints;
                   const double tJ33 = ((mP100.z * (aEta - 1) * (aXi + 1)) - (mP000.z * (aEta - 1) * (aXi - 1)) -
                                        (mP110.z * (aEta + 1) * (aXi + 1)) + (mP010.z * (aEta + 1) * (aXi - 1)) +
                                        (mP001.z * (aEta - 1) * (aXi - 1)) - (mP101.z * (aEta - 1) * (aXi + 1)) +
                                        (mP111.z * (aEta + 1) * (aXi + 1)) - (mP011.z * (aEta + 1) * (aXi - 1))) /
                                       tNumberOfPoints;
                   const double tM1 = ((mP100.x * (aEta - 1) * (aMu - 1) * (aXi + 1)) -
                                       (mP000.x * (aEta - 1) * (aMu - 1) * (aXi - 1)) -
                                       (mP110.x * (aEta + 1) * (aMu - 1) * (aXi + 1)) +
                                       (mP010.x * (aEta + 1) * (aMu - 1) * (aXi - 1)) +
                                       (mP001.x * (aEta - 1) * (aMu + 1) * (aXi - 1)) -
                                       (mP101.x * (aEta - 1) * (aMu + 1) * (aXi + 1)) +
                                       (mP111.x * (aEta + 1) * (aMu + 1) * (aXi + 1)) -
                                       (mP011.x * (aEta + 1) * (aMu + 1) * (aXi - 1))) /
                                      tNumberOfPoints;
                   const double tM2 = ((mP100.y * (aEta - 1) * (aMu - 1) * (aXi + 1)) -
                                       (mP000.y * (aEta - 1) * (aMu - 1) * (aXi - 1)) -
                                       (mP110.y * (aEta + 1) * (aMu - 1) * (aXi + 1)) +
                                       (mP010.y * (aEta + 1) * (aMu - 1) * (aXi - 1)) +
                                       (mP001.y * (aEta - 1) * (aMu + 1) * (aXi - 1)) -
                                       (mP101.y * (aEta - 1) * (aMu + 1) * (aXi + 1)) +
                                       (mP111.y * (aEta + 1) * (aMu + 1) * (aXi + 1)) -
                                       (mP011.y * (aEta + 1) * (aMu + 1) * (aXi - 1))) /
                                      tNumberOfPoints;
                   const double tM3 = ((mP100.z * (aEta - 1) * (aMu - 1) * (aXi + 1)) -
                                       (mP000.z * (aEta - 1) * (aMu - 1) * (aXi - 1)) -
                                       (mP110.z * (aEta + 1) * (aMu - 1) * (aXi + 1)) +
                                       (mP010.z * (aEta + 1) * (aMu - 1) * (aXi - 1)) +
                                       (mP001.z * (aEta - 1) * (aMu + 1) * (aXi - 1)) -
                                       (mP101.z * (aEta - 1) * (aMu + 1) * (aXi + 1)) +
                                       (mP111.z * (aEta + 1) * (aMu + 1) * (aXi + 1)) -
                                       (mP011.z * (aEta + 1) * (aMu + 1) * (aXi - 1))) /
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
        common::Vector3{(mP110.y * mP100.z) - (mP100.y * mP110.z) - (mP100.y * mP010.z) + (mP010.y * mP100.z) +
                            (mP100.y * mP001.z) - (mP110.y * mP010.z) + (mP010.y * mP110.z) - (mP001.y * mP100.z) +
                            (mP100.y * mP101.z) - (mP101.y * mP100.z) - (mP010.y * mP001.z) + (mP001.y * mP010.z) -
                            (mP001.y * mP101.z) + (mP101.y * mP001.z) - (mP010.y * mP011.z) + (mP011.y * mP010.z) +
                            (mP001.y * mP011.z) - (mP011.y * mP001.z),
                        (mP100.x * mP110.z) - (mP110.x * mP100.z) + (mP100.x * mP010.z) - (mP010.x * mP100.z) -
                            (mP100.x * mP001.z) + (mP110.x * mP010.z) - (mP010.x * mP110.z) + (mP001.x * mP100.z) -
                            (mP100.x * mP101.z) + (mP101.x * mP100.z) + (mP010.x * mP001.z) - (mP001.x * mP010.z) +
                            (mP001.x * mP101.z) - (mP101.x * mP001.z) + (mP010.x * mP011.z) - (mP011.x * mP010.z) -
                            (mP001.x * mP011.z) + (mP011.x * mP001.z),
                        (mP110.x * mP100.y) - (mP100.x * mP110.y) - (mP100.x * mP010.y) + (mP010.x * mP100.y) +
                            (mP100.x * mP001.y) - (mP110.x * mP010.y) + (mP010.x * mP110.y) - (mP001.x * mP100.y) +
                            (mP100.x * mP101.y) - (mP101.x * mP100.y) - (mP010.x * mP001.y) + (mP001.x * mP010.y) -
                            (mP001.x * mP101.y) + (mP101.x * mP001.y) - (mP010.x * mP011.y) + (mP011.x * mP010.y) +
                            (mP001.x * mP011.y) - (mP011.x * mP001.y)} *
            kHexahedronVolumeFactor,
        common::Vector3{(mP000.y * mP110.z) - (mP110.y * mP000.z) + (mP000.y * mP010.z) - (mP010.y * mP000.z) -
                            (mP000.y * mP001.z) + (mP001.y * mP000.z) - (mP000.y * mP101.z) - (mP110.y * mP010.z) +
                            (mP010.y * mP110.z) + (mP101.y * mP000.z) + (mP110.y * mP101.z) - (mP101.y * mP110.z) +
                            (mP110.y * mP111.z) - (mP111.y * mP110.z) - (mP001.y * mP101.z) + (mP101.y * mP001.z) -
                            (mP101.y * mP111.z) + (mP111.y * mP101.z),
                        (mP110.x * mP000.z) - (mP000.x * mP110.z) - (mP000.x * mP010.z) + (mP010.x * mP000.z) +
                            (mP000.x * mP001.z) - (mP001.x * mP000.z) + (mP000.x * mP101.z) + (mP110.x * mP010.z) -
                            (mP010.x * mP110.z) - (mP101.x * mP000.z) - (mP110.x * mP101.z) + (mP101.x * mP110.z) -
                            (mP110.x * mP111.z) + (mP111.x * mP110.z) + (mP001.x * mP101.z) - (mP101.x * mP001.z) +
                            (mP101.x * mP111.z) - (mP111.x * mP101.z),
                        (mP000.x * mP110.y) - (mP110.x * mP000.y) + (mP000.x * mP010.y) - (mP010.x * mP000.y) -
                            (mP000.x * mP001.y) + (mP001.x * mP000.y) - (mP000.x * mP101.y) - (mP110.x * mP010.y) +
                            (mP010.x * mP110.y) + (mP101.x * mP000.y) + (mP110.x * mP101.y) - (mP101.x * mP110.y) +
                            (mP110.x * mP111.y) - (mP111.x * mP110.y) - (mP001.x * mP101.y) + (mP101.x * mP001.y) -
                            (mP101.x * mP111.y) + (mP111.x * mP101.y)} *
            kHexahedronVolumeFactor,
        common::Vector3{(mP100.y * mP000.z) - (mP000.y * mP100.z) + (mP000.y * mP010.z) - (mP010.y * mP000.z) +
                            (mP100.y * mP010.z) - (mP010.y * mP100.z) - (mP100.y * mP101.z) + (mP101.y * mP100.z) -
                            (mP100.y * mP111.z) + (mP111.y * mP100.z) + (mP010.y * mP111.z) - (mP111.y * mP010.z) +
                            (mP010.y * mP011.z) - (mP011.y * mP010.z) - (mP101.y * mP111.z) + (mP111.y * mP101.z) -
                            (mP111.y * mP011.z) + (mP011.y * mP111.z),
                        (mP000.x * mP100.z) - (mP100.x * mP000.z) - (mP000.x * mP010.z) + (mP010.x * mP000.z) -
                            (mP100.x * mP010.z) + (mP010.x * mP100.z) + (mP100.x * mP101.z) - (mP101.x * mP100.z) +
                            (mP100.x * mP111.z) - (mP111.x * mP100.z) - (mP010.x * mP111.z) + (mP111.x * mP010.z) -
                            (mP010.x * mP011.z) + (mP011.x * mP010.z) + (mP101.x * mP111.z) - (mP111.x * mP101.z) +
                            (mP111.x * mP011.z) - (mP011.x * mP111.z),
                        (mP100.x * mP000.y) - (mP000.x * mP100.y) + (mP000.x * mP010.y) - (mP010.x * mP000.y) +
                            (mP100.x * mP010.y) - (mP010.x * mP100.y) - (mP100.x * mP101.y) + (mP101.x * mP100.y) -
                            (mP100.x * mP111.y) + (mP111.x * mP100.y) + (mP010.x * mP111.y) - (mP111.x * mP010.y) +
                            (mP010.x * mP011.y) - (mP011.x * mP010.y) - (mP101.x * mP111.y) + (mP111.x * mP101.y) -
                            (mP111.x * mP011.y) + (mP011.x * mP111.y)} *
            kHexahedronVolumeFactor,
        common::Vector3{(mP100.y * mP000.z) - (mP000.y * mP100.z) - (mP000.y * mP110.z) + (mP110.y * mP000.z) -
                            (mP100.y * mP110.z) + (mP110.y * mP100.z) + (mP000.y * mP001.z) - (mP001.y * mP000.z) +
                            (mP000.y * mP011.z) - (mP011.y * mP000.z) - (mP110.y * mP111.z) + (mP111.y * mP110.z) -
                            (mP110.y * mP011.z) + (mP011.y * mP110.z) + (mP001.y * mP011.z) - (mP011.y * mP001.z) -
                            (mP111.y * mP011.z) + (mP011.y * mP111.z),
                        (mP000.x * mP100.z) - (mP100.x * mP000.z) + (mP000.x * mP110.z) - (mP110.x * mP000.z) +
                            (mP100.x * mP110.z) - (mP110.x * mP100.z) - (mP000.x * mP001.z) + (mP001.x * mP000.z) -
                            (mP000.x * mP011.z) + (mP011.x * mP000.z) + (mP110.x * mP111.z) - (mP111.x * mP110.z) +
                            (mP110.x * mP011.z) - (mP011.x * mP110.z) - (mP001.x * mP011.z) + (mP011.x * mP001.z) +
                            (mP111.x * mP011.z) - (mP011.x * mP111.z),
                        (mP100.x * mP000.y) - (mP000.x * mP100.y) - (mP000.x * mP110.y) + (mP110.x * mP000.y) -
                            (mP100.x * mP110.y) + (mP110.x * mP100.y) + (mP000.x * mP001.y) - (mP001.x * mP000.y) +
                            (mP000.x * mP011.y) - (mP011.x * mP000.y) - (mP110.x * mP111.y) + (mP111.x * mP110.y) -
                            (mP110.x * mP011.y) + (mP011.x * mP110.y) + (mP001.x * mP011.y) - (mP011.x * mP001.y) -
                            (mP111.x * mP011.y) + (mP011.x * mP111.y)} *
            kHexahedronVolumeFactor,
        common::Vector3{(mP000.y * mP100.z) - (mP100.y * mP000.z) - (mP000.y * mP010.z) + (mP010.y * mP000.z) +
                            (mP000.y * mP101.z) - (mP101.y * mP000.z) + (mP100.y * mP101.z) - (mP101.y * mP100.z) -
                            (mP000.y * mP011.z) + (mP011.y * mP000.z) - (mP010.y * mP011.z) + (mP011.y * mP010.z) +
                            (mP101.y * mP111.z) - (mP111.y * mP101.z) + (mP101.y * mP011.z) - (mP011.y * mP101.z) +
                            (mP111.y * mP011.z) - (mP011.y * mP111.z),
                        (mP100.x * mP000.z) - (mP000.x * mP100.z) + (mP000.x * mP010.z) - (mP010.x * mP000.z) -
                            (mP000.x * mP101.z) + (mP101.x * mP000.z) - (mP100.x * mP101.z) + (mP101.x * mP100.z) +
                            (mP000.x * mP011.z) - (mP011.x * mP000.z) + (mP010.x * mP011.z) - (mP011.x * mP010.z) -
                            (mP101.x * mP111.z) + (mP111.x * mP101.z) - (mP101.x * mP011.z) + (mP011.x * mP101.z) -
                            (mP111.x * mP011.z) + (mP011.x * mP111.z),
                        (mP000.x * mP100.y) - (mP100.x * mP000.y) - (mP000.x * mP010.y) + (mP010.x * mP000.y) +
                            (mP000.x * mP101.y) - (mP101.x * mP000.y) + (mP100.x * mP101.y) - (mP101.x * mP100.y) -
                            (mP000.x * mP011.y) + (mP011.x * mP000.y) - (mP010.x * mP011.y) + (mP011.x * mP010.y) +
                            (mP101.x * mP111.y) - (mP111.x * mP101.y) + (mP101.x * mP011.y) - (mP011.x * mP101.y) +
                            (mP111.x * mP011.y) - (mP011.x * mP111.y)} *
            kHexahedronVolumeFactor,
        common::Vector3{(mP000.y * mP100.z) - (mP100.y * mP000.z) + (mP100.y * mP110.z) - (mP110.y * mP100.z) -
                            (mP000.y * mP001.z) + (mP001.y * mP000.z) - (mP100.y * mP001.z) + (mP001.y * mP100.z) +
                            (mP100.y * mP111.z) - (mP111.y * mP100.z) + (mP110.y * mP111.z) - (mP111.y * mP110.z) -
                            (mP001.y * mP111.z) + (mP111.y * mP001.z) - (mP001.y * mP011.z) + (mP011.y * mP001.z) +
                            (mP111.y * mP011.z) - (mP011.y * mP111.z),
                        (mP100.x * mP000.z) - (mP000.x * mP100.z) - (mP100.x * mP110.z) + (mP110.x * mP100.z) +
                            (mP000.x * mP001.z) - (mP001.x * mP000.z) + (mP100.x * mP001.z) - (mP001.x * mP100.z) -
                            (mP100.x * mP111.z) + (mP111.x * mP100.z) - (mP110.x * mP111.z) + (mP111.x * mP110.z) +
                            (mP001.x * mP111.z) - (mP111.x * mP001.z) + (mP001.x * mP011.z) - (mP011.x * mP001.z) -
                            (mP111.x * mP011.z) + (mP011.x * mP111.z),
                        (mP000.x * mP100.y) - (mP100.x * mP000.y) + (mP100.x * mP110.y) - (mP110.x * mP100.y) -
                            (mP000.x * mP001.y) + (mP001.x * mP000.y) - (mP100.x * mP001.y) + (mP001.x * mP100.y) +
                            (mP100.x * mP111.y) - (mP111.x * mP100.y) + (mP110.x * mP111.y) - (mP111.x * mP110.y) -
                            (mP001.x * mP111.y) + (mP111.x * mP001.y) - (mP001.x * mP011.y) + (mP011.x * mP001.y) +
                            (mP111.x * mP011.y) - (mP011.x * mP111.y)} *
            kHexahedronVolumeFactor,
        common::Vector3{(mP100.y * mP110.z) - (mP110.y * mP100.z) + (mP110.y * mP010.z) - (mP010.y * mP110.z) -
                            (mP100.y * mP101.z) + (mP101.y * mP100.z) - (mP110.y * mP101.z) + (mP101.y * mP110.z) +
                            (mP110.y * mP011.z) + (mP001.y * mP101.z) - (mP101.y * mP001.z) - (mP011.y * mP110.z) +
                            (mP010.y * mP011.z) - (mP011.y * mP010.z) - (mP001.y * mP011.z) + (mP011.y * mP001.z) -
                            (mP101.y * mP011.z) + (mP011.y * mP101.z),
                        (mP110.x * mP100.z) - (mP100.x * mP110.z) - (mP110.x * mP010.z) + (mP010.x * mP110.z) +
                            (mP100.x * mP101.z) - (mP101.x * mP100.z) + (mP110.x * mP101.z) - (mP101.x * mP110.z) -
                            (mP110.x * mP011.z) - (mP001.x * mP101.z) + (mP101.x * mP001.z) + (mP011.x * mP110.z) -
                            (mP010.x * mP011.z) + (mP011.x * mP010.z) + (mP001.x * mP011.z) - (mP011.x * mP001.z) +
                            (mP101.x * mP011.z) - (mP011.x * mP101.z),
                        (mP100.x * mP110.y) - (mP110.x * mP100.y) + (mP110.x * mP010.y) - (mP010.x * mP110.y) -
                            (mP100.x * mP101.y) + (mP101.x * mP100.y) - (mP110.x * mP101.y) + (mP101.x * mP110.y) +
                            (mP110.x * mP011.y) + (mP001.x * mP101.y) - (mP101.x * mP001.y) - (mP011.x * mP110.y) +
                            (mP010.x * mP011.y) - (mP011.x * mP010.y) - (mP001.x * mP011.y) + (mP011.x * mP001.y) -
                            (mP101.x * mP011.y) + (mP011.x * mP101.y)} *
            kHexahedronVolumeFactor,
        common::Vector3{(mP010.y * mP000.z) - (mP000.y * mP010.z) + (mP000.y * mP001.z) - (mP001.y * mP000.z) +
                            (mP110.y * mP010.z) - (mP010.y * mP110.z) + (mP010.y * mP001.z) - (mP001.y * mP010.z) -
                            (mP110.y * mP111.z) + (mP111.y * mP110.z) - (mP010.y * mP111.z) + (mP001.y * mP101.z) -
                            (mP101.y * mP001.z) + (mP111.y * mP010.z) + (mP001.y * mP111.z) - (mP111.y * mP001.z) +
                            (mP101.y * mP111.z) - (mP111.y * mP101.z),
                        (mP000.x * mP010.z) - (mP010.x * mP000.z) - (mP000.x * mP001.z) + (mP001.x * mP000.z) -
                            (mP110.x * mP010.z) + (mP010.x * mP110.z) - (mP010.x * mP001.z) + (mP001.x * mP010.z) +
                            (mP110.x * mP111.z) - (mP111.x * mP110.z) + (mP010.x * mP111.z) - (mP001.x * mP101.z) +
                            (mP101.x * mP001.z) - (mP111.x * mP010.z) - (mP001.x * mP111.z) + (mP111.x * mP001.z) -
                            (mP101.x * mP111.z) + (mP111.x * mP101.z),
                        (mP010.x * mP000.y) - (mP000.x * mP010.y) + (mP000.x * mP001.y) - (mP001.x * mP000.y) +
                            (mP110.x * mP010.y) - (mP010.x * mP110.y) + (mP010.x * mP001.y) - (mP001.x * mP010.y) -
                            (mP110.x * mP111.y) + (mP111.x * mP110.y) - (mP010.x * mP111.y) + (mP001.x * mP101.y) -
                            (mP101.x * mP001.y) + (mP111.x * mP010.y) + (mP001.x * mP111.y) - (mP111.x * mP001.y) +
                            (mP101.x * mP111.y) - (mP111.x * mP101.y)} *
            kHexahedronVolumeFactor};
}
}  // namespace plato::third_party_integration::stk_io
