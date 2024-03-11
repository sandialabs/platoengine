#include <gtest/gtest.h>

#include "plato/geometry/library/GeometryFactory.hpp"
#include "plato/main/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/utilities/STKUtilities.hpp"

namespace plato::main::library::unittest
{
TEST(GeometryFactory, ValidBrickShapeGeometry)
{
    namespace pf = plato;
    input_parser::ParsedInput tInput;
    tInput.mBrickShapeGeometry = pf::test_utilities::create_valid_brick_shape_geometry();
    tInput.mObjectives = {pf::test_utilities::create_valid_example_objective()};
    tInput.mOptimizationParameters = pf::test_utilities::create_valid_example_optimization_parameters();

    const ValidatedInput tData = make_validated_input(tInput);
    EXPECT_NO_THROW(auto tUnused = pf::geometry::library::make_geometry_data(tData.geometry()));
}

TEST(GeometryFactory, ValidTopology)
{
    namespace pf = plato;

    input_parser::ParsedInput tInput;
    tInput.mDensityTopology = pf::test_utilities::create_valid_density_topology_geometry();
    tInput.mObjectives = {pf::test_utilities::create_valid_example_objective()};
    tInput.mOptimizationParameters = pf::test_utilities::create_valid_example_optimization_parameters();

    const std::filesystem::path tMeshFileName{tInput.mDensityTopology.value().mesh_name.value().mName};
    pf::utilities::write_mesh(tMeshFileName, pf::utilities::create_mesh("generated:3x3x4|bbox:-1,-2,-1,2,1,2"));

    const ValidatedInput tData = make_validated_input(tInput);

    EXPECT_NO_THROW(auto tUnused = pf::geometry::library::make_geometry_data(tData.geometry()));

    std::filesystem::remove(tMeshFileName);
}
}  // namespace plato::main::library::unittest
