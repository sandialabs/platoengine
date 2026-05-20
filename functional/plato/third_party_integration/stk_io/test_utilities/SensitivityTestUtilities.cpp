#include "plato/third_party_integration/stk_io/test_utilities/SensitivityTestUtilities.hpp"

#include <array>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::stk_io::test_utilities
{
auto axes_aligned_hex_volume_sensitivities(const Cuboid& aCuboid)
    -> std::array<common::Vector3, primitives::kNumberOfHexahedronNodes>
{
    return std::array{common::Vector3{.mX = -aCuboid.mWidth * aCuboid.mHeight,
                                      .mY = -aCuboid.mLength * aCuboid.mHeight,
                                      .mZ = -aCuboid.mLength * aCuboid.mWidth} /
                          4,
                      common::Vector3{.mX = aCuboid.mWidth * aCuboid.mHeight,
                                      .mY = -aCuboid.mLength * aCuboid.mHeight,
                                      .mZ = -aCuboid.mLength * aCuboid.mWidth} /
                          4,
                      common::Vector3{.mX = aCuboid.mWidth * aCuboid.mHeight,
                                      .mY = aCuboid.mLength * aCuboid.mHeight,
                                      .mZ = -aCuboid.mLength * aCuboid.mWidth} /
                          4,
                      common::Vector3{.mX = -aCuboid.mWidth * aCuboid.mHeight,
                                      .mY = aCuboid.mLength * aCuboid.mHeight,
                                      .mZ = -aCuboid.mLength * aCuboid.mWidth} /
                          4,
                      common::Vector3{.mX = -aCuboid.mWidth * aCuboid.mHeight,
                                      .mY = -aCuboid.mLength * aCuboid.mHeight,
                                      .mZ = aCuboid.mLength * aCuboid.mWidth} /
                          4,
                      common::Vector3{.mX = aCuboid.mWidth * aCuboid.mHeight,
                                      .mY = -aCuboid.mLength * aCuboid.mHeight,
                                      .mZ = aCuboid.mLength * aCuboid.mWidth} /
                          4,
                      common::Vector3{.mX = aCuboid.mWidth * aCuboid.mHeight,
                                      .mY = aCuboid.mLength * aCuboid.mHeight,
                                      .mZ = aCuboid.mLength * aCuboid.mWidth} /
                          4,
                      common::Vector3{.mX = -aCuboid.mWidth * aCuboid.mHeight,
                                      .mY = aCuboid.mLength * aCuboid.mHeight,
                                      .mZ = aCuboid.mLength * aCuboid.mWidth} /
                          4};
}

auto axes_aligned_tet_volume_sensitivities(const Cuboid& aCuboid)
    -> std::array<common::Vector3, primitives::kNumberOfTetrahedronNodes>
{
    constexpr auto tTetrahedronVolumeScale = 1.0 / 6.0;
    return std::array{
        common::Vector3{.mX = -aCuboid.mWidth * aCuboid.mHeight,
                        .mY = -aCuboid.mLength * aCuboid.mHeight,
                        .mZ = -aCuboid.mLength * aCuboid.mWidth} *
            tTetrahedronVolumeScale,
        common::Vector3{.mX = aCuboid.mWidth * aCuboid.mHeight, .mY = 0, .mZ = 0} * tTetrahedronVolumeScale,
        common::Vector3{.mX = 0, .mY = aCuboid.mLength * aCuboid.mHeight, .mZ = 0} * tTetrahedronVolumeScale,
        common::Vector3{.mX = 0, .mY = 0, .mZ = aCuboid.mLength * aCuboid.mWidth} * tTetrahedronVolumeScale};
}

auto axes_aligned_tri_volume_sensitivities(const Rectangle& aRectangle)
    -> std::array<common::Vector3, primitives::kNumberOfTriangleNodes>
{
    return std::array{common::Vector3{.mX = -aRectangle.mWidth, .mY = -aRectangle.mLength, .mZ = 0.0} / 2,
                      common::Vector3{.mX = aRectangle.mWidth, .mY = 0.0, .mZ = 0.0} / 2,
                      common::Vector3{.mX = 0.0, .mY = aRectangle.mLength, .mZ = 0.0} / 2};
}

auto axes_aligned_quad_volume_sensitivities(const Rectangle& aRectangle)
    -> std::array<common::Vector3, primitives::kNumberOfQuadrilateralNodes>
{
    return std::array{common::Vector3{.mX = -aRectangle.mWidth, .mY = -aRectangle.mLength, .mZ = 0.0} / 2,
                      common::Vector3{.mX = aRectangle.mWidth, .mY = -aRectangle.mLength, .mZ = 0.0} / 2,
                      common::Vector3{.mX = aRectangle.mWidth, .mY = aRectangle.mLength, .mZ = 0.0} / 2,
                      common::Vector3{.mX = -aRectangle.mWidth, .mY = aRectangle.mLength, .mZ = 0.0} / 2};
}
}  // namespace plato::third_party_integration::stk_io::test_utilities
