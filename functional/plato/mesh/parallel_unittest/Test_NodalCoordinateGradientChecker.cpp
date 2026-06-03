#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/mesh/test_utilities/NodalCoordinateGradientCheckerUtilities.hpp"
#include "plato/test_utilities/TestContext.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::mesh::unittest
{

namespace
{
using ParallelNodalCoordinateGradientCheckerFixture =
    third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;

}

TEST_F(ParallelNodalCoordinateGradientCheckerFixture, MakeNodalCoordinateGradientChecker)
{
    const auto tWorldCommunicator = boost::mpi::communicator{};
    test_utilities::run_coordinate_gradient_check_test(mMeshFilePath, TEST_CONTEXT("Parallel nodal gradient check."),
                                                       tWorldCommunicator);
    tWorldCommunicator.barrier();
}
}  // namespace plato::mesh::unittest
