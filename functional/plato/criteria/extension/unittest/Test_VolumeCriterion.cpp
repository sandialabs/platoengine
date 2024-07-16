#include <gtest/gtest.h>

#include <string>

#include "plato/criteria/extension/VolumeCriterion.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::criteria::extension::unittest
{
namespace
{
constexpr std::string_view kMeshFile = "brick.exo";

void test_volume_criteria_from_ctor_and_function(
    const VolumeCriterion& tVolumeCriterion,
    const core::Function<double, linear_algebra::DynamicVector<double>, const core::MeshProxy&>& aFunction,
    const double aGoldVolume)
{
    constexpr double tConstantControls = 0.75;

    const core::MeshProxy tMeshProxy{
        kMeshFile, std::vector<double>(third_party_integration::stk_io::element_size(kMeshFile), tConstantControls)};

    EXPECT_EQ(tVolumeCriterion.f(tMeshProxy), aGoldVolume * tConstantControls);
    EXPECT_EQ(tVolumeCriterion.f(tMeshProxy), aFunction.f(tMeshProxy));
}

void test_scaled_and_unscaled_on_ctor_and_function(
    const third_party_integration::stk_io::CommandGenerator& aCommandGenerator)
{
    third_party_integration::stk_io::write_mesh(kMeshFile,
                                                third_party_integration::stk_io::generate_mesh(aCommandGenerator));
    test_volume_criteria_from_ctor_and_function(VolumeCriterion{}, make_volume_constraint_function(),
                                                aCommandGenerator.volume());
    test_volume_criteria_from_ctor_and_function(VolumeCriterion{1.0 / aCommandGenerator.volume()},
                                                make_volume_fraction_constraint_function(), 1);

    EXPECT_TRUE(std::filesystem::remove(kMeshFile));
}

void test_volume_criteria_derivative_from_ctor_and_function(
    const VolumeCriterion& tVolumeCriterion,
    const core::Function<double, linear_algebra::DynamicVector<double>, const core::MeshProxy&>& aFunction,
    const std::vector<double>& aGold)
{
    const std::vector<double> tAssignedDensities{0.5, 0.4, 0.3};  // Not 1 to make certain DF does not depend on them
    const core::MeshProxy tMeshProxy{kMeshFile, tAssignedDensities};
    const auto tResult = tVolumeCriterion.df(tMeshProxy);
    const auto tResultFromFunction = aFunction.df(tMeshProxy);

    ASSERT_EQ(tResult.size(), third_party_integration::stk_io::element_size(kMeshFile));
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
    third_party_integration::stk_io::write_mesh(kMeshFile,
                                                third_party_integration::stk_io::generate_mesh(tCommandGenerator));

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

}  // namespace plato::criteria::extension::unittest
