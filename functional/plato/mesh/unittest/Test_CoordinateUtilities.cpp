#include <gtest/gtest.h>

#include "plato/mesh/CoordinateUtilities.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/utilities/IndexRange.hpp"
#include "plato/utilities/Zip.hpp"

namespace plato::mesh::unittest
{
TEST(CoordinateUtilities, NodalFieldToDynamicVector)
{
    namespace tpi = third_party_integration;
    const std::vector<tpi::common::Coordinate> tCoordinates{
        {.x = 0, .y = 1, .z = 2},
        {.x = 3, .y = 4, .z = 5},
        {.x = 6, .y = 7, .z = 8},
    };

    constexpr auto tSpatialDimensions{3};
    const auto tDynamicVector = nodal_vector_field_to_dynamic_vector(tCoordinates, tSpatialDimensions);
    for (const auto& [tValue, tGold] : utilities::Zip{tDynamicVector.stdVector(), utilities::IndexRange{8}})
    {
        ASSERT_EQ(tValue, tGold);
    }
}

TEST(CoordinateUtilities, DynamicVectorToNodalCoordinates)
{
    namespace tpi = third_party_integration;
    const auto tDynamicVector = linear_algebra::DynamicVector<double>{{0, 1, 2, 3, 4, 5}};
    const auto tGoldCoordinates =
        std::vector<tpi::common::Coordinate>{{.x = 0, .y = 1, .z = 2}, {.x = 3, .y = 4, .z = 5}};

    constexpr auto tSpatialDimensions{3};
    const auto tCoordinates = dynamic_vector_to_nodal_coordinates(tDynamicVector, tSpatialDimensions);
    ASSERT_EQ(tGoldCoordinates.size(), tCoordinates.size());
    for (const auto& [tCoordinate, tGoldCoordinate] : utilities::Zip{tCoordinates, tGoldCoordinates})
    {
        tpi::common::test_utilities::test_double_equality_of_components(
            tCoordinate, tGoldCoordinate, TEST_CONTEXT("Dynamic vector to vector of coordinates"));
    }
}

TEST(CoordinateUtilities, DynamicVectorRoundTrip)
{
    const auto tDynamicVector = linear_algebra::DynamicVector<double>{{0, 1, 2, 3, 4, 5}};
    constexpr auto tSpatialDimensions{3};
    const auto tCoordinates = dynamic_vector_to_nodal_coordinates(tDynamicVector, tSpatialDimensions);
    for (const auto& [tValue, tGold] :
         utilities::Zip{tDynamicVector.stdVector(),
                        nodal_vector_field_to_dynamic_vector(tCoordinates, tSpatialDimensions).stdVector()})
    {
        ASSERT_EQ(tValue, tGold);
    }
}

TEST(CoordinateUtilities, NodalCoordinatesRoundTrip)
{
    namespace tpi = third_party_integration;
    const std::vector<tpi::common::Coordinate> tCoordinates{
        {.x = 88, .y = 51, .z = 59},
        {.x = 86, .y = 77, .z = 21},
        {.x = 16, .y = 37, .z = 38},
    };
    constexpr auto tSpatialDimensions{3};
    const auto tDynamicVector = nodal_vector_field_to_dynamic_vector(tCoordinates, tSpatialDimensions);
    const auto tReturnCoordinates = dynamic_vector_to_nodal_coordinates(tDynamicVector, tSpatialDimensions);

    ASSERT_EQ(tCoordinates.size(), tReturnCoordinates.size());
    for (const auto& [tCoordinate, tReturnCoordinate] : utilities::Zip{tCoordinates, tReturnCoordinates})
    {
        tpi::common::test_utilities::test_double_equality_of_components(tCoordinate, tReturnCoordinate,
                                                                        TEST_CONTEXT("Nodal coordinates round trip"));
    }
}

}  // namespace plato::mesh::unittest
