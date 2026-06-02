#include <gtest/gtest.h>

#include <boost/mpi/communicator.hpp>

#include "plato/mesh/test_utilities/NodalCoordinateGradientCheckerUtilities.hpp"
#include "plato/third_party_integration/stk_io/test_utilities/MeshFixtures.hpp"

namespace plato::mesh::unittest
{

namespace
{
using NodalCoordinateGradientCheckerFixture = third_party_integration::stk_io::test_utilities::OneBlock3x1x1HexMesh;

}  // namespace

TEST_F(NodalCoordinateGradientCheckerFixture, MakeNodalCoordinateGradientChecker)
{
    test_utilities::run_coordinate_gradient_check_test(
        mMeshFilePath, TEST_CONTEXT("Serial nodal gradient check."),
        boost::mpi::communicator{MPI_COMM_SELF, boost::mpi::comm_attach});
}

}  // namespace plato::mesh::unittest
