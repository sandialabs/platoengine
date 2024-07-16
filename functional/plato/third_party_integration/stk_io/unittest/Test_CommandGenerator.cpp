#include <gtest/gtest.h>

#include <optional>
#include <string_view>

#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"

namespace plato::third_party_integration::stk_io::unittest
{

TEST(CommandGenerator, Default)
{
    const CommandGenerator tCommandGenerator;
    constexpr std::string_view tGold{"generated:1x1x1|bbox:0,0,0,1,1,1"};
    EXPECT_EQ(tCommandGenerator.toString(), tGold);
    EXPECT_DOUBLE_EQ(tCommandGenerator.volume(), 1.0);
    EXPECT_EQ(tCommandGenerator.numberOfElements(), 1u);
    EXPECT_EQ(tCommandGenerator.numberOfNodes(), 8u);
}

TEST(CommandGenerator, CustomHexBox)
{
    const CommandBounds tLowerBounds{-1, -2, -3};
    const CommandBounds tUpperBounds{1, 2, 3};
    const CommandNumberOfElements tElements{3, 4, 1};
    const auto tCommandGenerator = CommandGenerator{tElements, tLowerBounds, tUpperBounds, CommandElementType::Hex};
    constexpr std::string_view tGold{"generated:3x4x1|bbox:-1,-2,-3,1,2,3"};
    EXPECT_EQ(tCommandGenerator.toString(), tGold);
    EXPECT_DOUBLE_EQ(tCommandGenerator.volume(), 2 * 4 * 6);
    EXPECT_EQ(tCommandGenerator.numberOfElements(), 3u * 4u * 1u);
    EXPECT_EQ(tCommandGenerator.numberOfNodes(), 40u);
}

TEST(CommandGenerator, CustomTetBox)
{
    const CommandBounds tLowerBounds{0, 0, 0};
    const CommandBounds tUpperBounds{4, 4, 4};
    const CommandNumberOfElements tElements{2, 2, 2};
    const auto tCommandGenerator = CommandGenerator{tElements, tLowerBounds, tUpperBounds, CommandElementType::Tet};
    constexpr std::string_view tGold{"generated:2x2x2|bbox:0,0,0,4,4,4|tets"};
    EXPECT_EQ(tCommandGenerator.toString(), tGold);
    EXPECT_DOUBLE_EQ(tCommandGenerator.volume(), 4 * 4 * 4);
    EXPECT_EQ(tCommandGenerator.numberOfElements(), 2u * 2u * 2u * 6u);
    EXPECT_EQ(tCommandGenerator.numberOfNodes(), 27u);
}

TEST(CommandGenerator, CustomHexWithPrecision)
{
    const CommandBounds tLowerBounds{0.12345678901234567, 0.12345678901234567, 0.12345678901234567};
    const CommandBounds tUpperBounds{4.12345678901234567, 4.12345678901234567, 4.12345678901234567};
    const CommandNumberOfElements tElements{2, 2, 2};
    const auto tCommandGenerator =
        CommandGenerator{tElements, tLowerBounds, tUpperBounds, CommandElementType::Tet, {}, {}, 16};
    constexpr std::string_view tGold{
        "generated:2x2x2|bbox:0.1234567890123457,0.1234567890123457,0.1234567890123457,4.123456789012345,4."
        "123456789012345,4.123456789012345|tets"};
    EXPECT_EQ(tCommandGenerator.toString(), tGold);
    EXPECT_DOUBLE_EQ(tCommandGenerator.volume(), 4 * 4 * 4);
    EXPECT_EQ(tCommandGenerator.numberOfElements(), 2u * 2u * 2u * 6u);
    EXPECT_EQ(tCommandGenerator.numberOfNodes(), 27u);
}

TEST(CommandGenerator, SideSets)
{
    const CommandBounds tLowerBounds{0, 0, 0};
    const CommandBounds tUpperBounds{1, 1, 1};
    const CommandNumberOfElements tElements{1, 1, 1};

    const auto tCommandGenerator = CommandGenerator{
        tElements,
        tLowerBounds,
        tUpperBounds,
        CommandElementType::Hex,
        {},
        {UseLowerX{true}, UseUpperX{true}, UseLowerY{true}, UseUpperY{true}, UseLowerZ{true}, UseUpperZ{true}}};
    constexpr std::string_view tGold{"generated:1x1x1|bbox:0,0,0,1,1,1|sideset:xXyYzZ"};
    EXPECT_EQ(tCommandGenerator.toString(), tGold);
}

TEST(CommandGenerator, NodeSets)
{
    const CommandBounds tLowerBounds{0, 0, 0};
    const CommandBounds tUpperBounds{1, 1, 1};
    const CommandNumberOfElements tElements{1, 1, 1};
    const auto tCommandGenerator = CommandGenerator{
        tElements,
        tLowerBounds,
        tUpperBounds,
        CommandElementType::Hex,
        {UseLowerX{true}, UseUpperX{true}, UseLowerY{true}, UseUpperY{true}, UseLowerZ{true}, UseUpperZ{true}},
        {}};

    constexpr std::string_view tGold{"generated:1x1x1|bbox:0,0,0,1,1,1|nodeset:xXyYzZ"};
    EXPECT_EQ(tCommandGenerator.toString(), tGold);
}

TEST(CommandGeneratorDetail, xXyYzZ)
{
    {
        NodeSetSideSetIdentifiers tNodeSetSideSetIdentifiers;
        constexpr std::string_view tGold{""};
        EXPECT_EQ(detail::xyz_boundary_string(tNodeSetSideSetIdentifiers), tGold);
    }
    {
        NodeSetSideSetIdentifiers tNodeSetSideSetIdentifiers;
        tNodeSetSideSetIdentifiers.mLowerX.mValue = true;

        constexpr std::string_view tGold{"x"};
        EXPECT_EQ(detail::xyz_boundary_string(tNodeSetSideSetIdentifiers), tGold);
    }
    {
        NodeSetSideSetIdentifiers tNodeSetSideSetIdentifiers;
        tNodeSetSideSetIdentifiers.mLowerX.mValue = true;
        tNodeSetSideSetIdentifiers.mUpperX.mValue = true;

        constexpr std::string_view tGold{"xX"};
        EXPECT_EQ(detail::xyz_boundary_string(tNodeSetSideSetIdentifiers), tGold);
    }
    {
        NodeSetSideSetIdentifiers tNodeSetSideSetIdentifiers{UseLowerX{false}, UseUpperX{false}, UseLowerY{true},
                                                             UseUpperY{false}, UseLowerZ{false}, UseUpperZ{true}};
        constexpr std::string_view tGold{"yZ"};
        EXPECT_EQ(detail::xyz_boundary_string(tNodeSetSideSetIdentifiers), tGold);
    }
    {
        NodeSetSideSetIdentifiers tNodeSetSideSetIdentifiers{UseLowerX{true}, UseUpperX{true}, UseLowerY{true},
                                                             UseUpperY{true}, UseLowerZ{true}, UseUpperZ{true}};
        constexpr std::string_view tGold{"xXyYzZ"};
        EXPECT_EQ(detail::xyz_boundary_string(tNodeSetSideSetIdentifiers), tGold);
    }
    {
        NodeSetSideSetIdentifiers tNodeSetSideSetIdentifiers{UseLowerX{true},  UseUpperX{false}, UseLowerY{true},
                                                             UseUpperY{false}, UseLowerZ{true},  UseUpperZ{false}};
        constexpr std::string_view tGold{"xyz"};
        EXPECT_EQ(detail::xyz_boundary_string(tNodeSetSideSetIdentifiers), tGold);
    }
}

}  // namespace plato::third_party_integration::stk_io::unittest
