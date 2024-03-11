#include <gtest/gtest.h>

#include "plato/criteria/extension/NodalSumObjective.hpp"
#include "plato/utilities/STKUtilities.hpp"

namespace plato::criteria::extension::unittest
{
namespace
{
constexpr std::string_view kBrickFile = "brick.exo";
constexpr std::string_view kOneBlockCommand = "generated:1x1x1|bbox:0,0,0,1,1,1";
}  // namespace

TEST(NodalSumObjective, Value111)
{
    namespace pfu = plato::utilities;
    constexpr std::string_view tFileName = "test.exo";
    pfu::write_mesh(tFileName, pfu::create_mesh("generated:1x1x1|bbox:0,0,0,1,1,1"));

    constexpr double tExpectedValue = 12.0;
    EXPECT_EQ(NodalSumObjective{}.f(core::MeshProxy{tFileName, {}}), tExpectedValue);
    EXPECT_TRUE(std::filesystem::remove(tFileName));
}

TEST(NodalSumObjective, Value211)
{
    namespace pfu = plato::utilities;
    constexpr std::string_view tFileName = "test.exo";
    pfu::write_mesh(tFileName, pfu::create_mesh("generated:2x1x1|bbox:0,0,0,1,1,1"));

    constexpr double tExpectedValue = 18.0;
    EXPECT_EQ(NodalSumObjective{}.f(core::MeshProxy{tFileName, {}}), tExpectedValue);
    EXPECT_TRUE(std::filesystem::remove(tFileName));
}

TEST(NodalSumObjective, Value0)
{
    namespace pfu = plato::utilities;
    constexpr std::string_view tFileName = "test.exo";
    pfu::write_mesh(tFileName, pfu::create_mesh("generated:1x1x1|bbox:-2,-1,-3,2,1,3"));

    constexpr double tExpectedValue = 0.0;
    EXPECT_EQ(NodalSumObjective{}.f(core::MeshProxy{tFileName, {}}), tExpectedValue);
    EXPECT_TRUE(std::filesystem::remove(tFileName));
}

TEST(NodalSumObjective, Gradient111)
{
    namespace pfu = plato::utilities;
    constexpr std::string_view tFileName = "test.exo";
    auto tBulk = pfu::create_mesh("generated:1x2x3|bbox:0,0,0,1,1,1");
    pfu::write_mesh(tFileName, tBulk);

    const auto tNodalSum = NodalSumObjective{};

    constexpr int tNumCoordsPerNode = 3;
    constexpr int tNumNodes = 24;
    const auto tExpected = std::vector<double>(tNumCoordsPerNode * tNumNodes, 1.0);
    const std::vector tComputed = tNodalSum.df(core::MeshProxy{tFileName, {}}).stdVector();
    EXPECT_EQ(tComputed, tExpected);
    EXPECT_TRUE(std::filesystem::remove(tFileName));
}

TEST(NodalSumObjective, Value)
{
    namespace pfu = plato::utilities;
    pfu::write_mesh(kBrickFile, pfu::create_mesh(kOneBlockCommand));
    core::MeshProxy tMeshProxy{kBrickFile, {}};

    const NodalSumObjective tPass;
    EXPECT_EQ(tPass.f(tMeshProxy), 12);
    EXPECT_TRUE(std::filesystem::exists(kBrickFile));
    EXPECT_TRUE(std::filesystem::remove(kBrickFile));
}

TEST(NodalSumObjective, Gradient)
{
    namespace pfu = plato::utilities;
    pfu::write_mesh(kBrickFile, pfu::create_mesh(kOneBlockCommand));
    core::MeshProxy tMeshProxy{kBrickFile, {}};

    const NodalSumObjective tPass;
    const std::vector<double> tGold(24, 1);
    EXPECT_EQ(tPass.df(tMeshProxy).stdVector(), tGold);
    EXPECT_TRUE(std::filesystem::exists(kBrickFile));
    EXPECT_TRUE(std::filesystem::remove(kBrickFile));
}
}  // namespace plato::criteria::extension::unittest
