#include <gtest/gtest.h>

#include <string>

#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/criteria/extension/VolumeCriterion.hpp"
#include "plato/mesh/DesignVariableConversion.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshQuantities.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::criteria::extension::unittest
{
namespace
{
using third_party_integration::stk_io::test_utilities::TwoDThreeBlockMesh;

constexpr std::string_view kMeshFile = "brick.exo";

void test_volume_criteria_from_ctor_and_function(const VolumeCriterion& tVolumeCriterion,
                                                 const library::CriterionFunction& aFunction,
                                                 const double aGoldVolume)
{
    constexpr double tConstantControls = 0.75;

    const auto tMesh = mesh::EntityCounts{mesh::Mesh{kMeshFile}};
    const auto tControls = std::vector<double>(tMesh.numberOfElements(), tConstantControls);
    const auto tAnalysisDomainMesh = mesh::DesignVariablesConversion{tMesh}.elementFieldToAnalysisDomainMesh(
        mesh::ElementFieldVectorReference{std::cref(tControls)});

    EXPECT_EQ(tVolumeCriterion.f(tAnalysisDomainMesh), aGoldVolume * tConstantControls);
    EXPECT_EQ(tVolumeCriterion.f(tAnalysisDomainMesh),
              aFunction.evaluate<core::evaluation::kFunction>(tAnalysisDomainMesh));
}

void test_scaled_and_unscaled_on_ctor_and_function(
    const third_party_integration::stk_io::CommandGenerator& aCommandGenerator)
{
    third_party_integration::stk_io::write_mesh(kMeshFile, aCommandGenerator);
    test_volume_criteria_from_ctor_and_function(VolumeCriterion{}, make_volume_constraint_function(),
                                                aCommandGenerator.volume());
    test_volume_criteria_from_ctor_and_function(VolumeCriterion{1.0 / aCommandGenerator.volume()},
                                                make_volume_fraction_constraint_function(), 1);

    EXPECT_TRUE(std::filesystem::remove(kMeshFile));
}

void test_volume_criteria_derivative_from_ctor_and_function(const VolumeCriterion& tVolumeCriterion,
                                                            const library::CriterionFunction& aFunction,
                                                            const std::vector<double>& aGold)
{
    const auto tMesh = mesh::Mesh{kMeshFile};
    const auto tAssignedDensities =
        std::vector<double>{0.5, 0.4, 0.3};  // Not 1 to make certain DF does not depend on them
    const auto tAnalysisDomainMesh = mesh::DesignVariablesConversion{tMesh}.elementFieldToAnalysisDomainMesh(
        mesh::ElementFieldVectorReference{std::cref(tAssignedDensities)});
    const auto tResult = tVolumeCriterion.df(tAnalysisDomainMesh);
    const auto tResultFromFunction = aFunction.evaluate<core::evaluation::kFirstDerivative>(tAnalysisDomainMesh);

    ASSERT_EQ(tResult.size(), mesh::EntityCounts{tMesh}.numberOfElements());
    ASSERT_EQ(tResult.size(), aGold.size());
    ASSERT_EQ(tResult.size(), tResultFromFunction.size());

    for (unsigned int iIndex = 0; iIndex < tResult.size(); ++iIndex)
    {
        EXPECT_DOUBLE_EQ(tResult[iIndex], aGold[iIndex]);
        EXPECT_DOUBLE_EQ(tResultFromFunction[iIndex], aGold[iIndex]);
    }
}

}  // namespace

TEST(VolumeCriterion, Volume)
{
    test_scaled_and_unscaled_on_ctor_and_function(third_party_integration::stk_io::CommandGenerator{
        {4, 4, 4}, {0, 0, 0}, {2.0, 2.0, 3.0}, third_party_integration::stk_io::CommandElementType::Hex});
}

TEST(VolumeCriterion, VolumeTetMesh)
{
    test_scaled_and_unscaled_on_ctor_and_function(third_party_integration::stk_io::CommandGenerator{
        {4, 4, 4}, {0, 0, 0}, {1.0, 2.0, 3.0}, third_party_integration::stk_io::CommandElementType::Tet});
}

TEST(VolumeCriterion, DerivativeOfScaledVolumeOnControls)
{
    const third_party_integration::stk_io::CommandGenerator tCommandGenerator{
        {1, 1, 3}, {0, 0, 0}, {2.0, 1.0, 3.0}, third_party_integration::stk_io::CommandElementType::Hex};
    third_party_integration::stk_io::write_mesh(kMeshFile, tCommandGenerator);

    {
        const std::vector<double> tGold{2, 2, 2};
        test_volume_criteria_derivative_from_ctor_and_function(VolumeCriterion{}, make_volume_constraint_function(),
                                                               tGold);
    }

    {
        constexpr double tHexVolume = 2;
        const double tOneOverVolume = 1.0 / tCommandGenerator.volume();
        const std::vector<double> tGold(3, tHexVolume * tOneOverVolume);
        test_volume_criteria_derivative_from_ctor_and_function(VolumeCriterion{tOneOverVolume},
                                                               make_volume_fraction_constraint_function(), tGold);
    }

    EXPECT_TRUE(std::filesystem::remove(kMeshFile));
}

TEST_F(TwoDThreeBlockMesh, VolumeCriterionWithFixedBlocks)
{
    auto tFixedBlockNames = std::set<std::string>{mBlockNames[0]};
    const auto tMesh = mesh::Mesh{mMeshFilePath, std::move(tFixedBlockNames)};

    const auto tDensityVector = std::vector<double>{0.25, 0.25, 1.0};
    const auto tAnalysisDomainMesh = mesh::DesignVariablesConversion{tMesh}.elementFieldToAnalysisDomainMesh(
        mesh::ElementFieldVectorReference{tDensityVector});

    const auto tResult = VolumeCriterion{}.f(tAnalysisDomainMesh);
    constexpr auto tExpected = double{4.5};
    EXPECT_DOUBLE_EQ(tExpected, tResult);
}

TEST_F(TwoDThreeBlockMesh, GradientVolumeCriterionWithFixedBlocks)
{
    auto tFixedBlockNames = std::set<std::string>{mBlockNames[0], mBlockNames[2]};
    const auto tMesh = mesh::Mesh{mMeshFilePath, std::move(tFixedBlockNames)};
    constexpr auto tNumberOfNodalDensities = 2U;
    const auto tDensityVector = std::vector<double>(tNumberOfNodalDensities, 1.0);
    const auto tAnalysisDomainMesh = mesh::DesignVariablesConversion{tMesh}.elementFieldToAnalysisDomainMesh(
        mesh::ElementFieldVectorReference{tDensityVector});

    const auto tResult = VolumeCriterion{}.df(tAnalysisDomainMesh);
    const auto tExpected = mesh::MeshQuantities{tMesh}.designDomainElementVolumes();

    EXPECT_EQ(tExpected, tResult.stdVector());
}

}  // namespace plato::criteria::extension::unittest
