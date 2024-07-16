#include <gtest/gtest.h>

#include <filesystem>

#include "plato/core/MeshProxy.hpp"
#include "plato/criteria/extension/SharedLibCriterion.hpp"
#include "plato/integration_tests/utilities/AppConfigurationTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"
#include "plato/utilities/Exception.hpp"

namespace plato::integration_tests::serial
{
namespace
{
// These tests use the mass objective library but through the shared library interface so they are testing the
// shared library more generically
constexpr std::string_view kLibPath = "libPlatoTestMassObjective.so";
const auto kMeshGenerator = third_party_integration::stk_io::CommandGenerator{
    {1, 1, 1}, {-1, -1, -1}, {1, 1, 1}, third_party_integration::stk_io::CommandElementType::Hex};

void generate_bad_library_and_do_nothing()
{
    // This function should throw an exception
    const auto tTestConfiguration = utilities::test_app_configuration("badRobot.so");
    const auto tBad = criteria::extension::SharedLibCriterion{
        tTestConfiguration, tTestConfiguration.mConfiguration.mCriteria.front(), {}};
    std::cout << tBad.f(core::MeshProxy{"dne.exo", {}}) << std::endl;
}

criteria::extension::SharedLibCriterion test_shared_lib_criterion()
{
    const auto tTestConfiguration = utilities::test_app_configuration(kLibPath);
    return criteria::extension::SharedLibCriterion{
        tTestConfiguration, tTestConfiguration.mConfiguration.mCriteria.front(), {}};
}
}  // namespace

TEST(SharedLibObjective, BadLibraryPath)
{
    EXPECT_THROW(generate_bad_library_and_do_nothing(), plato::utilities::Exception);
}

TEST(SharedLibObjective, CallValue)
{
    const auto tSharedLib = test_shared_lib_criterion();
    constexpr std::string_view tMeshName = "massTest.exo";
    third_party_integration::stk_io::write_mesh(tMeshName,
                                                third_party_integration::stk_io::generate_mesh(kMeshGenerator));
    const double tMass = tSharedLib.f(core::MeshProxy{tMeshName, {}});
    EXPECT_DOUBLE_EQ(tMass, 8.0);
    std::filesystem::remove(tMeshName);
}

TEST(SharedLibObjective, CallGradient)
{
    const auto tSharedLib = test_shared_lib_criterion();
    constexpr std::string_view tMeshName = "massTest.exo";
    third_party_integration::stk_io::write_mesh(tMeshName,
                                                third_party_integration::stk_io::generate_mesh(kMeshGenerator));
    const auto tGrad = tSharedLib.df(core::MeshProxy{tMeshName, {}});

    const std::vector<double> tGold(24, 1.0);
    EXPECT_EQ(tGrad.stdVector(), tGold);

    std::filesystem::remove(tMeshName);
}

TEST(SharedLibObjective, ValueUsingFunction)
{
    const auto tFunction = criteria::extension::make_shared_lib_function(test_shared_lib_criterion());
    constexpr std::string_view tMeshName = "massTest.exo";
    third_party_integration::stk_io::write_mesh(tMeshName,
                                                third_party_integration::stk_io::generate_mesh(kMeshGenerator));
    const double tMass = tFunction.f(core::MeshProxy{tMeshName, {}});
    EXPECT_DOUBLE_EQ(tMass, 8.0);

    std::filesystem::remove(tMeshName);
}
}  // namespace plato::integration_tests::serial
