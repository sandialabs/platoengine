#include "plato/third_party_integration/stk_io/test_utilities/SensitivityTestUtilities.hpp"

#include <array>

#include "plato/third_party_integration/common/Vector3.hpp"

namespace plato::third_party_integration::stk_io::test_utilities
{
auto axes_aligned_hex_volume_sensitivities(const Cuboid& aCuboid) -> std::array<common::Vector3, 8>
{
    return std::array{common::Vector3{.x = -aCuboid.mWidth * aCuboid.mHeight,
                                      .y = -aCuboid.mLength * aCuboid.mHeight,
                                      .z = -aCuboid.mLength * aCuboid.mWidth} /
                          4,
                      common::Vector3{.x = aCuboid.mWidth * aCuboid.mHeight,
                                      .y = -aCuboid.mLength * aCuboid.mHeight,
                                      .z = -aCuboid.mLength * aCuboid.mWidth} /
                          4,
                      common::Vector3{.x = aCuboid.mWidth * aCuboid.mHeight,
                                      .y = aCuboid.mLength * aCuboid.mHeight,
                                      .z = -aCuboid.mLength * aCuboid.mWidth} /
                          4,
                      common::Vector3{.x = -aCuboid.mWidth * aCuboid.mHeight,
                                      .y = aCuboid.mLength * aCuboid.mHeight,
                                      .z = -aCuboid.mLength * aCuboid.mWidth} /
                          4,
                      common::Vector3{.x = -aCuboid.mWidth * aCuboid.mHeight,
                                      .y = -aCuboid.mLength * aCuboid.mHeight,
                                      .z = aCuboid.mLength * aCuboid.mWidth} /
                          4,
                      common::Vector3{.x = aCuboid.mWidth * aCuboid.mHeight,
                                      .y = -aCuboid.mLength * aCuboid.mHeight,
                                      .z = aCuboid.mLength * aCuboid.mWidth} /
                          4,
                      common::Vector3{.x = aCuboid.mWidth * aCuboid.mHeight,
                                      .y = aCuboid.mLength * aCuboid.mHeight,
                                      .z = aCuboid.mLength * aCuboid.mWidth} /
                          4,
                      common::Vector3{.x = -aCuboid.mWidth * aCuboid.mHeight,
                                      .y = aCuboid.mLength * aCuboid.mHeight,
                                      .z = aCuboid.mLength * aCuboid.mWidth} /
                          4};
}

auto axes_aligned_tet_volume_sensitivities(const Cuboid& aCuboid) -> std::array<common::Vector3, 4>
{
    return std::array{common::Vector3{.x = -aCuboid.mWidth * aCuboid.mHeight,
                                      .y = -aCuboid.mLength * aCuboid.mHeight,
                                      .z = -aCuboid.mLength * aCuboid.mWidth} /
                          6,
                      common::Vector3{.x = aCuboid.mWidth * aCuboid.mHeight, .y = 0, .z = 0} / 6,
                      common::Vector3{.x = 0, .y = aCuboid.mLength * aCuboid.mHeight, .z = 0} / 6,
                      common::Vector3{.x = 0, .y = 0, .z = aCuboid.mLength * aCuboid.mWidth} / 6};
}

auto axes_aligned_tri_volume_sensitivities(const Rectangle& aRectangle) -> std::array<common::Vector3, 3>
{
    return std::array{common::Vector3{.x = -aRectangle.mWidth, .y = -aRectangle.mLength, .z = 0.0} / 2,
                      common::Vector3{.x = aRectangle.mWidth, .y = 0.0, .z = 0.0} / 2,
                      common::Vector3{.x = 0.0, .y = aRectangle.mLength, .z = 0.0} / 2};
}

auto axes_aligned_quad_volume_sensitivities(const Rectangle& aRectangle) -> std::array<common::Vector3, 4>
{
    return std::array{common::Vector3{.x = -aRectangle.mWidth, .y = -aRectangle.mLength, .z = 0.0} / 2,
                      common::Vector3{.x = aRectangle.mWidth, .y = -aRectangle.mLength, .z = 0.0} / 2,
                      common::Vector3{.x = aRectangle.mWidth, .y = aRectangle.mLength, .z = 0.0} / 2,
                      common::Vector3{.x = -aRectangle.mWidth, .y = aRectangle.mLength, .z = 0.0} / 2};
}
}  // namespace plato::third_party_integration::stk_io::test_utilities
