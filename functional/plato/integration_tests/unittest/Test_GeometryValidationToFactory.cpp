#include <gtest/gtest.h>

#include "plato/criteria/library/ObjectiveInputBlock.hpp"
#include "plato/filter/extension/IdentityFilter.hpp"
#include "plato/geometry/extension/BrickShapeGeometry.hpp"
#include "plato/geometry/extension/DensityTopology.hpp"
#include "plato/geometry/library/GeometryFactory.hpp"
#include "plato/input_parser/InputBlockUtilities.hpp"
#include "plato/input_validation/ValidatedInput.hpp"
#include "plato/process_manager/extension/ROLOptimization.hpp"
#include "plato/third_party_integration/stk_io/CommandGenerator.hpp"
#include "plato/third_party_integration/stk_io/WriteUtilities.hpp"

namespace plato::integration_tests::unittest
{
TEST(GeometryFactory, ValidBrickShapeGeometry)
{
    const auto tInput = geometry::extension::create_valid_brick_shape_geometry_input() |
                        criteria::library::create_valid_example_objective_input() |
                        process_manager::extension::create_valid_example_rol_optimization_input();

    const auto tData = input_validation::make_validated_input(tInput).value();
    EXPECT_NO_THROW([[maybe_unused]] const auto tUnused =
                        geometry::library::make_geometry_data(tData.get<input_parser::ComponentType::kGeometry>()));
}

TEST(GeometryFactory, ValidTopology)
{
    const auto tGeometryInput = geometry::extension::create_valid_density_topology_geometry_input();
    const auto tInput = tGeometryInput | criteria::library::create_valid_example_objective_input() |
                        filter::extension::create_valid_identity_filter_input() |
                        process_manager::extension::create_valid_example_rol_optimization_input();

    const auto tMeshFileName = std::filesystem::path{tGeometryInput.mesh_name.value().mToken};
    const auto tCommandGenerator = third_party_integration::stk_io::CommandGenerator{
        {3, 3, 4}, {-1, -2, -1}, {2, 1, 2}, third_party_integration::stk_io::CommandElementType::Hex};
    third_party_integration::stk_io::write_mesh(tMeshFileName, tCommandGenerator);

    const auto tData = input_validation::make_validated_input(tInput);
    ASSERT_TRUE(tData.hasValue());
    EXPECT_NO_THROW(auto tUnused = geometry::library::make_geometry_data(
                        tData.value().get<input_parser::ComponentType::kGeometry>()));

    std::filesystem::remove(tMeshFileName);
}

TEST(GeometryValidation, BrickShapeGeometry)
{
    const auto tValidInputBase = criteria::library::create_valid_example_objective_input() |
                                 process_manager::extension::create_valid_example_rol_optimization_input();
    const auto tValidInput = tValidInputBase | geometry::extension::create_valid_brick_shape_geometry_input();
    EXPECT_TRUE(input_validation::make_validated_input(tValidInput).hasValue());

    const auto tInvalidInputNoMeshName = tValidInputBase | input_parser::new_brick_shape_geometry{};
    EXPECT_TRUE(input_validation::make_validated_input(tInvalidInputNoMeshName).hasError());

    const auto tInvalidInputTwoGeometries =
        tValidInput | geometry::extension::create_valid_brick_shape_geometry_input();
    EXPECT_TRUE(input_validation::make_validated_input(tInvalidInputNoMeshName).hasError());
}

}  // namespace plato::integration_tests::unittest
