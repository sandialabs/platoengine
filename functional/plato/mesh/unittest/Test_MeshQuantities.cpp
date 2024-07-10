#include <gtest/gtest.h>

#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshQuantities.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::mesh::unittest
{
TEST(MeshQuantities, AverageNodalDensity)
{
    const auto tCommandGenerator = third_party_integration::stk_io::CommandGenerator{{4, 4, 4}, {0, 0, 0}, {4, 4, 4}};
    const auto tMeshFileName = std::filesystem::path{"mesh.exo"};
    third_party_integration::stk_io::write_mesh(tMeshFileName, tCommandGenerator);

    const auto tTotalNumberOfNodes = tCommandGenerator.numberOfNodes();
    const auto tTotalVolume = tCommandGenerator.volume();
    const auto tGold = static_cast<double>(tTotalNumberOfNodes) / tTotalVolume;

    const auto tMesh = Mesh{tMeshFileName};
    const double tResult = average_nodal_density(tMesh);

    EXPECT_DOUBLE_EQ(tGold, tResult);

    std::filesystem::remove(tMeshFileName);
}
}  // namespace plato::mesh::unittest
