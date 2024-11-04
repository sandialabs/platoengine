#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <optional>

#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_search/Utilities.hpp"
#include "plato/utilities/DataFilePath.hpp"

namespace plato::third_party_integration::stk_search::unittest
{

TEST(STKSearchUtilities, NodalIdentifiers)
{
    const std::vector<common::Coordinate> tNodalCoordinates{{0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {3, 0, 0}, {4, 0, 0}};

    std::vector<SearchPointWithIdentifier> tNodalCoordsWithIdentifiers =
        search_points_with_identifiers(tNodalCoordinates);

    ASSERT_EQ(tNodalCoordinates.size(), tNodalCoordsWithIdentifiers.size());
    for (unsigned int tIndex = 0; tIndex < tNodalCoordinates.size(); ++tIndex)
    {
        const common::Coordinate tResult = convert_search_point(tNodalCoordsWithIdentifiers[tIndex].first);
        common::test_utilities::test_double_equality_of_components(tResult, tNodalCoordinates[tIndex],
                                                                   TEST_CONTEXT("Checking search point is node"));
        EXPECT_EQ(tNodalCoordsWithIdentifiers[tIndex].second.id(), tIndex);
    }
}

TEST(STKSearchUtilities, IsInSearchResults)
{
    const std::vector<common::Coordinate> tNodalCoordinates{{0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {3, 0, 0}, {4, 0, 0}};
    std::vector<SearchPointWithIdentifier> tNodalCoordsWithIdentifiers =
        search_points_with_identifiers(tNodalCoordinates);

    constexpr common::Coordinate tCenter{0, 0, 0};
    constexpr double tSearchRadius{2};
    const auto tSphere = create_sphere(tCenter, STKRadius{tSearchRadius});
    const auto tIdentifier = Identifier{0, 0};
    const auto tSphereWithID = SearchSphereWithIdentifier{tSphere, tIdentifier};
    const auto tSearchResults =
        perform_stk_search({tSphereWithID}, tNodalCoordsWithIdentifiers, boost::mpi::communicator{});

    EXPECT_TRUE(is_in_search_results(tNodalCoordsWithIdentifiers[0].second, tSearchResults));
    EXPECT_TRUE(is_in_search_results(tNodalCoordsWithIdentifiers[1].second, tSearchResults));
    EXPECT_TRUE(is_in_search_results(tNodalCoordsWithIdentifiers[2].second, tSearchResults));
    EXPECT_FALSE(is_in_search_results(tNodalCoordsWithIdentifiers[3].second, tSearchResults));
    EXPECT_FALSE(is_in_search_results(tNodalCoordsWithIdentifiers[4].second, tSearchResults));
}

namespace
{
auto create_example_search_points_with_ids(const int aRank) -> std::vector<SearchPointWithIdentifier>
{
    return std::vector<SearchPointWithIdentifier>{SearchPointWithIdentifier{{1, 0, 0}, Identifier{0, aRank}},
                                                  SearchPointWithIdentifier{{2, 0, 0}, Identifier{1, aRank}},
                                                  SearchPointWithIdentifier{{2, 2, 2}, Identifier{2, aRank}},
                                                  SearchPointWithIdentifier{{0, 0, 0}, Identifier{3, aRank}}};
}
}  // namespace

TEST(STKSearchUtilities, FindPointsInSphere)
{
    const boost::mpi::communicator tCommunicator{};
    constexpr common::Coordinate tCenter{1, 0, 0};
    constexpr double tSearchRadius{2.1};
    const auto tThisRank = tCommunicator.rank();
    const auto tSphere = create_sphere(tCenter, STKRadius{tSearchRadius});
    const auto tIdentifier = Identifier{0, tThisRank};
    const auto tSphereWithID = SearchSphereWithIdentifier{tSphere, tIdentifier};

    const auto tLocalSearchPointWithIdentifiers = create_example_search_points_with_ids(tThisRank);
    const auto tSearchResults =
        perform_stk_search({tSphereWithID}, tLocalSearchPointWithIdentifiers, boost::mpi::communicator{});

    EXPECT_EQ(tSearchResults.size(), 3u);
}

TEST(STKSearchUtilities, FindPointsInMultipleSpheres)
{
    const boost::mpi::communicator tCommunicator{};
    constexpr common::Coordinate tCenterOne{1, 0, 0};
    constexpr common::Coordinate tCenterTwo{2, 2, 2};
    constexpr double tSearchRadius{2.1};
    const auto tThisRank = tCommunicator.rank();
    const auto tSphereOne = create_sphere(tCenterOne, STKRadius{tSearchRadius});
    const auto tIdentifierOne = Identifier{0, tThisRank};
    const auto tSphereWithIDOne = SearchSphereWithIdentifier{tSphereOne, tIdentifierOne};
    const auto tSphereTwo = create_sphere(tCenterTwo, STKRadius{tSearchRadius});
    const auto tIdentifierTwo = Identifier{1, tThisRank};
    const auto tSphereWithIDTwo = SearchSphereWithIdentifier{tSphereTwo, tIdentifierTwo};

    const auto tLocalSearchPointWithIdentifiers = create_example_search_points_with_ids(tThisRank);

    const auto tSearchResults =
        perform_stk_search({tSphereWithIDOne, tSphereWithIDTwo}, tLocalSearchPointWithIdentifiers, tCommunicator);

    EXPECT_EQ(tSearchResults.size(), 4u);
}

}  // namespace plato::third_party_integration::stk_search::unittest
