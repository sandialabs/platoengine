#include <gtest/gtest.h>

#include "plato/mesh/Mesh.hpp"
#include "plato/mesh/MeshQuantities.hpp"
#include "plato/mesh/unittest/Fixtures.hpp"

namespace plato::mesh::unittest
{

TEST_F(OneBlock3x1x1HexMesh, MeshQuantitiesOneBlock)
{
    const auto tMesh = MeshQuantities{Mesh{mMeshFilePath}};

    EXPECT_DOUBLE_EQ(tMesh.volume(), mCommandGenerator.volume());

    const auto tTotalNumberOfNodes = mCommandGenerator.numberOfNodes();
    const auto tTotalVolume = mCommandGenerator.volume();
    const auto tExpectedNodalDensity = static_cast<double>(tTotalNumberOfNodes) / tTotalVolume;
    EXPECT_DOUBLE_EQ(tMesh.averageNodalDensity(), tExpectedNodalDensity);
}
}  // namespace plato::mesh::unittest
