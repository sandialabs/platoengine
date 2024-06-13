#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <string_view>

#include "plato/core/MeshProxy.hpp"
#include "plato/criteria/extension/SharedLibCriterion.hpp"
#include "plato/utilities/STKCommandGenerator.hpp"
#include "plato/utilities/STKUtilities.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::integration_tests::parallel
{
namespace
{
constexpr auto kLibPath = std::string_view{"libPlatoTestMassObjective.so"};
constexpr auto kMeshName = std::string_view{"massTest.exo"};
const auto kMeshCommand = utilities::STKCommandGenerator{
    /*.mElements=*/{1u, 1u, 1u}, /*.mLowerBounds=*/{-1.0, -1.0, -1.0}, /*.mUpperBounds=*/{1.0, 1.0, 1.0}};
}  // namespace

TEST(ParallelMassObjective, CallValueAndGradient)
{
    auto tComm = boost::mpi::communicator{};
    EXPECT_GT(tComm.size(), 1u);

    const auto tSharedLib = criteria::extension::SharedLibCriterion{std::string{kLibPath}, {}, tComm};

    const auto tRankMeshName = utilities::concatenate(kMeshName, '.', tComm.rank());
    utilities::write_mesh(tRankMeshName, utilities::create_mesh(kMeshCommand.toString()));

    const double tMass = tSharedLib.f(core::MeshProxy{tRankMeshName, {}});
    EXPECT_DOUBLE_EQ(tMass, kMeshCommand.volume());

    const auto tGrad = tSharedLib.df(core::MeshProxy{tRankMeshName, {}});
    const std::vector<double> tGold(24, 1.0);
    EXPECT_EQ(tGrad.stdVector(), tGold);

    std::filesystem::remove(tRankMeshName);
}
}  // namespace plato::integration_tests::parallel
