#ifndef PLATO_THIRDPARTYINTEGRATION_STKIO_SENSITIVITYTESTUTILITIES
#define PLATO_THIRDPARTYINTEGRATION_STKIO_SENSITIVITYTESTUTILITIES

#include <gtest/gtest.h>

#include <array>

#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::third_party_integration::common
{
struct Vector3;
}

namespace plato::third_party_integration::stk_io::test_utilities
{
/// @brief struct for storing dimensions of bounding cuboid in 3D
struct Cuboid
{
    double mLength;
    double mWidth;
    double mHeight;
};

/// @brief struct for storing dimensions of bounding rectangle in 2D
struct Rectangle
{
    double mLength;
    double mWidth;
};

/// @brief Test sensitivity values stored in a container with common::Vector3 entries against gold
template <typename ComputedContainer, typename ExpectedContainer>
void test_nodal_sensitivities_with_expected(const ComputedContainer& aComputedSensitivities,
                                            const ExpectedContainer& aExpectedSensitivities,
                                            const plato::test_utilities::TestContext& aTestContext);

/// @brief Computes an array of sensitivities for a single hex aligned with the coordinate axes
/// @post The nodal sensitivities are returned for nodes ordered as follows:
//                           (0,0,0), (1,0,0), (1,1,0), (0,1,0)
//                           (0,0,1), (1,0,1), (1,1,1), (0,1,1)
auto axes_aligned_hex_volume_sensitivities(const Cuboid& aCuboid) -> std::array<common::Vector3, 8>;

/// @brief Computes an array of sensitivities for a single tet aligned with the coordinate axes
/// @post The nodal sensitivities are returned for nodes ordered as follows:
//                           (0,0,0), (1,0,0), (0,1,0), (0,0,1)
auto axes_aligned_tet_volume_sensitivities(const Cuboid& aCuboid) -> std::array<common::Vector3, 4>;

/// @brief Computes an array of sensitivities for a single tri aligned with the coordinate axes
/// @post The nodal sensitivities are returned for nodes ordered as follows:
//                           (0,0,0), (1,0,0), (0,1,0)
auto axes_aligned_tri_volume_sensitivities(const Rectangle& aRectangle) -> std::array<common::Vector3, 3>;

/// @brief Computes an array of sensitivities for a single quad aligned with the coordinate axes
/// @post The nodal sensitivities are returned for nodes ordered as follows:
//                           (0,0,0), (1,0,0), (1,1,0), (0,1,0)
auto axes_aligned_quad_volume_sensitivities(const Rectangle& aRectangle) -> std::array<common::Vector3, 4>;

template <typename ComputedContainer, typename ExpectedContainer>
void test_nodal_sensitivities_with_expected(const ComputedContainer& aComputedSensitivities,
                                            const ExpectedContainer& aExpectedSensitivities,
                                            const plato::test_utilities::TestContext& aTestContext)
{
    ASSERT_EQ(aComputedSensitivities.size(), aExpectedSensitivities.size());
    for (const auto& [tComputed, tExpected] : utilities::Zip{aComputedSensitivities, aExpectedSensitivities})
    {
        common::test_utilities::test_double_equality_of_components(tComputed, tExpected, aTestContext);
    }
}
}  // namespace plato::third_party_integration::stk_io::test_utilities

#endif
