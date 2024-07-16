#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>
#include <filesystem>
#include <string_view>

#include "plato/core/MeshProxy.hpp"
#include "plato/criteria/extension/SharedLibCriterion.hpp"
#include "plato/integration_tests/utilities/AppConfigurationTestUtilities.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"
#include "plato/utilities/StringUtilities.hpp"

namespace plato::integration_tests::parallel
{
namespace
{
constexpr auto kLibPath = std::string_view{"libPlatoTestMassObjective.so"};
constexpr auto kMeshName = std::string_view{"massTest.exo"};
const auto kMeshGenerator = third_party_integration::stk_io::CommandGenerator{
    /*.mElements=*/{1u, 1u, 1u}, /*.mLowerBounds=*/{-1.0, -1.0, -1.0}, /*.mUpperBounds=*/{1.0, 1.0, 1.0}};
}  // namespace

TEST(ParallelMassObjective, CallValueAndGradient)
{
    auto tComm = boost::mpi::communicator{};
    EXPECT_GT(tComm.size(), 1u);

    const auto tTestConfiguration = utilities::test_app_configuration(kLibPath);
    // Get parallel criterion:
    const auto tParallelCriterion = std::find_if(tTestConfiguration.mConfiguration.mCriteria.cbegin(),
                                                 tTestConfiguration.mConfiguration.mCriteria.cend(),
                                                 [](const auto& tCriterion) { return tCriterion.mIsParallelized; });
    ASSERT_NE(tParallelCriterion, tTestConfiguration.mConfiguration.mCriteria.cend());
    const auto tSharedLib = criteria::extension::SharedLibCriterion{tTestConfiguration, *tParallelCriterion, {}, tComm};

    const auto tRankMeshName = plato::utilities::concatenate(kMeshName, '.', tComm.rank());
    third_party_integration::stk_io::write_mesh(tRankMeshName,
                                                third_party_integration::stk_io::generate_mesh(kMeshGenerator));

    const double tMass = tSharedLib.f(core::MeshProxy{tRankMeshName, {}});
    EXPECT_DOUBLE_EQ(tMass, kMeshGenerator.volume());

    const auto tGrad = tSharedLib.df(core::MeshProxy{tRankMeshName, {}});
    const std::vector<double> tGold(24, 1.0);
    EXPECT_EQ(tGrad.stdVector(), tGold);

    std::filesystem::remove(tRankMeshName);
}
}  // namespace plato::integration_tests::parallel
