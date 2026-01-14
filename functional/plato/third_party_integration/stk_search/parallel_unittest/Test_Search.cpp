#include <gtest/gtest.h>

#include <boost/mpi/collectives.hpp>
#include <boost/mpi/communicator.hpp>

#include "plato/third_party_integration/common/test_utilities/CoordinateTestUtilities.hpp"
#include "plato/third_party_integration/stk_search/Utilities.hpp"
#include "plato/utilities/RankSplitVector.hpp"

namespace plato::third_party_integration::stk_search::parallel_unittest
{

namespace
{

constexpr auto kNumRanks = int{4};
constexpr double kSearchRadius = 3.0;
[[nodiscard]] auto create_search_points_and_spheres_with_ids()
    -> std::pair<std::vector<SearchPointWithIdentifier>, std::vector<SearchSphereWithIdentifier>>
{
    constexpr unsigned int tLocalSize = 4;
    const auto tComm = boost::mpi::communicator{};
    const auto tRank = tComm.rank();

    std::vector<SearchPointWithIdentifier> tSearchPoints;
    tSearchPoints.reserve(tLocalSize);

    std::vector<SearchSphereWithIdentifier> tSearchSphereCenters;
    tSearchSphereCenters.reserve(tLocalSize);

    for (unsigned int tIndex = 0; tIndex < tLocalSize; ++tIndex)
    {
        Identifier tIdentifier{static_cast<int>(tLocalSize * tRank + tIndex), tRank};
        const double tX = tRank;
        const double tY = tIndex;
        const double tZ = 0;

        SearchPointWithIdentifier tPoint{convert_coordinate(common::Coordinate{tX, tY, tZ}), tIdentifier};
        tSearchPoints.push_back(tPoint);

        const auto tSphere = SearchSphereWithIdentifier{
            create_sphere(common::Coordinate{tX, tY, tZ + 0.4}, STKRadius{kSearchRadius}), tIdentifier};
        tSearchSphereCenters.push_back(tSphere);
    }
    return {tSearchPoints, tSearchSphereCenters};
}

[[nodiscard]] auto create_parallel_consistency_search_points_and_spheres_with_ids(const int tWorldRank)
    -> std::pair<std::vector<SearchPointWithIdentifier>, std::vector<SearchSphereWithIdentifier>>
{
    constexpr unsigned int tFullSize = 1037;

    std::vector<SearchPointWithIdentifier> tSearchPoints;
    tSearchPoints.reserve(tFullSize);

    std::vector<SearchSphereWithIdentifier> tSearchSphereCenters;
    tSearchSphereCenters.reserve(tFullSize);

    unsigned int tStartIndex = 0;
    unsigned int tStep = 1;
    if (tWorldRank > 0)
    {
        tStartIndex = tWorldRank - 1;
        tStep = 3;
    }

    for (unsigned int tIndex = tStartIndex; tIndex < tFullSize; tIndex = tIndex + tStep)
    {
        Identifier tIdentifier{static_cast<int>(tIndex), tWorldRank};
        const double tX = tIndex % 10;
        const double tY = std::ceil(tIndex / 10);
        const double tZ = std::ceil(tIndex / 100);

        SearchPointWithIdentifier tPoint{convert_coordinate(common::Coordinate{tX, tY, tZ}), tIdentifier};

        tSearchPoints.push_back(tPoint);
        const auto tSphere = SearchSphereWithIdentifier{
            create_sphere(common::Coordinate{tX + 0.5, tY, tZ}, STKRadius{kSearchRadius}), tIdentifier};
        tSearchSphereCenters.push_back(tSphere);
    }

    return {tSearchPoints, tSearchSphereCenters};
}

boost::mpi::communicator split_comm_world()
{
    auto tWorldComm = boost::mpi::communicator{};
    const auto tRank = tWorldComm.rank();
    const auto tColor = utilities::rank_group_color({1, 3}, utilities::RankNamedType{tRank});
    return tWorldComm.split(tColor.mValue);
}

void verify_first_entry_belongs_to_this_rank(const SearchResults& aSearchResults,
                                             const boost::mpi::communicator& aCommunicator)
{
    for (const auto& x : aSearchResults)
    {
        EXPECT_EQ(x.first.proc(), aCommunicator.rank());
    }
}

}  // namespace

TEST(StkSearch, MPISize)
{
    const auto tComm = boost::mpi::communicator{};
    EXPECT_EQ(tComm.size(), kNumRanks);
}

TEST(StkSearch, Search)
{
    const auto tComm = boost::mpi::communicator{};
    const auto [tLocalSearchPoints, tLocalSearchSphereCenters] = create_search_points_and_spheres_with_ids();
    const auto tLocalSearchResults = stk_search(tLocalSearchSphereCenters, tLocalSearchPoints, tComm);
    verify_first_entry_belongs_to_this_rank(tLocalSearchResults, tComm);
}

TEST(StkSearch, ParallelConsistency)
{
    const auto tWorldComm = boost::mpi::communicator{};

    const auto [tLocalSearchPoints, tLocalSearchSphereCenters] =
        create_parallel_consistency_search_points_and_spheres_with_ids(tWorldComm.rank());

    const auto tSplitComm = split_comm_world();

    const auto tLocalSearchResults = stk_search(tLocalSearchSphereCenters, tLocalSearchPoints, tSplitComm);
    unsigned int tRankZeroValue = 0;
    unsigned int tOtherRanksSumValue = tLocalSearchResults.size();
    if (tWorldComm.rank() == 0)
    {
        tRankZeroValue = tLocalSearchResults.size();
    }
    else
    {
        boost::mpi::all_reduce(tSplitComm, boost::mpi::inplace(tOtherRanksSumValue), std::plus<unsigned int>());
    }
    boost::mpi::broadcast(tWorldComm, tRankZeroValue, 0);
    EXPECT_EQ(tOtherRanksSumValue, tRankZeroValue);
    verify_first_entry_belongs_to_this_rank(tLocalSearchResults, tWorldComm);
}

}  // namespace plato::third_party_integration::stk_search::parallel_unittest
