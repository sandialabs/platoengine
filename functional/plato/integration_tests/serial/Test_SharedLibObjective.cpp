#include <gtest/gtest.h>

#include <filesystem>

#include "plato/core/MeshProxy.hpp"
#include "plato/criteria/extension/SharedLibCriterion.hpp"
#include "plato/utilities/Exception.hpp"
#include "plato/utilities/STKCommandGenerator.hpp"
#include "plato/utilities/STKUtilities.hpp"

namespace plato::integration_tests::serial
{
namespace
{
// These tests use the mass objective library but through the shared library interface so they are testing the shared
// library more generically
constexpr std::string_view kLibPath = "libPlatoTestMassObjective.so";

const std::string kSTKCommand =
    utilities::STKCommandGenerator{{1, 1, 1}, {-1, -1, -1}, {1, 1, 1}, utilities::STKCommandElementType::Hex}
        .toString();

void generate_bad_library_and_do_nothing()
{
    // This function should throw an exception
    const auto tBad = criteria::extension::SharedLibCriterion{std::string{"badRobot.so"}, {}};
    std::cout << tBad.f(core::MeshProxy{"dne.exo", {}}) << std::endl;
}
}  // namespace

TEST(SharedLibObjective, BadLibraryPath)
{
    EXPECT_THROW(generate_bad_library_and_do_nothing(), plato::utilities::Exception);
}

TEST(SharedLibObjective, CallValue)
{
    namespace pfu = plato::utilities;
    const auto tSharedLib = criteria::extension::SharedLibCriterion{std::string{kLibPath}, {}};

    constexpr std::string_view tMeshName = "massTest.exo";
    pfu::write_mesh(tMeshName, pfu::create_mesh(kSTKCommand));
    const double tMass = tSharedLib.f(core::MeshProxy{tMeshName, {}});
    EXPECT_DOUBLE_EQ(tMass, 8.0);

    std::filesystem::remove(tMeshName);
}

TEST(SharedLibObjective, CallGradient)
{
    namespace pfu = plato::utilities;
    const auto tSharedLib = criteria::extension::SharedLibCriterion{std::string{kLibPath}, {}};

    constexpr std::string_view tMeshName = "massTest.exo";
    pfu::write_mesh(tMeshName, pfu::create_mesh(kSTKCommand));
    const auto tGrad = tSharedLib.df(core::MeshProxy{tMeshName, {}});

    const std::vector<double> tGold(24, 1.0);
    EXPECT_EQ(tGrad.stdVector(), tGold);

    std::filesystem::remove(tMeshName);
}

TEST(SharedLibObjective, ValueUsingFunction)
{
    namespace pfu = plato::utilities;
    const auto tFunction = criteria::extension::make_shared_lib_function(
        criteria::extension::SharedLibCriterion{std::string{kLibPath}, {}});

    constexpr std::string_view tMeshName = "massTest.exo";
    pfu::write_mesh(tMeshName, pfu::create_mesh(kSTKCommand));
    const double tMass = tFunction.f(core::MeshProxy{tMeshName, {}});
    EXPECT_DOUBLE_EQ(tMass, 8.0);

    std::filesystem::remove(tMeshName);
}
}  // namespace plato::integration_tests::serial
