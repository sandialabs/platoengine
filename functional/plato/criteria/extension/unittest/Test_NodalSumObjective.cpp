#include <gtest/gtest.h>

#include <string_view>

#include "plato/criteria/extension/NodalSumObjective.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::criteria::extension::unittest
{
namespace
{
constexpr std::string_view kBrickFile = "brick.exo";
constexpr std::string_view kTestFile = "test.exo";
}  // namespace

TEST(NodalSumObjective, Value111)
{
    third_party_integration::stk_io::write_mesh(
        kTestFile, third_party_integration::stk_io::generate_mesh(third_party_integration::stk_io::CommandGenerator{}));

    constexpr double tExpectedValue = 12.0;
    EXPECT_EQ(NodalSumObjective{}.f(core::MeshProxy{kTestFile, {}}), tExpectedValue);
    test_utilities::test_for_existence_and_remove({kTestFile}, TEST_CONTEXT("Checking existence of file"));
}

TEST(NodalSumObjective, Value211)
{
    third_party_integration::stk_io::write_mesh(
        kTestFile,
        third_party_integration::stk_io::generate_mesh(third_party_integration::stk_io::CommandGenerator{{2, 1, 1}}));

    constexpr double tExpectedValue = 18.0;
    EXPECT_EQ(NodalSumObjective{}.f(core::MeshProxy{kTestFile, {}}), tExpectedValue);
    test_utilities::test_for_existence_and_remove({kTestFile}, TEST_CONTEXT("Checking existence of file"));
}

TEST(NodalSumObjective, Value0)
{
    const third_party_integration::stk_io::CommandGenerator tCommandGenerator{
        {1, 1, 1}, {-2, -1, -3}, {2, 1, 3}, third_party_integration::stk_io::CommandElementType::Hex};
    third_party_integration::stk_io::write_mesh(kTestFile,
                                                third_party_integration::stk_io::generate_mesh(tCommandGenerator));

    constexpr double tExpectedValue = 0.0;
    EXPECT_EQ(NodalSumObjective{}.f(core::MeshProxy{kTestFile, {}}), tExpectedValue);
    test_utilities::test_for_existence_and_remove({kTestFile}, TEST_CONTEXT("Checking existence of file"));
}

TEST(NodalSumObjective, Gradient111)
{
    const third_party_integration::stk_io::CommandGenerator tCommandGenerator{
        {1, 2, 3}, {0, 0, 0}, {1, 1, 1}, third_party_integration::stk_io::CommandElementType::Hex};
    auto tBulk = third_party_integration::stk_io::generate_mesh(tCommandGenerator);
    third_party_integration::stk_io::write_mesh(kTestFile, tBulk);

    const auto tNodalSum = NodalSumObjective{};

    constexpr int tNumCoordsPerNode = 3;
    const auto tExpected = std::vector<double>(tNumCoordsPerNode * tCommandGenerator.numberOfNodes(), 1.0);
    const std::vector tComputed = tNodalSum.df(core::MeshProxy{kTestFile, {}}).stdVector();
    EXPECT_EQ(tComputed, tExpected);
    test_utilities::test_for_existence_and_remove({kTestFile}, TEST_CONTEXT("Checking existence of file"));
}

TEST(NodalSumObjective, Value)
{
    third_party_integration::stk_io::write_mesh(kBrickFile, third_party_integration::stk_io::generate_mesh(
                                                                third_party_integration::stk_io::CommandGenerator{}));
    core::MeshProxy tMeshProxy{kBrickFile, {}};

    const NodalSumObjective tPass;
    EXPECT_EQ(tPass.f(tMeshProxy), 12);
    test_utilities::test_for_existence_and_remove({kBrickFile}, TEST_CONTEXT("Checking existence of file"));
}

TEST(NodalSumObjective, Gradient)
{
    third_party_integration::stk_io::write_mesh(kBrickFile, third_party_integration::stk_io::generate_mesh(
                                                                third_party_integration::stk_io::CommandGenerator{}));
    core::MeshProxy tMeshProxy{kBrickFile, {}};

    const NodalSumObjective tPass;
    const std::vector<double> tGold(24, 1);
    EXPECT_EQ(tPass.df(tMeshProxy).stdVector(), tGold);
    test_utilities::test_for_existence_and_remove({kBrickFile}, TEST_CONTEXT("Checking existence of file"));
}
}  // namespace plato::criteria::extension::unittest
