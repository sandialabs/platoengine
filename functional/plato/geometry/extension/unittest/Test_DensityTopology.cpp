#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>
#include <numeric>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Comm.hpp>
#include <stk_util/parallel/Parallel.hpp>
#include <string_view>
#include <vector>

#include "plato/core/MeshProxy.hpp"
#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/input_parser/InputBlocks.hpp"
#include "plato/linear_algebra/JacobianColumnEvaluator.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::geometry::extension::unittest
{
namespace
{
const auto kDensityInput = plato::test_utilities::create_valid_density_topology_geometry();

constexpr unsigned int kExpectedDensitySize = 8;  // Based on mesh generation command below (1x1x1)

void create_small_mesh(const std::string& aFileName)
{
    ASSERT_EQ(stk::parallel_machine_size(MPI_COMM_WORLD), 1);
    const third_party_integration::stk_io::CommandGenerator tCommandGenerator{
        {1, 1, 1}, {-1, -2, -1}, {2, 1, 2}, third_party_integration::stk_io::CommandElementType::Hex};

    auto bulk = third_party_integration::stk_io::generate_mesh(tCommandGenerator);
    third_party_integration::stk_io::write_mesh(aFileName, bulk);
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

    const auto tMeshProxy = tDensityTopology.generateMesh(tDesignVec);
    EXPECT_EQ(tMeshProxy.mNodalDensities, tDesignVars);

    EXPECT_TRUE(std::filesystem::remove(kDensityInput.mesh_name->mToken));
}

TEST(DensityTopology, InitialGuess)
{
    create_small_mesh(kDensityInput.mesh_name->mToken);

    const linear_algebra::DynamicVector<double> tInitialGuess =
        DensityTopology::initialGuess(kDensityInput.mesh_name->mToken);

    EXPECT_EQ(tInitialGuess.size(), kExpectedDensitySize);

    for (const double val : tInitialGuess.stdVector())
    {
        EXPECT_EQ(val, 0.5);
    }

    EXPECT_TRUE(std::filesystem::remove(kDensityInput.mesh_name->mToken));
}

TEST(DensityTopology, Bounds)
{
    create_small_mesh(kDensityInput.mesh_name->mToken);

    const auto [tLowerBounds, tUpperBounds] = DensityTopology::bounds(kDensityInput.mesh_name->mToken);

    EXPECT_EQ(tLowerBounds.size(), kExpectedDensitySize);
    EXPECT_EQ(tUpperBounds.size(), kExpectedDensitySize);

    EXPECT_TRUE(std::all_of(tLowerBounds.cbegin(), tLowerBounds.cend(), [](const double aVal) { return aVal == 0.0; }));
    EXPECT_TRUE(std::all_of(tUpperBounds.cbegin(), tUpperBounds.cend(), [](const double aVal) { return aVal == 1.0; }));

    EXPECT_TRUE(std::filesystem::remove(kDensityInput.mesh_name->mToken));
}
}  // namespace plato::geometry::extension::unittest
