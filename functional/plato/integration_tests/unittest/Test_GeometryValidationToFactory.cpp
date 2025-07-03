#include <gtest/gtest.h>

#include "plato/criteria/library/test_utilities/ExampleInputBlocks.hpp"
#include "plato/filter/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/geometry/library/GeometryFactory.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/process_manager/extension/test_utilities/ExampleInputBlocks.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"

namespace plato::integration_tests::unittest
{
TEST(GeometryFactory, ValidBrickShapeGeometry)
{
    const auto tInput = geometry::extension::test_utilities::create_valid_brick_shape_geometry_input() |
                        criteria::library::test_utilities::create_valid_example_objective_input() |
                        process_manager::extension::test_utilities::create_valid_example_rol_optimization_input();

    const auto tData = input_validation::make_validated_input(tInput).value();
    EXPECT_NO_THROW([[maybe_unused]] const auto tUnused =
                        geometry::library::make_geometry_data(tData.get<components::ComponentType::kGeometry>()));
}

TEST(GeometryFactory, ValidTopology)
{
    const auto tGeometryInput = geometry::extension::test_utilities::create_valid_density_topology_geometry_input();
    const auto tInput = tGeometryInput | criteria::library::test_utilities::create_valid_example_objective_input() |
                        filter::extension::test_utilities::create_valid_identity_filter_input() |
                        process_manager::extension::test_utilities::create_valid_example_rol_optimization_input();

    const auto tMeshFileName = std::filesystem::path{tGeometryInput.mesh_name.value().mToken};
    const auto tCommandGenerator = third_party_integration::stk_io::CommandGenerator{
        {3, 3, 4}, {-1, -2, -1}, {2, 1, 2}, third_party_integration::stk_io::CommandElementType::Hex};
    third_party_integration::stk_io::write_mesh(tMeshFileName, tCommandGenerator);

    const auto tData = input_validation::make_validated_input(tInput);
    ASSERT_TRUE(tData.hasValue());
    EXPECT_NO_THROW(auto tUnused = geometry::library::make_geometry_data(
                        tData.value().get<components::ComponentType::kGeometry>()));

    std::filesystem::remove(tMeshFileName);
}

TEST(GeometryValidation, BrickShapeGeometry)
{
    const auto tValidInputBase =
        criteria::library::test_utilities::create_valid_example_objective_input() |
        process_manager::extension::test_utilities::create_valid_example_rol_optimization_input();
    const auto tValidInput =
        tValidInputBase | geometry::extension::test_utilities::create_valid_brick_shape_geometry_input();
    EXPECT_TRUE(input_validation::make_validated_input(tValidInput).hasValue());

    const auto tInvalidInputNoMeshName = tValidInputBase | input_parser::brick_shape_geometry{};
    EXPECT_TRUE(input_validation::make_validated_input(tInvalidInputNoMeshName).hasError());

    const auto tInvalidInputTwoGeometries =
        tValidInput | geometry::extension::test_utilities::create_valid_brick_shape_geometry_input();
    EXPECT_TRUE(input_validation::make_validated_input(tInvalidInputNoMeshName).hasError());
}

}  // namespace plato::integration_tests::unittest
