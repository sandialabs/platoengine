#include <gtest/gtest.h>

#include <string_view>

#include "plato/criteria/extension/NodalSumObjective.hpp"
#include "plato/test_utilities/FilesystemTestUtility.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/utilities/STKCommandGenerator.hpp"
#include "plato/utilities/STKUtilities.hpp"

namespace plato::criteria::extension::unittest
{
namespace
{
constexpr std::string_view kBrickFile = "brick.exo";
constexpr std::string_view kTestFile = "test.exo";
}  // namespace

TEST(NodalSumObjective, Value111)
{
    namespace pfu = plato::utilities;

    pfu::write_mesh(kTestFile, pfu::create_mesh(pfu::STKCommandGenerator{}.toString()));

    constexpr double tExpectedValue = 12.0;
    EXPECT_EQ(NodalSumObjective{}.f(core::MeshProxy{kTestFile, {}}), tExpectedValue);
    test_utilities::test_for_existence_and_remove({kTestFile}, TEST_CONTEXT("Checking existence of file"));
}

TEST(NodalSumObjective, Value211)
{
    namespace pfu = plato::utilities;
    pfu::write_mesh(kTestFile, pfu::create_mesh(pfu::STKCommandGenerator{{2, 1, 1}}.toString()));

    constexpr double tExpectedValue = 18.0;
    EXPECT_EQ(NodalSumObjective{}.f(core::MeshProxy{kTestFile, {}}), tExpectedValue);
    test_utilities::test_for_existence_and_remove({kTestFile}, TEST_CONTEXT("Checking existence of file"));
}

TEST(NodalSumObjective, Value0)
{
    namespace pfu = plato::utilities;
    const pfu::STKCommandGenerator tSTKCommandGenerator{
        {1, 1, 1}, {-2, -1, -3}, {2, 1, 3}, pfu::STKCommandElementType::Hex};
    pfu::write_mesh(kTestFile, pfu::create_mesh(tSTKCommandGenerator.toString()));

    constexpr double tExpectedValue = 0.0;
    EXPECT_EQ(NodalSumObjective{}.f(core::MeshProxy{kTestFile, {}}), tExpectedValue);
    test_utilities::test_for_existence_and_remove({kTestFile}, TEST_CONTEXT("Checking existence of file"));
}

TEST(NodalSumObjective, Gradient111)
{
    namespace pfu = plato::utilities;
    const pfu::STKCommandGenerator tSTKCommandGenerator{
        {1, 2, 3}, {0, 0, 0}, {1, 1, 1}, pfu::STKCommandElementType::Hex};
    auto tBulk = pfu::create_mesh(tSTKCommandGenerator.toString());
    pfu::write_mesh(kTestFile, tBulk);

    const auto tNodalSum = NodalSumObjective{};

    constexpr int tNumCoordsPerNode = 3;
    const auto tExpected = std::vector<double>(tNumCoordsPerNode * tSTKCommandGenerator.numberOfNodes(), 1.0);
    const std::vector tComputed = tNodalSum.df(core::MeshProxy{kTestFile, {}}).stdVector();
    EXPECT_EQ(tComputed, tExpected);
    test_utilities::test_for_existence_and_remove({kTestFile}, TEST_CONTEXT("Checking existence of file"));
}

TEST(NodalSumObjective, Value)
{
    namespace pfu = plato::utilities;
    pfu::write_mesh(kBrickFile, pfu::create_mesh(pfu::STKCommandGenerator{}.toString()));
    core::MeshProxy tMeshProxy{kBrickFile, {}};

    const NodalSumObjective tPass;
    EXPECT_EQ(tPass.f(tMeshProxy), 12);
    test_utilities::test_for_existence_and_remove({kBrickFile}, TEST_CONTEXT("Checking existence of file"));
}

TEST(NodalSumObjective, Gradient)
{
    namespace pfu = plato::utilities;
    pfu::write_mesh(kBrickFile, pfu::create_mesh(pfu::STKCommandGenerator{}.toString()));
    core::MeshProxy tMeshProxy{kBrickFile, {}};

    const NodalSumObjective tPass;
    const std::vector<double> tGold(24, 1);
    EXPECT_EQ(tPass.df(tMeshProxy).stdVector(), tGold);
    test_utilities::test_for_existence_and_remove({kBrickFile}, TEST_CONTEXT("Checking existence of file"));
}
}  // namespace plato::criteria::extension::unittest
