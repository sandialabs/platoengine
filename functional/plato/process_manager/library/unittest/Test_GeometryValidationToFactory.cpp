#include <gtest/gtest.h>

#include "plato/geometry/library/GeometryFactory.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/utilities/STKCommandGenerator.hpp"
#include "plato/utilities/STKUtilities.hpp"

namespace plato::process_manager::library::unittest
{
TEST(GeometryFactory, ValidBrickShapeGeometry)
{
    namespace pf = plato;
    input_parser::ParsedInput tInput;
    tInput.mBrickShapeGeometry = pf::test_utilities::create_valid_brick_shape_geometry();
    tInput.mObjectives = {pf::test_utilities::create_valid_example_objective()};
    tInput.mROLOptimization = pf::test_utilities::create_valid_example_rol_optimization();

    const ValidatedInput tData = make_validated_input(tInput);
    EXPECT_NO_THROW(auto tUnused = pf::geometry::library::make_geometry_data(tData.geometry()));
}

TEST(GeometryFactory, ValidTopology)
{
    namespace pftu = plato::test_utilities;

    input_parser::ParsedInput tInput;
    tInput.mDensityTopology = pftu::create_valid_density_topology_geometry();
    tInput.mObjectives = {pftu::create_valid_example_objective()};
    tInput.mROLOptimization = pftu::create_valid_example_rol_optimization();

    const std::filesystem::path tMeshFileName{tInput.mDensityTopology.value().mesh_name.value().mName};
    const utilities::STKCommandGenerator tSTKCommandGenerator{
        {3, 3, 4}, {-1, -2, -1}, {2, 1, 2}, utilities::STKCommandElementType::Hex};
    utilities::write_mesh(tMeshFileName, utilities::create_mesh(tSTKCommandGenerator.toString()));

    const ValidatedInput tData = make_validated_input(tInput);

    EXPECT_NO_THROW(auto tUnused = geometry::library::make_geometry_data(tData.geometry()));

    std::filesystem::remove(tMeshFileName);
}
}  // namespace plato::process_manager::library::unittest
