#include "plato/third_party_integration/stk_io/Hexahedron.hpp"

#include "plato/third_party_integration/common/QuadratureRules.hpp"

namespace plato::third_party_integration::stk_io
{
double Hexahedron::volume() const
{
    return ((p000.x * p110.y * p100.z) - (p000.x * p100.y * p110.z) + (p100.x * p000.y * p110.z) -
            (p100.x * p110.y * p000.z) - (p110.x * p000.y * p100.z) + (p110.x * p100.y * p000.z) -
            (p000.x * p100.y * p010.z) + (p000.x * p010.y * p100.z) + (p100.x * p000.y * p010.z) -
            (p100.x * p010.y * p000.z) - (p010.x * p000.y * p100.z) + (p010.x * p100.y * p000.z) +
            (p000.x * p100.y * p001.z) - (p000.x * p110.y * p010.z) + (p000.x * p010.y * p110.z) -
            (p000.x * p001.y * p100.z) - (p100.x * p000.y * p001.z) + (p100.x * p001.y * p000.z) +
            (p110.x * p000.y * p010.z) - (p110.x * p010.y * p000.z) - (p010.x * p000.y * p110.z) +
            (p010.x * p110.y * p000.z) + (p001.x * p000.y * p100.z) - (p001.x * p100.y * p000.z) +
            (p000.x * p100.y * p101.z) - (p000.x * p101.y * p100.z) - (p100.x * p000.y * p101.z) -
            (p100.x * p110.y * p010.z) + (p100.x * p010.y * p110.z) + (p100.x * p101.y * p000.z) +
            (p110.x * p100.y * p010.z) - (p110.x * p010.y * p100.z) - (p010.x * p100.y * p110.z) +
            (p010.x * p110.y * p100.z) + (p101.x * p000.y * p100.z) - (p101.x * p100.y * p000.z) -
            (p000.x * p010.y * p001.z) + (p000.x * p001.y * p010.z) + (p010.x * p000.y * p001.z) -
            (p010.x * p001.y * p000.z) - (p001.x * p000.y * p010.z) + (p001.x * p010.y * p000.z) +
            (p100.x * p110.y * p101.z) - (p100.x * p101.y * p110.z) - (p110.x * p100.y * p101.z) +
            (p110.x * p101.y * p100.z) + (p101.x * p100.y * p110.z) - (p101.x * p110.y * p100.z) -
            (p000.x * p001.y * p101.z) + (p000.x * p101.y * p001.z) + (p100.x * p110.y * p111.z) -
            (p100.x * p111.y * p110.z) - (p110.x * p100.y * p111.z) + (p110.x * p111.y * p100.z) +
            (p001.x * p000.y * p101.z) - (p001.x * p101.y * p000.z) - (p101.x * p000.y * p001.z) +
            (p101.x * p001.y * p000.z) + (p111.x * p100.y * p110.z) - (p111.x * p110.y * p100.z) -
            (p000.x * p010.y * p011.z) + (p000.x * p011.y * p010.z) - (p100.x * p001.y * p101.z) +
            (p100.x * p101.y * p001.z) + (p010.x * p000.y * p011.z) - (p010.x * p011.y * p000.z) +
            (p001.x * p100.y * p101.z) - (p001.x * p101.y * p100.z) - (p101.x * p100.y * p001.z) +
            (p101.x * p001.y * p100.z) - (p011.x * p000.y * p010.z) + (p011.x * p010.y * p000.z) +
            (p000.x * p001.y * p011.z) - (p000.x * p011.y * p001.z) + (p110.x * p010.y * p111.z) -
            (p110.x * p111.y * p010.z) - (p010.x * p110.y * p111.z) + (p010.x * p111.y * p110.z) -
            (p001.x * p000.y * p011.z) + (p001.x * p011.y * p000.z) + (p111.x * p110.y * p010.z) -
            (p111.x * p010.y * p110.z) + (p011.x * p000.y * p001.z) - (p011.x * p001.y * p000.z) -
            (p100.x * p101.y * p111.z) + (p100.x * p111.y * p101.z) + (p110.x * p010.y * p011.z) -
            (p110.x * p011.y * p010.z) - (p010.x * p110.y * p011.z) + (p010.x * p011.y * p110.z) +
            (p101.x * p100.y * p111.z) - (p101.x * p111.y * p100.z) - (p111.x * p100.y * p101.z) +
            (p111.x * p101.y * p100.z) + (p011.x * p110.y * p010.z) - (p011.x * p010.y * p110.z) -
            (p110.x * p101.y * p111.z) + (p110.x * p111.y * p101.z) + (p101.x * p110.y * p111.z) -
            (p101.x * p111.y * p110.z) - (p111.x * p110.y * p101.z) + (p111.x * p101.y * p110.z) +
            (p010.x * p001.y * p011.z) - (p010.x * p011.y * p001.z) - (p001.x * p010.y * p011.z) +
            (p001.x * p011.y * p010.z) + (p011.x * p010.y * p001.z) - (p011.x * p001.y * p010.z) -
            (p110.x * p111.y * p011.z) + (p110.x * p011.y * p111.z) + (p001.x * p101.y * p111.z) -
            (p001.x * p111.y * p101.z) - (p101.x * p001.y * p111.z) + (p101.x * p111.y * p001.z) +
            (p111.x * p110.y * p011.z) + (p111.x * p001.y * p101.z) - (p111.x * p101.y * p001.z) -
            (p111.x * p011.y * p110.z) - (p011.x * p110.y * p111.z) + (p011.x * p111.y * p110.z) -
            (p010.x * p111.y * p011.z) + (p010.x * p011.y * p111.z) + (p001.x * p101.y * p011.z) -
            (p001.x * p011.y * p101.z) - (p101.x * p001.y * p011.z) + (p101.x * p011.y * p001.z) +
            (p111.x * p010.y * p011.z) - (p111.x * p011.y * p010.z) - (p011.x * p010.y * p111.z) +
            (p011.x * p001.y * p101.z) - (p011.x * p101.y * p001.z) + (p011.x * p111.y * p010.z) +
            (p001.x * p111.y * p011.z) - (p001.x * p011.y * p111.z) - (p111.x * p001.y * p011.z) +
            (p111.x * p011.y * p001.z) + (p011.x * p001.y * p111.z) - (p011.x * p111.y * p001.z) +
            (p101.x * p111.y * p011.z) - (p101.x * p011.y * p111.z) - (p111.x * p101.y * p011.z) +
            (p111.x * p011.y * p101.z) + (p011.x * p101.y * p111.z) - (p011.x * p111.y * p101.z)) /
           12;
}

common::Coordinate Hexahedron::centroid() const
{
    constexpr int kDimensions{3};
    constexpr auto kQuadraturePoints = common::kDegree3OneDimensionalQuadraturePoints;
    return common::tensor_product_unit_integral<kDimensions, kQuadraturePoints.size(), kQuadraturePoints>(
               [*this](const auto tXi, const auto tEta, const auto tMu)
               {
                   const double tJ11 = ((p100.x * (tEta - 1) * (tMu - 1)) - (p000.x * (tEta - 1) * (tMu - 1)) -
                                        (p110.x * (tEta + 1) * (tMu - 1)) + (p010.x * (tEta + 1) * (tMu - 1)) +
                                        (p001.x * (tEta - 1) * (tMu + 1)) - (p101.x * (tEta - 1) * (tMu + 1)) +
                                        (p111.x * (tEta + 1) * (tMu + 1)) - (p011.x * (tEta + 1) * (tMu + 1))) /
                                       8;
                   const double tJ12 = ((p100.y * (tEta - 1) * (tMu - 1)) - (p000.y * (tEta - 1) * (tMu - 1)) -
                                        (p110.y * (tEta + 1) * (tMu - 1)) + (p010.y * (tEta + 1) * (tMu - 1)) +
                                        (p001.y * (tEta - 1) * (tMu + 1)) - (p101.y * (tEta - 1) * (tMu + 1)) +
                                        (p111.y * (tEta + 1) * (tMu + 1)) - (p011.y * (tEta + 1) * (tMu + 1))) /
                                       8;
                   const double tJ13 = ((p100.z * (tEta - 1) * (tMu - 1)) - (p000.z * (tEta - 1) * (tMu - 1)) -
                                        (p110.z * (tEta + 1) * (tMu - 1)) + (p010.z * (tEta + 1) * (tMu - 1)) +
                                        (p001.z * (tEta - 1) * (tMu + 1)) - (p101.z * (tEta - 1) * (tMu + 1)) +
                                        (p111.z * (tEta + 1) * (tMu + 1)) - (p011.z * (tEta + 1) * (tMu + 1))) /
                                       8;
                   const double tJ21 = ((p100.x * (tMu - 1) * (tXi + 1)) - (p000.x * (tMu - 1) * (tXi - 1)) -
                                        (p110.x * (tMu - 1) * (tXi + 1)) + (p010.x * (tMu - 1) * (tXi - 1)) +
                                        (p001.x * (tMu + 1) * (tXi - 1)) - (p101.x * (tMu + 1) * (tXi + 1)) +
                                        (p111.x * (tMu + 1) * (tXi + 1)) - (p011.x * (tMu + 1) * (tXi - 1))) /
                                       8;
                   const double tJ22 = ((p100.y * (tMu - 1) * (tXi + 1)) - (p000.y * (tMu - 1) * (tXi - 1)) -
                                        (p110.y * (tMu - 1) * (tXi + 1)) + (p010.y * (tMu - 1) * (tXi - 1)) +
                                        (p001.y * (tMu + 1) * (tXi - 1)) - (p101.y * (tMu + 1) * (tXi + 1)) +
                                        (p111.y * (tMu + 1) * (tXi + 1)) - (p011.y * (tMu + 1) * (tXi - 1))) /
                                       8;
                   const double tJ23 = ((p100.z * (tMu - 1) * (tXi + 1)) - (p000.z * (tMu - 1) * (tXi - 1)) -
                                        (p110.z * (tMu - 1) * (tXi + 1)) + (p010.z * (tMu - 1) * (tXi - 1)) +
                                        (p001.z * (tMu + 1) * (tXi - 1)) - (p101.z * (tMu + 1) * (tXi + 1)) +
                                        (p111.z * (tMu + 1) * (tXi + 1)) - (p011.z * (tMu + 1) * (tXi - 1))) /
                                       8;
                   const double tJ31 = ((p100.x * (tEta - 1) * (tXi + 1)) - (p000.x * (tEta - 1) * (tXi - 1)) -
                                        (p110.x * (tEta + 1) * (tXi + 1)) + (p010.x * (tEta + 1) * (tXi - 1)) +
                                        (p001.x * (tEta - 1) * (tXi - 1)) - (p101.x * (tEta - 1) * (tXi + 1)) +
                                        (p111.x * (tEta + 1) * (tXi + 1)) - (p011.x * (tEta + 1) * (tXi - 1))) /
                                       8;
                   const double tJ32 = ((p100.y * (tEta - 1) * (tXi + 1)) - (p000.y * (tEta - 1) * (tXi - 1)) -
                                        (p110.y * (tEta + 1) * (tXi + 1)) + (p010.y * (tEta + 1) * (tXi - 1)) +
                                        (p001.y * (tEta - 1) * (tXi - 1)) - (p101.y * (tEta - 1) * (tXi + 1)) +
                                        (p111.y * (tEta + 1) * (tXi + 1)) - (p011.y * (tEta + 1) * (tXi - 1))) /
                                       8;
                   const double tJ33 = ((p100.z * (tEta - 1) * (tXi + 1)) - (p000.z * (tEta - 1) * (tXi - 1)) -
                                        (p110.z * (tEta + 1) * (tXi + 1)) + (p010.z * (tEta + 1) * (tXi - 1)) +
                                        (p001.z * (tEta - 1) * (tXi - 1)) - (p101.z * (tEta - 1) * (tXi + 1)) +
                                        (p111.z * (tEta + 1) * (tXi + 1)) - (p011.z * (tEta + 1) * (tXi - 1))) /
                                       8;
                   const double tM1 =
                       ((p100.x * (tEta - 1) * (tMu - 1) * (tXi + 1)) - (p000.x * (tEta - 1) * (tMu - 1) * (tXi - 1)) -
                        (p110.x * (tEta + 1) * (tMu - 1) * (tXi + 1)) + (p010.x * (tEta + 1) * (tMu - 1) * (tXi - 1)) +
                        (p001.x * (tEta - 1) * (tMu + 1) * (tXi - 1)) - (p101.x * (tEta - 1) * (tMu + 1) * (tXi + 1)) +
                        (p111.x * (tEta + 1) * (tMu + 1) * (tXi + 1)) - (p011.x * (tEta + 1) * (tMu + 1) * (tXi - 1))) /
                       8;
                   const double tM2 =
                       ((p100.y * (tEta - 1) * (tMu - 1) * (tXi + 1)) - (p000.y * (tEta - 1) * (tMu - 1) * (tXi - 1)) -
                        (p110.y * (tEta + 1) * (tMu - 1) * (tXi + 1)) + (p010.y * (tEta + 1) * (tMu - 1) * (tXi - 1)) +
                        (p001.y * (tEta - 1) * (tMu + 1) * (tXi - 1)) - (p101.y * (tEta - 1) * (tMu + 1) * (tXi + 1)) +
                        (p111.y * (tEta + 1) * (tMu + 1) * (tXi + 1)) - (p011.y * (tEta + 1) * (tMu + 1) * (tXi - 1))) /
                       8;
                   const double tM3 =
                       ((p100.z * (tEta - 1) * (tMu - 1) * (tXi + 1)) - (p000.z * (tEta - 1) * (tMu - 1) * (tXi - 1)) -
                        (p110.z * (tEta + 1) * (tMu - 1) * (tXi + 1)) + (p010.z * (tEta + 1) * (tMu - 1) * (tXi - 1)) +
                        (p001.z * (tEta - 1) * (tMu + 1) * (tXi - 1)) - (p101.z * (tEta - 1) * (tMu + 1) * (tXi + 1)) +
                        (p111.z * (tEta + 1) * (tMu + 1) * (tXi + 1)) - (p011.z * (tEta + 1) * (tMu + 1) * (tXi - 1))) /
                       8;

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
            (p110.y * p100.z) - (p100.y * p110.z) - (p100.y * p010.z) + (p010.y * p100.z) + (p100.y * p001.z) -
                (p110.y * p010.z) + (p010.y * p110.z) - (p001.y * p100.z) + (p100.y * p101.z) - (p101.y * p100.z) -
                (p010.y * p001.z) + (p001.y * p010.z) - (p001.y * p101.z) + (p101.y * p001.z) - (p010.y * p011.z) +
                (p011.y * p010.z) + (p001.y * p011.z) - (p011.y * p001.z),
            (p100.x * p110.z) - (p110.x * p100.z) + (p100.x * p010.z) - (p010.x * p100.z) - (p100.x * p001.z) +
                (p110.x * p010.z) - (p010.x * p110.z) + (p001.x * p100.z) - (p100.x * p101.z) + (p101.x * p100.z) +
                (p010.x * p001.z) - (p001.x * p010.z) + (p001.x * p101.z) - (p101.x * p001.z) + (p010.x * p011.z) -
                (p011.x * p010.z) - (p001.x * p011.z) + (p011.x * p001.z),
            (p110.x * p100.y) - (p100.x * p110.y) - (p100.x * p010.y) + (p010.x * p100.y) + (p100.x * p001.y) -
                (p110.x * p010.y) + (p010.x * p110.y) - (p001.x * p100.y) + (p100.x * p101.y) - (p101.x * p100.y) -
                (p010.x * p001.y) + (p001.x * p010.y) - (p001.x * p101.y) + (p101.x * p001.y) - (p010.x * p011.y) +
                (p011.x * p010.y) + (p001.x * p011.y) - (p011.x * p001.y)} /
            12,
        common::Vector3{
            (p000.y * p110.z) - (p110.y * p000.z) + (p000.y * p010.z) - (p010.y * p000.z) - (p000.y * p001.z) +
                (p001.y * p000.z) - (p000.y * p101.z) - (p110.y * p010.z) + (p010.y * p110.z) + (p101.y * p000.z) +
                (p110.y * p101.z) - (p101.y * p110.z) + (p110.y * p111.z) - (p111.y * p110.z) - (p001.y * p101.z) +
                (p101.y * p001.z) - (p101.y * p111.z) + (p111.y * p101.z),
            (p110.x * p000.z) - (p000.x * p110.z) - (p000.x * p010.z) + (p010.x * p000.z) + (p000.x * p001.z) -
                (p001.x * p000.z) + (p000.x * p101.z) + (p110.x * p010.z) - (p010.x * p110.z) - (p101.x * p000.z) -
                (p110.x * p101.z) + (p101.x * p110.z) - (p110.x * p111.z) + (p111.x * p110.z) + (p001.x * p101.z) -
                (p101.x * p001.z) + (p101.x * p111.z) - (p111.x * p101.z),
            (p000.x * p110.y) - (p110.x * p000.y) + (p000.x * p010.y) - (p010.x * p000.y) - (p000.x * p001.y) +
                (p001.x * p000.y) - (p000.x * p101.y) - (p110.x * p010.y) + (p010.x * p110.y) + (p101.x * p000.y) +
                (p110.x * p101.y) - (p101.x * p110.y) + (p110.x * p111.y) - (p111.x * p110.y) - (p001.x * p101.y) +
                (p101.x * p001.y) - (p101.x * p111.y) + (p111.x * p101.y)} /
            12,
        common::Vector3{
            (p100.y * p000.z) - (p000.y * p100.z) + (p000.y * p010.z) - (p010.y * p000.z) + (p100.y * p010.z) -
                (p010.y * p100.z) - (p100.y * p101.z) + (p101.y * p100.z) - (p100.y * p111.z) + (p111.y * p100.z) +
                (p010.y * p111.z) - (p111.y * p010.z) + (p010.y * p011.z) - (p011.y * p010.z) - (p101.y * p111.z) +
                (p111.y * p101.z) - (p111.y * p011.z) + (p011.y * p111.z),
            (p000.x * p100.z) - (p100.x * p000.z) - (p000.x * p010.z) + (p010.x * p000.z) - (p100.x * p010.z) +
                (p010.x * p100.z) + (p100.x * p101.z) - (p101.x * p100.z) + (p100.x * p111.z) - (p111.x * p100.z) -
                (p010.x * p111.z) + (p111.x * p010.z) - (p010.x * p011.z) + (p011.x * p010.z) + (p101.x * p111.z) -
                (p111.x * p101.z) + (p111.x * p011.z) - (p011.x * p111.z),
            (p100.x * p000.y) - (p000.x * p100.y) + (p000.x * p010.y) - (p010.x * p000.y) + (p100.x * p010.y) -
                (p010.x * p100.y) - (p100.x * p101.y) + (p101.x * p100.y) - (p100.x * p111.y) + (p111.x * p100.y) +
                (p010.x * p111.y) - (p111.x * p010.y) + (p010.x * p011.y) - (p011.x * p010.y) - (p101.x * p111.y) +
                (p111.x * p101.y) - (p111.x * p011.y) + (p011.x * p111.y)} /
            12,
        common::Vector3{
            (p100.y * p000.z) - (p000.y * p100.z) - (p000.y * p110.z) + (p110.y * p000.z) - (p100.y * p110.z) +
                (p110.y * p100.z) + (p000.y * p001.z) - (p001.y * p000.z) + (p000.y * p011.z) - (p011.y * p000.z) -
                (p110.y * p111.z) + (p111.y * p110.z) - (p110.y * p011.z) + (p011.y * p110.z) + (p001.y * p011.z) -
                (p011.y * p001.z) - (p111.y * p011.z) + (p011.y * p111.z),
            (p000.x * p100.z) - (p100.x * p000.z) + (p000.x * p110.z) - (p110.x * p000.z) + (p100.x * p110.z) -
                (p110.x * p100.z) - (p000.x * p001.z) + (p001.x * p000.z) - (p000.x * p011.z) + (p011.x * p000.z) +
                (p110.x * p111.z) - (p111.x * p110.z) + (p110.x * p011.z) - (p011.x * p110.z) - (p001.x * p011.z) +
                (p011.x * p001.z) + (p111.x * p011.z) - (p011.x * p111.z),
            (p100.x * p000.y) - (p000.x * p100.y) - (p000.x * p110.y) + (p110.x * p000.y) - (p100.x * p110.y) +
                (p110.x * p100.y) + (p000.x * p001.y) - (p001.x * p000.y) + (p000.x * p011.y) - (p011.x * p000.y) -
                (p110.x * p111.y) + (p111.x * p110.y) - (p110.x * p011.y) + (p011.x * p110.y) + (p001.x * p011.y) -
                (p011.x * p001.y) - (p111.x * p011.y) + (p011.x * p111.y)} /
            12,
        common::Vector3{
            (p000.y * p100.z) - (p100.y * p000.z) - (p000.y * p010.z) + (p010.y * p000.z) + (p000.y * p101.z) -
                (p101.y * p000.z) + (p100.y * p101.z) - (p101.y * p100.z) - (p000.y * p011.z) + (p011.y * p000.z) -
                (p010.y * p011.z) + (p011.y * p010.z) + (p101.y * p111.z) - (p111.y * p101.z) + (p101.y * p011.z) -
                (p011.y * p101.z) + (p111.y * p011.z) - (p011.y * p111.z),
            (p100.x * p000.z) - (p000.x * p100.z) + (p000.x * p010.z) - (p010.x * p000.z) - (p000.x * p101.z) +
                (p101.x * p000.z) - (p100.x * p101.z) + (p101.x * p100.z) + (p000.x * p011.z) - (p011.x * p000.z) +
                (p010.x * p011.z) - (p011.x * p010.z) - (p101.x * p111.z) + (p111.x * p101.z) - (p101.x * p011.z) +
                (p011.x * p101.z) - (p111.x * p011.z) + (p011.x * p111.z),
            (p000.x * p100.y) - (p100.x * p000.y) - (p000.x * p010.y) + (p010.x * p000.y) + (p000.x * p101.y) -
                (p101.x * p000.y) + (p100.x * p101.y) - (p101.x * p100.y) - (p000.x * p011.y) + (p011.x * p000.y) -
                (p010.x * p011.y) + (p011.x * p010.y) + (p101.x * p111.y) - (p111.x * p101.y) + (p101.x * p011.y) -
                (p011.x * p101.y) + (p111.x * p011.y) - (p011.x * p111.y)} /
            12,
        common::Vector3{
            (p000.y * p100.z) - (p100.y * p000.z) + (p100.y * p110.z) - (p110.y * p100.z) - (p000.y * p001.z) +
                (p001.y * p000.z) - (p100.y * p001.z) + (p001.y * p100.z) + (p100.y * p111.z) - (p111.y * p100.z) +
                (p110.y * p111.z) - (p111.y * p110.z) - (p001.y * p111.z) + (p111.y * p001.z) - (p001.y * p011.z) +
                (p011.y * p001.z) + (p111.y * p011.z) - (p011.y * p111.z),
            (p100.x * p000.z) - (p000.x * p100.z) - (p100.x * p110.z) + (p110.x * p100.z) + (p000.x * p001.z) -
                (p001.x * p000.z) + (p100.x * p001.z) - (p001.x * p100.z) - (p100.x * p111.z) + (p111.x * p100.z) -
                (p110.x * p111.z) + (p111.x * p110.z) + (p001.x * p111.z) - (p111.x * p001.z) + (p001.x * p011.z) -
                (p011.x * p001.z) - (p111.x * p011.z) + (p011.x * p111.z),
            (p000.x * p100.y) - (p100.x * p000.y) + (p100.x * p110.y) - (p110.x * p100.y) - (p000.x * p001.y) +
                (p001.x * p000.y) - (p100.x * p001.y) + (p001.x * p100.y) + (p100.x * p111.y) - (p111.x * p100.y) +
                (p110.x * p111.y) - (p111.x * p110.y) - (p001.x * p111.y) + (p111.x * p001.y) - (p001.x * p011.y) +
                (p011.x * p001.y) + (p111.x * p011.y) - (p011.x * p111.y)} /
            12,
        common::Vector3{
            (p100.y * p110.z) - (p110.y * p100.z) + (p110.y * p010.z) - (p010.y * p110.z) - (p100.y * p101.z) +
                (p101.y * p100.z) - (p110.y * p101.z) + (p101.y * p110.z) + (p110.y * p011.z) + (p001.y * p101.z) -
                (p101.y * p001.z) - (p011.y * p110.z) + (p010.y * p011.z) - (p011.y * p010.z) - (p001.y * p011.z) +
                (p011.y * p001.z) - (p101.y * p011.z) + (p011.y * p101.z),
            (p110.x * p100.z) - (p100.x * p110.z) - (p110.x * p010.z) + (p010.x * p110.z) + (p100.x * p101.z) -
                (p101.x * p100.z) + (p110.x * p101.z) - (p101.x * p110.z) - (p110.x * p011.z) - (p001.x * p101.z) +
                (p101.x * p001.z) + (p011.x * p110.z) - (p010.x * p011.z) + (p011.x * p010.z) + (p001.x * p011.z) -
                (p011.x * p001.z) + (p101.x * p011.z) - (p011.x * p101.z),
            (p100.x * p110.y) - (p110.x * p100.y) + (p110.x * p010.y) - (p010.x * p110.y) - (p100.x * p101.y) +
                (p101.x * p100.y) - (p110.x * p101.y) + (p101.x * p110.y) + (p110.x * p011.y) + (p001.x * p101.y) -
                (p101.x * p001.y) - (p011.x * p110.y) + (p010.x * p011.y) - (p011.x * p010.y) - (p001.x * p011.y) +
                (p011.x * p001.y) - (p101.x * p011.y) + (p011.x * p101.y)} /
            12,
        common::Vector3{
            (p010.y * p000.z) - (p000.y * p010.z) + (p000.y * p001.z) - (p001.y * p000.z) + (p110.y * p010.z) -
                (p010.y * p110.z) + (p010.y * p001.z) - (p001.y * p010.z) - (p110.y * p111.z) + (p111.y * p110.z) -
                (p010.y * p111.z) + (p001.y * p101.z) - (p101.y * p001.z) + (p111.y * p010.z) + (p001.y * p111.z) -
                (p111.y * p001.z) + (p101.y * p111.z) - (p111.y * p101.z),
            (p000.x * p010.z) - (p010.x * p000.z) - (p000.x * p001.z) + (p001.x * p000.z) - (p110.x * p010.z) +
                (p010.x * p110.z) - (p010.x * p001.z) + (p001.x * p010.z) + (p110.x * p111.z) - (p111.x * p110.z) +
                (p010.x * p111.z) - (p001.x * p101.z) + (p101.x * p001.z) - (p111.x * p010.z) - (p001.x * p111.z) +
                (p111.x * p001.z) - (p101.x * p111.z) + (p111.x * p101.z),
            (p010.x * p000.y) - (p000.x * p010.y) + (p000.x * p001.y) - (p001.x * p000.y) + (p110.x * p010.y) -
                (p010.x * p110.y) + (p010.x * p001.y) - (p001.x * p010.y) - (p110.x * p111.y) + (p111.x * p110.y) -
                (p010.x * p111.y) + (p001.x * p101.y) - (p101.x * p001.y) + (p111.x * p010.y) + (p001.x * p111.y) -
                (p111.x * p001.y) + (p101.x * p111.y) - (p111.x * p101.y)} /
            12};
}
}  // namespace plato::third_party_integration::stk_io
