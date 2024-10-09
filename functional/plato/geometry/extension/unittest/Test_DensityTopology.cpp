#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>
#include <numeric>
#include <stk_mesh/base/Comm.hpp>
#include <stk_util/parallel/Parallel.hpp>
#include <vector>

#include "plato/analysis/AnalysisDomainMesh.hpp"
#include "plato/analysis/AnalysisDomainMeshSequentialView.hpp"
#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/linear_algebra/JacobianColumnEvaluator.hpp"
#include "plato/mesh/EntityCounts.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/ReadUtilities.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshWithFieldWriter.hpp"

namespace plato::geometry::extension::unittest
{

using NodalDensityMesh = third_party_integration::stk_io::test_utilities::MeshWithNodalDensities;

namespace
{

const auto kDensityInput = plato::test_utilities::create_valid_density_topology_geometry();

constexpr unsigned int kExpectedDensitySize = 8;  // Based on mesh generation command below (1x1x1)

void create_small_mesh(const std::string& aFileName)
{
    ASSERT_EQ(stk::parallel_machine_size(MPI_COMM_WORLD), 1);
    const third_party_integration::stk_io::CommandGenerator tCommandGenerator{
        {1, 1, 1}, {-1, -2, -1}, {2, 1, 2}, third_party_integration::stk_io::CommandElementType::Hex};

    third_party_integration::stk_io::write_mesh(aFileName, tCommandGenerator);
}
}  // namespace

TEST(DensityTopology, Jacobian)
{
    create_small_mesh(kDensityInput.mesh_name->mToken);

    const DensityTopology tDensityTopology(kDensityInput, filter::extension::make_identity_filter_function());

    const std::vector<double> tDesignVars = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8};
    const linear_algebra::DynamicVector<double> tDesignVec(tDesignVars);
    const int tNumDesignParameters = tDesignVec.size();

    const linear_algebra::JacobianMultiplier tJacobian = tDensityTopology.jacobian(tDesignVec);

    std::vector<double> tRowVec(tNumDesignParameters, 0.0);
    std::iota(tRowVec.begin(), tRowVec.end(), 1.0);
    const linear_algebra::DynamicVector<double> tRolVec(tRowVec);

    // Jacobian is identity matrix
    const std::vector<double> tGold = tRowVec;
    const linear_algebra::DynamicVector<double> tRes = tRolVec * tJacobian;
    EXPECT_EQ(tRes.stdVector(), tGold);

    EXPECT_TRUE(std::filesystem::remove(kDensityInput.mesh_name->mToken));
}

TEST(DensityTopology, GenerateMesh)
{
    create_small_mesh(kDensityInput.mesh_name->mToken);

    const DensityTopology tDensityTopology(kDensityInput, filter::extension::make_identity_filter_function());

    const std::vector<double> tDesignVars = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8};
    const linear_algebra::DynamicVector<double> tDesignVec(tDesignVars);

    const auto tAnalysisDomainMesh = tDensityTopology.generateMesh(tDesignVec);
    const auto tDensities =
        analysis::mesh_analysis_to_vector(analysis::AnalysisDomainMeshSequentialView{tAnalysisDomainMesh});
    const auto [tDensityValues, tIDMap] = analysis::split_scalar_field_values(tDensities);
    EXPECT_EQ(tDensityValues, tDesignVars);

    EXPECT_TRUE(std::filesystem::remove(kDensityInput.mesh_name->mToken));
}

namespace
{
void test_uniform_initial_guess_against_gold(const linear_algebra::DynamicVector<double>& aInitialGuess,
                                             const double aGold,
                                             const test_utilities::TestContext& aTestContext)
{
    EXPECT_EQ(aInitialGuess.size(), kExpectedDensitySize) << aTestContext;

    for (const double val : aInitialGuess.stdVector())
    {
        EXPECT_EQ(val, aGold) << aTestContext;
    }
}

}  // namespace

TEST(DensityTopology, InitialGuess)
{
    create_small_mesh(kDensityInput.mesh_name->mToken);

    {
        const linear_algebra::DynamicVector<double> tInitialGuess = DensityTopology::initialGuess(kDensityInput);
        test_uniform_initial_guess_against_gold(tInitialGuess, kDensityInput.initial_density_value.value(),
                                                TEST_CONTEXT("Given 0.5 in input"));
    }
    {
        auto tDensityInput = kDensityInput;
        tDensityInput.initial_density_value = 0.3;
        const linear_algebra::DynamicVector<double> tInitialGuess = DensityTopology::initialGuess(tDensityInput);
        test_uniform_initial_guess_against_gold(tInitialGuess, tDensityInput.initial_density_value.value(),
                                                TEST_CONTEXT("Given 0.3 in input"));
    }

    EXPECT_TRUE(std::filesystem::remove(kDensityInput.mesh_name->mToken));
}

namespace
{
[[nodiscard]] auto density_input_for_test_fixture(const std::filesystem::path& aMeshName,
                                                  const std::string_view aFieldName) -> input_parser::density_topology
{
    auto tDensityInput = kDensityInput;
    tDensityInput.initial_density_value = boost::none;
    tDensityInput.mesh_name = input_parser::FileName{aMeshName};
    tDensityInput.initial_density_field_name = input_parser::IdentifierString{std::string{aFieldName}};
    return tDensityInput;
}

}  // namespace

TEST_F(NodalDensityMesh, InitialDensityFromMesh)
{
    const auto tDensityInput = density_input_for_test_fixture(mMeshName, mFieldName);
    const auto tResult = detail::initial_density_value_from_mesh(tDensityInput);

    std::vector<double> tGold(mGoldNumbering.size());
    std::iota(tGold.begin(), tGold.end(), 1.0);
    EXPECT_EQ(tResult, tGold);
}

TEST(DensityTopology, Bounds)
{
    create_small_mesh(kDensityInput.mesh_name->mToken);

    const auto [tLowerBounds, tUpperBounds] = DensityTopology::bounds(kDensityInput);

    EXPECT_EQ(tLowerBounds.size(), kExpectedDensitySize);
    EXPECT_EQ(tUpperBounds.size(), kExpectedDensitySize);

    EXPECT_TRUE(std::all_of(tLowerBounds.cbegin(), tLowerBounds.cend(), [](const double aVal) { return aVal == 0.0; }));
    EXPECT_TRUE(std::all_of(tUpperBounds.cbegin(), tUpperBounds.cend(), [](const double aVal) { return aVal == 1.0; }));

    EXPECT_TRUE(std::filesystem::remove(kDensityInput.mesh_name->mToken));
}

}  // namespace plato::geometry::extension::unittest
