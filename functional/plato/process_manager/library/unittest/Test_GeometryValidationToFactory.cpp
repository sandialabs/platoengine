#include <gtest/gtest.h>

#include "plato/geometry/library/GeometryFactory.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/process_manager/library/ValidatedInput.hpp"
#include "plato/test_utilities/InputGeneration.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/Utilities.hpp"

namespace plato::process_manager::library::unittest
{
TEST(GeometryFactory, ValidBrickShapeGeometry)
{
    input_parser::ParsedInput tInput;
    tInput.mBrickShapeGeometry = test_utilities::create_valid_brick_shape_geometry();
    tInput.mObjectives = {test_utilities::create_valid_example_objective()};
    tInput.mROLOptimization = test_utilities::create_valid_example_rol_optimization();

    const ValidatedInput tData = make_validated_input(tInput);
    EXPECT_NO_THROW(auto tUnused = geometry::library::make_geometry_data(tData.geometry()));
}

TEST(GeometryFactory, ValidTopology)
{
    namespace pftu = plato::test_utilities;

    const input_parser::ParsedInput tInput =
        pftu::create_valid_density_topology_geometry() | pftu::create_valid_example_objective() |
        pftu::create_valid_identity_filter() | pftu::create_valid_example_rol_optimization();

    const std::filesystem::path tMeshFileName{tInput.mDensityTopology.value().mesh_name.value().mToken};
    const third_party_integration::stk_io::CommandGenerator tCommandGenerator{
        {3, 3, 4}, {-1, -2, -1}, {2, 1, 2}, third_party_integration::stk_io::CommandElementType::Hex};
    third_party_integration::stk_io::write_mesh(tMeshFileName,
                                                third_party_integration::stk_io::generate_mesh(tCommandGenerator));

    const ValidatedInput tData = make_validated_input(tInput);

    EXPECT_NO_THROW(auto tUnused = geometry::library::make_geometry_data(tData.geometry()));

    std::filesystem::remove(tMeshFileName);
}
}  // namespace plato::process_manager::library::unittest
