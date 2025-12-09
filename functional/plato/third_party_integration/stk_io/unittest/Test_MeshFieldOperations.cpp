#include <gtest/gtest.h>

#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/MeshFieldOperations.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/utilities/Enumerate.hpp"

namespace plato::third_party_integration::stk_io::unittest
{
TEST(MeshFieldOperations, GlobalToLocalNodeIndex)
{
    const auto tIDs = std::vector<std::size_t>{0, 10, 11, 13, 42};
    for (const auto [tExpected, tGlobalID] : utilities::enumerate(tIDs))
    {
        EXPECT_EQ(tExpected, detail::global_to_local_node_index(tIDs, tGlobalID));
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
                                                     .mType = CommandElementType::Tet};

    const auto tMesh = generate_bulk_data(tMeshGenerator);
    ASSERT_TRUE(tMesh);

    constexpr auto tNodalValue = 1.0;
    const auto tField = std::vector(tMeshGenerator.numberOfNodes(), tNodalValue);
    const auto tNodalAverage = nodal_average(tField, *tMesh);

    const auto tExpected = std::vector(tMeshGenerator.numberOfElements(), tNodalValue);
    EXPECT_EQ(tExpected, tNodalAverage);
}

TEST(MeshFieldOperations, NodalAverageHexMesh)
{
    constexpr auto tMeshGenerator = CommandGenerator{.mElements = {1, 2, 1},
                                                     .mLowerBounds = {0.0, 0.0, 0.0},
                                                     .mUpperBounds = {2.0, 2.0, 2.0},
                                                     .mType = CommandElementType::Hex};

    const auto tMesh = generate_bulk_data(tMeshGenerator);
    ASSERT_TRUE(tMesh);

    const auto tNodalCoordinates = nodal_coordinates(*tMesh);
    auto tYCoordinate =
        tNodalCoordinates | std::views::transform([](const auto& aCoordinate) { return aCoordinate.y; });

    const auto tNodalAverage = nodal_average(tYCoordinate, *tMesh);
    const auto tExpected = std::vector{0.5, 1.5};
    EXPECT_EQ(tNodalAverage, tExpected);
}
}  // namespace plato::third_party_integration::stk_io::unittest
