#include <gtest/gtest.h>

#include <cstddef>
#include <functional>
#include <ranges>
#include <vector>

#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/MeshFieldOperations.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"
#include "plato/utilities/Enumerate.hpp"
#include "stk_topology/topology.hpp"

namespace plato::third_party_integration::stk_io::unittest
{
namespace
{
using MeshFieldOperationsTwoDThreeBlock = test_utilities::TwoDThreeBlockMesh;
}

TEST(MeshFieldOperations, GlobalToLocalIndex)
{
    const auto tIDs = std::vector<std::size_t>{0, 10, 11, 13, 42};
    for (const auto [tExpected, tGlobalID] : utilities::enumerate(tIDs))
    {
        EXPECT_EQ(tExpected, detail::global_to_local_index(tIDs, tGlobalID));
    }
}

TEST(MeshFieldOperations, SingleElementNodalAverage)
{
    constexpr auto tMeshGenerator =
        CommandGenerator{.mElements = {1, 1, 1}, .mLowerBounds = {0.0, 0.0, 0.0}, .mUpperBounds = {1.0, 1.0, 1.0}};

    const auto tMesh = generate_bulk_data(tMeshGenerator);
    ASSERT_TRUE(tMesh);

    ASSERT_EQ(tMeshGenerator.numberOfNodes(), 8U);
    const auto tField = std::vector{0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
    const auto tGlobalNodeIDs = node_ids(*tMesh, tMesh->mesh_meta_data().universal_part());
    const auto tNodalAverageField = detail::single_element_nodal_average(
        tMesh->begin_entities(stk::topology::ELEMENT_RANK)->second, tField, tGlobalNodeIDs, *tMesh);

    EXPECT_EQ(tNodalAverageField, 3.5);
}

TEST(MeshFieldOperations, NodalAverageTetMesh)
{
    constexpr auto tMeshGenerator = CommandGenerator{.mElements = {1, 1, 1},
                                                     .mLowerBounds = {0.0, 0.0, 0.0},
                                                     .mUpperBounds = {1.0, 1.0, 1.0},
                                                     .mType = CommandElementType::kTet};

    const auto tMesh = generate_bulk_data(tMeshGenerator);
    ASSERT_TRUE(tMesh);

    constexpr auto tNodalValue = 1.0;
    const auto tField = std::vector(tMeshGenerator.numberOfNodes(), tNodalValue);
    const auto tNodalAverage = element_averaged_nodal_values(tField, *tMesh);

    const auto tExpected = std::vector(tMeshGenerator.numberOfElements(), tNodalValue);
    EXPECT_EQ(tExpected, tNodalAverage);
}

TEST(MeshFieldOperations, NodalAverageHexMesh)
{
    constexpr auto tMeshGenerator = CommandGenerator{.mElements = {1, 2, 1},
                                                     .mLowerBounds = {0.0, 0.0, 0.0},
                                                     .mUpperBounds = {2.0, 2.0, 2.0},
                                                     .mType = CommandElementType::kHex};

    const auto tMesh = generate_bulk_data(tMeshGenerator);
    ASSERT_TRUE(tMesh);

    const auto tNodalCoordinates = nodal_coordinates(*tMesh);
    auto tYCoordinate =
        tNodalCoordinates | std::views::transform([](const auto& aCoordinate) { return aCoordinate.y; });

    const auto tNodalAverage = element_averaged_nodal_values(tYCoordinate, *tMesh);
    const auto tExpected = std::vector{0.5, 1.5};
    EXPECT_EQ(tNodalAverage, tExpected);
}

TEST_F(MeshFieldOperationsTwoDThreeBlock, NodalAverage)
{
    const auto tBulkData = read_mesh_bulk_data(mMeshFilePath);

    const auto tNodalCoordinates = nodal_coordinates(*tBulkData);
    auto tXCoordinate = tNodalCoordinates | std::views::transform(
                                                [](const auto& aCoordinate)
                                                {
                                                    constexpr auto tArbitraryMultiplier = 3.0;
                                                    return tArbitraryMultiplier * aCoordinate.x;
                                                });

    const auto tNodalAverage = element_averaged_nodal_values(tXCoordinate, *tBulkData);
    // Computed by hand based on connectivity
    // This mesh also has a non-trivial element map, and so the order is based on sorted element global id
    const auto tExpected = std::vector{-2.0, -4.0, 3.0, 1.0, 3.0, 5.0, 3.0};
    EXPECT_EQ(tNodalAverage, tExpected);
}

TEST(MeshFieldOperations, NodalAverageElementProjectionHexMesh)
{
    constexpr auto tMeshGenerator = CommandGenerator{.mElements = {1, 2, 1},
                                                     .mLowerBounds = {0.0, 0.0, 0.0},
                                                     .mUpperBounds = {2.0, 2.0, 2.0},
                                                     .mType = CommandElementType::kHex};

    const auto tMesh = generate_bulk_data(tMeshGenerator);
    ASSERT_TRUE(tMesh);

    constexpr auto tElementValue1 = 1.0;
    constexpr auto tElementValue2 = 2.0;
    const auto tElementValues = std::vector{tElementValue1, tElementValue2};
    ASSERT_EQ(tElementValues.size(), tMeshGenerator.numberOfElements());

    const auto tElementProjection = nodal_average_element_projection(tElementValues, *tMesh);
    // Computed by hand based on the mesh connectivity
    constexpr auto tNumberOfNodesPerElement = 8.0;
    constexpr auto tXEquals0Expected = tElementValue1 / tNumberOfNodesPerElement;
    constexpr auto tXEquals1Expected = (tElementValue1 + tElementValue2) / tNumberOfNodesPerElement;
    constexpr auto tXEquals2Expected = tElementValue2 / tNumberOfNodesPerElement;
    const auto tExpected = std::vector{tXEquals0Expected, tXEquals0Expected, tXEquals1Expected, tXEquals1Expected,
                                       tXEquals2Expected, tXEquals2Expected, tXEquals0Expected, tXEquals0Expected,
                                       tXEquals1Expected, tXEquals1Expected, tXEquals2Expected, tXEquals2Expected};
    EXPECT_EQ(tElementProjection, tExpected);
}

TEST_F(MeshFieldOperationsTwoDThreeBlock, NodalAverageElementProjection)
{
    const auto tBulkData = read_mesh_bulk_data(mMeshFilePath);

    auto tIota = std::views::iota(1U, mExpectedNumberOfElements + 1U) |
                 std::views::transform(
                     [](const auto aIndex)
                     {
                         constexpr auto tArbitraryMultiplier = 3.0;
                         return tArbitraryMultiplier * static_cast<double>(aIndex);
                     });
    const auto tElementValues = std::vector(tIota.begin(), tIota.end());

    const auto tNodalProjection = nodal_average_element_projection(tElementValues, *tBulkData);
    // Computed by hand based on connectivity
    const auto tExpected = std::vector{3.0, 14.25, 5.25, 2.0, 15.25, 2.25, 9.0, 11.0, 22.0};
    EXPECT_EQ(tNodalProjection, tExpected);
}

TEST_F(MeshFieldOperationsTwoDThreeBlock, NodalAverageElementProjectionOnPart)
{
    const auto tBulkData = read_mesh_bulk_data(mMeshFilePath);
    auto* const tPart = tBulkData->mesh_meta_data().get_mesh_parts().front();
    const auto tPartReferenceVector = PartReferenceVector{{std::cref(*tPart)}};
    const auto tElementValues = std::vector{3.0, 6.0, 9.0, 12.0};

    const auto tNodalProjection = nodal_average_element_projection(tElementValues, *tBulkData, tPartReferenceVector);
    // Block 1 is 4 tris aligned with cardinal directions W,S,E,N that make a rectangle. The nodes in order go: NW, NE,
    // SW, SE, Center. Nodal average element projection divides the element value by the number of nodes (3) and
    // accumulates the element contributions to that node.
    const auto tExpected = std::vector{5.0, 7.0, 3.0, 5.0, 10.0};
    EXPECT_EQ(tNodalProjection, tExpected);
}

TEST_F(MeshFieldOperationsTwoDThreeBlock, ElementAveragedNodalValues)
{
    const auto tBulkData = read_mesh_bulk_data(mMeshFilePath);
    auto* const tPart = tBulkData->mesh_meta_data().get_mesh_parts().front();
    const auto tPartReferenceVector = PartReferenceVector{{std::cref(*tPart)}};
    const auto tNodalValues = std::vector{3.0, 6.0, 9.0, 12.0, 15.0};

    const auto tElementAveragedNodalValues =
        element_averaged_nodal_values(tNodalValues, *tBulkData, tPartReferenceVector);
    // Block 1 is 4 tris aligned with cardinal directions W,S,E,N that make a rectangle. The nodes in order go: NW, NE,
    // SW, SE, Center.
    const auto tExpected = std::vector{9.0, 12.0, 11.0, 8.0};
    EXPECT_EQ(tElementAveragedNodalValues, tExpected);
}
}  // namespace plato::third_party_integration::stk_io::unittest
