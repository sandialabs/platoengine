#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "plato/utilities/LevenshteinDistance.hpp"

namespace plato::utilities::unittest
{
namespace
{
struct WarcraftUnit
{
    std::string mName;
    int mGoldCost;
    auto operator==(const WarcraftUnit&) const -> bool = default;
};
}  // namespace

TEST(SortByLevenshteinDistance, SortsVectorOfStringsUsingIdentityAccessor)
{
    const auto tSortedNames =
        levenshtein_distance_sorted_container(std::vector<std::string>{"catapult", "grunt", "ogre", "peon", "Ogre"},
                                              "grnt", [](const auto& aName) { return std::string_view{aName}; });
    const auto tSortedGold = std::vector<std::string>{"grunt", "Ogre", "ogre", "peon", "catapult"};
    EXPECT_EQ(tSortedGold, tSortedNames);
}

TEST(SortByLevenshteinDistance, SortsVectorOfStructsUsingStructMemberAccessor)
{
    const auto tSortedUnits = levenshtein_distance_sorted_container(
        std::vector<WarcraftUnit>{{.mName = "death_knight", .mGoldCost = 1200},
                                  {.mName = "grunt", .mGoldCost = 600},
                                  {.mName = "dragon", .mGoldCost = 2500},
                                  {.mName = "troll_axethrower", .mGoldCost = 500}},
        "dragn", [](const auto& aUnit) { return std::string_view{aUnit.mName}; });

    const auto tGold = std::vector<WarcraftUnit>{{.mName = "dragon", .mGoldCost = 2500},
                                                 {.mName = "grunt", .mGoldCost = 600},
                                                 {.mName = "death_knight", .mGoldCost = 1200},
                                                 {.mName = "troll_axethrower", .mGoldCost = 500}};
    EXPECT_EQ(tSortedUnits, tGold);
}

TEST(LevenshteinDistanceDetail, EmptyStringsHaveZeroDistance)
{
    EXPECT_EQ(detail::levenshtein_distance("", ""), std::size_t{0});
}

TEST(LevenshteinDistanceDetail, EmptyStringDistanceIsOtherStringLength)
{
    EXPECT_EQ(detail::levenshtein_distance("", "orc"), std::size_t{3});
    EXPECT_EQ(detail::levenshtein_distance("grunt", ""), std::size_t{5});
}

TEST(LevenshteinDistanceDetail, IdenticalStringsHaveZeroDistance)
{
    EXPECT_EQ(detail::levenshtein_distance("level_set_bounds", "level_set_bounds"), std::size_t{0});
    EXPECT_EQ(detail::levenshtein_distance("zug_zug", "Zug_Zug"), std::size_t{0});
}

TEST(LevenshteinDistanceDetail, SingleInsertion)
{
    EXPECT_EQ(detail::levenshtein_distance("meshname", "mesh_name"), std::size_t{1});
    EXPECT_EQ(detail::levenshtein_distance("orc", "orcs"), std::size_t{1});
}

TEST(LevenshteinDistanceDetail, SingleDeletion)
{
    EXPECT_EQ(detail::levenshtein_distance("mesh_name", "meshname"), std::size_t{1});
    EXPECT_EQ(detail::levenshtein_distance("ogres", "ogre"), std::size_t{1});
}

TEST(LevenshteinDistanceDetail, SingleSubstitution)
{
    EXPECT_EQ(detail::levenshtein_distance("level_set_boundZ", "level_set_bounds"), std::size_t{1});
    EXPECT_EQ(detail::levenshtein_distance("grunt", "grOnt"), std::size_t{1});
}

TEST(LevenshteinDistanceDetail, MultipleEdits)
{
    EXPECT_EQ(detail::levenshtein_distance("kitten", "sitting"), std::size_t{3});
    EXPECT_EQ(detail::levenshtein_distance("saturday", "Sunday"), std::size_t{3});
    EXPECT_EQ(detail::levenshtein_distance("sphere_pattern", "sphere_list"), std::size_t{6});
}

TEST(LevenshteinDistanceDetail, DistanceIsSymmetric)
{
    const auto tLeVelSetBoundZ = std::string_view{"LeVelSetBoundZ"};
    const auto tLevelSetBounds = std::string_view{"level_set_bounds"};

    EXPECT_EQ(detail::levenshtein_distance(tLeVelSetBoundZ, tLevelSetBounds),
              detail::levenshtein_distance(tLevelSetBounds, tLeVelSetBoundZ));
}

TEST(LevenshteinDistanceDetail, HandlesStringsOfDifferentLengths)
{
    EXPECT_EQ(detail::levenshtein_distance("a", "abcdef"), std::size_t{5});
    EXPECT_EQ(detail::levenshtein_distance("abcdef", "a"), std::size_t{5});
}

TEST(LevenshteinDistanceDetail, HandlesLongerParserKeywordLikeStrings)
{
    EXPECT_EQ(detail::levenshtein_distance("max_edge_length_percentage_for_snaping",
                                           "max_edge_length_percentage_for_snapping"),
              std::size_t{1});

    EXPECT_EQ(detail::levenshtein_distance("initial_feild_name", "initial_field_name"), std::size_t{2});
}

}  // namespace plato::utilities::unittest
