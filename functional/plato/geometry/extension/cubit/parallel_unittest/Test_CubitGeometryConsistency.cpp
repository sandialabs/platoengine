#include <gtest/gtest.h>

#include "plato/geometry/extension/cubit/test_utilities/CubitTestFixture.hpp"

namespace plato::geometry::extension::cubit::parallel_unittest
{

using test_utilities::CubitTestFixture;

namespace
{
class CubitGeometryConsistency : public test_utilities::CubitTestFixture
{
};

}  // namespace

TEST_F(CubitGeometryConsistency, ParallelGenerateMesh) {}

TEST_F(CubitGeometryConsistency, ParallelJacobian) { checkJacobian(); }

TEST_F(CubitGeometryConsistency, ParallelAdjointJacobian) { checkAdjointJacobian(); }

}  // namespace plato::geometry::extension::cubit::parallel_unittest
